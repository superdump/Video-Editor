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
#include "videoeditoritem.h"

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
    QAbstractListModel(parent), m_position(0), m_positionTimer(this), m_size(0),
    m_width(0), m_height(0), m_fpsn(0), m_fpsd(0)
{
    QHash<int, QByteArray> roles;
    roles.insert( 33 , "uri" );
    roles.insert( 34 , "fileName" );
    roles.insert( 35 , "inPoint" );
    roles.insert( 36 , "duration" );
    setRoleNames(roles);

    connect(&m_positionTimer, SIGNAL(timeout()), SLOT(updatePosition()));

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
    gst_element_set_state ((GstElement*) m_pipeline, GST_STATE_PAUSED);
    m_duration = GST_CLOCK_TIME_NONE;
    m_progress = 0.0;
}

QDeclarativeVideoEditor::~QDeclarativeVideoEditor()
{
    m_positionTimer.stop();
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
        const VideoEditorItem *item = m_items.at(index.row());
        switch (role) {
        case 33:
            ret = QVariant(item->getUri());
            break;
        case 34:
            ret = QVariant(item->getFileName());
            break;
        case 35:
            ret = QVariant(item->getInPoint());
            break;
        case 36:
            ret = QVariant(item->getDuration());
            break;
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

void timeline_filesource_maxduration_cb (GObject *, GParamSpec *, gpointer user_data)
{
    VideoEditorItem *item = (VideoEditorItem *)user_data;

    quint64 dur = GST_CLOCK_TIME_NONE;
    g_object_get (item->getTlfs(), "max-duration", &dur, NULL);
    item->setDuration(dur);
}

void QDeclarativeVideoEditor::objectUpdated(VideoEditorItem *item)
{
    int row = m_items.indexOf(item);
    emit dataChanged(index(row), index(row));
}

bool QDeclarativeVideoEditor::append(const QString &value)
{
    qDebug() << "Appending new item:" << value;
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    VideoEditorItem *item = new VideoEditorItem();

    item->setTlfs(ges_timeline_filesource_new(value.toUtf8().data()));
    item->setUri(value);
    QFileInfo file(value.toUtf8().data());
    item->setFileName(file.fileName());
    item->setDurHdlrID(g_signal_connect(item->getTlfs(), "notify::max-duration",
                       G_CALLBACK(timeline_filesource_maxduration_cb), item));
    connect(item, SIGNAL(durationChanged(VideoEditorItem*)),
            SLOT(objectUpdated(VideoEditorItem*)));

    m_items.append(item);

    bool r = ges_timeline_layer_add_object(m_timelineLayer, (GESTimelineObject*) item->getTlfs());
    if (r) m_size++;

    endInsertRows();
    return r;
}

QVariant QDeclarativeVideoEditor::getObjDuration(int idx)
{
    if (idx >= 0 && idx < rowCount()) {
        return data(index(idx), 36);
    }
    return 1000000000; // one second for safety
}

void QDeclarativeVideoEditor::move(int from, int to)
{
    qDebug() << "Moving media object from " << from << " to " << to;
    beginResetModel();
    const VideoEditorItem *item = m_items.at(from);
    ges_simple_timeline_layer_move_object(GES_SIMPLE_TIMELINE_LAYER (m_timelineLayer),
                                          (GESTimelineObject *)item->getTlfs(), to);
    m_items.move(from, to);
    endResetModel();
}

void QDeclarativeVideoEditor::removeAt(int idx)
{
    VideoEditorItem *item = m_items.takeAt(idx);
    ges_timeline_layer_remove_object(m_timelineLayer, (GESTimelineObject *)item->getTlfs());
    g_signal_handler_disconnect(item->getTlfs(), item->getDurHdlrID());
    delete item;
    m_size--;
}

void QDeclarativeVideoEditor::removeAll()
{
    beginRemoveRows(QModelIndex(), 0, rowCount());
    while(m_items.isEmpty() == false)
        removeAt(0);
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
QDeclarativeVideoEditor::handleBusMessage (GstBus *, GstMessage *msg)
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

qint64 QDeclarativeVideoEditor::getDuration()
{
    GstFormat format = GST_FORMAT_TIME;
    gst_element_query_duration (GST_ELEMENT (m_pipeline), &format, &m_duration);
    qDebug() << "Got duration :" << m_duration;
    return m_duration;
}

void QDeclarativeVideoEditor::setDuration(qint64 duration)
{
    qDebug() << "Composition duration: " << m_duration << " to " << duration;
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

qint64 QDeclarativeVideoEditor::getPosition()
{
    return this->m_position;
}

void QDeclarativeVideoEditor::setPosition(qint64 position)
{
    this->m_position = position;
}

void QDeclarativeVideoEditor::emitProgressChanged()
{
    emit progressChanged();
}

void QDeclarativeVideoEditor::updatePosition()
{
    double progress = this->getProgress();
    if (progress == -1.0) {
        qDebug() << "Stoping progress polling";
        progress = 0.0;
        m_positionTimer.stop();
        return;
    }

    double duration = this->getDuration();
    if(duration == -1) {
        //unknown
        this->setProgress(0);
    } else {
        gint64 cur_pos = GST_CLOCK_TIME_NONE;
        GstFormat format_time = GST_FORMAT_TIME;
        gst_element_query_position (GST_ELEMENT (this->getPipeline()), &format_time, &cur_pos);

        if(duration < 0 || cur_pos < 0) {
            this->setProgress (0);
            this->setPosition(0);
            qDebug() << "Render progress unknown";
        } else {
            this->setPosition(cur_pos);
            this->setProgress ((double)cur_pos / duration);
            qDebug() << "Render progress " << this->getProgress() * 100
                 << "% (" << cur_pos << "/" << duration << ")";
        }
    }

    if (this->getProgress() < 0.0) {
        this->setProgress(0.0);
        this->setPosition(0);
        qDebug() << "Stoping progress polling";
        m_positionTimer.stop();
        return;
    }

    emit emitProgressChanged();
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

    m_positionTimer.start(500);
    //g_timeout_add (500, updateProgress, this);

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
    m_positionTimer.start(500);
    gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_PLAYING);
}


void QDeclarativeVideoEditor::pause()
{
    m_positionTimer.stop();
    gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_PAUSED);
}
