#ifndef GSTDSPGETCAPS_H
#define GSTDSPGETCAPS_H

#include <gst/gst.h>

void gstdspgetcaps_pipeline_encodebin_added(GstBin * bin, GstElement * element, gpointer udata);
void gstdspgetcaps_bin_find_encodebin(GstBin * bin);

#endif // GSTDSPGETCAPS_H
