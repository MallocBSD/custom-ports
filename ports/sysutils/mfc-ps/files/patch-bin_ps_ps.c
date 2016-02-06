--- bin/ps/ps.c.orig	2015-12-29 09:01:39 UTC
+++ bin/ps/ps.c
@@ -73,6 +73,7 @@ __FBSDID("$FreeBSD: releng/10.1/bin/ps/p
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
+#include <libxo/xo.h>
 
 #include "ps.h"
 
@@ -178,13 +179,15 @@ main(int argc, char *argv[])
 	KINFO *kinfo = NULL, *next_KINFO;
 	KINFO_STR *ks;
 	struct varent *vent;
-	struct winsize ws;
-	const char *nlistf, *memf, *fmtstr, *str;
+	struct winsize ws = { .ws_row = 0 };
+	const char *nlistf, *memf, *str;
 	char *cols;
 	int all, ch, elem, flag, _fmt, i, lineno, linelen, left;
 	int descendancy, nentries, nkept, nselectors;
 	int prtheader, wflag, what, xkeep, xkeep_implied;
+	int fwidthmin, fwidthmax;
 	char errbuf[_POSIX2_LINE_MAX];
+	char fmtbuf[_POSIX2_LINE_MAX];
 
 	(void) setlocale(LC_ALL, "");
 	time(&now);			/* Used by routines in print.c. */
@@ -221,6 +224,11 @@ main(int argc, char *argv[])
 	init_list(&uidlist, addelem_uid, sizeof(uid_t), "user");
 	memf = _PATH_DEVNULL;
 	nlistf = NULL;
+
+	argc = xo_parse_args(argc, argv);
+	if (argc < 0)
+		exit(1);
+
 	while ((ch = getopt(argc, argv, PS_ARGS)) != -1)
 		switch (ch) {
 		case 'A':
@@ -363,7 +371,7 @@ main(int argc, char *argv[])
 #endif
 		case 'T':
 			if ((optarg = ttyname(STDIN_FILENO)) == NULL)
-				errx(1, "stdin: not a terminal");
+				xo_errx(1, "stdin: not a terminal");
 			/* FALLTHROUGH */
 		case 't':
 			add_list(&ttylist, optarg);
@@ -434,8 +442,7 @@ main(int argc, char *argv[])
 		argv++;
 	}
 	if (*argv) {
-		fprintf(stderr, "%s: illegal argument: %s\n",
-		    getprogname(), *argv);
+		xo_warnx("illegal argument: %s\n", *argv);
 		usage();
 	}
 	if (optfatal)
@@ -445,7 +452,7 @@ main(int argc, char *argv[])
 
 	kd = kvm_openfiles(nlistf, memf, NULL, O_RDONLY, errbuf);
 	if (kd == 0)
-		errx(1, "%s", errbuf);
+		xo_errx(1, "%s", errbuf);
 
 	if (!_fmt)
 		parsefmt(dfmt, 0);
@@ -453,7 +460,7 @@ main(int argc, char *argv[])
 	if (nselectors == 0) {
 		uidlist.l.ptr = malloc(sizeof(uid_t));
 		if (uidlist.l.ptr == NULL)
-			errx(1, "malloc failed");
+			xo_errx(1, "malloc failed");
 		nselectors = 1;
 		uidlist.count = uidlist.maxcount = 1;
 		*uidlist.l.uids = getuid();
@@ -515,11 +522,11 @@ main(int argc, char *argv[])
 	nentries = -1;
 	kp = kvm_getprocs(kd, what, flag, &nentries);
 	if ((kp == NULL && nentries > 0) || (kp != NULL && nentries < 0))
-		errx(1, "%s", kvm_geterr(kd));
+		xo_errx(1, "%s", kvm_geterr(kd));
 	nkept = 0;
 	if (nentries > 0) {
 		if ((kinfo = malloc(nentries * sizeof(*kinfo))) == NULL)
-			errx(1, "malloc failed");
+			xo_errx(1, "malloc failed");
 		for (i = nentries; --i >= 0; ++kp) {
 			/*
 			 * If the user specified multiple selection-criteria,
@@ -629,37 +636,43 @@ main(int argc, char *argv[])
 	/*
 	 * Print header.
 	 */
+	xo_open_container("process-information");
 	printheader();
+	if (xo_get_style(NULL) != XO_STYLE_TEXT)
+		termwidth = UNLIMITED;
 
 	/*
 	 * Output formatted lines.
 	 */
+	xo_open_list("process");
 	for (i = lineno = 0; i < nkept; i++) {
 		linelen = 0;
+		xo_open_instance("process");
 		STAILQ_FOREACH(vent, &varlist, next_ve) {
-	        	if (vent->var->flag & LJUST)
-				fmtstr = "%-*s";
-			else
-				fmtstr = "%*s";
-
 			ks = STAILQ_FIRST(&kinfo[i].ki_ks);
 			STAILQ_REMOVE_HEAD(&kinfo[i].ki_ks, ks_next);
 			/* Truncate rightmost column if necessary.  */
+			fwidthmax = _POSIX2_LINE_MAX;
 			if (STAILQ_NEXT(vent, next_ve) == NULL &&
 			   termwidth != UNLIMITED && ks->ks_str != NULL) {
 				left = termwidth - linelen;
 				if (left > 0 && left < (int)strlen(ks->ks_str))
-					ks->ks_str[left] = '\0';
+					fwidthmax = left;
 			}
+
 			str = ks->ks_str;
 			if (str == NULL)
 				str = "-";
 			/* No padding for the last column, if it's LJUST. */
-			if (STAILQ_NEXT(vent, next_ve) == NULL &&
-			    vent->var->flag & LJUST)
-				linelen += printf(fmtstr, 0, str);
-			else
-				linelen += printf(fmtstr, vent->var->width, str);
+			fwidthmin = (xo_get_style(NULL) != XO_STYLE_TEXT ||
+			    (STAILQ_NEXT(vent, next_ve) == NULL &&
+			    (vent->var->flag & LJUST))) ? 0 : vent->var->width;
+			snprintf(fmtbuf, sizeof(fmtbuf), "{:%s/%%%s%d..%ds}",
+			    vent->var->field ?: vent->var->name,
+			    (vent->var->flag & LJUST) ? "-" : "",
+			    fwidthmin, fwidthmax);
+			xo_emit(fmtbuf, str);
+			linelen += fwidthmin;
 
 			if (ks->ks_str != NULL) {
 				free(ks->ks_str);
@@ -669,17 +682,22 @@ main(int argc, char *argv[])
 			ks = NULL;
 
 			if (STAILQ_NEXT(vent, next_ve) != NULL) {
-				(void)putchar(' ');
+				xo_emit("{P: }");
 				linelen++;
 			}
 		}
-		(void)putchar('\n');
+	        xo_emit("\n");
+		xo_close_instance("process");
 		if (prtheader && lineno++ == prtheader - 4) {
-			(void)putchar('\n');
+			xo_emit("\n");
 			printheader();
 			lineno = 0;
 		}
 	}
+	xo_close_list("process");
+	xo_close_container("process-information");
+	xo_finish();
+
 	free_list(&gidlist);
 	free_list(&jidlist);
 	free_list(&pidlist);
@@ -705,9 +723,9 @@ addelem_gid(struct listinfo *inf, const 
 
 	if (*elem == '\0' || strlen(elem) >= MAXLOGNAME) {
 		if (*elem == '\0')
-			warnx("Invalid (zero-length) %s name", inf->lname);
+			xo_warnx("Invalid (zero-length) %s name", inf->lname);
 		else
-			warnx("%s name too long: %s", inf->lname, elem);
+			xo_warnx("%s name too long: %s", inf->lname, elem);
 		optfatal = 1;
 		return (0);		/* Do not add this value. */
 	}
@@ -732,7 +750,7 @@ addelem_gid(struct listinfo *inf, const 
 	if (grp == NULL)
 		grp = getgrnam(elem);
 	if (grp == NULL) {
-		warnx("No %s %s '%s'", inf->lname, nameorID, elem);
+		xo_warnx("No %s %s '%s'", inf->lname, nameorID, elem);
 		optfatal = 1;
 		return (0);
 	}
@@ -748,14 +766,14 @@ addelem_jid(struct listinfo *inf, const 
 	int tempid;
 
 	if (*elem == '\0') {
-		warnx("Invalid (zero-length) jail id");
+		xo_warnx("Invalid (zero-length) jail id");
 		optfatal = 1;
 		return (0);		/* Do not add this value. */
 	}
 
 	tempid = jail_getid(elem);
 	if (tempid < 0) {
-		warnx("Invalid %s: %s", inf->lname, elem);
+		xo_warnx("Invalid %s: %s", inf->lname, elem);
 		optfatal = 1;
 		return (0);
 	}
@@ -773,7 +791,7 @@ addelem_pid(struct listinfo *inf, const 
 	long tempid;
 
 	if (*elem == '\0') {
-		warnx("Invalid (zero-length) process id");
+		xo_warnx("Invalid (zero-length) process id");
 		optfatal = 1;
 		return (0);		/* Do not add this value. */
 	}
@@ -781,10 +799,10 @@ addelem_pid(struct listinfo *inf, const 
 	errno = 0;
 	tempid = strtol(elem, &endp, 10);
 	if (*endp != '\0' || tempid < 0 || elem == endp) {
-		warnx("Invalid %s: %s", inf->lname, elem);
+		xo_warnx("Invalid %s: %s", inf->lname, elem);
 		errno = ERANGE;
 	} else if (errno != 0 || tempid > pid_max) {
-		warnx("%s too large: %s", inf->lname, elem);
+		xo_warnx("%s too large: %s", inf->lname, elem);
 		errno = ERANGE;
 	}
 	if (errno == ERANGE) {
@@ -855,19 +873,19 @@ addelem_tty(struct listinfo *inf, const 
 	if (ttypath) {
 		if (stat(ttypath, &sb) == -1) {
 			if (pathbuf3[0] != '\0')
-				warn("%s, %s, and %s", pathbuf3, pathbuf2,
+				xo_warn("%s, %s, and %s", pathbuf3, pathbuf2,
 				    ttypath);
 			else
-				warn("%s", ttypath);
+				xo_warn("%s", ttypath);
 			optfatal = 1;
 			return (0);
 		}
 		if (!S_ISCHR(sb.st_mode)) {
 			if (pathbuf3[0] != '\0')
-				warnx("%s, %s, and %s: Not a terminal",
+				xo_warnx("%s, %s, and %s: Not a terminal",
 				    pathbuf3, pathbuf2, ttypath);
 			else
-				warnx("%s: Not a terminal", ttypath);
+				xo_warnx("%s: Not a terminal", ttypath);
 			optfatal = 1;
 			return (0);
 		}
@@ -887,9 +905,9 @@ addelem_uid(struct listinfo *inf, const 
 
 	if (*elem == '\0' || strlen(elem) >= MAXLOGNAME) {
 		if (*elem == '\0')
-			warnx("Invalid (zero-length) %s name", inf->lname);
+			xo_warnx("Invalid (zero-length) %s name", inf->lname);
 		else
-			warnx("%s name too long: %s", inf->lname, elem);
+			xo_warnx("%s name too long: %s", inf->lname, elem);
 		optfatal = 1;
 		return (0);		/* Do not add this value. */
 	}
@@ -899,12 +917,12 @@ addelem_uid(struct listinfo *inf, const 
 		errno = 0;
 		bigtemp = strtoul(elem, &endp, 10);
 		if (errno != 0 || *endp != '\0' || bigtemp > UID_MAX)
-			warnx("No %s named '%s'", inf->lname, elem);
+			xo_warnx("No %s named '%s'", inf->lname, elem);
 		else {
 			/* The string is all digits, so it might be a userID. */
 			pwd = getpwuid((uid_t)bigtemp);
 			if (pwd == NULL)
-				warnx("No %s name or ID matches '%s'",
+				xo_warnx("No %s name or ID matches '%s'",
 				    inf->lname, elem);
 		}
 	}
@@ -961,7 +979,7 @@ add_list(struct listinfo *inf, const cha
 			while (*argp != '\0' && strchr(W_SEP T_SEP,
 			    *argp) == NULL)
 				argp++;
-			warnx("Value too long: %.*s", (int)(argp - savep),
+			xo_warnx("Value too long: %.*s", (int)(argp - savep),
 			    savep);
 			optfatal = 1;
 		}
@@ -1062,7 +1080,7 @@ descendant_sort(KINFO *ki, int items)
 			continue;
 		}
 		if ((ki[src].ki_d.prefix = malloc(lvl * 2 + 1)) == NULL)
-			errx(1, "malloc failed");
+			xo_errx(1, "malloc failed");
 		for (n = 0; n < lvl - 2; n++) {
 			ki[src].ki_d.prefix[n * 2] =
 			    path[n / 8] & 1 << (n % 8) ? '|' : ' ';
@@ -1100,7 +1118,7 @@ expand_list(struct listinfo *inf)
 	newlist = realloc(inf->l.ptr, newmax * inf->elemsize);
 	if (newlist == NULL) {
 		free(inf->l.ptr);
-		errx(1, "realloc to %d %ss failed", newmax, inf->lname);
+		xo_errx(1, "realloc to %d %ss failed", newmax, inf->lname);
 	}
 	inf->maxcount = newmax;
 	inf->l.ptr = newlist;
@@ -1174,7 +1192,7 @@ format_output(KINFO *ki)
 		str = (v->oproc)(ki, vent);
 		ks = malloc(sizeof(*ks));
 		if (ks == NULL)
-			errx(1, "malloc failed");
+			xo_errx(1, "malloc failed");
 		ks->ks_str = str;
 		STAILQ_INSERT_TAIL(&ki->ki_ks, ks, ks_next);
 		if (str != NULL) {
@@ -1240,7 +1258,7 @@ saveuser(KINFO *ki)
 		else
 			asprintf(&ki->ki_args, "(%s)", ki->ki_p->ki_comm);
 		if (ki->ki_args == NULL)
-			errx(1, "malloc failed");
+			xo_errx(1, "malloc failed");
 	} else {
 		ki->ki_args = NULL;
 	}
@@ -1251,7 +1269,7 @@ saveuser(KINFO *ki)
 		else
 			ki->ki_env = strdup("()");
 		if (ki->ki_env == NULL)
-			errx(1, "malloc failed");
+			xo_errx(1, "malloc failed");
 	} else {
 		ki->ki_env = NULL;
 	}
@@ -1372,7 +1390,7 @@ kludge_oldps_options(const char *optlist
 	 * original value.
 	 */
 	if ((newopts = ns = malloc(len + 3)) == NULL)
-		errx(1, "malloc failed");
+		xo_errx(1, "malloc failed");
 
 	if (*origval != '-')
 		*ns++ = '-';	/* add option flag */
@@ -1401,7 +1419,7 @@ pidmax_init(void)
 
 	intsize = sizeof(pid_max);
 	if (sysctlbyname("kern.pid_max", &pid_max, &intsize, NULL, 0) < 0) {
-		warn("unable to read kern.pid_max");
+		xo_warn("unable to read kern.pid_max");
 		pid_max = 99999;
 	}
 }
@@ -1411,7 +1429,7 @@ usage(void)
 {
 #define	SINGLE_OPTS	"[-aCcde" OPT_LAZY_f "HhjlmrSTuvwXxZ]"
 
-	(void)fprintf(stderr, "%s\n%s\n%s\n%s\n",
+	(void)xo_error("%s\n%s\n%s\n%s\n",
 	    "usage: ps " SINGLE_OPTS " [-O fmt | -o fmt] [-G gid[,gid...]]",
 	    "          [-J jid[,jid...]] [-M core] [-N system]",
 	    "          [-p pid[,pid...]] [-t tty[,tty...]] [-U user[,user...]]",
