--- bin/ls/print.c.orig	2015-12-29 08:18:18 UTC
+++ bin/ls/print.c
@@ -36,7 +36,7 @@ static char sccsid[] = "@(#)print.c	8.4 
 #endif /* not lint */
 #endif
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/bin/ls/print.c 242840 2012-11-09 20:19:56Z peter $");
+__FBSDID("$FreeBSD: head/bin/ls/print.c 291959 2015-12-07 20:48:28Z bapt $");
 
 #include <sys/param.h>
 #include <sys/stat.h>
@@ -47,17 +47,20 @@ __FBSDID("$FreeBSD: releng/10.1/bin/ls/p
 #include <fts.h>
 #include <langinfo.h>
 #include <libutil.h>
+#include <limits.h>
 #include <stdio.h>
 #include <stdint.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 #include <unistd.h>
+#include <wchar.h>
 #ifdef COLORLS
 #include <ctype.h>
 #include <termcap.h>
 #include <signal.h>
 #endif
+#include <libxo/xo.h>
 
 #include "ls.h"
 #include "extern.h"
@@ -65,9 +68,9 @@ __FBSDID("$FreeBSD: releng/10.1/bin/ls/p
 static int	printaname(const FTSENT *, u_long, u_long);
 static void	printdev(size_t, dev_t);
 static void	printlink(const FTSENT *);
-static void	printtime(time_t);
+static void	printtime(const char *, time_t);
 static int	printtype(u_int);
-static void	printsize(size_t, off_t);
+static void	printsize(const char *, size_t, off_t);
 #ifdef COLORLS
 static void	endcolor(int);
 static int	colortype(mode_t);
@@ -104,31 +107,118 @@ static struct {
 } colors[C_NUMCOLORS];
 #endif
 
+static size_t padding_for_month[12];
+static size_t month_max_size = 0;
+
 void
 printscol(const DISPLAY *dp)
 {
 	FTSENT *p;
 
+	xo_open_list("entry");
 	for (p = dp->list; p; p = p->fts_link) {
 		if (IS_NOPRINT(p))
 			continue;
+		xo_open_instance("entry");
 		(void)printaname(p, dp->s_inode, dp->s_block);
-		(void)putchar('\n');
+		xo_close_instance("entry");
+		xo_emit("\n");
 	}
+	xo_close_list("entry");
 }
 
 /*
  * print name in current style
  */
 int
-printname(const char *name)
+printname(const char *field, const char *name)
+{
+	char fmt[BUFSIZ];
+	char *s = getname(name);
+	int rc;
+	
+	snprintf(fmt, sizeof(fmt), "{:%s/%%hs}", field);
+	rc = xo_emit(fmt, s);
+	free(s);
+	return rc;
+}
+
+static const char *
+get_abmon(int mon)
+{
+
+	switch (mon) {
+	case 0: return (nl_langinfo(ABMON_1));
+	case 1: return (nl_langinfo(ABMON_2));
+	case 2: return (nl_langinfo(ABMON_3));
+	case 3: return (nl_langinfo(ABMON_4));
+	case 4: return (nl_langinfo(ABMON_5));
+	case 5: return (nl_langinfo(ABMON_6));
+	case 6: return (nl_langinfo(ABMON_7));
+	case 7: return (nl_langinfo(ABMON_8));
+	case 8: return (nl_langinfo(ABMON_9));
+	case 9: return (nl_langinfo(ABMON_10));
+	case 10: return (nl_langinfo(ABMON_11));
+	case 11: return (nl_langinfo(ABMON_12));
+	}
+
+	/* should never happen */
+	abort();
+}
+
+static size_t
+mbswidth(const char *month)
+{
+	wchar_t wc;
+	size_t width, donelen, clen, w;
+
+	width = donelen = 0;
+	while ((clen = mbrtowc(&wc, month + donelen, MB_LEN_MAX, NULL)) != 0) {
+		if (clen == (size_t)-1 || clen == (size_t)-2)
+			return (-1);
+		donelen += clen;
+		if ((w = wcwidth(wc)) == (size_t)-1)
+			return (-1);
+		width += w;
+	}
+
+	return (width);
+}
+
+static void
+compute_abbreviated_month_size(void)
+{
+	int i;
+	size_t width;
+	size_t months_width[12];
+
+	for (i = 0; i < 12; i++) {
+		width = mbswidth(get_abmon(i));
+		if (width == (size_t)-1) {
+			month_max_size = -1;
+			return;
+		}
+		months_width[i] = width;
+		if (width > month_max_size)
+			month_max_size = width;
+	}
+
+	for (i = 0; i < 12; i++)
+		padding_for_month[i] = month_max_size - months_width[i];
+}
+
+/*
+ * print name in current style
+ */
+char *
+getname(const char *name)
 {
 	if (f_octal || f_octal_escape)
-		return prn_octal(name);
+		return get_octal(name);
 	else if (f_nonprint)
-		return prn_printable(name);
+		return get_printable(name);
 	else
-		return prn_normal(name);
+		return strdup(name);
 }
 
 void
@@ -144,46 +234,83 @@ printlong(const DISPLAY *dp)
 
 	if ((dp->list == NULL || dp->list->fts_level != FTS_ROOTLEVEL) &&
 	    (f_longform || f_size)) {
-		(void)printf("total %lu\n", howmany(dp->btotal, blocksize));
+		xo_emit("{L:total} {:total-blocks/%lu}\n",
+			howmany(dp->btotal, blocksize));
 	}
 
+	xo_open_list("entry");
 	for (p = dp->list; p; p = p->fts_link) {
+		char *name, *type;
 		if (IS_NOPRINT(p))
 			continue;
+		xo_open_instance("entry");
 		sp = p->fts_statp;
+		name = getname(p->fts_name);
+		if (name)
+		    xo_emit("{ke:name/%hs}", name);
 		if (f_inode)
-			(void)printf("%*ju ",
+			xo_emit("{t:inode/%*ju} ",
 			    dp->s_inode, (uintmax_t)sp->st_ino);
 		if (f_size)
-			(void)printf("%*jd ",
+			xo_emit("{t:blocks/%*jd} ",
 			    dp->s_block, howmany(sp->st_blocks, blocksize));
 		strmode(sp->st_mode, buf);
 		aclmode(buf, p);
 		np = p->fts_pointer;
-		(void)printf("%s %*u %-*s  %-*s  ", buf, dp->s_nlink,
-		    sp->st_nlink, dp->s_user, np->user, dp->s_group,
-		    np->group);
+		xo_attr("value", "%03o", (int) sp->st_mode & ALLPERMS);
+		if (f_numericonly) {
+			xo_emit("{t:mode/%s}{e:mode_octal/%03o} {t:links/%*u} {td:user/%-*s}{e:user/%ju}  {td:group/%-*s}{e:group/%ju}  ",
+				buf, (int) sp->st_mode & ALLPERMS, dp->s_nlink, sp->st_nlink,
+				dp->s_user, np->user, (uintmax_t)sp->st_uid, dp->s_group, np->group, (uintmax_t)sp->st_gid);
+		} else {
+			xo_emit("{t:mode/%s}{e:mode_octal/%03o} {t:links/%*u} {t:user/%-*s}  {t:group/%-*s}  ",
+				buf, (int) sp->st_mode & ALLPERMS, dp->s_nlink, sp->st_nlink,
+				dp->s_user, np->user, dp->s_group, np->group);
+		}
+		if (S_ISBLK(sp->st_mode))
+			asprintf(&type, "block");
+		if (S_ISCHR(sp->st_mode))
+			asprintf(&type, "character");
+		if (S_ISDIR(sp->st_mode))
+			asprintf(&type, "directory");
+		if (S_ISFIFO(sp->st_mode))
+			asprintf(&type, "fifo");
+		if (S_ISLNK(sp->st_mode))
+			asprintf(&type, "symlink");
+		if (S_ISREG(sp->st_mode))
+			asprintf(&type, "regular");
+		if (S_ISSOCK(sp->st_mode))
+			asprintf(&type, "socket");
+		if (S_ISWHT(sp->st_mode))
+			asprintf(&type, "whiteout");
+		xo_emit("{e:type/%s}", type);
+		free(type);
 		if (f_flags)
-			(void)printf("%-*s ", dp->s_flags, np->flags);
+			xo_emit("{:flags/%-*s} ", dp->s_flags, np->flags);
 		if (f_label)
-			(void)printf("%-*s ", dp->s_label, np->label);
+			xo_emit("{t:label/%-*s} ", dp->s_label, np->label);
 		if (S_ISCHR(sp->st_mode) || S_ISBLK(sp->st_mode))
 			printdev(dp->s_size, sp->st_rdev);
 		else
-			printsize(dp->s_size, sp->st_size);
+			printsize("size", dp->s_size, sp->st_size);
 		if (f_accesstime)
-			printtime(sp->st_atime);
+			printtime("access-time", sp->st_atime);
 		else if (f_birthtime)
-			printtime(sp->st_birthtime);
+			printtime("birth-time", sp->st_birthtime);
 		else if (f_statustime)
-			printtime(sp->st_ctime);
+			printtime("change-time", sp->st_ctime);
 		else
-			printtime(sp->st_mtime);
+			printtime("modify-time", sp->st_mtime);
 #ifdef COLORLS
 		if (f_color)
 			color_printed = colortype(sp->st_mode);
 #endif
-		(void)printname(p->fts_name);
+
+		if (name) {
+		    xo_emit("{dk:name/%hs}", name);
+		    free(name);
+		}
+		
 #ifdef COLORLS
 		if (f_color && color_printed)
 			endcolor(0);
@@ -192,8 +319,10 @@ printlong(const DISPLAY *dp)
 			(void)printtype(sp->st_mode);
 		if (S_ISLNK(sp->st_mode))
 			printlink(p);
-		(void)putchar('\n');
+		xo_close_instance("entry");
+		xo_emit("\n");
 	}
+	xo_close_list("entry");
 }
 
 void
@@ -202,23 +331,27 @@ printstream(const DISPLAY *dp)
 	FTSENT *p;
 	int chcnt;
 
+	xo_open_list("entry");
 	for (p = dp->list, chcnt = 0; p; p = p->fts_link) {
 		if (p->fts_number == NO_PRINT)
 			continue;
 		/* XXX strlen does not take octal escapes into account. */
 		if (strlen(p->fts_name) + chcnt +
 		    (p->fts_link ? 2 : 0) >= (unsigned)termwidth) {
-			putchar('\n');
+			xo_emit("\n");
 			chcnt = 0;
 		}
+		xo_open_instance("file");
 		chcnt += printaname(p, dp->s_inode, dp->s_block);
+		xo_close_instance("file");
 		if (p->fts_link) {
-			printf(", ");
+			xo_emit(", ");
 			chcnt += 2;
 		}
 	}
+	xo_close_list("entry");
 	if (chcnt)
-		putchar('\n');
+		xo_emit("\n");
 }
 
 void
@@ -252,7 +385,6 @@ printcol(const DISPLAY *dp)
 	if (dp->entries > lastentries) {
 		if ((narray =
 		    realloc(array, dp->entries * sizeof(FTSENT *))) == NULL) {
-			warn(NULL);
 			printscol(dp);
 			return;
 		}
@@ -283,17 +415,21 @@ printcol(const DISPLAY *dp)
 
 	if ((dp->list == NULL || dp->list->fts_level != FTS_ROOTLEVEL) &&
 	    (f_longform || f_size)) {
-		(void)printf("total %lu\n", howmany(dp->btotal, blocksize));
+		xo_emit("{L:total} {:total-blocks/%lu}\n",
+			howmany(dp->btotal, blocksize));
 	}
 
+	xo_open_list("entry");
 	base = 0;
 	for (row = 0; row < numrows; ++row) {
 		endcol = colwidth;
 		if (!f_sortacross)
 			base = row;
 		for (col = 0, chcnt = 0; col < numcols; ++col) {
+			xo_open_instance("entry");
 			chcnt += printaname(array[base], dp->s_inode,
 			    dp->s_block);
+			xo_close_instance("entry");
 			if (f_sortacross)
 				base++;
 			else
@@ -304,13 +440,14 @@ printcol(const DISPLAY *dp)
 			    <= endcol) {
 				if (f_sortacross && col + 1 >= numcols)
 					break;
-				(void)putchar(f_notabs ? ' ' : '\t');
+				xo_emit(f_notabs ? " " : "\t");
 				chcnt = cnt;
 			}
 			endcol += colwidth;
 		}
-		(void)putchar('\n');
+		xo_emit("\n");
 	}
+	xo_close_list("entry");
 }
 
 /*
@@ -329,16 +466,16 @@ printaname(const FTSENT *p, u_long inode
 	sp = p->fts_statp;
 	chcnt = 0;
 	if (f_inode)
-		chcnt += printf("%*ju ",
+		chcnt += xo_emit("{t:inode/%*ju} ",
 		    (int)inodefield, (uintmax_t)sp->st_ino);
 	if (f_size)
-		chcnt += printf("%*jd ",
+		chcnt += xo_emit("{t:size/%*jd} ",
 		    (int)sizefield, howmany(sp->st_blocks, blocksize));
 #ifdef COLORLS
 	if (f_color)
 		color_printed = colortype(sp->st_mode);
 #endif
-	chcnt += printname(p->fts_name);
+	chcnt += printname("name", p->fts_name);
 #ifdef COLORLS
 	if (f_color && color_printed)
 		endcolor(0);
@@ -354,14 +491,39 @@ printaname(const FTSENT *p, u_long inode
 static void
 printdev(size_t width, dev_t dev)
 {
+	xo_emit("{:device/%#*jx} ", (u_int)width, (uintmax_t)dev);
+}
 
-	(void)printf("%#*jx ", (u_int)width, (uintmax_t)dev);
+static size_t
+ls_strftime(char *str, size_t len, const char *fmt, const struct tm *tm)
+{
+	char *posb, nfmt[BUFSIZ];
+	const char *format = fmt;
+	size_t ret;
+
+	if ((posb = strstr(fmt, "%b")) != NULL) {
+		if (month_max_size == 0) {
+			compute_abbreviated_month_size();
+		}
+		if (month_max_size > 0) {
+			snprintf(nfmt, sizeof(nfmt),  "%.*s%s%*s%s",
+			    (int)(posb - fmt), fmt,
+			    get_abmon(tm->tm_mon),
+			    (int)padding_for_month[tm->tm_mon],
+			    "",
+			    posb + 2);
+			format = nfmt;
+		}
+	}
+	ret = strftime(str, len, format, tm);
+	return (ret);
 }
 
 static void
-printtime(time_t ftime)
+printtime(const char *field, time_t ftime)
 {
 	char longstring[80];
+	char fmt[BUFSIZ];
 	static time_t now = 0;
 	const char *format;
 	static int d_first = -1;
@@ -383,9 +545,13 @@ printtime(time_t ftime)
 	else
 		/* mmm dd  yyyy || dd mmm  yyyy */
 		format = d_first ? "%e %b  %Y" : "%b %e  %Y";
-	strftime(longstring, sizeof(longstring), format, localtime(&ftime));
-	fputs(longstring, stdout);
-	fputc(' ', stdout);
+	ls_strftime(longstring, sizeof(longstring), format, localtime(&ftime));
+
+	snprintf(fmt, sizeof(fmt), "{d:%s/%%hs} ", field);
+	xo_attr("value", "%ld", (long) ftime);
+	xo_emit(fmt, longstring);
+	snprintf(fmt, sizeof(fmt), "{en:%s/%%ld}", field);
+	xo_emit(fmt, (long) ftime);
 }
 
 static int
