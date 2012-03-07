#ifndef QDECLARATIVEVIDEOEDITOR_H
#define QDECLARATIVEVIDEOEDITOR_H

#include <QAbstractListModel>
extern "C" {
#include <ges/ges.h>
}

class QDeclarativeVideoEditor : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QDeclarativeVideoEditor(QObject *parent = 0);
    virtual ~QDeclarativeVideoEditor();

    //QAbstractListModel
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    Q_INVOKABLE bool append(const QString &value, int role=Qt::EditRole);

    //videoeditor API
    Q_INVOKABLE void render();

    Q_INVOKABLE QString getFileName(void);
    Q_INVOKABLE void setFileName(QString filename);


    gboolean handleBusMessage(GstBus * bus, GstMessage * msg);

signals:
    /**
      * Emitted when an error occurred.
      *
      * Message is user displayable text about the error.
      * Debug is further information to help debugging the issue.
      */
    void error(QString message, QString debug);

public slots:


protected:
    GESTimeline *m_timeline;
    GESTimelineLayer *m_timelineLayer;
    GESTimelinePipeline *m_pipeline;

    int m_size;

    QString m_filename;

    Q_DISABLE_COPY(QDeclarativeVideoEditor)
};

#endif // QDECLARATIVEVIDEOEDITOR_H
