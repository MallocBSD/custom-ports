--- usr.bin/procstat/procstat_threads.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_threads.c
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
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat_threads.c 249668 2013-04-20 07:50:59Z trociny $
+ * $FreeBSD: head/usr.bin/procstat/procstat_threads.c 287486 2015-09-05 17:02:01Z allanjude $
  */
 
 #include <sys/param.h>
@@ -45,11 +46,17 @@ procstat_threads(struct procstat *procst
 	struct kinfo_proc *kip;
 	unsigned int count, i;
 	const char *str;
+	char *threadid;
 
 	if (!hflag)
-		printf("%5s %6s %-16s %-16s %2s %4s %-7s %-9s\n", "PID",
+		xo_emit("{T:/%5s %6s %-16s %-16s %2s %4s %-7s %-9s}\n", "PID",
 		    "TID", "COMM", "TDNAME", "CPU", "PRI", "STATE", "WCHAN");
 
+	xo_emit("{ek:process_id/%d}", kipp->ki_pid);
+	xo_emit("{e:command/%s}", strlen(kipp->ki_comm) ?
+		    kipp->ki_comm : "-");
+	xo_open_container("threads");
+
 	kip = procstat_getprocs(procstat, KERN_PROC_PID | KERN_PROC_INC_THREAD,
 	    kipp->ki_pid, &count);
 	if (kip == NULL)
@@ -57,20 +64,25 @@ procstat_threads(struct procstat *procst
 	kinfo_proc_sort(kip, count);
 	for (i = 0; i < count; i++) {
 		kipp = &kip[i];
-		printf("%5d ", kipp->ki_pid);
-		printf("%6d ", kipp->ki_tid);
-		printf("%-16s ", strlen(kipp->ki_comm) ?
+		asprintf(&threadid, "%d", kipp->ki_tid);
+		if (threadid == NULL)
+			xo_errc(1, ENOMEM, "Failed to allocate memory in "
+			    "procstat_threads()");
+		xo_open_container(threadid);
+		xo_emit("{dk:process_id/%5d/%d} ", kipp->ki_pid);
+		xo_emit("{:thread_id/%6d/%d} ", kipp->ki_tid);
+		xo_emit("{d:command/%-16s/%s} ", strlen(kipp->ki_comm) ?
 		    kipp->ki_comm : "-");
-		printf("%-16s ", (strlen(kipp->ki_tdname) &&
+		xo_emit("{:thread_name/%-16s/%s} ", (strlen(kipp->ki_tdname) &&
 		    (strcmp(kipp->ki_comm, kipp->ki_tdname) != 0)) ?
 		    kipp->ki_tdname : "-");
 		if (kipp->ki_oncpu != 255)
-			printf("%3d ", kipp->ki_oncpu);
+			xo_emit("{:cpu/%3d/%d} ", kipp->ki_oncpu);
 		else if (kipp->ki_lastcpu != 255)
-			printf("%3d ", kipp->ki_lastcpu);
+			xo_emit("{:cpu/%3d/%d} ", kipp->ki_lastcpu);
 		else
-			printf("%3s ", "-");
-		printf("%4d ", kipp->ki_pri.pri_level);
+			xo_emit("{:cpu/%3s/%s} ", "-");
+		xo_emit("{:priority/%4d/%d} ", kipp->ki_pri.pri_level);
 		switch (kipp->ki_stat) {
 		case SRUN:
 			str = "run";
@@ -104,15 +116,19 @@ procstat_threads(struct procstat *procst
 			str = "??";
 			break;
 		}
-		printf("%-7s ", str);
+		xo_emit("{:run_state/%-7s/%s} ", str);
 		if (kipp->ki_kiflag & KI_LOCKBLOCK) {
-			printf("*%-8s ", strlen(kipp->ki_lockname) ?
+			xo_emit("{:lock_name/*%-8s/%s} ",
+			    strlen(kipp->ki_lockname) ?
 			    kipp->ki_lockname : "-");
 		} else {
-			printf("%-9s ", strlen(kipp->ki_wmesg) ?
-			    kipp->ki_wmesg : "-");
+			xo_emit("{:wait_channel/%-9s/%s} ",
+			    strlen(kipp->ki_wmesg) ? kipp->ki_wmesg : "-");
 		}
-		printf("\n");
+		xo_close_container(threadid);
+		free(threadid);
+		xo_emit("\n");
 	}
+	xo_close_container("threads");
 	procstat_freeprocs(procstat, kip);
 }
