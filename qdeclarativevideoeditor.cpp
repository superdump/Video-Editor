/* QDeclarativeVideoEditor
 * Copyright (C) 2012 Thiago Sousa Santos <thiago.sousa.santos@collabora.co.uk>
 * Copyright (C) 2012 Robert Swain <robert.swain@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "qdeclarativevideoeditor.h"

#include <QDebug>
#include <QDateTime>
#include <QFileInfo>

extern "C" {
    #include "gstcapstricks.h"
}

#define RENDERING_FAILED "Rendering failed"
#define NO_MEDIA "Add clips before exporting"

static gboolean bus_call(GstBus * bus, GstMessage * msg, gpointer data);

QDeclarativeVideoEditor::QDeclarativeVideoEditor(QObject *parent) :
    QAbstractListModel(parent), m_size(0),
    m_width(0), m_height(0), m_fpsn(0), m_fpsd(0)
{
    QHash<int, QByteArray> roles;
    roles.insert( 33 , "uri" );
    roles.insert( 34 , "fileName" );
    setRoleNames(roles);

    m_timeline = ges_timeline_new_audio_video();
    m_timelineLayer = (GESTimelineLayer*) ges_simple_timeline_layer_new();
    ges_timeline_add_layer(m_timeline, m_timelineLayer);
    m_pipeline = ges_timeline_pipeline_new();

    GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (m_pipeline));
    gst_bus_add_watch (bus, bus_call, this);
    gst_object_unref (bus);

    /*
     * gst-dsp encoders seems to not proxy downstream caps correctly, this can make
     * GES fail to render some projects. We override the default getcaps on our own
     */
    g_signal_connect(m_pipeline, "element-added", (GCallback) gstcapstricks_pipeline_element_added, NULL);

    ges_timeline_pipeline_add_timeline (m_pipeline, m_timeline);

    m_vsink = gst_element_factory_make ("omapxvsink", "previewvsink");
    ges_timeline_pipeline_preview_set_video_sink (m_pipeline, m_vsink);
    gst_x_overlay_set_render_rectangle (GST_X_OVERLAY (m_vsink),
                                        171, 0,
                                        512, 288);

    ges_timeline_pipeline_set_mode (m_pipeline, TIMELINE_MODE_PREVIEW);
    m_duration = GST_CLOCK_TIME_NONE;
    m_progress = 0.0;
}

QDeclarativeVideoEditor::~QDeclarativeVideoEditor()
{
    gst_element_set_state ((GstElement*) m_pipeline, GST_STATE_NULL);
    gst_object_unref (m_vsink);
    gst_object_unref (m_pipeline);
}

int QDeclarativeVideoEditor::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? m_size : 0;
}

QVariant QDeclarativeVideoEditor::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.row() < m_size) {
        QVariant ret = NULL;
        switch (role) {
        case 33:
        {
            GESTimelineFileSource *src = (GESTimelineFileSource*) ges_simple_timeline_layer_nth((GESSimpleTimelineLayer*) m_timelineLayer, index.row());
            QVariant ret = QVariant(ges_timeline_filesource_get_uri(src));
            break;
        }
        case 34:
        {
            GESTimelineFileSource *src = (GESTimelineFileSource*) ges_simple_timeline_layer_nth((GESSimpleTimelineLayer*) m_timelineLayer, index.row());
            QFileInfo file(QString(ges_timeline_filesource_get_uri(src)));
            ret = QVariant(file.fileName());
            break;
        }
        default:
        {
            qDebug() << "Unknown role: " << role;
            break;
        }
        }
        return ret;
    } else {
        return QVariant();
    }
}

bool QDeclarativeVideoEditor::append(const QString &value)
{
    qDebug() << "Appending new item:" << value;
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    GESTimelineFileSource *src = ges_timeline_filesource_new(value.toUtf8().data());
    bool r = ges_timeline_layer_add_object(m_timelineLayer, (GESTimelineObject*) src);
    if (r) m_size++;
    endInsertRows();
    return r;
}

