/* VideoEditor ImageProvider
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

VideoEditorImageProviderRequest::VideoEditorImageProviderRequest(QObject *parent, const QString uri, GstClockTime timestamp,
                                                                 bool perc, const QSize requestedSize) :
    QObject(parent), m_uri(uri), m_requestedSize(requestedSize), m_timestamp(timestamp), m_perc(perc),
    m_pipeline(NULL), m_videosink(NULL), m_width(0), m_height(0), m_thumbnail(NULL), m_state(IDLE)
{
}

VideoEditorImageProviderRequest::~VideoEditorImageProviderRequest()
{
    if(m_thumbnail) {
        gst_buffer_unref(m_thumbnail);
    }
    if(m_videosink) {
        gst_object_unref(m_videosink);
    }
    if(m_pipeline) {
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref (m_pipeline);
    }
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

    return QImage(GST_BUFFER_DATA(m_thumbnail), m_width, m_height, QImage::Format_RGB888).copy();
}

void VideoEditorImageProviderRequest::setThumbnail(GstBuffer *buffer)
{
    GstCaps *outcaps = NULL;
    GstVideoFormat *format = NULL;
    int caps_width = -1, caps_height = -1, par_n = 1, par_d = 1;

    gst_video_format_parse_caps(GST_BUFFER_CAPS (buffer), format, &caps_width, &caps_height);
    gst_video_parse_caps_pixel_aspect_ratio(GST_BUFFER_CAPS (buffer), &par_n, &par_d);

    double dar = -1.0;
    if (caps_height && par_d)
        dar = (par_n * caps_width) / (double)(par_d * caps_height);

    m_width = m_requestedSize.width() > 0 ? m_requestedSize.width() : -1;
    m_height = m_requestedSize.height() > 0 ? m_requestedSize.height() : -1;

    if (m_width == -1) {
        if (m_height == -1) {
            m_width = 100;
            m_height = 50;
        } else {
            // calc m_width = m_height * DAR
            m_width = (int)(m_height * dar);
        }
    } else if (m_height == -1) {
        // calc m_height = m_width / DAR
        m_height = (int)(m_width / dar);
    }

    outcaps = gst_video_format_new_caps(GST_VIDEO_FORMAT_RGB, m_width, m_height, 1, 1, 1, 1);
    if(m_thumbnail) {
        gst_buffer_unref (m_thumbnail);
    }
    m_thumbnail = gst_video_convert_frame(buffer, outcaps, GST_CLOCK_TIME_NONE, NULL);
    gst_caps_unref (outcaps);
    finish();
}

bool VideoEditorImageProviderRequest::handleBusMessage(GstBus *, GstMessage *msg)
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
        fail();
        break;
    }
    case GST_MESSAGE_STATE_CHANGED:
    {
        if(GST_MESSAGE_SRC (msg) == (GstObject *) m_pipeline) {
            GstState state;
            gst_message_parse_state_changed(msg, NULL, &state, NULL);
            if(state == GST_STATE_PAUSED) {
                if(m_state == STARTED) {
                    qDebug() << "Playbin reached paused state";
                    if (m_timestamp == 0) {
                        m_state = GENERATING;
                        GstBuffer *last_buffer;
                        g_object_get(m_videosink, "last-buffer", &last_buffer, NULL);

                        setThumbnail(last_buffer);
                        gst_buffer_unref(last_buffer);
                        gst_element_set_state ((GstElement *) m_pipeline, GST_STATE_NULL);
                        ret = FALSE;
                    } else {
                        qDebug() << "Initiating seek";
                        m_state = SEEKING_START;
                        gint64 seek_ts = m_timestamp;
                        GstFormat format = GST_FORMAT_TIME;
                        gint64 dur = 0;
                        if(m_perc) {
                            qDebug() << "Percentage seek";
                            if(gst_element_query_duration(GST_ELEMENT(m_pipeline), &format, &dur)) {
                                seek_ts = m_timestamp * (dur / 100.0);
                            } else {
                                qDebug() << "Duration query failed";
                                fail();
                                ret=FALSE;
                            }
                        }
                        if(ret) {
                            qDebug() << "Sending seek to " << seek_ts << "/" << dur;
                            bool seek = gst_element_seek_simple(GST_ELEMENT(m_pipeline), format,
                                                                GST_SEEK_FLAG_FLUSH, seek_ts);
                            if(!seek) {
                                qDebug() << "Seek failed";
                                //Failed
                                fail();
                                ret = FALSE;
                            }
                        }
                    }
                }
            }
        }
    }
        break;
    case GST_MESSAGE_ASYNC_DONE:
    {
        if(GST_MESSAGE_SRC(msg) == (GstObject*) m_pipeline) {
            qDebug() << "Async done";
            if(m_state == SEEKING) {
                GstBuffer *last_buffer;
                g_object_get(m_videosink, "last-buffer", &last_buffer, NULL);

                qDebug() << "Seeking finished" << last_buffer;
                setThumbnail(last_buffer);
                gst_buffer_unref(last_buffer);
                gst_element_set_state ((GstElement *) m_pipeline, GST_STATE_NULL);
                ret = FALSE;
            } else if(m_state == SEEKING_START) {
                /*
                 * TODO we are discarding the first async-done because it seems
                 * to come from the first state change.
                 *
                 * Need to investigate what happens with various formats and check
                 * for a way that wouldn't rely on this.
                 *
                 * Ideas: clear the bus of async-done messages before issuing the seek
                 */
                m_state = SEEKING;
            }
        }
    }
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

    g_object_set (m_pipeline, "uri", m_uri.toUtf8().data(), NULL);

    GstStateChangeReturn changeret = gst_element_set_state (m_pipeline, GST_STATE_PAUSED);
    if(changeret == GST_STATE_CHANGE_FAILURE) {
        qDebug() << "Failed to get thumbnail" << m_uri;
        fail();
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        return;
    }

    qDebug() << "Entering message loop";
    bool go = true;
    bus = gst_pipeline_get_bus(GST_PIPELINE (m_pipeline));
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

void VideoEditorImageProviderRequest::fail() {
    m_state = FAILED;
    emit requestFinished(this);
}

bool VideoEditorImageProviderRequest::hasFinished() const
{
    return m_state == FINISHED || m_state == FAILED;
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

void VideoEditorImageProvider::requestFinished(VideoEditorImageProviderRequest*)
{
    m_mutex.lock();
    //wake them all and let them test if their request has finished
    m_requestFinishedCondition.wakeAll();
    m_mutex.unlock();
}

VideoEditorImageProviderRequest* VideoEditorImageProvider::addRequest(const QString uriAndTimestamp, const QSize requestedSize)
{
    QString uri = uriAndTimestamp;
    int index = uri.lastIndexOf('#');
    GstClockTime timestamp = 0;
    bool perc = false;

    if(index != -1) {
        QString tsstr = uri.right(uri.length() - (index + 1));
        uri = uri.left(index);

        if(tsstr.endsWith('%')) {
            perc = true;
            tsstr = tsstr.left(tsstr.length()-1);
        }

        timestamp = tsstr.toULongLong();
    }

    qDebug() << "Requested thumbnail for" << uri << " # " << timestamp << (perc ? "%" : "");

    VideoEditorImageProviderRequest *request = new VideoEditorImageProviderRequest(NULL, uri, timestamp, perc, requestedSize);
    connect(request, SIGNAL(requestFinished(VideoEditorImageProviderRequest*)), SLOT(requestFinished(VideoEditorImageProviderRequest*)));
    request->startRequest();
    return request;
}
