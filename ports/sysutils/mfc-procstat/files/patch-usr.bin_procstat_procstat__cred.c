--- usr.bin/procstat/procstat_cred.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_cred.c
@@ -1,5 +1,6 @@
 /*-
  * Copyright (c) 2007-2008 Robert N. M. Watson
+ * Copyright (c) 2015 Allan Jude <allanjude@freebsd.org>
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
@@ -23,7 +24,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat_cred.c 249673 2013-04-20 07:58:20Z trociny $
+ * $FreeBSD: head/usr.bin/procstat/procstat_cred.c 287486 2015-09-05 17:02:01Z allanjude $
  */
 
 #include <sys/param.h>
@@ -48,21 +49,22 @@ procstat_cred(struct procstat *procstat,
 	gid_t *groups;
 
 	if (!hflag)
-		printf("%5s %-16s %5s %5s %5s %5s %5s %5s %5s %5s %-15s\n",
+		xo_emit("{T:/%5s %-16s %5s %5s %5s %5s %5s %5s %5s %5s %-15s}\n",
 		    "PID", "COMM", "EUID", "RUID", "SVUID", "EGID", "RGID",
 		    "SVGID", "UMASK", "FLAGS", "GROUPS");
 
-	printf("%5d ", kipp->ki_pid);
-	printf("%-16s ", kipp->ki_comm);
-	printf("%5d ", kipp->ki_uid);
-	printf("%5d ", kipp->ki_ruid);
-	printf("%5d ", kipp->ki_svuid);
-	printf("%5d ", kipp->ki_groups[0]);
-	printf("%5d ", kipp->ki_rgid);
-	printf("%5d ", kipp->ki_svgid);
-	printf("%5s ", get_umask(procstat, kipp));
-	printf("%s", kipp->ki_cr_flags & CRED_FLAG_CAPMODE ? "C" : "-");
-	printf("     ");
+	xo_emit("{k:process_id/%5d/%d} ", kipp->ki_pid);
+	xo_emit("{:command/%-16s/%s} ", kipp->ki_comm);
+	xo_emit("{:uid/%5d} ", kipp->ki_uid);
+	xo_emit("{:ruid/%5d} ", kipp->ki_ruid);
+	xo_emit("{:svuid/%5d} ", kipp->ki_svuid);
+	xo_emit("{:group/%5d} ", kipp->ki_groups[0]);
+	xo_emit("{:rgid/%5d} ", kipp->ki_rgid);
+	xo_emit("{:svgid/%5d} ", kipp->ki_svgid);
+	xo_emit("{:umask/%5s} ", get_umask(procstat, kipp));
+	xo_emit("{:cr_flags/%s}", kipp->ki_cr_flags & CRED_FLAG_CAPMODE ?
+	    "C" : "-");
+	xo_emit("{P:     }");
 
 	groups = NULL;
 	/*
@@ -76,12 +78,14 @@ procstat_cred(struct procstat *procstat,
 		ngroups = kipp->ki_ngroups;
 		groups = kipp->ki_groups;
 	}
+	xo_open_list("groups");
 	for (i = 0; i < ngroups; i++)
-		printf("%s%d", (i > 0) ? "," : "", groups[i]);
+		xo_emit("{D:/%s}{l:groups/%d}", (i > 0) ? "," : "", groups[i]);
 	if (groups != kipp->ki_groups)
 		procstat_freegroups(procstat, groups);
 
-	printf("\n");
+	xo_close_list("groups");
+	xo_emit("\n");
 }
 
 static const char *
