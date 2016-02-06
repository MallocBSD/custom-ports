--- usr.bin/iscsictl/periphs.c.orig	2015-12-29 02:29:49 UTC
+++ usr.bin/iscsictl/periphs.c
@@ -31,7 +31,7 @@
  */
 
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/usr.bin/iscsictl/periphs.c 265532 2014-05-07 08:04:41Z trasz $");
+__FBSDID("$FreeBSD: head/usr.bin/iscsictl/periphs.c 281461 2015-04-12 09:36:50Z trasz $");
 
 #include <sys/ioctl.h>
 #include <sys/stdint.h>
@@ -46,7 +46,6 @@ __FBSDID("$FreeBSD: releng/10.1/usr.bin/
 #include <inttypes.h>
 #include <limits.h>
 #include <fcntl.h>
-#include <err.h>
 
 #include <cam/cam.h>
 #include <cam/cam_debug.h>
@@ -58,6 +57,7 @@ __FBSDID("$FreeBSD: releng/10.1/usr.bin/
 #include <cam/scsi/smp_all.h>
 #include <cam/ata/ata_all.h>
 #include <camlib.h>
+#include <libxo/xo.h>
 
 #include "iscsictl.h"
 
@@ -67,10 +67,11 @@ print_periphs(int session_id)
 	union ccb ccb;
 	int bufsize, fd;
 	unsigned int i;
-	int skip_bus, skip_device;
+	int have_path_id, skip_bus, skip_device;
+	path_id_t path_id;
 
 	if ((fd = open(XPT_DEVICE, O_RDWR)) == -1) {
-		warn("couldn't open %s", XPT_DEVICE);
+		xo_warn("couldn't open %s", XPT_DEVICE);
 		return;
 	}
 
@@ -89,7 +90,7 @@ print_periphs(int session_id)
 	ccb.cdm.match_buf_len = bufsize;
 	ccb.cdm.matches = (struct dev_match_result *)malloc(bufsize);
 	if (ccb.cdm.matches == NULL) {
-		warnx("can't malloc memory for matches");
+		xo_warnx("can't malloc memory for matches");
 		close(fd);
 		return;
 	}
@@ -102,23 +103,27 @@ print_periphs(int session_id)
 	ccb.cdm.num_patterns = 0;
 	ccb.cdm.pattern_buf_len = 0;
 
+	path_id = -1; /* Make GCC happy. */
+	have_path_id = 0;
 	skip_bus = 1;
 	skip_device = 1;
 
+	xo_open_container("devices");
+	xo_open_list("lun");
 	/*
 	 * We do the ioctl multiple times if necessary, in case there are
 	 * more than 100 nodes in the EDT.
 	 */
 	do {
 		if (ioctl(fd, CAMIOCOMMAND, &ccb) == -1) {
-			warn("error sending CAMIOCOMMAND ioctl");
+			xo_warn("error sending CAMIOCOMMAND ioctl");
 			break;
 		}
 
 		if ((ccb.ccb_h.status != CAM_REQ_CMP)
 		 || ((ccb.cdm.status != CAM_DEV_MATCH_LAST)
 		    && (ccb.cdm.status != CAM_DEV_MATCH_MORE))) {
-			warnx("got CAM error %#x, CDM error %d\n",
+			xo_warnx("got CAM error %#x, CDM error %d\n",
 			      ccb.ccb_h.status, ccb.cdm.status);
 			break;
 		}
@@ -141,7 +146,6 @@ print_periphs(int session_id)
 					//printf("wrong unit, %d != %d\n", bus_result->unit_number, session_id);
 					continue;
 				}
-
 				skip_bus = 0;
 			}
 			case DEV_MATCH_DEVICE: {
@@ -151,7 +155,6 @@ print_periphs(int session_id)
 					continue;
 
 				skip_device = 0;
-
 				break;
 			}
 			case DEV_MATCH_PERIPH: {
@@ -166,9 +169,17 @@ print_periphs(int session_id)
 				if (strcmp(periph_result->periph_name, "pass") == 0)
 					continue;
 
-				fprintf(stdout, "%s%d ",
-					periph_result->periph_name,
-					periph_result->unit_number);
+				xo_open_instance("lun");
+				xo_emit("{e:lun/%d}", periph_result->target_lun);
+				xo_emit("{Vq:device/%s%d} ",
+				    periph_result->periph_name,
+				    periph_result->unit_number);
+				xo_close_instance("lun");
+
+				if (have_path_id == 0) {
+					path_id = periph_result->path_id;
+					have_path_id = 1;
+				}
 
 				break;
 			}
@@ -180,6 +191,11 @@ print_periphs(int session_id)
 
 	} while ((ccb.ccb_h.status == CAM_REQ_CMP)
 		&& (ccb.cdm.status == CAM_DEV_MATCH_MORE));
+	xo_close_list("lun");
+
+	xo_emit("{e:scbus/%d}{e:bus/%d}{e:target/%d}",
+		have_path_id ? (int)path_id : -1, 0, have_path_id ? 0 : -1);
+	xo_close_container("devices");
 
 	close(fd);
 }
