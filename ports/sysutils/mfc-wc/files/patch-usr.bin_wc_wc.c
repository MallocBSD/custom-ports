--- usr.bin/wc/wc.c.orig	2015-12-29 09:27:09 UTC
+++ usr.bin/wc/wc.c
@@ -40,7 +40,7 @@ static char sccsid[] = "@(#)wc.c	8.1 (Be
 #endif
 
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/usr.bin/wc/wc.c 227201 2011-11-06 08:19:00Z ed $");
+__FBSDID("$FreeBSD: head/usr.bin/wc/wc.c 281617 2015-04-16 21:44:35Z bdrewery $");
 
 #include <sys/param.h>
 #include <sys/stat.h>
@@ -57,10 +57,12 @@ __FBSDID("$FreeBSD: releng/10.1/usr.bin/
 #include <unistd.h>
 #include <wchar.h>
 #include <wctype.h>
+#include <libxo/xo.h>
 
 static uintmax_t tlinect, twordct, tcharct, tlongline;
 static int doline, doword, dochar, domulti, dolongline;
 static volatile sig_atomic_t siginfo;
+static xo_handle_t *stderr_handle;
 
 static void	show_cnt(const char *file, uintmax_t linect, uintmax_t wordct,
 		    uintmax_t charct, uintmax_t llct);
@@ -74,6 +76,14 @@ siginfo_handler(int sig __unused)
 	siginfo = 1;
 }
 
+static void
+reset_siginfo(void)
+{
+
+	signal(SIGINFO, SIG_DFL);
+	siginfo = 0;
+}
+
 int
 main(int argc, char *argv[])
 {
@@ -81,6 +91,10 @@ main(int argc, char *argv[])
 
 	(void) setlocale(LC_CTYPE, "");
 
+	argc = xo_parse_args(argc, argv);
+	if (argc < 0)
+		return (argc);
+
 	while ((ch = getopt(argc, argv, "clmwL")) != -1)
 		switch((char)ch) {
 		case 'l':
@@ -113,21 +127,37 @@ main(int argc, char *argv[])
 	if (doline + doword + dochar + domulti + dolongline == 0)
 		doline = doword = dochar = 1;
 
+	stderr_handle = xo_create_to_file(stderr, XO_STYLE_TEXT, 0);
+	xo_open_container("wc");
+	xo_open_list("file");
+
 	errors = 0;
 	total = 0;
 	if (!*argv) {
+	 	xo_open_instance("file");
 		if (cnt((char *)NULL) != 0)
 			++errors;
+	 	xo_close_instance("file");
 	} else {
 		do {
+	 		xo_open_instance("file");
 			if (cnt(*argv) != 0)
 				++errors;
+	 		xo_close_instance("file");
 			++total;
 		} while(*++argv);
 	}
 
-	if (total > 1)
+	xo_close_list("file");
+
+	if (total > 1) {
+		xo_open_container("total");
 		show_cnt("total", tlinect, twordct, tcharct, tlongline);
+		xo_close_container("total");
+	}
+
+	xo_close_container("wc");
+	xo_finish();
 	exit(errors == 0 ? 0 : 1);
 }
 
@@ -135,27 +165,27 @@ static void
 show_cnt(const char *file, uintmax_t linect, uintmax_t wordct,
     uintmax_t charct, uintmax_t llct)
 {
-	FILE *out;
+	xo_handle_t *xop;
 
 	if (!siginfo)
-		out = stdout;
+		xop = NULL;
 	else {
-		out = stderr;
+		xop = stderr_handle;
 		siginfo = 0;
 	}
 
 	if (doline)
-		(void)fprintf(out, " %7ju", linect);
+		xo_emit_h(xop, " {:lines/%7ju/%ju}", linect);
 	if (doword)
-		(void)fprintf(out, " %7ju", wordct);
+		xo_emit_h(xop, " {:words/%7ju/%ju}", wordct);
 	if (dochar || domulti)
-		(void)fprintf(out, " %7ju", charct);
+		xo_emit_h(xop, " {:characters/%7ju/%ju}", charct);
 	if (dolongline)
-		(void)fprintf(out, " %7ju", llct);
+		xo_emit_h(xop, " {:long-lines/%7ju/%ju}", llct);
 	if (file != NULL)
-		(void)fprintf(out, " %s\n", file);
+		xo_emit_h(xop, " {:filename/%s}\n", file);
 	else
-		(void)fprintf(out, "\n");
+		xo_emit_h(xop, "\n");
 }
 
 static int
@@ -176,7 +206,7 @@ cnt(const char *file)
 		fd = STDIN_FILENO;
 	else {
 		if ((fd = open(file, O_RDONLY, 0)) < 0) {
-			warn("%s: open", file);
+			xo_warn("%s: open", file);
 			return (1);
 		}
 		if (doword || (domulti && MB_CUR_MAX != 1))
@@ -189,7 +219,7 @@ cnt(const char *file)
 		if (doline) {
 			while ((len = read(fd, buf, MAXBSIZE))) {
 				if (len == -1) {
-					warn("%s: read", file);
+					xo_warn("%s: read", file);
 					(void)close(fd);
 					return (1);
 				}
@@ -207,6 +237,7 @@ cnt(const char *file)
 					} else
 						tmpll++;
 			}
+			reset_siginfo();
 			tlinect += linect;
 			if (dochar)
 				tcharct += charct;
@@ -224,11 +255,12 @@ cnt(const char *file)
 		 */
 		if (dochar || domulti) {
 			if (fstat(fd, &sb)) {
-				warn("%s: fstat", file);
+				xo_warn("%s: fstat", file);
 				(void)close(fd);
 				return (1);
 			}
 			if (S_ISREG(sb.st_mode)) {
+				reset_siginfo();
 				charct = sb.st_size;
 				show_cnt(file, linect, wordct, charct, llct);
 				tcharct += charct;
@@ -244,7 +276,7 @@ word:	gotsp = 1;
 	memset(&mbs, 0, sizeof(mbs));
 	while ((len = read(fd, buf, MAXBSIZE)) != 0) {
 		if (len == -1) {
-			warn("%s: read", file != NULL ? file : "stdin");
+			xo_warn("%s: read", file != NULL ? file : "stdin");
 			(void)close(fd);
 			return (1);
 		}
@@ -259,7 +291,7 @@ word:	gotsp = 1;
 			    (size_t)-1) {
 				if (!warned) {
 					errno = EILSEQ;
-					warn("%s",
+					xo_warn("%s",
 					    file != NULL ? file : "stdin");
 					warned = 1;
 				}
@@ -289,9 +321,10 @@ word:	gotsp = 1;
 			}
 		}
 	}
+	reset_siginfo();
 	if (domulti && MB_CUR_MAX > 1)
 		if (mbrtowc(NULL, NULL, 0, &mbs) == (size_t)-1 && !warned)
-			warn("%s", file != NULL ? file : "stdin");
+			xo_warn("%s", file != NULL ? file : "stdin");
 	if (doline)
 		tlinect += linect;
 	if (doword)
@@ -310,6 +343,6 @@ word:	gotsp = 1;
 static void
 usage(void)
 {
-	(void)fprintf(stderr, "usage: wc [-Lclmw] [file ...]\n");
+	xo_error("usage: wc [-Lclmw] [file ...]\n");
 	exit(1);
 }
