--- usr.bin/procstat/procstat.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat.c
@@ -1,5 +1,6 @@
 /*-
  * Copyright (c) 2007, 2011 Robert N. M. Watson
+ * Copyright (c) 2015 Allan Jude <allanjude@freebsd.org>
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
@@ -23,7 +24,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat.c 267979 2014-06-27 20:34:22Z jhb $
+ * $FreeBSD: head/usr.bin/procstat/procstat.c 287486 2015-09-05 17:02:01Z allanjude $
  */
 
 #include <sys/param.h>
@@ -40,24 +41,31 @@
 #include "procstat.h"
 
 static int aflag, bflag, cflag, eflag, fflag, iflag, jflag, kflag, lflag, rflag;
-static int sflag, tflag, vflag, xflag;
+static int sflag, tflag, vflag, xflag, Sflag;
 int	hflag, nflag, Cflag, Hflag;
 
 static void
 usage(void)
 {
 
-	fprintf(stderr, "usage: procstat [-CHhn] [-M core] [-N system] "
-	    "[-w interval] \n");
-	fprintf(stderr, "                [-b | -c | -e | -f | -i | -j | -k | "
-	    "-l | -r | -s | -t | -v | -x]\n");
-	fprintf(stderr, "                [-a | pid | core ...]\n");
+	xo_error("usage: procstat [-CHhn] [-M core] [-N system] "
+	    "[-w interval]\n"
+	    "                [-b | -c | -e | -f | -i | -j | -k | "
+	    "-l | -r | -s | -S | -t | -v | -x]\n"
+	    "                [-a | pid | core ...]\n");
+	xo_finish();
 	exit(EX_USAGE);
 }
 
 static void
 procstat(struct procstat *prstat, struct kinfo_proc *kipp)
 {
+	char *pidstr = NULL;
+
+	asprintf(&pidstr, "%d", kipp->ki_pid);
+	if (pidstr == NULL)
+		xo_errc(1, ENOMEM, "Failed to allocate memory in procstat()");
+	xo_open_container(pidstr);
 
 	if (bflag)
 		procstat_bin(prstat, kipp);
@@ -87,6 +95,9 @@ procstat(struct procstat *prstat, struct
 		procstat_auxv(prstat, kipp);
 	else
 		procstat_basic(kipp);
+
+	xo_close_container(pidstr);
+	free(pidstr);
 }
 
 /*
@@ -124,11 +135,15 @@ main(int argc, char *argv[])
 	pid_t pid;
 	char *dummy;
 	char *nlistf, *memf;
+	const char *xocontainer;
 	int cnt;
 
 	interval = 0;
 	memf = nlistf = NULL;
-	while ((ch = getopt(argc, argv, "CHN:M:abcefijklhrstvw:x")) != -1) {
+	argc = xo_parse_args(argc, argv);
+	xocontainer = "basic";
+
+	while ((ch = getopt(argc, argv, "CHN:M:abcefijklhrsStvw:x")) != -1) {
 		switch (ch) {
 		case 'C':
 			Cflag++;
@@ -144,40 +159,52 @@ main(int argc, char *argv[])
 		case 'N':
 			nlistf = optarg;
 			break;
+		case 'S':
+			Sflag++;
+			xocontainer = "cs";
+			break;
 		case 'a':
 			aflag++;
 			break;
 
 		case 'b':
 			bflag++;
+			xocontainer = "binary";
 			break;
 
 		case 'c':
 			cflag++;
+			xocontainer = "arguments";
 			break;
 
 		case 'e':
 			eflag++;
+			xocontainer = "environment";
 			break;
 
 		case 'f':
 			fflag++;
+			xocontainer = "files";
 			break;
 
 		case 'i':
 			iflag++;
+			xocontainer = "signals";
 			break;
 
 		case 'j':
 			jflag++;
+			xocontainer = "thread_signals";
 			break;
 
 		case 'k':
 			kflag++;
+			xocontainer = "kstack";
 			break;
 
 		case 'l':
 			lflag++;
+			xocontainer = "rlimit";
 			break;
 
 		case 'n':
@@ -190,18 +217,22 @@ main(int argc, char *argv[])
 
 		case 'r':
 			rflag++;
+			xocontainer = "rusage";
 			break;
 
 		case 's':
 			sflag++;
+			xocontainer = "credentials";
 			break;
 
 		case 't':
 			tflag++;
+			xocontainer = "threads";
 			break;
 
 		case 'v':
 			vflag++;
+			xocontainer = "vm";
 			break;
 
 		case 'w':
@@ -215,6 +246,7 @@ main(int argc, char *argv[])
 
 		case 'x':
 			xflag++;
+			xocontainer = "auxv";
 			break;
 
 		case '?':
@@ -228,7 +260,7 @@ main(int argc, char *argv[])
 
 	/* We require that either 0 or 1 mode flags be set. */
 	tmp = bflag + cflag + eflag + fflag + iflag + jflag + (kflag ? 1 : 0) +
-	    lflag + rflag + sflag + tflag + vflag + xflag;
+	    lflag + rflag + sflag + tflag + vflag + xflag + Sflag;
 	if (!(tmp == 0 || tmp == 1))
 		usage();
 
@@ -249,18 +281,23 @@ main(int argc, char *argv[])
 	else
 		prstat = procstat_open_sysctl();
 	if (prstat == NULL)
-		errx(1, "procstat_open()");
+		xo_errx(1, "procstat_open()");
 	do {
+		xo_set_version(PROCSTAT_XO_VERSION);
+		xo_open_container("procstat");
+		xo_open_container(xocontainer);
+
 		if (aflag) {
 			p = procstat_getprocs(prstat, KERN_PROC_PROC, 0, &cnt);
 			if (p == NULL)
-				errx(1, "procstat_getprocs()");
+				xo_errx(1, "procstat_getprocs()");
 			kinfo_proc_sort(p, cnt);
 			for (i = 0; i < cnt; i++) {
 				procstat(prstat, &p[i]);
 
 				/* Suppress header after first process. */
 				hflag = 1;
+				xo_flush();
 			}
 			procstat_freeprocs(prstat, p);
 		}
@@ -271,9 +308,10 @@ main(int argc, char *argv[])
 					usage();
 				pid = l;
 
-				p = procstat_getprocs(prstat, KERN_PROC_PID, pid, &cnt);
+				p = procstat_getprocs(prstat, KERN_PROC_PID,
+				    pid, &cnt);
 				if (p == NULL)
-					errx(1, "procstat_getprocs()");
+					xo_errx(1, "procstat_getprocs()");
 				if (cnt != 0)
 					procstat(prstat, p);
 				procstat_freeprocs(prstat, p);
@@ -286,7 +324,7 @@ main(int argc, char *argv[])
 				p = procstat_getprocs(cprstat, KERN_PROC_PID,
 				    -1, &cnt);
 				if (p == NULL)
-					errx(1, "procstat_getprocs()");
+					xo_errx(1, "procstat_getprocs()");
 				if (cnt != 0)
 					procstat(cprstat, p);
 				procstat_freeprocs(cprstat, p);
@@ -295,9 +333,15 @@ main(int argc, char *argv[])
 			/* Suppress header after first process. */
 			hflag = 1;
 		}
+
+		xo_close_container(xocontainer);
+		xo_close_container("procstat");
+		xo_finish();
 		if (interval)
 			sleep(interval);
 	} while (interval);
+
 	procstat_close(prstat);
+
 	exit(0);
 }
