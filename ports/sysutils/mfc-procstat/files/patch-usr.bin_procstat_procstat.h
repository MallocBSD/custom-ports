--- usr.bin/procstat/procstat.h.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat.h
@@ -1,5 +1,6 @@
 /*-
  * Copyright (c) 2007 Robert N. M. Watson
+ * Copyright (c) 2015 Allan Jude <allanjude@freebsd.org>
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
@@ -23,12 +24,16 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat.h 267979 2014-06-27 20:34:22Z jhb $
+ * $FreeBSD: head/usr.bin/procstat/procstat.h 287486 2015-09-05 17:02:01Z allanjude $
  */
 
+#include <libxo/xo.h>
+
 #ifndef PROCSTAT_H
 #define	PROCSTAT_H
 
+#define PROCSTAT_XO_VERSION "1"
+
 extern int	hflag, nflag, Cflag, Hflag;
 
 struct kinfo_proc;
@@ -39,6 +44,7 @@ void	procstat_auxv(struct procstat *prst
 void	procstat_basic(struct kinfo_proc *kipp);
 void	procstat_bin(struct procstat *prstat, struct kinfo_proc *kipp);
 void	procstat_cred(struct procstat *prstat, struct kinfo_proc *kipp);
+void	procstat_cs(struct procstat *prstat, struct kinfo_proc *kipp);
 void	procstat_env(struct procstat *prstat, struct kinfo_proc *kipp);
 void	procstat_files(struct procstat *prstat, struct kinfo_proc *kipp);
 void	procstat_kstack(struct procstat *prstat, struct kinfo_proc *kipp,
