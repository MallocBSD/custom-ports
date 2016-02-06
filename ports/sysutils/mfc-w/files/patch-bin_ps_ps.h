--- bin/ps/ps.h.orig	2015-12-29 06:03:45 UTC
+++ bin/ps/ps.h
@@ -27,7 +27,7 @@
  * SUCH DAMAGE.
  *
  *	@(#)ps.h	8.1 (Berkeley) 5/31/93
- * $FreeBSD: releng/10.1/bin/ps/ps.h 225868 2011-09-29 06:31:42Z trasz $
+ * $FreeBSD: head/bin/ps/ps.h 283304 2015-05-22 23:07:55Z marcel $
  */
 
 #include <sys/queue.h>
@@ -65,6 +65,7 @@ typedef struct var {
 	const char *name;	/* name(s) of variable */
 	const char *header;	/* default header */
 	const char *alias;	/* aliases */
+	const char *field;	/* xo field name */
 #define	COMM	0x01		/* needs exec arguments and environment (XXX) */
 #define	LJUST	0x02		/* left adjust on output (trailing blanks) */
 #define	USER	0x04		/* needs user structure */
