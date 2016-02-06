--- bin/ls/ls.c.orig	2015-12-29 08:18:18 UTC
+++ bin/ls/ls.c
@@ -42,7 +42,7 @@ static char sccsid[] = "@(#)ls.c	8.5 (Be
 #endif /* not lint */
 #endif
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/bin/ls/ls.c 242840 2012-11-09 20:19:56Z peter $");
+__FBSDID("$FreeBSD: head/bin/ls/ls.c 285734 2015-07-21 05:03:59Z allanjude $");
 
 #include <sys/param.h>
 #include <sys/stat.h>
@@ -66,6 +66,7 @@ __FBSDID("$FreeBSD: releng/10.1/bin/ls/l
 #include <termcap.h>
 #include <signal.h>
 #endif
+#include <libxo/xo.h>
 
 #include "ls.h"
 #include "extern.h"
@@ -118,7 +119,7 @@ static int f_nofollow;		/* don't follow 
        int f_nonprint;		/* show unprintables as ? */
 static int f_nosort;		/* don't sort output */
        int f_notabs;		/* don't use tab-separated multi-col output */
-static int f_numericonly;	/* don't convert uid/gid to name */
+       int f_numericonly;	/* don't convert uid/gid to name */
        int f_octal;		/* show unprintables as \xxx */
        int f_octal_escape;	/* like f_octal but use C escapes if possible */
 static int f_recursive;		/* ls subdirectories also */
@@ -157,6 +158,7 @@ main(int argc, char *argv[])
 	struct winsize win;
 	int ch, fts_options, notused;
 	char *p;
+	const char *errstr = NULL;
 #ifdef COLORLS
 	char termcapbuf[1024];	/* termcap definition buffer */
 	char tcapbuf[512];	/* capability buffer */
@@ -169,7 +171,7 @@ main(int argc, char *argv[])
 	if (isatty(STDOUT_FILENO)) {
 		termwidth = 80;
 		if ((p = getenv("COLUMNS")) != NULL && *p != '\0')
-			termwidth = atoi(p);
+			termwidth = strtonum(p, 0, INT_MAX, &errstr);
 		else if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) != -1 &&
 		    win.ws_col > 0)
 			termwidth = win.ws_col;
@@ -179,12 +181,22 @@ main(int argc, char *argv[])
 		/* retrieve environment variable, in case of explicit -C */
 		p = getenv("COLUMNS");
 		if (p)
-			termwidth = atoi(p);
+			termwidth = strtonum(p, 0, INT_MAX, &errstr);
 	}
 
+	if (errstr)
+		termwidth = 80;
+
 	fts_options = FTS_PHYSICAL;
 	if (getenv("LS_SAMESORT"))
 		f_samesort = 1;
+
+	argc = xo_parse_args(argc, argv);
+	if (argc < 0)
+		return (1);
+	xo_set_flags(NULL, XOF_COLUMNS);
+	xo_set_version(LS_XO_VERSION);
+
 	while ((ch = getopt(argc, argv,
 	    "1ABCD:FGHILPRSTUWXZabcdfghiklmnopqrstuwxy,")) != -1) {
 		switch (ch) {
@@ -226,6 +238,9 @@ main(int argc, char *argv[])
 			f_accesstime = 0;
 			f_statustime = 0;
 			break;
+		case 'f':
+			f_nosort = 1;
+		       /* FALLTHROUGH */
 		case 'a':
 			fts_options |= FTS_SEEDOT;
 			/* FALLTHROUGH */
@@ -300,9 +315,6 @@ main(int argc, char *argv[])
 			f_listdir = 1;
 			f_recursive = 0;
 			break;
-		case 'f':
-			f_nosort = 1;
-			break;
 		case 'g':	/* Compatibility with 4.3BSD. */
 			break;
 		case 'h':
@@ -381,7 +393,7 @@ main(int argc, char *argv[])
 				f_color = 1;
 		}
 #else
-		warnx("color support not compiled in");
+		xo_warnx("color support not compiled in");
 #endif /*COLORLS*/
 
 #ifdef COLORLS
@@ -413,9 +425,14 @@ main(int argc, char *argv[])
 
 	/*
 	 * If not -F, -P, -d or -l options, follow any symbolic links listed on
-	 * the command line.
+	 * the command line, unless in color mode in which case we need to
+	 * distinguish file type for a symbolic link itself and its target.
 	 */
-	if (!f_nofollow && !f_longform && !f_listdir && (!f_type || f_slash))
+	if (!f_nofollow && !f_longform && !f_listdir && (!f_type || f_slash)
+#ifdef COLORLS
+	    && !f_color
+#endif
+	    )
 		fts_options |= FTS_COMFOLLOW;
 
 	/*
@@ -474,10 +491,13 @@ main(int argc, char *argv[])
 	else
 		printfcn = printcol;
 
+	xo_open_container("file-information");
 	if (argc)
 		traverse(argc, argv, fts_options);
 	else
 		traverse(1, dotav, fts_options);
+	xo_close_container("file-information");
+	xo_finish();
 	exit(rval);
 }
 
@@ -495,10 +515,11 @@ traverse(int argc, char *argv[], int opt
 	FTS *ftsp;
 	FTSENT *p, *chp;
 	int ch_options;
+	int first = 1;
 
 	if ((ftsp =
 	    fts_open(argv, options, f_nosort ? NULL : mastercmp)) == NULL)
-		err(1, "fts_open");
+		xo_err(1, "fts_open");
 
 	/*
 	 * We ignore errors from fts_children here since they will be
@@ -520,11 +541,11 @@ traverse(int argc, char *argv[], int opt
 	while ((p = fts_read(ftsp)) != NULL)
 		switch (p->fts_info) {
 		case FTS_DC:
-			warnx("%s: directory causes a cycle", p->fts_name);
+			xo_warnx("%s: directory causes a cycle", p->fts_name);
 			break;
 		case FTS_DNR:
 		case FTS_ERR:
-			warnx("%s: %s", p->fts_path, strerror(p->fts_errno));
+			xo_warnx("%s: %s", p->fts_path, strerror(p->fts_errno));
 			rval = 1;
 			break;
 		case FTS_D:
@@ -532,31 +553,40 @@ traverse(int argc, char *argv[], int opt
 			    p->fts_name[0] == '.' && !f_listdot)
 				break;
 
+			if (first) {
+				first = 0;
+				xo_open_list("directory");
+			}
+			xo_open_instance("directory");
+
 			/*
 			 * If already output something, put out a newline as
 			 * a separator.  If multiple arguments, precede each
 			 * directory with its name.
 			 */
 			if (output) {
-				putchar('\n');
-				(void)printname(p->fts_path);
-				puts(":");
+				xo_emit("\n");
+				(void)printname("path", p->fts_path);
+				xo_emit(":\n");
 			} else if (argc > 1) {
-				(void)printname(p->fts_path);
-				puts(":");
+				(void)printname("path", p->fts_path);
+				xo_emit(":\n");
 				output = 1;
 			}
 			chp = fts_children(ftsp, ch_options);
 			display(p, chp, options);
 
+			xo_close_instance("directory");
 			if (!f_recursive && chp != NULL)
 				(void)fts_set(ftsp, p, FTS_SKIP);
 			break;
 		default:
 			break;
 		}
