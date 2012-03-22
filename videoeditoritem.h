/* VideoEditorItem
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

#ifndef VIDEOEDITORITEM_H
#define VIDEOEDITORITEM_H

#include <QObject>
#include <QVariant>

extern "C" {
#include <ges/ges.h>
}

class VideoEditorItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString uri READ getUri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(QString fileName READ getFileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(quint64 inPoint READ getInPoint WRITE setInPoint NOTIFY inPointChanged)
    Q_PROPERTY(quint64 maxDuration READ getMaxDuration WRITE setMaxDuration NOTIFY maxDurationChanged)
    Q_PROPERTY(quint64 duration READ getDuration WRITE setDuration NOTIFY durationChanged)
public:
    explicit VideoEditorItem(QObject *parent = 0);
    virtual ~VideoEditorItem();

    QString getUri() const;
    bool setUri(QString uri);
    QString getFileName() const;
    bool setFileName(QString fileName);
    quint64 getInPoint() const;
    bool setInPoint(quint64 inPoint);
    quint64 getDuration() const;
    bool setDuration(quint64 duration);
    quint64 getMaxDuration() const;
    bool setMaxDuration(quint64 duration);
    GESTimelineFileSource *getTlfs() const;
    void setTlfs(GESTimelineFileSource *tlfs);
    unsigned long getDurHdlrID() const;
    void setDurHdlrID(unsigned long handler_id);

signals:

    void uriChanged();
    void fileNameChanged();
    void inPointChanged(VideoEditorItem*);
    void durationChanged(VideoEditorItem*);
    void maxDurationChanged(VideoEditorItem*);

public slots:

private:
    GESTimelineFileSource *m_tlfs;
    QString m_uri;
    QString m_fileName;
    quint64 m_inPoint;
    quint64 m_maxDuration;
    quint64 m_duration;
    unsigned long m_dur_hdlr_id;

    Q_DISABLE_COPY(VideoEditorItem)
};

#endif // VIDEOEDITORITEM_H
