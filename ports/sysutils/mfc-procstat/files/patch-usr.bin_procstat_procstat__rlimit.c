--- usr.bin/procstat/procstat_rlimit.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_rlimit.c
@@ -1,5 +1,6 @@
 /*-
  * Copyright (c) 2011 Mikolaj Golub
+ * Copyright (c) 2015 Allan Jude <allanjude@freebsd.org>
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
@@ -23,7 +24,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat_rlimit.c 249675 2013-04-20 08:01:00Z trociny $
+ * $FreeBSD: head/usr.bin/procstat/procstat_rlimit.c 287486 2015-09-05 17:02:01Z allanjude $
  */
 
 #include <sys/param.h>
@@ -46,7 +47,7 @@
 static struct {
 	const char *name;
 	const char *suffix;
-} rlimit_param[13] = {
+} rlimit_param[14] = {
 	{"cputime",          "sec"},
 	{"filesize",         "B  "},
 	{"datasize",         "B  "},
@@ -60,9 +61,10 @@ static struct {
 	{"vmemoryuse",       "B  "},
 	{"pseudo-terminals", "   "},
 	{"swapuse",          "B  "},
+	{"kqueues",          "   "},
 };
 
-#if RLIM_NLIMITS > 13
+#if RLIM_NLIMITS > 14
 #error "Resource limits have grown. Add new entries to rlimit_param[]."
 #endif
 
@@ -92,15 +94,33 @@ procstat_rlimit(struct procstat *prstat,
 	int i;
 
 	if (!hflag) {
-		printf("%5s %-16s %-16s %16s %16s\n",
+		xo_emit("{T:/%5s %-16s %-16s %16s %16s}\n",
 		    "PID", "COMM", "RLIMIT", "SOFT     ", "HARD     ");
 	}
+	xo_emit("{ek:process_id/%5d}{e:command/%-16s/%s}", kipp->ki_pid,
+	    kipp->ki_comm);
 	for (i = 0; i < RLIM_NLIMITS; i++) {
 		if (procstat_getrlimit(prstat, kipp, i, &rlimit) == -1)
 			return;
-		printf("%5d %-16s %-16s ", kipp->ki_pid, kipp->ki_comm,
+		xo_emit("{dk:process_id/%5d} {d:command/%-16s} "
+		    "{d:rlimit_param/%-16s} ", kipp->ki_pid, kipp->ki_comm,
 		    rlimit_param[i].name);
-		printf("%16s ", humanize_rlimit(i, rlimit.rlim_cur));
-		printf("%16s\n", humanize_rlimit(i, rlimit.rlim_max));
+
+		xo_open_container(rlimit_param[i].name);
+		if (rlimit.rlim_cur == RLIM_INFINITY)
+			xo_emit("{e:soft_limit/infinity}");
+		else
+			xo_emit("{e:soft_limit/%U}", rlimit.rlim_cur);
+
+		if (rlimit.rlim_max == RLIM_INFINITY)
+			xo_emit("{e:hard_limit/infinity}");
+		else
+			xo_emit("{e:hard_limit/%U}", rlimit.rlim_max);
+		xo_close_container(rlimit_param[i].name);
+
+		xo_emit("{d:rlim_cur/%16s} ",
+		    humanize_rlimit(i, rlimit.rlim_cur));
+		xo_emit("{d:rlim_max/%16s}\n",
+		    humanize_rlimit(i, rlimit.rlim_max));
 	}
 }
