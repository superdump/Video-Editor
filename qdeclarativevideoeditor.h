#ifndef QDECLARATIVEVIDEOEDITOR_H
#define QDECLARATIVEVIDEOEDITOR_H

#include <QAbstractListModel>
extern "C" {
#include <ges/ges.h>
}

class QDeclarativeVideoEditor : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(double progress READ getProgress NOTIFY progressChanged)
public:
    explicit QDeclarativeVideoEditor(QObject *parent = 0);
    virtual ~QDeclarativeVideoEditor();

    //QAbstractListModel
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    //List manipulation API (not using the usual listmodel API)
    Q_INVOKABLE bool append(const QString &value);

    //videoeditor API
    Q_INVOKABLE void render();
    Q_INVOKABLE void cancelRender();

    gboolean handleBusMessage(GstBus * bus, GstMessage * msg);

    gint64 getDuration();
    void setDuration(gint64 duration);
    double getProgress();
    void setProgress(double progress);
    void emitProgressChanged();

    GESTimelinePipeline *getPipeline ();

private:
    gint64 m_duration;
    double m_progress;

signals:
    /**
      * Emitted when an error occurred.
      *
      * Message is user displayable text about the error.
      * Debug is further information to help debugging the issue.
      */
    void error(QString message, QString debug);

    /**
      * Emitted when the progress property has been updated.
      *
      * Read the value from the property.
      */
    void progressChanged();

    /**
      * Emitted when the pipeline hits EOS from rendering
      */
    void renderComplete();

public slots:

protected:
    GESTimeline *m_timeline;
    GESTimelineLayer *m_timelineLayer;
    GESTimelinePipeline *m_pipeline;

    int m_size;

    Q_DISABLE_COPY(QDeclarativeVideoEditor)
};

#endif // QDECLARATIVEVIDEOEDITOR_H
