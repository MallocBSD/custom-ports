--- bin/df/df.c.orig	2015-12-29 08:07:23 UTC
+++ bin/df/df.c
@@ -44,7 +44,7 @@ static char sccsid[] = "@(#)df.c	8.9 (Be
 #endif /* not lint */
 #endif
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/bin/df/df.c 249924 2013-04-26 12:27:30Z uqs $");
+__FBSDID("$FreeBSD: head/bin/df/df.c 287236 2015-08-28 00:44:58Z delphij $");
 
 #include <sys/param.h>
 #include <sys/stat.h>
@@ -60,6 +60,7 @@ __FBSDID("$FreeBSD: releng/10.1/bin/df/d
 #include <string.h>
 #include <sysexits.h>
 #include <unistd.h>
+#include <libxo/xo.h>
 
 #include "extern.h"
 
@@ -82,7 +83,7 @@ static char	 *getmntpt(const char *);
 static int	  int64width(int64_t);
 static char	 *makenetvfslist(void);
 static void	  prthuman(const struct statfs *, int64_t);
-static void	  prthumanval(int64_t);
+static void	  prthumanval(const char *, int64_t);
 static intmax_t	  fsbtoblk(int64_t, uint64_t, u_long);
 static void	  prtstat(struct statfs *, struct maxwidths *);
 static size_t	  regetmntinfo(struct statfs **, long, const char **);
@@ -119,6 +120,11 @@ main(int argc, char *argv[])
 	totalbuf.f_bsize = DEV_BSIZE;
 	strlcpy(totalbuf.f_mntfromname, "total", MNAMELEN);
 	vfslist = NULL;
+
+	argc = xo_parse_args(argc, argv);
+	if (argc < 0)
+		exit(1);
+
 	while ((ch = getopt(argc, argv, "abcgHhiklmnPt:T,")) != -1)
 		switch (ch) {
 		case 'a':
@@ -161,7 +167,7 @@ main(int argc, char *argv[])
 			break;
 		case 'l':
 			if (vfslist != NULL)
-				errx(1, "-l and -t are mutually exclusive.");
+				xo_errx(1, "-l and -t are mutually exclusive.");
 			vfslist = makevfslist(makenetvfslist());
 			lflag = 1;
 			break;
@@ -174,9 +180,9 @@ main(int argc, char *argv[])
 			break;
 		case 't':
 			if (lflag)
-				errx(1, "-l and -t are mutually exclusive.");
+				xo_errx(1, "-l and -t are mutually exclusive.");
 			if (vfslist != NULL)
-				errx(1, "only one -t option may be specified");
+				xo_errx(1, "only one -t option may be specified");
 			fstype = optarg;
 			vfslist = makevfslist(optarg);
 			break;
@@ -202,16 +208,19 @@ main(int argc, char *argv[])
 		/* just the filesystems specified on the command line */
 		mntbuf = malloc(argc * sizeof(*mntbuf));
 		if (mntbuf == NULL)
-			err(1, "malloc()");
+			xo_err(1, "malloc()");
 		mntsize = 0;
 		/* continued in for loop below */
 	}
 
+	xo_open_container("storage-system-information");
+	xo_open_list("filesystem");
+
 	/* iterate through specified filesystems */
 	for (; *argv; argv++) {
 		if (stat(*argv, &stbuf) < 0) {
 			if ((mntpt = getmntpt(*argv)) == NULL) {
-				warn("%s", *argv);
+				xo_warn("%s", *argv);
 				rv = 1;
 				continue;
 			}
@@ -220,20 +229,20 @@ main(int argc, char *argv[])
 				mdev.fspec = *argv;
 				mntpath = strdup("/tmp/df.XXXXXX");
 				if (mntpath == NULL) {
-					warn("strdup failed");
+					xo_warn("strdup failed");
 					rv = 1;
 					continue;
 				}
 				mntpt = mkdtemp(mntpath);
 				if (mntpt == NULL) {
-					warn("mkdtemp(\"%s\") failed", mntpath);
+					xo_warn("mkdtemp(\"%s\") failed", mntpath);
 					rv = 1;
 					free(mntpath);
 					continue;
 				}
 				if (mount(fstype, mntpt, MNT_RDONLY,
 				    &mdev) != 0) {
-					warn("%s", *argv);
+					xo_warn("%s", *argv);
 					rv = 1;
 					(void)rmdir(mntpt);
 					free(mntpath);
@@ -244,7 +253,7 @@ main(int argc, char *argv[])
 					if (cflag)
 						addstat(&totalbuf, &statfsbuf);
 				} else {
-					warn("%s", *argv);
+					xo_warn("%s", *argv);
 					rv = 1;
 				}
 				(void)unmount(mntpt, 0);
@@ -260,7 +269,7 @@ main(int argc, char *argv[])
 		 * implement nflag here.
 		 */
 		if (statfs(mntpt, &statfsbuf) < 0) {
-			warn("%s", mntpt);
+			xo_warn("%s", mntpt);
 			rv = 1;
 			continue;
 		}
@@ -294,9 +303,15 @@ main(int argc, char *argv[])
 	for (i = 0; i < mntsize; i++)
 		if (aflag || (mntbuf[i].f_flags & MNT_IGNORE) == 0)
 			prtstat(&mntbuf[i], &maxwidths);
+
+	xo_close_list("filesystem");
+
 	if (cflag)
 		prtstat(&totalbuf, &maxwidths);
-	return (rv);
+
+	xo_close_container("storage-system-information");
+	xo_finish();
+	exit(rv);
 }
 
 static char *
@@ -341,7 +356,7 @@ regetmntinfo(struct statfs **mntbufp, lo
 		if (nflag || error < 0)
 			if (i != j) {
 				if (error < 0)
-					warnx("%s stats possibly stale",
+					xo_warnx("%s stats possibly stale",
 					    mntbuf[i].f_mntonname);
 				mntbuf[j] = mntbuf[i];
 			}
@@ -354,13 +369,13 @@ static void
 prthuman(const struct statfs *sfsp, int64_t used)
 {
 
-	prthumanval(sfsp->f_blocks * sfsp->f_bsize);
-	prthumanval(used * sfsp->f_bsize);
-	prthumanval(sfsp->f_bavail * sfsp->f_bsize);
+	prthumanval("  {:blocks/%6s}", sfsp->f_blocks * sfsp->f_bsize);
+	prthumanval("  {:used/%6s}", used * sfsp->f_bsize);
+	prthumanval("  {:available/%6s}", sfsp->f_bavail * sfsp->f_bsize);
 }
 
 static void
-prthumanval(int64_t bytes)
+prthumanval(const char *fmt, int64_t bytes)
 {
 	char buf[6];
 	int flags;
@@ -372,14 +387,15 @@ prthumanval(int64_t bytes)
 	humanize_number(buf, sizeof(buf) - (bytes < 0 ? 0 : 1),
 	    bytes, "", HN_AUTOSCALE, flags);
 
-	(void)printf("  %6s", buf);
+	xo_attr("value", "%lld", (long long) bytes);
+	xo_emit(fmt, buf);
 }
 
 /*
  * Print an inode count in "human-readable" format.
  */
 static void
-prthumanvalinode(int64_t bytes)
+prthumanvalinode(const char *fmt, int64_t bytes)
 {
 	char buf[6];
 	int flags;
@@ -389,7 +405,8 @@ prthumanvalinode(int64_t bytes)
 	humanize_number(buf, sizeof(buf) - (bytes < 0 ? 0 : 1),
 	    bytes, "", HN_AUTOSCALE, flags);
 
-	(void)printf(" %5s", buf);
+	xo_attr("value", "%lld", (long long) bytes);
+	xo_emit(fmt, buf);
 }
 
 /*
@@ -434,70 +451,77 @@ prtstat(struct statfs *sfsp, struct maxw
 		mwp->used = imax(mwp->used, (int)strlen("Used"));
 		mwp->avail = imax(mwp->avail, (int)strlen("Avail"));
 
-		(void)printf("%-*s", mwp->mntfrom, "Filesystem");
+		xo_emit("{T:/%-*s}", mwp->mntfrom, "Filesystem");
 		if (Tflag)
-			(void)printf("  %-*s", mwp->fstype, "Type");
-		(void)printf(" %*s %*s %*s Capacity", mwp->total, header,
-		    mwp->used, "Used", mwp->avail, "Avail");
+			xo_emit("  {T:/%-*s}", mwp->fstype, "Type");
+		xo_emit(" {T:/%*s} {T:/%*s} {T:/%*s} Capacity",
+			mwp->total, header,
+			mwp->used, "Used", mwp->avail, "Avail");
 		if (iflag) {
 			mwp->iused = imax(hflag ? 0 : mwp->iused,
 			    (int)strlen("  iused"));
 			mwp->ifree = imax(hflag ? 0 : mwp->ifree,
 			    (int)strlen("ifree"));
-			(void)printf(" %*s %*s %%iused",
+			xo_emit(" {T:/%*s} {T:/%*s} {T:\%iused}",
 			    mwp->iused - 2, "iused", mwp->ifree, "ifree");
 		}
-		(void)printf("  Mounted on\n");
+		xo_emit("  {T:Mounted on}\n");
 	}
+
+	xo_open_instance("filesystem");
 	/* Check for 0 block size.  Can this happen? */
 	if (sfsp->f_bsize == 0) {
-		warnx ("File system %s does not have a block size, assuming 512.",
+		xo_warnx ("File system %s does not have a block size, assuming 512.",
 		    sfsp->f_mntonname);
 		sfsp->f_bsize = 512;
 	}
-	(void)printf("%-*s", mwp->mntfrom, sfsp->f_mntfromname);
+	xo_emit("{tk:name/%-*s}", mwp->mntfrom, sfsp->f_mntfromname);
 	if (Tflag)
-		(void)printf("  %-*s", mwp->fstype, sfsp->f_fstypename);
+		xo_emit("  {:type/%-*s}", mwp->fstype, sfsp->f_fstypename);
 	used = sfsp->f_blocks - sfsp->f_bfree;
 	availblks = sfsp->f_bavail + used;
 	if (hflag) {
 		prthuman(sfsp, used);
 	} else {
 		if (thousands)
-		    format = " %*j'd %*j'd %*j'd";
+		    format = " {t:total-blocks/%*j'd} {t:used-blocks/%*j'd} "
+			"{t:available-blocks/%*j'd}";
 		else
-		    format = " %*jd %*jd %*jd";
-		(void)printf(format,
+		    format = " {t:total-blocks/%*jd} {t:used-blocks/%*jd} "
+			"{t:available-blocks/%*jd}";
+		xo_emit(format,
 		    mwp->total, fsbtoblk(sfsp->f_blocks,
 		    sfsp->f_bsize, blocksize),
 		    mwp->used, fsbtoblk(used, sfsp->f_bsize, blocksize),
 		    mwp->avail, fsbtoblk(sfsp->f_bavail,
 		    sfsp->f_bsize, blocksize));
 	}
-	(void)printf(" %5.0f%%",
+	xo_emit(" {:used-percent/%5.0f}{U:%%}",
 	    availblks == 0 ? 100.0 : (double)used / (double)availblks * 100.0);
 	if (iflag) {
 		inodes = sfsp->f_files;
 		used = inodes - sfsp->f_ffree;
 		if (hflag) {
-			(void)printf("  ");
-			prthumanvalinode(used);
-			prthumanvalinode(sfsp->f_ffree);
+			xo_emit("  ");
+			prthumanvalinode(" {:inodes-used/%5s}", used);
+			prthumanvalinode(" {:inodes-free/%5s}", sfsp->f_ffree);
 		} else {
 			if (thousands)
-			    format = " %*j'd %*j'd";
+			    format = " {:inodes-used/%*j'd} {:inodes-free/%*j'd}";
 			else
-			    format = " %*jd %*jd";
-			(void)printf(format, mwp->iused, (intmax_t)used,
+			    format = " {:inodes-used/%*jd} {:inodes-free/%*jd}";
+			xo_emit(format, mwp->iused, (intmax_t)used,
 			    mwp->ifree, (intmax_t)sfsp->f_ffree);
 		}
-		(void)printf(" %4.0f%% ", inodes == 0 ? 100.0 :
-		    (double)used / (double)inodes * 100.0);
+		xo_emit(" {:inodes-used-percent/%4.0f}{U:%%} ",
+			inodes == 0 ? 100.0 :
+			(double)used / (double)inodes * 100.0);
 	} else
-		(void)printf("  ");
+		xo_emit("  ");
 	if (strncmp(sfsp->f_mntfromname, "total", MNAMELEN) != 0)
-		(void)printf("  %s", sfsp->f_mntonname);
-	(void)printf("\n");
+		xo_emit("  {:mounted-on}", sfsp->f_mntonname);
+	xo_emit("\n");
+	xo_close_instance("filesystem");
 }
 
 static void
@@ -564,7 +588,7 @@ static void
 usage(void)
 {
 
-	(void)fprintf(stderr,
+	xo_error(
 "usage: df [-b | -g | -H | -h | -k | -m | -P] [-acilnT] [-t type] [-,]\n"
 "          [file | filesystem ...]\n");
 	exit(EX_USAGE);
@@ -579,24 +603,24 @@ makenetvfslist(void)
 	int cnt, i, maxvfsconf;
 
 	if (sysctlbyname("vfs.conflist", NULL, &buflen, NULL, 0) < 0) {
-		warn("sysctl(vfs.conflist)");
+		xo_warn("sysctl(vfs.conflist)");
 		return (NULL);
 	}
 	xvfsp = malloc(buflen);
 	if (xvfsp == NULL) {
-		warnx("malloc failed");
+		xo_warnx("malloc failed");
 		return (NULL);
 	}
 	keep_xvfsp = xvfsp;
 	if (sysctlbyname("vfs.conflist", xvfsp, &buflen, NULL, 0) < 0) {
-		warn("sysctl(vfs.conflist)");
+		xo_warn("sysctl(vfs.conflist)");
 		free(keep_xvfsp);
 		return (NULL);
 	}
 	maxvfsconf = buflen / sizeof(struct xvfsconf);
 
 	if ((listptr = malloc(sizeof(char*) * maxvfsconf)) == NULL) {
-		warnx("malloc failed");
+		xo_warnx("malloc failed");
 		free(keep_xvfsp);
 		return (NULL);
 	}
@@ -605,7 +629,7 @@ makenetvfslist(void)
 		if (xvfsp->vfc_flags & VFCF_NETWORK) {
 			listptr[cnt++] = strdup(xvfsp->vfc_name);
 			if (listptr[cnt-1] == NULL) {
-				warnx("malloc failed");
+				xo_warnx("malloc failed");
 				free(listptr);
 				free(keep_xvfsp);
 				return (NULL);
@@ -617,7 +641,7 @@ makenetvfslist(void)
 	if (cnt == 0 ||
 	    (str = malloc(sizeof(char) * (32 * cnt + cnt + 2))) == NULL) {
 		if (cnt > 0)
-			warnx("malloc failed");
+			xo_warnx("malloc failed");
 		free(listptr);
 		free(keep_xvfsp);
 		return (NULL);
