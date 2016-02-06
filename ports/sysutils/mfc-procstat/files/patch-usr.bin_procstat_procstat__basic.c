--- usr.bin/procstat/procstat_basic.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_basic.c
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
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat_basic.c 221807 2011-05-12 10:11:39Z stas $
+ * $FreeBSD: head/usr.bin/procstat/procstat_basic.c 287486 2015-09-05 17:02:01Z allanjude $
  */
 
 #include <sys/param.h>
@@ -42,24 +43,26 @@ procstat_basic(struct kinfo_proc *kipp)
 {
 
 	if (!hflag)
-		printf("%5s %5s %5s %5s %5s %3s %-8s %-9s %-13s %-12s\n",
+		xo_emit("{T:/%5s %5s %5s %5s %5s %3s %-8s %-9s %-13s %-12s}\n",
 		    "PID", "PPID", "PGID", "SID", "TSID", "THR", "LOGIN",
 		    "WCHAN", "EMUL", "COMM");
 
-	printf("%5d ", kipp->ki_pid);
-	printf("%5d ", kipp->ki_ppid);
-	printf("%5d ", kipp->ki_pgid);
-	printf("%5d ", kipp->ki_sid);
-	printf("%5d ", kipp->ki_tsid);
-	printf("%3d ", kipp->ki_numthreads);
-	printf("%-8s ", strlen(kipp->ki_login) ? kipp->ki_login : "-");
+	xo_emit("{k:process_id/%5d/%d} ", kipp->ki_pid);
+	xo_emit("{:parent_process_id/%5d/%d} ", kipp->ki_ppid);
+	xo_emit("{:process_group_id/%5d/%d} ", kipp->ki_pgid);
+	xo_emit("{:session_id/%5d/%d} ", kipp->ki_sid);
+	xo_emit("{:terminal_session_id/%5d/%d} ", kipp->ki_tsid);
+	xo_emit("{:threads/%3d/%d} ", kipp->ki_numthreads);
+	xo_emit("{:login/%-8s/%s} ", strlen(kipp->ki_login) ?
+	    kipp->ki_login : "-");
 	if (kipp->ki_kiflag & KI_LOCKBLOCK) {
-		printf("*%-8s ", strlen(kipp->ki_lockname) ?
+		xo_emit("{:lockname/*%-8s/%s} ", strlen(kipp->ki_lockname) ?
 		    kipp->ki_lockname : "-");
 	} else {
-		printf("%-9s ", strlen(kipp->ki_wmesg) ?
+		xo_emit("{:wait_channel/%-9s/%s} ", strlen(kipp->ki_wmesg) ?
 		    kipp->ki_wmesg : "-");
 	}
-	printf("%-13s ", strcmp(kipp->ki_emul, "null") ? kipp->ki_emul : "-");
-	printf("%-12s\n", kipp->ki_comm);
+	xo_emit("{:emulation/%-13s/%s} ", strcmp(kipp->ki_emul, "null") ?
+	    kipp->ki_emul : "-");
+	xo_emit("{:command/%-12s/%s}\n", kipp->ki_comm);
 }
