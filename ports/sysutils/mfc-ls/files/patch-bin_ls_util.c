--- bin/ls/util.c.orig	2015-12-29 08:18:18 UTC
+++ bin/ls/util.c
@@ -36,7 +36,7 @@ static char sccsid[] = "@(#)util.c	8.3 (
 #endif /* not lint */
 #endif
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/bin/ls/util.c 245091 2013-01-06 02:50:38Z andrew $");
+__FBSDID("$FreeBSD: head/bin/ls/util.c 284198 2015-06-10 01:27:38Z marcel $");
 
 #include <sys/types.h>
 #include <sys/stat.h>
@@ -50,13 +50,19 @@ __FBSDID("$FreeBSD: releng/10.1/bin/ls/u
 #include <string.h>
 #include <wchar.h>
 #include <wctype.h>
+#include <libxo/xo.h>
 
 #include "ls.h"
 #include "extern.h"
 
 int
-prn_normal(const char *s)
+prn_normal(const char *field, const char *s)
 {
+	char fmt[_POSIX2_LINE_MAX];
+
+	snprintf(fmt, sizeof(fmt), "{:%s/%%hs}", field);
+	return xo_emit(fmt, s);
+#if 0
 	mbstate_t mbs;
 	wchar_t wc;
 	int i, n;
@@ -83,43 +89,47 @@ prn_normal(const char *s)
 			n += wcwidth(wc);
 	}
 	return (n);
+#endif
 }
 
-int
-prn_printable(const char *s)
+char *
+get_printable(const char *s)
 {
 	mbstate_t mbs;
 	wchar_t wc;
 	int i, n;
 	size_t clen;
+	int slen = strlen(s);
+	char *buf = alloca(slen + 1), *bp = buf;
 
 	memset(&mbs, 0, sizeof(mbs));
 	n = 0;
 	while ((clen = mbrtowc(&wc, s, MB_LEN_MAX, &mbs)) != 0) {
 		if (clen == (size_t)-1) {
-			putchar('?');
+			*bp++ = '?';
 			s++;
 			n++;
 			memset(&mbs, 0, sizeof(mbs));
 			continue;
 		}
 		if (clen == (size_t)-2) {
-			putchar('?');
+			*bp++ = '?';
 			n++;
 			break;
 		}
 		if (!iswprint(wc)) {
-			putchar('?');
+			*bp++ = '?';
 			s += clen;
 			n++;
 			continue;
 		}
 		for (i = 0; i < (int)clen; i++)
-			putchar((unsigned char)s[i]);
+			*bp++ = (unsigned char)s[i];
 		s += clen;
 		n += wcwidth(wc);
 	}
-	return (n);
+	*bp = '\0';
+	return strdup(buf);
 }
 
 /*
@@ -165,8 +175,8 @@ len_octal(const char *s, int len)
 	return (r);
 }
 
-int
-prn_octal(const char *s)
+char *
+get_octal(const char *s)
 {
 	static const char esc[] = "\\\\\"\"\aa\bb\ff\nn\rr\tt\vv";
 	const char *p;
@@ -175,6 +185,8 @@ prn_octal(const char *s)
 	size_t clen;
 	unsigned char ch;
 	int goodchar, i, len, prtlen;
+	int slen = strlen(s);
+	char *buf = alloca(slen * 4 + 1), *bp = buf;
 
 	memset(&mbs, 0, sizeof(mbs));
 	len = 0;
@@ -182,7 +194,7 @@ prn_octal(const char *s)
 		goodchar = clen != (size_t)-1 && clen != (size_t)-2;
 		if (goodchar && iswprint(wc) && wc != L'\"' && wc != L'\\') {
 			for (i = 0; i < (int)clen; i++)
-				putchar((unsigned char)s[i]);
+				*bp++ = (unsigned char)s[i];
 			len += wcwidth(wc);
 		} else if (goodchar && f_octal_escape &&
 #if WCHAR_MIN < 0
@@ -190,8 +202,8 @@ prn_octal(const char *s)
 #endif
 		    wc <= (wchar_t)UCHAR_MAX &&
 		    (p = strchr(esc, (char)wc)) != NULL) {
-			putchar('\\');
-			putchar(p[1]);
+			*bp ++ = '\\';
+			*bp++ = p[1];
 			len += 2;
 		} else {
 			if (goodchar)
@@ -202,10 +214,10 @@ prn_octal(const char *s)
 				prtlen = strlen(s);
 			for (i = 0; i < prtlen; i++) {
 				ch = (unsigned char)s[i];
-				putchar('\\');
-				putchar('0' + (ch >> 6));
-				putchar('0' + ((ch >> 3) & 7));
-				putchar('0' + (ch & 7));
+				*bp++ = '\\';
+				*bp++ = '0' + (ch >> 6);
+				*bp++ = '0' + ((ch >> 3) & 7);
+				*bp++ = '0' + (ch & 7);
 				len += 4;
 			}
 		}
@@ -217,13 +229,15 @@ prn_octal(const char *s)
 		} else
 			s += clen;
 	}
-	return (len);
+
+	*bp = '\0';
+	return strdup(buf);
 }
 
 void
 usage(void)
 {
-	(void)fprintf(stderr,
+	xo_error(
 #ifdef COLORLS
 	"usage: ls [-ABCFGHILPRSTUWZabcdfghiklmnopqrstuwxy1,] [-D format]"
 #else
