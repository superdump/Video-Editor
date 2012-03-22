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

#include <QDebug>

VideoEditorItem::VideoEditorItem(QObject *parent) :
    QObject(parent), m_tlfs(NULL), m_inPoint(-1), m_duration(-1), m_maxDuration(-1)
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

bool VideoEditorItem::setUri(QString uri)
{
    m_uri = uri;
    emit uriChanged();
    return true;
}

QString VideoEditorItem::getFileName() const
{
    return m_fileName;
}

bool VideoEditorItem::setFileName(QString fileName)
{
    m_fileName = fileName;
    emit fileNameChanged();
    return true;
}

quint64 VideoEditorItem::getInPoint() const
{
    return m_inPoint;
}

bool VideoEditorItem::setInPoint(quint64 inPoint)
{
    if (inPoint > m_maxDuration && m_maxDuration != -1) {
        qWarning() << "Invalid inPoint: " << inPoint << " must be 0 <= inPoint <= " << m_maxDuration;
        return false;
    }
    if (inPoint + m_duration > m_maxDuration &&
            m_duration != -1 && m_maxDuration != -1) {
        qWarning() << "Invalid inPoint (due to duration): " << inPoint + m_duration << " > " << m_maxDuration;
        return false;
    }
    m_inPoint = inPoint;
    emit inPointChanged(this);
    return true;
}

quint64 VideoEditorItem::getMaxDuration() const
{
    return m_maxDuration;
}

bool VideoEditorItem::setMaxDuration(quint64 duration)
{
    if (m_inPoint + m_duration > duration &&
            m_inPoint != -1 && m_duration != -1) {
        qWarning() << "Invalid maxDuration : " << m_inPoint + m_duration << " > " << duration;
        return false;
    }
    m_maxDuration = duration;
    emit maxDurationChanged(this);
    return true;
}

quint64 VideoEditorItem::getDuration() const
{
    return m_duration;
}

bool VideoEditorItem::setDuration(quint64 duration)
{
    if (duration > m_maxDuration && m_maxDuration != -1) {
        qWarning() << "Invalid duration: " << duration << " > " << m_maxDuration;
        return false;
    }
    if (m_inPoint + duration > m_maxDuration &&
            m_inPoint != -1 && m_maxDuration != -1) {
        qWarning() << "Invalid duration (due to inPoint): " << m_inPoint + duration << " > " << m_maxDuration;
        return false;
    }
    m_duration = duration;
    emit durationChanged(this);
    return true;
}

GESTimelineFileSource *VideoEditorItem::getTlfs() const
{
    return m_tlfs;
}

void VideoEditorItem::setTlfs(GESTimelineFileSource *tlfs)
{
    m_tlfs = tlfs;
}

unsigned long VideoEditorItem::getDurHdlrID() const
{
    return m_dur_hdlr_id;
}

void VideoEditorItem::setDurHdlrID(unsigned long handler_id)
{
    m_dur_hdlr_id = handler_id;
}