void QDeclarativeVideoEditor::move(int from, int to)
{
    qDebug() << "Moving media object from " << from << " to " << to;
    beginResetModel();
    GESTimelineObject *obj = ges_simple_timeline_layer_nth(GES_SIMPLE_TIMELINE_LAYER (m_timelineLayer), from);
    ges_simple_timeline_layer_move_object(GES_SIMPLE_TIMELINE_LAYER (m_timelineLayer), obj, to);
    endResetModel();
}

void QDeclarativeVideoEditor::removeAll()
{
    beginRemoveRows(QModelIndex(), 0, rowCount());
    while(m_size > 0) {
        GESTimelineObject *obj = ges_simple_timeline_layer_nth((GESSimpleTimelineLayer*) m_timelineLayer, m_size-1);
        ges_timeline_layer_remove_object(m_timelineLayer, obj);
        m_size--;
    }
    endRemoveRows();
}

GstEncodingProfile *QDeclarativeVideoEditor::createEncodingProfile() {
    GstEncodingProfile *profile = (GstEncodingProfile *)
            gst_encoding_container_profile_new("mp4", NULL, gst_caps_new_simple("video/quicktime",
                                                                                "variant", G_TYPE_STRING, "iso",
                                                                                NULL), NULL);

    GstEncodingProfile *video = NULL;
    if (m_width > 0 && m_height > 0 && m_fpsn > 0 && m_fpsd > 0) {
        video = (GstEncodingProfile *)
            gst_encoding_video_profile_new(gst_caps_new_simple("video/mpeg", "mpegversion",
                                                               G_TYPE_INT, 4,
                                                               "width", G_TYPE_INT, m_width,
                                                               "height", G_TYPE_INT, m_height,
                                                               "framerate", GST_TYPE_FRACTION_RANGE,
                                                               m_fpsn-1, m_fpsd, m_fpsn+1, m_fpsd, NULL),
                                           NULL, NULL, 1);
    } else {
        video = (GstEncodingProfile *)
            gst_encoding_video_profile_new(gst_caps_new_simple("video/mpeg", "mpegversion",
                                                               G_TYPE_INT, 4, NULL), NULL, NULL, 1);
    }
    GstEncodingProfile *audio = (GstEncodingProfile *)
            gst_encoding_audio_profile_new(gst_caps_new_simple("audio/mpeg", "mpegversion",
                                                               G_TYPE_INT, 4,
                                                               "rate", G_TYPE_INT, 48000,
                                                               "channels", G_TYPE_INT, 2, NULL), NULL, NULL, 0);

    gst_encoding_container_profile_add_profile((GstEncodingContainerProfile*) profile, video);
    gst_encoding_container_profile_add_profile((GstEncodingContainerProfile*) profile, audio);

    return profile;
}

gboolean
QDeclarativeVideoEditor::handleBusMessage (GstBus *bus, GstMessage *msg)
{
    switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
        qDebug() << "End of stream";
        setProgress(1.0);
        emit progressChanged();
        gst_element_set_state ((GstElement *) m_pipeline, GST_STATE_PAUSED);
        ges_timeline_pipeline_set_mode (m_pipeline, TIMELINE_MODE_PREVIEW);
        emit renderComplete();
        setProgress(-1.0);
        break;

    case GST_MESSAGE_ERROR: {
        gchar  *debug;
        GError *gerror;

        gst_message_parse_error (msg, &gerror, &debug);
        g_free (debug);

        qDebug() << "Error: " << gerror->message;
        emit error(RENDERING_FAILED, gerror->message);
        g_error_free (gerror);
        gst_element_set_state ((GstElement *) m_pipeline, GST_STATE_NULL);
        setProgress(-1.0);
        break;
    }
    default:
        break;
    }

    return TRUE;
}

static gboolean
bus_call(GstBus * bus, GstMessage * msg, gpointer data)
{
    QDeclarativeVideoEditor *self = (QDeclarativeVideoEditor*) data;
    return self->handleBusMessage(bus, msg);
}

QString getDateTimeString() {
    QDateTime current = QDateTime::currentDateTime();

    return current.toString("yyyyMMdd-hhmmss");
}

GESTimelinePipeline *QDeclarativeVideoEditor::getPipeline()
{
    return m_pipeline;
}

