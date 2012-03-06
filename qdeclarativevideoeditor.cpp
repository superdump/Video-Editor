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
}

QDeclarativeVideoEditor::~QDeclarativeVideoEditor()
{
    gst_object_unref (m_timeline);
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
            gst_object_unref(src);
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

void QDeclarativeVideoEditor::render()
{
    GESTimelinePipeline *pipeline = ges_timeline_pipeline_new();
    GstBus *bus = NULL;
    GstMessage *msg = NULL;

    qDebug() << "Render preparations started";

    if (!ges_timeline_pipeline_add_timeline (pipeline, (GESTimeline*) gst_object_ref (m_timeline))) {
        emit error(RENDERING_FAILED, "Failed to add timeline to pipeline");
        gst_object_unref (pipeline);
        return;
    }


    QString output_uri = "file:///home/user/MyDocs/VideoEditor - " + createFileNameFromCurrentTimestamp() + ".mp4";
    GstEncodingProfile *profile = createEncodingProfile();
    if (!ges_timeline_pipeline_set_render_settings (pipeline, output_uri.toUtf8().data(), profile)) {
        emit error(RENDERING_FAILED, "Failed setting rendering options");
        gst_object_unref (pipeline);
        gst_encoding_profile_unref(profile);
        return;
    }
    gst_encoding_profile_unref (profile);

    if (!ges_timeline_pipeline_set_mode (pipeline, TIMELINE_MODE_SMART_RENDER)) {
        emit error(RENDERING_FAILED, "Failed to set rendering mode");
        gst_object_unref (pipeline);
        gst_encoding_profile_unref(profile);
        return;
    }

    qDebug() << "Rendering";

    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    if(!gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING)) {
        gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
        gst_object_unref (bus);
        gst_object_unref (pipeline);

        emit error(RENDERING_FAILED, "Failed to set pipeline to playing state");
        return;
    }

    msg = gst_bus_timed_pop_filtered(bus, -1, (GstMessageType) (GST_MESSAGE_EOS | GST_MESSAGE_ERROR));

    if(GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
        GError *gerror = NULL;

        gst_message_parse_error(msg, &gerror, NULL);
        emit error(RENDERING_FAILED, gerror->message);
    } else {
        //TODO notify user
        qDebug() << "Rendering finished successfully";
    }
    gst_message_unref(msg);

    gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
    gst_object_unref (bus);
    gst_object_unref (pipeline);

    return;
}
