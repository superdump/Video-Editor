#include "qdeclarativevideoeditor.h"

#include <QDebug>
#include <QDateTime>

#define RENDERING_FAILED "Rendering failed"

QDeclarativeVideoEditor::QDeclarativeVideoEditor(QObject *parent) :
    QAbstractListModel(parent), m_size(0)
{
    QHash<int, QByteArray> roles;
    roles.insert( 33 , "uri" );
    setRoleNames(roles);

    m_timeline = ges_timeline_new_audio_video();
    m_timelineLayer = (GESTimelineLayer*) ges_simple_timeline_layer_new();
    ges_timeline_add_layer(m_timeline, m_timelineLayer);
    m_pipeline = ges_timeline_pipeline_new();
    ges_timeline_pipeline_add_timeline (m_pipeline, m_timeline);
    m_duration = GST_CLOCK_TIME_NONE;
    m_progress = 0.0;
}

QDeclarativeVideoEditor::~QDeclarativeVideoEditor()
{
    gst_element_set_state ((GstElement*) m_pipeline, GST_STATE_NULL);
    gst_object_unref (m_pipeline);
}

int QDeclarativeVideoEditor::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? m_size : 0;
}

QVariant QDeclarativeVideoEditor::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.row() < m_size) {
        switch (role) {
        default:
        {
            GESTimelineFileSource *src = (GESTimelineFileSource*) ges_simple_timeline_layer_nth((GESSimpleTimelineLayer*) m_timelineLayer, index.row());
            QVariant ret = QVariant(ges_timeline_filesource_get_uri(src));
            return ret;
        }
        }
    } else {
        return QVariant();
    }
}

Qt::ItemFlags QDeclarativeVideoEditor::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractListModel::flags(index);
    return flags | Qt::ItemIsEditable;
}

bool QDeclarativeVideoEditor::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qDebug() << "Setting data:" << index.isValid() << value.toString() << role;
    if(index.isValid() && role == Qt::EditRole) {
        if(index.row() == m_size) {
            //m_list.append(value.toString());
            //emit dataChanged(this->createIndex(index.row(),0), this->createIndex(index.row(),0));
            return false;
        }
    } else {
        qDebug() << "Invalid index or role";
    }
    return false;
}

bool QDeclarativeVideoEditor::append(const QString &value, int role)
{
    qDebug() << "Appending new item:" << value;
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    GESTimelineFileSource *src = ges_timeline_filesource_new(value.toUtf8().data());
    bool r = ges_timeline_layer_add_object(m_timelineLayer, (GESTimelineObject*) src);
    if (r) m_size++;
    endInsertRows();
    return r;
}

GstEncodingProfile *createEncodingProfile() {
    GstEncodingProfile *profile = (GstEncodingProfile *)
            gst_encoding_container_profile_new("mp4", NULL, gst_caps_new_simple("video/quicktime",
                                                                                "variant", G_TYPE_STRING, "iso",
                                                                                NULL), NULL);
    GstEncodingProfile *video = (GstEncodingProfile *)
            gst_encoding_video_profile_new(gst_caps_new_simple("video/mpeg", "mpegversion",
                                                               G_TYPE_INT, 4, NULL), NULL, NULL, 1);
    GstEncodingProfile *audio = (GstEncodingProfile *)
            gst_encoding_audio_profile_new(gst_caps_new_simple("audio/mpeg", "mpegversion",
                                                               G_TYPE_INT, 4, NULL), NULL, NULL, 0);

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
        gst_element_set_state ((GstElement *) m_pipeline, GST_STATE_NULL);
        //emit renderComplete();
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

        self->setProgress ((double)cur_pos / duration);
        qDebug() << "Render progress " << self->getProgress() * 100
             << "% (" << cur_pos << "/" << duration << ")";
    }
    emit self->emitProgressChanged();

    return true;
}

void QDeclarativeVideoEditor::render()
{
    GstBus *bus = NULL;

    qDebug() << "Render preparations started";

    QString output_uri = "file:///home/user/MyDocs/Movies/" + getDateTimeString() + ".mp4";

    GstEncodingProfile *profile = createEncodingProfile();
    if (!ges_timeline_pipeline_set_render_settings (m_pipeline, output_uri.toUtf8().data(), profile)) {
        emit error(RENDERING_FAILED, "Failed setting rendering options");
        gst_encoding_profile_unref(profile);
        return;
    }
    gst_encoding_profile_unref (profile);

    if (!ges_timeline_pipeline_set_mode (m_pipeline, TIMELINE_MODE_SMART_RENDER)) {
        emit error(RENDERING_FAILED, "Failed to set rendering mode");
        gst_encoding_profile_unref(profile);
        return;
    }

    qDebug() << "Rendering to " << output_uri;

    bus = gst_pipeline_get_bus (GST_PIPELINE (m_pipeline));
    gst_bus_add_watch (bus, bus_call, this);
    gst_object_unref (bus);

    // reset duration and progress
    setDuration(GST_CLOCK_TIME_NONE);
    setProgress(0.0);
    g_timeout_add (500, updateProgress, this);

    if(!gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_PLAYING)) {
        gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_NULL);
        gst_object_unref (bus);

        emit error(RENDERING_FAILED, "Failed to set pipeline to playing state");
        return;
    }
}
