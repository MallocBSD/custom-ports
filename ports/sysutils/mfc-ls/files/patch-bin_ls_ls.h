--- bin/ls/ls.h.orig	2015-12-29 08:18:18 UTC
+++ bin/ls/ls.h
@@ -30,13 +30,15 @@
  * SUCH DAMAGE.
  *
  *	from: @(#)ls.h	8.1 (Berkeley) 5/31/93
- * $FreeBSD: releng/10.1/bin/ls/ls.h 242807 2012-11-08 23:45:19Z grog $
+ * $FreeBSD: head/bin/ls/ls.h 285734 2015-07-21 05:03:59Z allanjude $
  */
 
 #define NO_PRINT	1
 
 #define HUMANVALSTR_LEN	5
 
+#define LS_XO_VERSION	"1"
+
 extern long blocksize;		/* block size units */
 
 extern int f_accesstime;	/* use time of last access */
@@ -58,6 +60,7 @@ extern int f_statustime;	/* use time of 
 extern int f_thousands;		/* show file sizes with thousands separators */
 extern char *f_timeformat;	/* user-specified time format */
 extern int f_notabs;		/* don't use tab-separated multi-col output */
+extern int f_numericonly;	/* don't convert uid/gid to name */
 extern int f_type;		/* add type character for non-regular files */
 #ifdef COLORLS
 extern int f_color;		/* add type in color for non-regular files */
