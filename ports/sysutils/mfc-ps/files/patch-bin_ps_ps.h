--- bin/ps/ps.h.orig	2015-12-29 09:01:39 UTC
+++ bin/ps/ps.h
@@ -65,6 +65,7 @@ typedef struct var {
 	const char *name;	/* name(s) of variable */
 	const char *header;	/* default header */
 	const char *alias;	/* aliases */
+	const char *field;	/* xo field name */
 #define	COMM	0x01		/* needs exec arguments and environment (XXX) */
 #define	LJUST	0x02		/* left adjust on output (trailing blanks) */
 #define	USER	0x04		/* needs user structure */