+	if (!first)
+		xo_close_list("directory");
 	if (errno)
-		err(1, "fts_read");
+		xo_err(1, "fts_read");
 }
 
 /*
@@ -603,7 +633,7 @@ display(const FTSENT *p, FTSENT *list, i
 		/* Fill-in "::" as "0:0:0" for the sake of scanf. */
 		jinitmax = malloc(strlen(initmax) * 2 + 2);
 		if (jinitmax == NULL)
-			err(1, "malloc");
+			xo_err(1, "malloc");
 		initmax2 = jinitmax;
 		if (*initmax == ':')
 			strcpy(initmax2, "0:"), initmax2 += 2;
@@ -674,7 +704,7 @@ display(const FTSENT *p, FTSENT *list, i
 	flags = NULL;
 	for (cur = list, entries = 0; cur; cur = cur->fts_link) {
 		if (cur->fts_info == FTS_ERR || cur->fts_info == FTS_NS) {
-			warnx("%s: %s",
+			xo_warnx("%s: %s",
 			    cur->fts_name, strerror(cur->fts_errno));
 			cur->fts_number = NO_PRINT;
 			rval = 1;
@@ -740,7 +770,7 @@ display(const FTSENT *p, FTSENT *list, i
 						flags = strdup("-");
 					}
 					if (flags == NULL)
-						err(1, "fflagstostr");
+						xo_err(1, "fflagstostr");
 					flen = strlen(flags);
 					if (flen > (size_t)maxflags)
 						maxflags = flen;
@@ -754,7 +784,7 @@ display(const FTSENT *p, FTSENT *list, i
 
 					error = mac_prepare_file_label(&label);
 					if (error == -1) {
-						warn("MAC label for %s/%s",
+						xo_warn("MAC label for %s/%s",
 						    cur->fts_parent->fts_path,
 						    cur->fts_name);
 						goto label_out;
@@ -775,7 +805,7 @@ display(const FTSENT *p, FTSENT *list, i
 						error = mac_get_link(name,
 						    label);
 					if (error == -1) {
-						warn("MAC label for %s/%s",
+						xo_warn("MAC label for %s/%s",
 						    cur->fts_parent->fts_path,
 						    cur->fts_name);
 						mac_free(label);
@@ -785,7 +815,7 @@ display(const FTSENT *p, FTSENT *list, i
 					error = mac_to_text(label,
 					    &labelstr);
 					if (error == -1) {
-						warn("MAC label for %s/%s",
+						xo_warn("MAC label for %s/%s",
 						    cur->fts_parent->fts_path,
 						    cur->fts_name);
 						mac_free(label);
@@ -803,7 +833,7 @@ label_out:
 
 				if ((np = malloc(sizeof(NAMES) + labelstrlen +
 				    ulen + glen + flen + 4)) == NULL)
-					err(1, "malloc");
+					xo_err(1, "malloc");
 
 				np->user = &np->data[0];
 				(void)strcpy(np->user, user);
