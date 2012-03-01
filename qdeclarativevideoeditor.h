#ifndef QDECLARATIVEVIDEOEDITOR_H
#define QDECLARATIVEVIDEOEDITOR_H

#include <QAbstractListModel>

class QDeclarativeVideoEditor : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QDeclarativeVideoEditor(QObject *parent = 0);

    //QAbstractListModel
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    Q_INVOKABLE bool append(const QString &value, int role=Qt::EditRole);

signals:

public slots:


protected:
    QList<QString> m_list;

    Q_DISABLE_COPY(QDeclarativeVideoEditor)
};

#endif // QDECLARATIVEVIDEOEDITOR_H
