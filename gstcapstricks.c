/* VideoEditor CapsTricks
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

#include "gstcapstricks.h"
#include <string.h>
#include <glib.h>

static void gstcapstricks_encodebin_find_elements(GstBin * bin);
static gboolean gstcapstricks_qtmux_setcaps(GstPad * sinkpad, GstCaps * caps);

/**
  * Store the old setcaps function of a pad and the last caps that was set on that
  * pad
  */
typedef struct {
  GstPadSetCapsFunction setcaps;
  GstCaps *caps;
} SetCapsData;
static GHashTable *qtmux_setcaps_functions = NULL;

void
gstcapstricks_init(void) {
    qtmux_setcaps_functions = g_hash_table_new(NULL, NULL);
}
//TODO make deinit function

static void
qtmux_setcaps_functions_add(GstElement * element, GstPad * sinkpad)
{
    SetCapsData *data = g_new0(SetCapsData, 1);

    data->setcaps = GST_PAD_SETCAPSFUNC(sinkpad);

    g_hash_table_insert(qtmux_setcaps_functions, gst_object_ref (element), data);

    gst_pad_set_setcaps_function(sinkpad, gstcapstricks_qtmux_setcaps);
}

static gboolean
is_encodebin(GstElement * element)
{
    GstElementFactory *factory = gst_element_get_factory(element);
    if(factory) {
        const char *name = GST_PLUGIN_FEATURE_NAME (factory);
        if(strcmp (name, "encodebin") == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean
is_dspenc(GstElement * element) {
    GstElementFactory *factory = gst_element_get_factory(element);
    const char *name;
    if(factory == NULL) {
        name = GST_ELEMENT_NAME(element);
    } else {
        name = GST_PLUGIN_FEATURE_NAME (factory);
    }

    //TODO a proper check would be to check that dsp is at the beginning and enc at the enc
    if (strstr(name, "dsp") != NULL && strstr(name, "enc") != NULL) {
        return TRUE;
    }
    return FALSE;
}

static gboolean
is_aacenc(GstElement * element) {
    GstElementFactory *factory = gst_element_get_factory(element);
    const char *name;
    if(factory == NULL) {
        name = GST_ELEMENT_NAME(element);
    } else {
        name = GST_PLUGIN_FEATURE_NAME (factory);
    }

    if (strstr(name, "aacenc") != NULL) {
        return TRUE;
    }
    return FALSE;
}

static gboolean
is_qtmux(GstElement * element) {
    GstElementFactory *factory = gst_element_get_factory(element);
    const char *name;
    if(factory == NULL) {
        name = GST_ELEMENT_NAME(element);
    } else {
        name = GST_PLUGIN_FEATURE_NAME (factory);
    }

    if (strstr(name, "qtmux") != NULL || strstr(name, "mp4mux") != NULL) {
        return TRUE;
    }
    return FALSE;
}

static gboolean
is_playsink(GstElement * element) {
    GstElementFactory *factory = gst_element_get_factory(element);
    const char *name;
    if(factory == NULL) {
        name = GST_ELEMENT_NAME(element);
    } else {
        name = GST_PLUGIN_FEATURE_NAME (factory);
    }

    if (strstr(name, "playsink") != NULL) {
        return TRUE;
    }
    return FALSE;
}

static gboolean
play_sink_multiple_seeks_send_event (GstElement * element, GstEvent * event)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS (element);

  GST_DEBUG ("%s", GST_EVENT_TYPE_NAME (event));

  return
      GST_ELEMENT_CLASS (g_type_class_peek_parent (klass))->send_event (element,
      event);
}

void
gstcapstricks_set_playsink_sendevent(GstElement * playsink)
{
    GstElementClass* klass;

    klass = GST_ELEMENT_GET_CLASS (playsink);
    klass->send_event = play_sink_multiple_seeks_send_event;
}

static gboolean
is_video_pad(GstPad * pad) {
    return strstr(GST_PAD_NAME(pad), "video") != NULL;
}

static GstCaps *
gstcapstricks_video_getcaps(GstPad * sinkpad)
{
    GstElement *parent = gst_pad_get_parent_element (sinkpad);
    GstPad *srcpad = gst_element_get_static_pad (parent, "src");
    GstCaps *caps;
    GstCaps *outcaps;
    GstCaps *downstreamcaps;
    guint i;

    downstreamcaps = gst_pad_get_allowed_caps(srcpad);

    if(downstreamcaps == NULL || gst_caps_is_any (downstreamcaps)) {
        caps = gst_caps_copy(gst_pad_get_pad_template_caps(sinkpad));
        goto end;
    }

    if(gst_caps_is_empty (downstreamcaps)) {
        caps = gst_caps_copy(downstreamcaps);
        goto end;
    }

    outcaps = gst_caps_new_empty();

    for(i = 0; i < gst_caps_get_size (downstreamcaps); i++) {
        GstStructure *structure = gst_caps_get_structure(downstreamcaps, i);
        const GValue *width;
        const GValue *height;
        const GValue *framerate;
        GstStructure *output;

        width = gst_structure_get_value(structure, "width");
        height = gst_structure_get_value(structure, "height");
        framerate = gst_structure_get_value(structure, "framerate");

        output = gst_structure_new ("video/x-raw-yuv", NULL);
        if(width)
            gst_structure_set_value(output, "width", width);
        if(height)
            gst_structure_set_value(output, "height", height);
        if(framerate)
            gst_structure_set_value(output, "framerate", framerate);

        gst_caps_append_structure(outcaps, output);
    }

    caps = gst_caps_intersect(outcaps, gst_pad_get_pad_template_caps(sinkpad));
    gst_caps_unref(outcaps);

end:
    gst_caps_unref (downstreamcaps);
    gst_object_unref (parent);
    gst_object_unref (srcpad);
    return caps;
}

static GstCaps *
gstcapstricks_audio_getcaps(GstPad * sinkpad)
{
    GstElement *parent = gst_pad_get_parent_element (sinkpad);
    GstPad *srcpad = gst_element_get_static_pad (parent, "src");
    GstCaps *caps;
    GstCaps *outcaps;
    GstCaps *downstreamcaps;
    guint i;

    downstreamcaps = gst_pad_get_allowed_caps(srcpad);

    if(downstreamcaps == NULL || gst_caps_is_any (downstreamcaps)) {
        caps = gst_caps_copy(gst_pad_get_pad_template_caps(sinkpad));
        goto end;
    }

    if(gst_caps_is_empty (downstreamcaps)) {
        caps = gst_caps_copy(downstreamcaps);
        goto end;
    }

    outcaps = gst_caps_new_empty();

    for(i = 0; i < gst_caps_get_size (downstreamcaps); i++) {
        GstStructure *structure = gst_caps_get_structure(downstreamcaps, i);
        const GValue *rate;
        const GValue *channels;
        GstStructure *output;

        rate = gst_structure_get_value(structure, "rate");
        channels = gst_structure_get_value(structure, "channels");

        output = gst_structure_new ("audio/x-raw-int", NULL);
        if(rate)
            gst_structure_set_value(output, "rate", rate);
        if(channels)
            gst_structure_set_value(output, "channels", channels);

        gst_caps_append_structure(outcaps, output);
    }

    caps = gst_caps_intersect(outcaps, gst_pad_get_pad_template_caps(sinkpad));
    gst_caps_unref(outcaps);

end:
    gst_caps_unref (downstreamcaps);
    gst_object_unref (parent);
    gst_object_unref (srcpad);
    return caps;
}

static void
gstcapstricks_set_video_getcaps(GstElement * element)
{
    GstPad *sinkpad = gst_element_get_static_pad (element, "sink");
    gst_pad_set_getcaps_function(sinkpad, gstcapstricks_video_getcaps);
    gst_object_unref (sinkpad);
}

static void
gstcapstricks_set_audio_getcaps(GstElement * element)
{
    GstPad *sinkpad = gst_element_get_static_pad (element, "sink");
    gst_pad_set_getcaps_function(sinkpad, gstcapstricks_audio_getcaps);
    gst_object_unref (sinkpad);
}

static void
qtmux_pad_added(GstElement * element, GstPad * pad, gpointer udata)
{
    if(is_video_pad (pad)) {
        qtmux_setcaps_functions_add(element, pad);
    }
}

static void
gstcapstricks_set_qtmux_setcaps(GstElement * element) {
    GHashTable *table = qtmux_setcaps_functions;

    if(g_hash_table_lookup(table, element)) {
        //this element is already at our hashtable
        return;
    }

    g_signal_connect(element, "pad-added", (GCallback) qtmux_pad_added, NULL);
    {
        GstIterator *iterator = gst_element_iterate_sink_pads(element);
        gboolean done = FALSE;
        GstPad *item;

        while (!done) {
            switch (gst_iterator_next (iterator, (gpointer) &item)) {
            case GST_ITERATOR_OK:
                if(is_video_pad(item)) {
                    qtmux_setcaps_functions_add(element, item);
                }
                gst_object_unref (item);
                break;
            case GST_ITERATOR_RESYNC:
                gst_iterator_resync (iterator);
                break;
            case GST_ITERATOR_ERROR:
                done = TRUE;
                break;
            case GST_ITERATOR_DONE:
                done = TRUE;
                break;
            }
        }

        gst_iterator_free (iterator);
    }
}

static gboolean
gstcapstricks_qtmux_setcaps(GstPad * sinkpad, GstCaps * caps)
{
    GstElement *element = gst_pad_get_parent_element(sinkpad);
    SetCapsData *data = g_hash_table_lookup(qtmux_setcaps_functions, element);
    GstStructure *newstructure;
    GstCaps *newcaps = NULL;
    gboolean ret;

    gst_object_unref (element);

    newstructure = gst_caps_get_structure(caps, 0);
    if(data->caps) {
        GstStructure *oldstructure = gst_caps_get_structure(data->caps, 0);
        const GValue *old_codec_data = gst_structure_get_value(oldstructure, "codec_data");
        const GValue *new_codec_data = gst_structure_get_value(newstructure, "codec_data");

        if(new_codec_data) {
            gst_caps_replace (&data->caps, caps);
            newcaps = gst_caps_ref (caps);
        } else {
            newcaps = gst_caps_copy (caps);
            gst_structure_set_value(gst_caps_get_structure(newcaps, 0), "codec_data", old_codec_data);
        }
    } else {
        if(gst_structure_has_field(newstructure, "codec_data")) {
            data->caps = gst_caps_ref (caps);
        }
        newcaps = gst_caps_ref (caps);
    }

    ret = data->setcaps(sinkpad, newcaps);
    gst_caps_unref (newcaps);
    return ret;
}

static void
gstcapstricks_add_get_caps(GstBin * bin, GstElement * element, gpointer udata)
{
    if(is_dspenc(element)) {
        //This should be a dsp encoder element, let's make it getcaps better
        gstcapstricks_set_video_getcaps(element);
    } else if(is_aacenc(element)) {
        gstcapstricks_set_audio_getcaps(element);
    } else if(is_qtmux(element)) {
        gstcapstricks_set_qtmux_setcaps(element);
    }
}

/**
  * Wait for encodebin to be added to look for muxers and encoders that are
  * added to it
  */
void
gstcapstricks_pipeline_element_added(GstBin * bin, GstElement * element, gpointer udata)
{
    if(is_encodebin(element)) {
        g_signal_connect(element, "element-added", (GCallback) gstcapstricks_add_get_caps, NULL);
        gstcapstricks_encodebin_find_elements(GST_BIN(element));
    } else if(is_playsink(element)) {
        gstcapstricks_set_playsink_sendevent(element);
    }
}

static
void gstcapstricks_encodebin_find_elements(GstBin * bin)
{
    GstIterator *iterator = gst_bin_iterate_elements(bin);
    GstElement *item;
    gboolean done = FALSE;

    while (!done) {
        switch (gst_iterator_next (iterator, (gpointer) &item)) {
        case GST_ITERATOR_OK:
            if(is_dspenc(item)) {
                gstcapstricks_set_video_getcaps(item);
            } else if(is_aacenc(item)) {
                gstcapstricks_set_audio_getcaps(item);
            } else if(is_qtmux(item)) {
                gstcapstricks_set_qtmux_setcaps(item);
            }
            gst_object_unref (item);
            break;
        case GST_ITERATOR_RESYNC:
            gst_iterator_resync (iterator);
            break;
        case GST_ITERATOR_ERROR:
            done = TRUE;
            break;
        case GST_ITERATOR_DONE:
            done = TRUE;
            break;
        }
    }

    gst_iterator_free (iterator);
}
