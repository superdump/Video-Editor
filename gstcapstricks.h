#ifndef GSTCAPSTRICKS_H
#define GSTCAPSTRICKS_H

#include <gst/gst.h>

void gstcapstricks_init(void);
void gstcapstricks_pipeline_encodebin_added(GstBin * bin, GstElement * element, gpointer udata);

#endif // GSTCAPSTRICKS_H
