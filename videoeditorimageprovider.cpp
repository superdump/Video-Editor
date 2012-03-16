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

#include "videoeditorimageprovider.h"

VideoEditorImageProviderRequest::VideoEditorImageProviderRequest(QObject *parent, const QString uri,
                                                                 const QSize requestedSize) :
    QObject(parent), m_uri(uri), m_requestedSize(requestedSize)
{
}

QImage VideoEditorImageProviderRequest::getThumbnailImage() const
{
    int width = 100;
    int height = 50;

    QImage image(m_requestedSize.width() > 0 ? m_requestedSize.width() : width,
                   m_requestedSize.height() > 0 ? m_requestedSize.height() : height, QImage::Format_ARGB32);

    image.fill(QColor("blue").rgba());

    return image;
}

void VideoEditorImageProviderRequest::startRequest()
{
    emit requestFinished(this);
}

bool VideoEditorImageProviderRequest::hasFinished() const
{
    return true;
}

VideoEditorImageProvider::VideoEditorImageProvider() :
    QObject(), QDeclarativeImageProvider(QDeclarativeImageProvider::Image),
    m_mutex(), m_requestFinishedCondition()
{
}

QImage VideoEditorImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    m_mutex.lock();
    VideoEditorImageProviderRequest *request = addRequest(id, requestedSize);

    while(!request->hasFinished()) {
        m_requestFinishedCondition.wait(&m_mutex);
    }

    QImage image = request->getThumbnailImage();
    delete request;
    m_mutex.unlock();
    return image;
}

void VideoEditorImageProvider::requestFinished(VideoEditorImageProviderRequest* request)
{
    m_mutex.lock();
    //wake them all and let them test if their request has finished
    m_requestFinishedCondition.wakeAll();
    m_mutex.unlock();
}

VideoEditorImageProviderRequest* VideoEditorImageProvider::addRequest(const QString uri, const QSize requestedSize)
{
    VideoEditorImageProviderRequest *request = new VideoEditorImageProviderRequest(NULL, uri, requestedSize);
    connect(request, SIGNAL(requestFinished(VideoEditorImageProviderRequest*)), SLOT(requestFinished(VideoEditorImageProviderRequest*)));
    request->startRequest();
    return request;
}
