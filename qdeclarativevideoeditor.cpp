#include "qdeclarativevideoeditor.h"

#include <QDebug>

QDeclarativeVideoEditor::QDeclarativeVideoEditor(QObject *parent) :
    QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert( 33 , "url" );
    setRoleNames(roles);
}

int QDeclarativeVideoEditor::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? m_list.size() : 0;
}

QVariant QDeclarativeVideoEditor::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.row() < m_list.size()) {
        switch (role) {
        default:
        {
            return QVariant(m_list.at(index.row()));
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
        if(index.row() == m_list.size()) {
            m_list.append(value.toString());
            emit dataChanged(this->createIndex(index.row(),0), this->createIndex(index.row(),0));
            return true;
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
    bool r = setData(this->createIndex(this->rowCount(), 0), QVariant(value), role);
    endInsertRows();
    return r;
}
