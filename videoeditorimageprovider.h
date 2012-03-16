/* VideoEditor main
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

#ifndef QVIDEOEDITORIMAGEPROVIDER_H
#define QVIDEOEDITORIMAGEPROVIDER_H

#include <QDeclarativeImageProvider>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>

class VideoEditorImageProviderRequest : public QObject
{
    Q_OBJECT
public:
    explicit VideoEditorImageProviderRequest(QObject *parent, const QString uri,
                                             const QSize requestedSize);

    void startRequest();

    bool hasFinished() const;
    QImage getThumbnailImage() const;

signals:
    void requestFinished(VideoEditorImageProviderRequest*);

private:
    QString m_uri;
    QSize m_requestedSize;
};

class VideoEditorImageProvider : public QObject, public QDeclarativeImageProvider
{
    Q_OBJECT
public:
    explicit VideoEditorImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

    VideoEditorImageProviderRequest *addRequest(const QString uri, const QSize requestedSize);
    
signals:
    
public slots:
    void requestFinished(VideoEditorImageProviderRequest*);

private:
    QMutex m_mutex;
    QWaitCondition m_requestFinishedCondition;
    
};

#endif // QVIDEOEDITORIMAGEPROVIDER_H
