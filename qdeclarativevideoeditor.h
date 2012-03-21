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

#ifndef QDECLARATIVEVIDEOEDITOR_H
#define QDECLARATIVEVIDEOEDITOR_H

#include <QAbstractListModel>
#include <QList>
#include <QTimer>
extern "C" {
#include <ges/ges.h>
#include <gst/interfaces/xoverlay.h>
}
#include "videoeditoritem.h"

class QDeclarativeVideoEditor : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(double progress READ getProgress NOTIFY progressChanged)
    Q_PROPERTY(qint64 position READ getPosition NOTIFY progressChanged)
    Q_PROPERTY(uint winId READ getWinId WRITE setWinId NOTIFY winIdChanged)
    Q_PROPERTY(qint64 duration READ getDuration WRITE setDuration NOTIFY durationChanged)

    Q_PROPERTY(uint renderWidth READ getRenderWidth NOTIFY renderResolutionChanged)
    Q_PROPERTY(uint renderHeight READ getRenderHeight NOTIFY renderResolutionChanged)
    Q_PROPERTY(uint renderFpsN READ getRenderFpsN NOTIFY renderResolutionChanged)
    Q_PROPERTY(uint renderFpsD READ getRenderFpsD NOTIFY renderResolutionChanged)

public:
    explicit QDeclarativeVideoEditor(QObject *parent = 0);
    virtual ~QDeclarativeVideoEditor();

    //QAbstractListModel
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    //List manipulation API (not using the usual listmodel API)
    Q_INVOKABLE bool append(const QString &value);
    Q_INVOKABLE QVariant getObjDuration(int idx);
    Q_INVOKABLE void move(int from, int to);
    Q_INVOKABLE void removeAt(int idx);
    Q_INVOKABLE void removeAll();

    //videoeditor API
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE bool render();
    Q_INVOKABLE void cancelRender();

    //render settings API
    Q_INVOKABLE void setRenderSettings(int width, int height, int fps_n, int fps_d);
    Q_INVOKABLE bool isRendering() const;

    gboolean handleBusMessage(GstBus * bus, GstMessage * msg);

    void updateDuration();
    qint64 getDuration();
    void setDuration(qint64 duration);
    double getProgress();
    void setProgress(double progress);
    qint64 getPosition();
    void setPosition(qint64 position);

    uint getRenderWidth() const;
    uint getRenderHeight() const;
    uint getRenderFpsN() const;
    uint getRenderFpsD() const;

    uint getWinId();
    Q_INVOKABLE void setWinId(uint winId);

    GESTimelinePipeline *getPipeline ();

protected:
    GstEncodingProfile *createEncodingProfile();

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

    /**
      * Emitted when the window id has been set
      */
    void winIdChanged();

    void durationChanged();

    void renderResolutionChanged();

public slots:
    void objectUpdated(VideoEditorItem*);
    void updatePosition();

private:
    gint64 m_duration;
    gint64 m_position;
    double m_progress;

    QTimer m_positionTimer;

    bool m_rendering;

    uint m_winId;
    QList<VideoEditorItem *> m_items;

    GESTimeline *m_timeline;
    GESTimelineLayer *m_timelineLayer;
    GESTimelinePipeline *m_pipeline;
    GstElement *m_vsink;

    int m_size;

    int m_width, m_height;
    int m_fpsn, m_fpsd;

    Q_DISABLE_COPY(QDeclarativeVideoEditor)
};

#endif // QDECLARATIVEVIDEOEDITOR_H
