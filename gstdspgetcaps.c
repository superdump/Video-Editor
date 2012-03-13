
#include "gstdspgetcaps.h"

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

static GstCaps *
gstdspgetcaps_get_caps(GstPad * sinkpad)
{
    GstElement *parent = gst_pad_get_parent_element (sinkpad);
    GstPad *srcpad = gst_element_get_static_pad (parent, "src");
    GstCaps *caps;
    GstCaps *outcaps;
    GstCaps *downstreamcaps;
    gint i;

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

static void
gstdspgetcaps_set_get_caps(GstElement * element)
{
    GstPad *sinkpad = gst_element_get_static_pad (element, "sink");
    gst_pad_set_getcaps_function(sinkpad, gstdspgetcaps_get_caps);
    gst_object_unref (sinkpad);
}

static void
gstdspgetcaps_add_get_caps(GstBin * bin, GstElement * element, gpointer udata)
{
    if(is_dspenc(element)) {
        //This should be a dsp encoder element, let's make it getcaps better
        gstdspgetcaps_set_get_caps(element);
    }
}

void
gstdspgetcaps_pipeline_encodebin_added(GstBin * bin, GstElement * element, gpointer udata)
{
    if(is_encodebin(element)) {
        g_signal_connect(element, "element-added", (GCallback) gstdspgetcaps_add_get_caps, NULL);
        gstdspgetcaps_bin_find_dspenc(GST_BIN(element));
    }
}

void gstdspgetcaps_bin_find_dspenc(GstBin * bin)
{
    GstIterator *iterator = gst_bin_iterate_elements(bin);
    GstElement *item;
    gboolean done = FALSE;

    while (!done) {
        switch (gst_iterator_next (iterator, (gpointer) &item)) {
        case GST_ITERATOR_OK:
            if(is_dspenc(item)) {
                gstdspgetcaps_set_get_caps(item);
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