@@ -394,7 +560,7 @@ printtype(u_int mode)
 
 	if (f_slash) {
 		if ((mode & S_IFMT) == S_IFDIR) {
-			(void)putchar('/');
+			xo_emit("{D:\\/}{e:type/directory}");
 			return (1);
 		}
 		return (0);
@@ -402,25 +568,25 @@ printtype(u_int mode)
 
 	switch (mode & S_IFMT) {
 	case S_IFDIR:
-		(void)putchar('/');
+		xo_emit("{D:/\\/}{e:type/directory}");
 		return (1);
 	case S_IFIFO:
-		(void)putchar('|');
+		xo_emit("{D:|}{e:type/fifo}");
 		return (1);
 	case S_IFLNK:
-		(void)putchar('@');
+		xo_emit("{D:@}{e:type/link}");
 		return (1);
 	case S_IFSOCK:
-		(void)putchar('=');
+		xo_emit("{D:=}{e:type/socket}");
 		return (1);
 	case S_IFWHT:
-		(void)putchar('%');
+		xo_emit("{D:%%}{e:type/whiteout}");
 		return (1);
 	default:
 		break;
 	}
 	if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
-		(void)putchar('*');
+		xo_emit("{D:*}{e:executable/}");
 		return (1);
 	}
 	return (0);
