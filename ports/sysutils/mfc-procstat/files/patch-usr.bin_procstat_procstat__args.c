--- usr.bin/procstat/procstat_args.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_args.c
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
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat_args.c 249680 2013-04-20 08:08:29Z trociny $
+ * $FreeBSD: head/usr.bin/procstat/procstat_args.c 287486 2015-09-05 17:02:01Z allanjude $
  */
 
 #include <sys/param.h>
@@ -40,40 +41,56 @@
 
 #include "procstat.h"
 
-static void
-do_args(struct procstat *procstat, struct kinfo_proc *kipp, int env)
+void
+procstat_args(struct procstat *procstat, struct kinfo_proc *kipp)
 {
 	int i;
 	char **args;
 
 	if (!hflag) {
-		printf("%5s %-16s %-53s\n", "PID", "COMM",
-		    env ? "ENVIRONMENT" : "ARGS");
+		xo_emit("{T:/%5s %-16s %-53s}\n", "PID", "COMM", "ARGS");
 	}
 
-	args = env ? procstat_getenvv(procstat, kipp, 0) :
-	    procstat_getargv(procstat, kipp, 0);
+	args = procstat_getargv(procstat, kipp, 0);
 
-	printf("%5d %-16s", kipp->ki_pid, kipp->ki_comm);
+	xo_emit("{k:process_id/%5d/%d} {:command/%-16s/%s}", kipp->ki_pid,
+	    kipp->ki_comm);
 
 	if (args == NULL) {
-		printf(" -\n");
+		xo_emit(" {d:args/-}\n");
 		return;
 	}
 
+	xo_open_list("arguments");
 	for (i = 0; args[i] != NULL; i++)
-		printf(" %s", args[i]);
-	printf("\n");
-}
-
-void
-procstat_args(struct procstat *procstat, struct kinfo_proc *kipp)
-{
-	do_args(procstat, kipp, 0);
+		xo_emit(" {l:args/%s}", args[i]);
+	xo_close_list("arguments");
+	xo_emit("\n");
 }
 
 void
 procstat_env(struct procstat *procstat, struct kinfo_proc *kipp)
 {
-	do_args(procstat, kipp, 1);
+	int i;
+	char **envs;
+
+	if (!hflag) {
+		xo_emit("{T:/%5s %-16s %-53s}\n", "PID", "COMM", "ENVIRONMENT");
+	}
+
+	envs = procstat_getenvv(procstat, kipp, 0);
+
+	xo_emit("{k:process_id/%5d/%d} {:command/%-16s/%s}", kipp->ki_pid,
+	    kipp->ki_comm);
+
+	if (envs == NULL) {
+		xo_emit(" {d:env/-}\n");
+		return;
+	}
+
+	xo_open_list("environment");
+	for (i = 0; envs[i] != NULL; i++)
+		xo_emit(" {l:env/%s}", envs[i]);
+	xo_close_list("environment");
+	xo_emit("\n");
 }
