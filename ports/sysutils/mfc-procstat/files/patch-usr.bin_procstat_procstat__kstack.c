--- usr.bin/procstat/procstat_kstack.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_kstack.c
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
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat_kstack.c 249685 2013-04-20 08:19:06Z trociny $
+ * $FreeBSD: head/usr.bin/procstat/procstat_kstack.c 287491 2015-09-05 19:28:41Z allanjude $
  */
 
 #include <sys/param.h>
@@ -105,6 +106,42 @@ kstack_cleanup(const char *old, char *ne
 	*cp_new = '\0';
 }
 
+static void
+kstack_cleanup_encoded(const char *old, char *new, int kflag)
+{
+	enum trace_state old_ts, ts;
+	const char *cp_old;
+	char *cp_new, *cp_loop, *cp_tofree, *cp_line;
+
+	ts = TS_FRAMENUM;
+	if (kflag == 1) {
+		for (cp_old = old, cp_new = new; *cp_old != '\0'; cp_old++) {
+			switch (*cp_old) {
+			case '\n':
+				*cp_new = *cp_old;
+				cp_new++;
+			case ' ':
+			case '+':
+				old_ts = ts;
+				ts = kstack_nextstate(old_ts);
+				continue;
+			}
+			if (ts == TS_FUNC) {
+				*cp_new = *cp_old;
+				cp_new++;
+			}
+		}
+		*cp_new = '\0';
+		cp_tofree = cp_loop = strdup(new);
+	} else
+		cp_tofree = cp_loop = strdup(old);
+        while ((cp_line = strsep(&cp_loop, "\n")) != NULL) {
+		if (strlen(cp_line) != 0 && *cp_line != 127)
+			xo_emit("{le:token/%s}", cp_line);
+	}
+	free(cp_tofree);
+}
+
 /*
  * Sort threads by tid.
  */
@@ -129,12 +166,12 @@ procstat_kstack(struct procstat *procsta
 {
 	struct kinfo_kstack *kkstp, *kkstp_free;
 	struct kinfo_proc *kip, *kip_free;
-	char trace[KKST_MAXLEN];
+	char trace[KKST_MAXLEN], encoded_trace[KKST_MAXLEN];
 	unsigned int i, j;
 	unsigned int kip_count, kstk_count;
 
 	if (!hflag)
-		printf("%5s %6s %-16s %-16s %-29s\n", "PID", "TID", "COMM",
+		xo_emit("{T:/%5s %6s %-16s %-16s %-29s}\n", "PID", "TID", "COMM",
 		    "TDNAME", "KSTACK");
 
 	kkstp = kkstp_free = procstat_getkstack(procstat, kipp, &kstk_count);
@@ -169,27 +206,27 @@ procstat_kstack(struct procstat *procsta
 		if (kipp == NULL)
 			continue;
 
-		printf("%5d ", kipp->ki_pid);
-		printf("%6d ", kkstp->kkst_tid);
-		printf("%-16s ", kipp->ki_comm);
-		printf("%-16s ", (strlen(kipp->ki_tdname) &&
+		xo_emit("{k:process_id/%5d/%d} ", kipp->ki_pid);
+		xo_emit("{:thread_id/%6d/%d} ", kkstp->kkst_tid);
+		xo_emit("{:command/%-16s/%s} ", kipp->ki_comm);
+		xo_emit("{:thread_name/%-16s/%s} ", (strlen(kipp->ki_tdname) &&
 		    (strcmp(kipp->ki_comm, kipp->ki_tdname) != 0)) ?
 		    kipp->ki_tdname : "-");
 
 		switch (kkstp->kkst_state) {
 		case KKST_STATE_RUNNING:
-			printf("%-29s\n", "<running>");
+			xo_emit("{:state/%-29s/%s}\n", "<running>");
 			continue;
 
 		case KKST_STATE_SWAPPED:
-			printf("%-29s\n", "<swapped>");
+			xo_emit("{:state/%-29s/%s}\n", "<swapped>");
 			continue;
 
 		case KKST_STATE_STACKOK:
 			break;
 
 		default:
-			printf("%-29s\n", "<unknown>");
+			xo_emit("{:state/%-29s/%s}\n", "<unknown>");
 			continue;
 		}
 
@@ -199,7 +236,10 @@ procstat_kstack(struct procstat *procsta
 		 * returns to spaces.
 		 */
 		kstack_cleanup(kkstp->kkst_trace, trace, kflag);
-		printf("%-29s\n", trace);
+		xo_open_list("trace");
+		kstack_cleanup_encoded(kkstp->kkst_trace, encoded_trace, kflag);
+		xo_close_list("trace");
+		xo_emit("{d:trace/%-29s}\n", trace);
 	}
 	procstat_freekstack(procstat, kkstp_free);
 	procstat_freeprocs(procstat, kip_free);