gint64 QDeclarativeVideoEditor::getDuration()
{
    if (m_duration == GST_CLOCK_TIME_NONE) {
        GstFormat format_time = GST_FORMAT_TIME;
        gst_element_query_duration (GST_ELEMENT (m_pipeline), &format_time, &m_duration);
    }
    return m_duration;
}

void QDeclarativeVideoEditor::setDuration(gint64 duration)
{
    m_duration = duration;
}

double QDeclarativeVideoEditor::getProgress()
{
    return m_progress;
}

void QDeclarativeVideoEditor::setProgress(double progress)
{
    m_progress = progress;
}

void QDeclarativeVideoEditor::emitProgressChanged()
{
    emit progressChanged();
}

gboolean updateProgress (gpointer data)
{
    QDeclarativeVideoEditor *self = (QDeclarativeVideoEditor*) data;

    double progress = self->getProgress();
    if (progress == -1.0) {
        qDebug() << "Stoping progress polling";
        progress = 0.0;
        return false;
    }

    double duration = self->getDuration();
    if(duration == -1) {
        //unknown
        self->setProgress(0);
    } else {
        gint64 cur_pos = GST_CLOCK_TIME_NONE;
        GstFormat format_time = GST_FORMAT_TIME;
        gst_element_query_position (GST_ELEMENT (self->getPipeline()), &format_time, &cur_pos);

        if(duration < 0 || cur_pos < 0) {
            self->setProgress (0);
            qDebug() << "Render progress unknown";
        } else {
            self->setProgress ((double)cur_pos / duration);
            qDebug() << "Render progress " << self->getProgress() * 100
                 << "% (" << cur_pos << "/" << duration << ")";
        }


    }

    if (self->getProgress() < 0.0) {
        self->setProgress(0.0);
        qDebug() << "Stoping progress polling";
        return false;
    }

    emit self->emitProgressChanged();

    return true;
}

void QDeclarativeVideoEditor::setRenderSettings(int width, int height, int fps_n, int fps_d)
{
    this->m_width = width;
    this->m_height = height;
    this->m_fpsn = fps_n;
    this->m_fpsd = fps_d;
}

bool QDeclarativeVideoEditor::render()
{
    //sanity check
    if (m_size < 1) {
        emit error(NO_MEDIA, "No media added to the timeline");
        return false;
    }

    qDebug() << "Render preparations started";

    QString output_uri = "file:///home/user/MyDocs/Movies/" + getDateTimeString() + ".mp4";

    GstEncodingProfile *profile = createEncodingProfile();
    if (!ges_timeline_pipeline_set_render_settings (m_pipeline, output_uri.toUtf8().data(), profile)) {
        emit error(RENDERING_FAILED, "Failed setting rendering options");
        gst_encoding_profile_unref(profile);
        return false;
    }
    gst_encoding_profile_unref (profile);

    if (!ges_timeline_pipeline_set_mode (m_pipeline, TIMELINE_MODE_RENDER)) {
        emit error(RENDERING_FAILED, "Failed to set rendering mode");
        gst_encoding_profile_unref(profile);
        return false;
    }

    qDebug() << "Rendering to " << output_uri;

    // reset duration and progress
    setDuration(GST_CLOCK_TIME_NONE);
    setProgress(0.0);
    qDebug() << "Starting progress polling";
    g_timeout_add (500, updateProgress, this);

    if(!gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_PLAYING)) {
        gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_NULL);

        emit error(RENDERING_FAILED, "Failed to set pipeline to playing state");
        return false;
    }
    return true;
}

void QDeclarativeVideoEditor::cancelRender()
{
    qDebug() << "Cancelling rendering operation";
    gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_PAUSED);
    setProgress(-1.0);
    ges_timeline_pipeline_set_mode (m_pipeline, TIMELINE_MODE_PREVIEW);
}

uint QDeclarativeVideoEditor::getWinId()
{
    return m_winId;
}

void QDeclarativeVideoEditor::setWinId(uint winId)
{
    m_winId = winId;
    gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (m_vsink), m_winId);
}

void QDeclarativeVideoEditor::play()
{
    gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_PLAYING);
}


void QDeclarativeVideoEditor::pause()
{
    gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_PAUSED);
}
