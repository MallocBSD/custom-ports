--- usr.bin/w/w.c.orig	2015-12-29 06:03:45 UTC
+++ usr.bin/w/w.c
@@ -29,7 +29,7 @@
 
 #include <sys/cdefs.h>
 
-__FBSDID("$FreeBSD: releng/10.1/usr.bin/w/w.c 253750 2013-07-28 18:44:17Z avg $");
+__FBSDID("$FreeBSD: head/usr.bin/w/w.c 287590 2015-09-09 05:17:04Z delphij $");
 
 #ifndef lint
 static const char copyright[] =
@@ -54,8 +54,10 @@ static const char sccsid[] = "@(#)w.c	8.
 #include <sys/proc.h>
 #include <sys/user.h>
 #include <sys/ioctl.h>
+#include <sys/sbuf.h>
 #include <sys/socket.h>
 #include <sys/tty.h>
+#include <sys/types.h>
 
 #include <machine/cpu.h>
 #include <netinet/in.h>
@@ -68,6 +70,7 @@ static const char sccsid[] = "@(#)w.c	8.
 #include <fcntl.h>
 #include <kvm.h>
 #include <langinfo.h>
+#include <libgen.h>
 #include <libutil.h>
 #include <limits.h>
 #include <locale.h>
@@ -82,6 +85,7 @@ static const char sccsid[] = "@(#)w.c	8.
 #include <unistd.h>
 #include <utmpx.h>
 #include <vis.h>
+#include <libxo/xo.h>
 
 #include "extern.h"
 
