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
#include <gst/video/video.h>

#include <QDebug>

VideoEditorImageProviderRequest::VideoEditorImageProviderRequest(QObject *parent, const QString uri,
                                                                 const QSize requestedSize) :
    QObject(parent), m_uri(uri), m_requestedSize(requestedSize), m_pipeline(NULL),
    m_videosink(NULL), m_state(IDLE), m_thumbnail(NULL)
{
}

VideoEditorImageProviderRequest::~VideoEditorImageProviderRequest()
{
}

QImage VideoEditorImageProviderRequest::getThumbnailImage() const
{
    int width = 100;
    int height = 50;
    if(m_thumbnail == NULL) {
        QImage image(m_requestedSize.width() > 0 ? m_requestedSize.width() : width,
                     m_requestedSize.height() > 0 ? m_requestedSize.height() : height, QImage::Format_ARGB32);

        image.fill(QColor("blue").rgba());
        return image;
    }

    return QImage(GST_BUFFER_DATA(m_thumbnail), m_width, m_height, QImage::Format_RGB888);
}

void VideoEditorImageProviderRequest::setThumbnail(GstBuffer *buffer)
{
    GstCaps *outcaps;

    m_width = m_requestedSize.width() > 0 ? m_requestedSize.width() : 100;
    m_height = m_requestedSize.height() > 0 ? m_requestedSize.height() : 50;

    outcaps = gst_video_format_new_caps(GST_VIDEO_FORMAT_RGB, m_width, m_height, 1, 1, 1, 1);
    m_thumbnail = gst_video_convert_frame(buffer, outcaps, GST_CLOCK_TIME_NONE, NULL);
    gst_caps_unref (outcaps);
    finish();
}

bool VideoEditorImageProviderRequest::handleBusMessage(GstBus *bus, GstMessage *msg)
{
    bool ret = TRUE;
    switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
        qDebug() << "ImageProviderRequest EOS";
        gst_element_set_state ((GstElement *) m_pipeline, GST_STATE_NULL);
        ret = FALSE;
        break;

    case GST_MESSAGE_ERROR: {
        gchar  *debug;
        GError *gerror;

        gst_message_parse_error (msg, &gerror, &debug);
        g_free (debug);

        qDebug() << "Image provider error:" << gerror->message;
        g_error_free (gerror);
        gst_element_set_state ((GstElement *) m_pipeline, GST_STATE_NULL);
        ret = FALSE;
        break;
    }
    case GST_MESSAGE_STATE_CHANGED:
    {
        if(GST_MESSAGE_SRC (msg) == (GstObject *) m_pipeline) {
            GstState state;
            gst_message_parse_state_changed(msg, NULL, &state, NULL);
            if(state == GST_STATE_PAUSED) {
                qDebug() << "Playbin reached paused state";
                m_state = GENERATING;
                GstBuffer *last_buffer;
                g_object_get(m_videosink, "last-buffer", &last_buffer, NULL);

                setThumbnail(last_buffer);
                gst_buffer_unref(last_buffer);
                gst_element_set_state ((GstElement *) m_pipeline, GST_STATE_NULL);
                ret = FALSE;
            }
        }
    }
        break;
    default:
        break;
    }

    return ret;
}

void VideoEditorImageProviderRequest::startRequest()
{
    GstBus *bus;

    //TODO check current state
    m_state = STARTED;
    if(m_pipeline == NULL) {
        m_pipeline = gst_element_factory_make("playbin2", NULL);


        m_videosink = gst_element_factory_make("fakesink", "video-sink");
        g_object_set (m_videosink, "sync", TRUE, "enable-last-buffer", TRUE, NULL);
        GstElement *audiosink = gst_element_factory_make ("fakesink", "audio-sink");

        gst_object_ref (m_videosink);
        g_object_set (m_pipeline, "video-sink", m_videosink, "audio-sink", audiosink, NULL);
    }
    bus = gst_pipeline_get_bus(GST_PIPELINE (m_pipeline));

    g_object_set (m_pipeline, "uri", m_uri.toUtf8().data(), NULL);

    GstStateChangeReturn changeret = gst_element_set_state (m_pipeline, GST_STATE_PAUSED);
    if(changeret == GST_STATE_CHANGE_FAILURE) {
        qDebug() << "Failed to get thumbnail" << m_uri;
        //TODO react with failure
    }

    qDebug() << "Entering message loop";
    bool go = true;
    while(go) {
        GstMessage *msg = gst_bus_pop(bus);
        if(msg == NULL) {
            sleep(1);
        } else {
            go = this->handleBusMessage(bus, msg);
        }
    }
    gst_object_unref (bus);
}

void VideoEditorImageProviderRequest::finish() {
    m_state = FINISHED;
    emit requestFinished(this);
}

bool VideoEditorImageProviderRequest::hasFinished() const
{
    return m_state == FINISHED;
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
