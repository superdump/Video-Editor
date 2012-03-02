#include "qdeclarativevideoeditor.h"

#include <QDebug>

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