@@ -430,7 +596,7 @@ printtype(u_int mode)
 static int
 putch(int c)
 {
-	(void)putchar(c);
+	xo_emit("{D:/%c}", c);
 	return 0;
 }
 
@@ -539,7 +705,7 @@ parsecolors(const char *cs)
 			if (c[j] >= '0' && c[j] <= '7') {
 				colors[i].num[j] = c[j] - '0';
 				if (!legacy_warn) {
-					warnx("LSCOLORS should use "
+					xo_warnx("LSCOLORS should use "
 					    "characters a-h instead of 0-9 ("
 					    "see the manual page)");
 				}
@@ -552,7 +718,7 @@ parsecolors(const char *cs)
 			} else if (tolower((unsigned char)c[j]) == 'x')
 				colors[i].num[j] = -1;
 			else {
-				warnx("invalid character '%c' in LSCOLORS"
+				xo_warnx("invalid character '%c' in LSCOLORS"
 				    " env var", c[j]);
 				colors[i].num[j] = -1;
 			}
@@ -584,18 +750,19 @@ printlink(const FTSENT *p)
 		(void)snprintf(name, sizeof(name),
 		    "%s/%s", p->fts_parent->fts_accpath, p->fts_name);
 	if ((lnklen = readlink(name, path, sizeof(path) - 1)) == -1) {
-		(void)fprintf(stderr, "\nls: %s: %s\n", name, strerror(errno));
+		xo_error("\nls: %s: %s\n", name, strerror(errno));
 		return;
 	}
 	path[lnklen] = '\0';
-	(void)printf(" -> ");
-	(void)printname(path);
+	xo_emit(" -> ");
+	(void)printname("target", path);
 }
 
 static void
-printsize(size_t width, off_t bytes)
+printsize(const char *field, size_t width, off_t bytes)
 {
-
+	char fmt[BUFSIZ];
+	
 	if (f_humanval) {
 		/*
 		 * Reserve one space before the size and allocate room for
@@ -605,13 +772,15 @@ printsize(size_t width, off_t bytes)
 
 		humanize_number(buf, sizeof(buf), (int64_t)bytes, "",
 		    HN_AUTOSCALE, HN_B | HN_NOSPACE | HN_DECIMAL);
-		(void)printf("%*s ", (u_int)width, buf);
-	} else if (f_thousands) {		/* with commas */
+		snprintf(fmt, sizeof(fmt), "{:%s/%%%ds} ", field, (int) width);
+		xo_attr("value", "%jd", (intmax_t) bytes);
+		xo_emit(fmt, buf);
+	} else {		/* with commas */
 		/* This format assignment needed to work round gcc bug. */
-		const char *format = "%*j'd ";
-		(void)printf(format, (u_int)width, bytes);
-	} else
-		(void)printf("%*jd ", (u_int)width, bytes);
+		snprintf(fmt, sizeof(fmt), "{:%s/%%%dj%sd} ",
+		     field, (int) width, f_thousands ? "'" : "");
+		xo_emit(fmt, (intmax_t) bytes);
+	}
 }
 
 /*
@@ -654,7 +823,7 @@ aclmode(char *buf, const FTSENT *p)
 			type = ACL_TYPE_NFS4;
 			supports_acls = 1;
 		} else if (ret < 0 && errno != EINVAL) {
-			warn("%s", name);
+			xo_warn("%s", name);
 			return;
 		}
 		if (supports_acls == 0) {
@@ -663,7 +832,7 @@ aclmode(char *buf, const FTSENT *p)
 				type = ACL_TYPE_ACCESS;
 				supports_acls = 1;
 			} else if (ret < 0 && errno != EINVAL) {
-				warn("%s", name);
+				xo_warn("%s", name);
 				return;
 			}
 		}
@@ -672,12 +841,12 @@ aclmode(char *buf, const FTSENT *p)
 		return;
 	facl = acl_get_link_np(name, type);
 	if (facl == NULL) {
-		warn("%s", name);
+		xo_warn("%s", name);
 		return;
 	}
 	if (acl_is_trivial_np(facl, &trivial)) {
 		acl_free(facl);
-		warn("%s", name);
+		xo_warn("%s", name);
 		return;
 	}
 	if (!trivial)