@@ -116,12 +120,11 @@ static struct entry {
 
 #define	W_DISPUSERSIZE	10
 #define	W_DISPLINESIZE	8
-#define	W_DISPHOSTSIZE	24
+#define	W_DISPHOSTSIZE	40
 
 static void		 pr_header(time_t *, int);
 static struct stat	*ttystat(char *);
 static void		 usage(int);
-static int		 this_is_uptime(const char *s);
 
 char *fmt_argv(char **, char *, char *, size_t);	/* ../../bin/ps/fmt.c */
 
@@ -132,8 +135,8 @@ main(int argc, char *argv[])
 	struct kinfo_proc *dkp;
 	struct stat *stp;
 	time_t touched;
-	int ch, i, nentries, nusers, wcmd, longidle, longattime, dropgid;
-	const char *memf, *nlistf, *p;
+	int ch, i, nentries, nusers, wcmd, longidle, longattime;
+	const char *memf, *nlistf, *p, *save_p;
 	char *x_suffix;
 	char buf[MAXHOSTNAMELEN], errbuf[_POSIX2_LINE_MAX];
 	char fn[MAXHOSTNAMELEN];
@@ -143,8 +146,12 @@ main(int argc, char *argv[])
 	use_ampm = (*nl_langinfo(T_FMT_AMPM) != '\0');
 	use_comma = (*nl_langinfo(RADIXCHAR) != ',');
 
+	argc = xo_parse_args(argc, argv);
+	if (argc < 0)
+		exit(1);
+
 	/* Are we w(1) or uptime(1)? */
-	if (this_is_uptime(argv[0]) == 0) {
+	if (strcmp(basename(argv[0]), "uptime") == 0) {
 		wcmd = 0;
 		p = "";
 	} else {
@@ -152,7 +159,6 @@ main(int argc, char *argv[])
 		p = "dhiflM:N:nsuw";
 	}
 
-	dropgid = 0;
 	memf = _PATH_DEVNULL;
 	nlistf = NULL;
 	while ((ch = getopt(argc, argv, p)) != -1)
@@ -169,11 +175,9 @@ main(int argc, char *argv[])
 		case 'M':
 			header = 0;
 			memf = optarg;
-			dropgid = 1;
 			break;
 		case 'N':
 			nlistf = optarg;
-			dropgid = 1;
 			break;
 		case 'n':
 			nflag = 1;
@@ -193,13 +197,6 @@ main(int argc, char *argv[])
 	_res.retrans = 2;	/* resolver timeout to 2 seconds per try */
 	_res.retry = 1;		/* only try once.. */
 
-	/*
-	 * Discard setgid privileges if not the running kernel so that bad
-	 * guys can't print interesting stuff from kernel memory.
-	 */
-	if (dropgid)
-		setgid(getgid());
-
 	if ((kd = kvm_openfiles(nlistf, memf, NULL, O_RDONLY, errbuf)) == NULL)
 		errx(1, "%s", errbuf);
 
@@ -254,9 +251,14 @@ main(int argc, char *argv[])
 	}
 	endutxent();
 
+	xo_open_container("uptime-information");
+
 	if (header || wcmd == 0) {
 		pr_header(&now, nusers);
 		if (wcmd == 0) {
+			xo_close_container("uptime-information");
+			xo_finish();
+
 			(void)kvm_close(kd);
 			exit(0);
 		}
@@ -268,7 +270,7 @@ main(int argc, char *argv[])
 #define HEADER_WHAT		"WHAT\n"
 #define WUSED  (W_DISPUSERSIZE + W_DISPLINESIZE + W_DISPHOSTSIZE + \
 		sizeof(HEADER_LOGIN_IDLE) + 3)	/* header width incl. spaces */ 
-		(void)printf("%-*.*s %-*.*s %-*.*s  %s", 
+		xo_emit("{T:/%-*.*s} {T:/%-*.*s} {T:/%-*.*s}  {T:/%s}", 
 				W_DISPUSERSIZE, W_DISPUSERSIZE, HEADER_USER,
 				W_DISPLINESIZE, W_DISPLINESIZE, HEADER_TTY,
 				W_DISPHOSTSIZE, W_DISPHOSTSIZE, HEADER_FROM,
@@ -342,6 +344,9 @@ main(int argc, char *argv[])
 		}
 	}
 
+	xo_open_container("user-table");
+	xo_open_list("user-entry");
+
 	for (ep = ehead; ep != NULL; ep = ep->next) {
 		struct addrinfo hints, *res;
 		struct sockaddr_storage ss;
@@ -351,7 +356,9 @@ main(int argc, char *argv[])
 		time_t t;
 		int isaddr;
 
-		p = *ep->utmp.ut_host ? ep->utmp.ut_host : "-";
+		xo_open_instance("user-entry");
+
+		save_p = p = *ep->utmp.ut_host ? ep->utmp.ut_host : "-";
 		if ((x_suffix = strrchr(p, ':')) != NULL) {
 			if ((dot = strchr(x_suffix, '.')) != NULL &&
 			    strchr(dot+1, '.') == NULL)
@@ -400,6 +407,9 @@ main(int argc, char *argv[])
 			p = buf;
 		}
 		if (dflag) {
+		        xo_open_container("process-table");
+		        xo_open_list("process-entry");
+
 			for (dkp = ep->dkp; dkp != NULL; dkp = debugproc(dkp)) {
 				const char *ptr;
 
@@ -407,24 +417,41 @@ main(int argc, char *argv[])
 				    dkp->ki_comm, NULL, MAXCOMLEN);
 				if (ptr == NULL)
 					ptr = "-";
-				(void)printf("\t\t%-9d %s\n",
+				xo_open_instance("process-entry");
+				xo_emit("\t\t{:process-id/%-9d/%d} {:command/%s}\n",
 				    dkp->ki_pid, ptr);
+				xo_close_instance("process-entry");
 			}
+		        xo_close_list("process-entry");
+		        xo_close_container("process-table");
 		}
-		(void)printf("%-*.*s %-*.*s %-*.*s ",
-		    W_DISPUSERSIZE, W_DISPUSERSIZE, ep->utmp.ut_user,
-		    W_DISPLINESIZE, W_DISPLINESIZE,
-		    *ep->utmp.ut_line ?
-		    (strncmp(ep->utmp.ut_line, "tty", 3) &&
-		    strncmp(ep->utmp.ut_line, "cua", 3) ?
-		    ep->utmp.ut_line : ep->utmp.ut_line + 3) : "-",
+		xo_emit("{:user/%-*.*s/%@**@s} {:tty/%-*.*s/%@**@s} ",
+			W_DISPUSERSIZE, W_DISPUSERSIZE, ep->utmp.ut_user,
+			W_DISPLINESIZE, W_DISPLINESIZE,
+			*ep->utmp.ut_line ?
+			(strncmp(ep->utmp.ut_line, "tty", 3) &&
+			 strncmp(ep->utmp.ut_line, "cua", 3) ?
+			 ep->utmp.ut_line : ep->utmp.ut_line + 3) : "-");
+
+		if (save_p && save_p != p)
+		    xo_attr("address", "%s", save_p);
+		xo_emit("{:from/%-*.*s/%@**@s} ",
 		    W_DISPHOSTSIZE, W_DISPHOSTSIZE, *p ? p : "-");
 		t = ep->utmp.ut_tv.tv_sec;
 		longattime = pr_attime(&t, &now);
 		longidle = pr_idle(ep->idle);
-		(void)printf("%.*s\n", argwidth - longidle - longattime,
+		xo_emit("{:command/%.*s/%@*@s}\n",
+		    argwidth - longidle - longattime,
 		    ep->args);
+
+		xo_close_instance("user-entry");
 	}
+
+	xo_close_list("user-entry");
+	xo_close_container("user-table");
+	xo_close_container("uptime-information");
+	xo_finish();
+
 	(void)kvm_close(kd);
 	exit(0);
 }
@@ -437,13 +464,15 @@ pr_header(time_t *nowp, int nusers)
 	struct timespec tp;
 	int days, hrs, i, mins, secs;
 	char buf[256];
+	struct sbuf *upbuf;
 
+	upbuf = sbuf_new_auto();
 	/*
 	 * Print time of day.
 	 */
 	if (strftime(buf, sizeof(buf),
 	    use_ampm ? "%l:%M%p" : "%k:%M", localtime(nowp)) != 0)
-		(void)printf("%s ", buf);
+		xo_emit("{:time-of-day/%s} ", buf);
 	/*
 	 * Print how long system has been up.
 	 */
@@ -457,35 +486,51 @@ pr_header(time_t *nowp, int nusers)
 		uptime %= 3600;
 		mins = uptime / 60;
 		secs = uptime % 60;
-		(void)printf(" up");
+		xo_emit(" up");
+		xo_emit("{e:uptime/%lu}", (unsigned long) tp.tv_sec);
+		xo_emit("{e:days/%d}{e:hours/%d}{e:minutes/%d}{e:seconds/%d}", days, hrs, mins, secs);
+
 		if (days > 0)
-			(void)printf(" %d day%s,", days, days > 1 ? "s" : "");
+			sbuf_printf(upbuf, " %d day%s,",
+				days, days > 1 ? "s" : "");
 		if (hrs > 0 && mins > 0)
-			(void)printf(" %2d:%02d,", hrs, mins);
+			sbuf_printf(upbuf, " %2d:%02d,", hrs, mins);
 		else if (hrs > 0)
-			(void)printf(" %d hr%s,", hrs, hrs > 1 ? "s" : "");
+			sbuf_printf(upbuf, " %d hr%s,",
+				hrs, hrs > 1 ? "s" : "");
 		else if (mins > 0)
-			(void)printf(" %d min%s,", mins, mins > 1 ? "s" : "");
-		else
-			(void)printf(" %d sec%s,", secs, secs > 1 ? "s" : "");
+			sbuf_printf(upbuf, " %d min%s,",
+				mins, mins > 1 ? "s" : "");
+		else 
+			sbuf_printf(upbuf, " %d sec%s,",
+				secs, secs > 1 ? "s" : "");
+		if (sbuf_finish(upbuf) != 0)
+			xo_err(1, "Could not generate output");
+		xo_emit("{:uptime-human/%s}", sbuf_data(upbuf));
+		sbuf_delete(upbuf);
 	}
 
 	/* Print number of users logged in to system */
-	(void)printf(" %d user%s", nusers, nusers == 1 ? "" : "s");
+	xo_emit(" {:users/%d} {N:user%s}", nusers, nusers == 1 ? "" : "s");
 
 	/*
 	 * Print 1, 5, and 15 minute load averages.
 	 */
 	if (getloadavg(avenrun, sizeof(avenrun) / sizeof(avenrun[0])) == -1)
-		(void)printf(", no load average information available\n");
+		xo_emit(", no load average information available\n");
 	else {
-		(void)printf(", load averages:");
+	        static const char *format[] = {
+		    " {:load-average-1/%.2f}",
+		    " {:load-average-5/%.2f}",
+		    " {:load-average-15/%.2f}",
+		};
+		xo_emit(", load averages:");
 		for (i = 0; i < (int)(sizeof(avenrun) / sizeof(avenrun[0])); i++) {
 			if (use_comma && i > 0)
-				(void)printf(",");
-			(void)printf(" %.2f", avenrun[i]);
+				xo_emit(",");
+			xo_emit(format[i], avenrun[i]);
 		}
-		(void)printf("\n");
+		xo_emit("\n");
 	}
 }
 
@@ -506,23 +551,9 @@ static void
 usage(int wcmd)
 {
 	if (wcmd)
-		(void)fprintf(stderr,
-		    "usage: w [-dhin] [-M core] [-N system] [user ...]\n");
+		xo_error("usage: w [-dhin] [-M core] [-N system] [user ...]\n");
 	else
-		(void)fprintf(stderr, "usage: uptime\n");
+		xo_error("usage: uptime\n");
+	xo_finish();
 	exit(1);
 }
-
-static int 
-this_is_uptime(const char *s)
-{
-	const char *u;
-
-	if ((u = strrchr(s, '/')) != NULL)
-		++u;
-	else
-		u = s;
-	if (strcmp(u, "uptime") == 0)
-		return (0);
-	return (-1);
-}
