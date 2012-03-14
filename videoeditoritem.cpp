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

#include "videoeditoritem.h"

VideoEditorItem::VideoEditorItem(QObject *parent) :
    QObject(parent), m_tlfs(NULL), m_inPoint(-1), m_duration(-1)
{
    // Nothing to init
}

VideoEditorItem::~VideoEditorItem()
{
    if (m_tlfs)
        gst_object_unref (m_tlfs);
}

QString VideoEditorItem::getUri() const
{
    return m_uri;
}

void VideoEditorItem::setUri(QString uri)
{
    m_uri = uri;
    emit uriChanged();
}

QString VideoEditorItem::getFileName() const
{
    return m_fileName;
}

void VideoEditorItem::setFileName(QString fileName)
{
    m_fileName = fileName;
    emit fileNameChanged();
}

quint64 VideoEditorItem::getInPoint() const
{
    return m_inPoint;
}

void VideoEditorItem::setInPoint(quint64 inPoint)
{
    m_inPoint = inPoint;
    emit inPointChanged();
}

quint64 VideoEditorItem::getDuration() const
{
    return m_duration;
}

void VideoEditorItem::setDuration(quint64 duration)
{
    m_duration = duration;
    emit durationChanged();
}

GESTimelineFileSource *VideoEditorItem::getTlfs() const
{
    return m_tlfs;
}

void VideoEditorItem::setTlfs(GESTimelineFileSource *tlfs)
{
    m_tlfs = tlfs;
}
