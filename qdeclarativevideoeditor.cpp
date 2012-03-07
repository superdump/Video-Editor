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

QString createFileNameFromCurrentTimestamp() {
    QDateTime current = QDateTime::currentDateTime();

    return QString((int) (current.toMSecsSinceEpoch()/1000));
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
        gst_element_set_state ((GstElement *) m_pipeline, GST_STATE_NULL);
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

void QDeclarativeVideoEditor::render()
{
    GstBus *bus = NULL;

    qDebug() << "Render preparations started";

    QString output_uri = "file:///home/user/MyDocs/VideoEditor.mp4";
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

    qDebug() << "Rendering";

    bus = gst_pipeline_get_bus (GST_PIPELINE (m_pipeline));
    gst_bus_add_watch (bus, bus_call, NULL);
    gst_object_unref (bus);
    if(!gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_PLAYING)) {
        gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_NULL);
        gst_object_unref (bus);

        emit error(RENDERING_FAILED, "Failed to set pipeline to playing state");
        return;
    }
}
