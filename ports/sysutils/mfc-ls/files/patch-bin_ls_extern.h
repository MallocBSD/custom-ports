--- bin/ls/extern.h.orig	2015-12-29 08:18:18 UTC
+++ bin/ls/extern.h
@@ -27,7 +27,7 @@
  * SUCH DAMAGE.
  *
  *	from: @(#)extern.h	8.1 (Berkeley) 5/31/93
- * $FreeBSD: releng/10.1/bin/ls/extern.h 242807 2012-11-08 23:45:19Z grog $
+ * $FreeBSD: head/bin/ls/extern.h 284198 2015-06-10 01:27:38Z marcel $
  */
 
 int	 acccmp(const FTSENT *, const FTSENT *);
@@ -45,14 +45,17 @@ int	 revsizecmp(const FTSENT *, const FT
 
 void	 printcol(const DISPLAY *);
 void	 printlong(const DISPLAY *);
-int	 printname(const char *);
+int	 printname(const char *, const char *);
 void	 printscol(const DISPLAY *);
 void	 printstream(const DISPLAY *);
 void	 usage(void);
-int	 prn_normal(const char *);
+int	 prn_normal(const char *, const char *);
+char *	 getname(const char *);
 size_t	 len_octal(const char *, int);
-int	 prn_octal(const char *);
-int	 prn_printable(const char *);
+int	 prn_octal(const char *, const char *);
+char *	 get_octal(const char *);
+int	 prn_printable(const char *, const char *);
+char *	 get_printable(const char *);
 #ifdef COLORLS
 void	 parsecolors(const char *cs);
 void	 colorquit(int);
