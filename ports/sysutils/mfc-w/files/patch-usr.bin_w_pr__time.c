--- usr.bin/w/pr_time.c.orig	2015-12-29 06:03:45 UTC
+++ usr.bin/w/pr_time.c
@@ -29,7 +29,7 @@
 
 #include <sys/cdefs.h>
 
-__FBSDID("$FreeBSD: releng/10.1/usr.bin/w/pr_time.c 216370 2010-12-11 08:32:16Z joel $");
+__FBSDID("$FreeBSD: head/usr.bin/w/pr_time.c 274151 2014-11-05 23:54:33Z marcel $");
 
 #ifndef lint
 static const char sccsid[] = "@(#)pr_time.c	8.2 (Berkeley) 4/4/94";
@@ -41,6 +41,7 @@ static const char sccsid[] = "@(#)pr_tim
 #include <stdio.h>
 #include <string.h>
 #include <wchar.h>
+#include <libxo/xo.h>
 
 #include "extern.h"
 
@@ -82,12 +83,14 @@ pr_attime(time_t *started, time_t *now)
 	(void)wcsftime(buf, sizeof(buf), fmt, &tp);
 	len = wcslen(buf);
 	width = wcswidth(buf, len);
+	xo_attr("since", "%lu", (unsigned long) *started);
+	xo_attr("delta", "%lu", (unsigned long) diff);
 	if (len == width)
-		(void)wprintf(L"%-7.7ls", buf);
+		xo_emit("{:login-time/%-7.7ls/%ls}", buf);
 	else if (width < 7)
-		(void)wprintf(L"%ls%.*s", buf, 7 - width, "      ");
+	        xo_emit("{:login-time/%ls}%.*s", buf, 7 - width, "      ");
 	else {
-		(void)wprintf(L"%ls", buf);
+		xo_emit("{:login-time/%ls}", buf);
 		offset = width - 7;
 	}
 	return (offset);
@@ -104,7 +107,7 @@ pr_idle(time_t idle)
 	/* If idle more than 36 hours, print as a number of days. */
 	if (idle >= 36 * 3600) {
 		int days = idle / 86400;
-		(void)printf(" %dday%s ", days, days > 1 ? "s" : " " );
+		xo_emit(" {:idle/%dday%s} ", days, days > 1 ? "s" : " " );
 		if (days >= 100)
 			return (2);
 		if (days >= 10)
@@ -113,15 +116,15 @@ pr_idle(time_t idle)
 
 	/* If idle more than an hour, print as HH:MM. */
 	else if (idle >= 3600)
-		(void)printf(" %2d:%02d ",
+		xo_emit(" {:idle/%2d:%02d/} ",
 		    (int)(idle / 3600), (int)((idle % 3600) / 60));
 
 	else if (idle / 60 == 0)
-		(void)printf("     - ");
+		xo_emit("     - ");
 
 	/* Else print the minutes idle. */
 	else
-		(void)printf("    %2d ", (int)(idle / 60));
+		xo_emit("    {:idle/%2d} ", (int)(idle / 60));
 
 	return (0); /* not idle longer than 9 days */
 }
