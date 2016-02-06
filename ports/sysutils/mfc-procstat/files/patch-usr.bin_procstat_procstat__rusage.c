--- usr.bin/procstat/procstat_rusage.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_rusage.c
@@ -1,6 +1,7 @@
 /*-
- * Copyright (c) 2012 Advanced Computing Technologies LLC
+ * Copyright (c) 2012 Hudson River Trading LLC
  * Written by: John H. Baldwin <jhb@FreeBSD.org>
+ * Copyright (c) 2015 Allan Jude <allanjude@freebsd.org>
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
@@ -26,7 +27,7 @@
  */
 
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/usr.bin/procstat/procstat_rusage.c 268108 2014-07-01 18:23:00Z jhb $");
+__FBSDID("$FreeBSD: head/usr.bin/procstat/procstat_rusage.c 287486 2015-09-05 17:02:01Z allanjude $");
 
 #include <sys/param.h>
 #include <sys/sysctl.h>
@@ -35,6 +36,7 @@ __FBSDID("$FreeBSD: releng/10.1/usr.bin/
 #include <libprocstat.h>
 #include <stdbool.h>
 #include <stdio.h>
+#include <stdlib.h>
 #include <libutil.h>
 
 #include "procstat.h"
@@ -79,7 +81,7 @@ format_time(struct timeval *tv)
 	else if (days > 0)
 		used += snprintf(buffer, sizeof(buffer), "%u days ", days);
 	
-	snprintf(buffer + used, sizeof(buffer) - used, "%02u:%02u:%02u.%06u   ",
+	snprintf(buffer + used, sizeof(buffer) - used, "%02u:%02u:%02u.%06u",
 	    hours, minutes, seconds, (unsigned int)tv->tv_usec);
 	return (buffer);
 }
@@ -103,10 +105,10 @@ static void
 print_prefix(struct kinfo_proc *kipp)
 {
 
-	printf("%5d ", kipp->ki_pid);
+	xo_emit("{d:process_id/%5d/%d} ", kipp->ki_pid);
 	if (Hflag)
-		printf("%6d ", kipp->ki_tid);
-	printf("%-16s ", kipp->ki_comm);
+		xo_emit("{d:thread_id/%6d/%d} ", kipp->ki_tid);
+	xo_emit("{d:command/%-16s/%s} ", kipp->ki_comm);
 }
 
 static void
@@ -114,21 +116,48 @@ print_rusage(struct kinfo_proc *kipp)
 {
 	long *lp;
 	unsigned int i;
+	char *field, *threadid;
 
 	print_prefix(kipp);
-	printf("%-14s %32s\n", "user time",
+	xo_emit("{d:resource/%-14s} {d:usage/%29s}{P:   }\n", "user time",
 	    format_time(&kipp->ki_rusage.ru_utime));
 	print_prefix(kipp);
-	printf("%-14s %32s\n", "system time",
+	xo_emit("{d:resource/%-14s} {d:usage/%29s}{P:   }\n", "system time",
 	    format_time(&kipp->ki_rusage.ru_stime));
+
+	if (Hflag) {
+		asprintf(&threadid, "%d", kipp->ki_tid);
+		if (threadid == NULL)
+			xo_errc(1, ENOMEM,
+			    "Failed to allocate memory in print_rusage()");
+		xo_open_container(threadid);
+		xo_emit("{e:thread_id/%d}", kipp->ki_tid);
+	} else {
+		xo_emit("{e:process_id/%d}", kipp->ki_pid);
+		xo_emit("{e:command/%s}", kipp->ki_comm);
+	}
+	xo_emit("{e:user time/%s}", format_time(&kipp->ki_rusage.ru_utime));
+	xo_emit("{e:system time/%s}", format_time(&kipp->ki_rusage.ru_stime));
+
 	lp = &kipp->ki_rusage.ru_maxrss;
 	for (i = 0; i < nitems(rusage_info); i++) {
 		print_prefix(kipp);
-		printf("%-32s %14s\n", rusage_info[i].ri_name,
+		asprintf(&field, "{e:%s/%%D}", rusage_info[i].ri_name);
+		if (field == NULL)
+			xo_errc(1, ENOMEM,
+			    "Failed to allocate memory in print_rusage()");
+		xo_emit(field, *lp);
+		free(field);
+		xo_emit("{d:resource/%-32s} {d:usage/%14s}\n",
+		    rusage_info[i].ri_name,
 		    format_value(*lp, rusage_info[i].ri_humanize,
-			rusage_info[i].ri_scale));
+		    rusage_info[i].ri_scale));
 		lp++;
 	}
+	if (Hflag) {
+		xo_close_container(threadid);
+		free(threadid);
+	}
 }
 
 void
@@ -138,10 +167,10 @@ procstat_rusage(struct procstat *procsta
 	unsigned int count, i;
 
 	if (!hflag) {
-		printf("%5s ", "PID");
+		xo_emit("{d:ta/%5s} ", "PID");
 		if (Hflag)
-			printf("%6s ", "TID");
-		printf("%-16s %-32s %14s\n", "COMM", "RESOURCE",
+			xo_emit("{d:tb/%6s} ", "TID");
+		xo_emit("{d:tc/%-16s %-32s %14s}\n", "COMM", "RESOURCE",
 		    "VALUE        ");
 	}
 
@@ -150,12 +179,19 @@ procstat_rusage(struct procstat *procsta
 		return;
 	}
 
+	xo_emit("{e:process_id/%d}", kipp->ki_pid);
+	xo_emit("{e:command/%s}", kipp->ki_comm);
+	xo_open_container("threads");
+
 	kip = procstat_getprocs(procstat, KERN_PROC_PID | KERN_PROC_INC_THREAD,
 	    kipp->ki_pid, &count);
 	if (kip == NULL)
 		return;
 	kinfo_proc_sort(kip, count);
-	for (i = 0; i < count; i++)
+	for (i = 0; i < count; i++) {
 		print_rusage(&kip[i]);
+	}
+
+	xo_close_container("threads");
 	procstat_freeprocs(procstat, kip);
 }
