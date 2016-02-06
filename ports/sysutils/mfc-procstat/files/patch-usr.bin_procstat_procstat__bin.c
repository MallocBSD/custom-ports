--- usr.bin/procstat/procstat_bin.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_bin.c
@@ -1,5 +1,6 @@
 /*-
  * Copyright (c) 2007 Robert N. M. Watson
+ * Copyright (c) 2015 Allan Jude <allanjude@freebsd.org>
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
@@ -23,7 +24,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat_bin.c 249678 2013-04-20 08:05:04Z trociny $
+ * $FreeBSD: head/usr.bin/procstat/procstat_bin.c 287486 2015-09-05 17:02:01Z allanjude $
  */
 
 #include <sys/param.h>
@@ -46,7 +47,8 @@ procstat_bin(struct procstat *prstat, st
 	static char pathname[PATH_MAX];
 
 	if (!hflag)
-		printf("%5s %-16s %8s %s\n", "PID", "COMM", "OSREL", "PATH");
+		xo_emit("{T:/%5s %-16s %8s %s}\n", "PID", "COMM", "OSREL",
+		    "PATH");
 
 	if (procstat_getpathname(prstat, kipp, pathname, sizeof(pathname)) != 0)
 		return;
@@ -55,8 +57,8 @@ procstat_bin(struct procstat *prstat, st
 	if (procstat_getosrel(prstat, kipp, &osrel) != 0)
 		return;
 
-	printf("%5d ", kipp->ki_pid);
-	printf("%-16s ", kipp->ki_comm);
-	printf("%8d ", osrel);
-	printf("%s\n", pathname);
+	xo_emit("{k:process_id/%5d/%d} ", kipp->ki_pid);
+	xo_emit("{:command/%-16s/%s} ", kipp->ki_comm);
+	xo_emit("{:osrel/%8d/%d} ", osrel);
+	xo_emit("{:pathname/%s}\n", pathname);
 }
