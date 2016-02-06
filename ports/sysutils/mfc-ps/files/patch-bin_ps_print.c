--- bin/ps/print.c.orig	2015-12-29 09:01:39 UTC
+++ bin/ps/print.c
@@ -61,6 +61,7 @@ __FBSDID("$FreeBSD: releng/10.1/bin/ps/p
 #include <string.h>
 #include <unistd.h>
 #include <vis.h>
+#include <libxo/xo.h>
 
 #include "ps.h"
 
@@ -85,15 +86,15 @@ printheader(void)
 		v = vent->var;
 		if (v->flag & LJUST) {
 			if (STAILQ_NEXT(vent, next_ve) == NULL)	/* last one */
-				(void)printf("%s", vent->header);
+				xo_emit("{T:/%s}", vent->header);
 			else
-				(void)printf("%-*s", v->width, vent->header);
+				xo_emit("{T:/%-*s}", v->width, vent->header);
 		} else
-			(void)printf("%*s", v->width, vent->header);
+			xo_emit("{T:/%*s}", v->width, vent->header);
 		if (STAILQ_NEXT(vent, next_ve) != NULL)
-			(void)putchar(' ');
+			xo_emit("{P: }");
 	}
-	(void)putchar('\n');
+	xo_emit("\n");
 }
 
 char *
@@ -102,7 +103,7 @@ arguments(KINFO *k, VARENT *ve)
 	char *vis_args;
 
 	if ((vis_args = malloc(strlen(k->ki_args) * 4 + 1)) == NULL)
-		errx(1, "malloc failed");
+		xo_errx(1, "malloc failed");
 	strvis(vis_args, k->ki_args, VIS_TAB | VIS_NL | VIS_NOSLASH);
 
 	if (STAILQ_NEXT(ve, next_ve) != NULL && strlen(vis_args) > ARGUMENTS_WIDTH)
@@ -130,7 +131,7 @@ command(KINFO *k, VARENT *ve)
 		return (str);
 	}
 	if ((vis_args = malloc(strlen(k->ki_args) * 4 + 1)) == NULL)
-		errx(1, "malloc failed");
+		xo_errx(1, "malloc failed");
 	strvis(vis_args, k->ki_args, VIS_TAB | VIS_NL | VIS_NOSLASH);
 
 	if (STAILQ_NEXT(ve, next_ve) == NULL) {
@@ -139,7 +140,7 @@ command(KINFO *k, VARENT *ve)
 		if (k->ki_env) {
 			if ((vis_env = malloc(strlen(k->ki_env) * 4 + 1))
 			    == NULL)
-				errx(1, "malloc failed");
+				xo_errx(1, "malloc failed");
 			strvis(vis_env, k->ki_env,
 			    VIS_TAB | VIS_NL | VIS_NOSLASH);
 		} else
@@ -215,7 +216,7 @@ state(KINFO *k, VARENT *ve __unused)
 
 	buf = malloc(16);
 	if (buf == NULL)
-		errx(1, "malloc failed");
+		xo_errx(1, "malloc failed");
 
 	flag = k->ki_p->ki_flag;
 	tdflags = k->ki_p->ki_tdflags;	/* XXXKSE */
@@ -392,7 +393,7 @@ started(KINFO *k, VARENT *ve __unused)
 
 	buf = malloc(buflen);
 	if (buf == NULL)
-		errx(1, "malloc failed");
+		xo_errx(1, "malloc failed");
 
 	if (use_ampm < 0)
 		use_ampm = (*nl_langinfo(T_FMT_AMPM) != '\0');
@@ -421,7 +422,7 @@ lstarted(KINFO *k, VARENT *ve __unused)
 
 	buf = malloc(buflen);
 	if (buf == NULL)
-		errx(1, "malloc failed");
+		xo_errx(1, "malloc failed");
 
 	then = k->ki_p->ki_start.tv_sec;
 	(void)strftime(buf, buflen, "%c", localtime(&then));
@@ -767,7 +768,7 @@ printval(void *bp, VAR *v)
 		(void)asprintf(&str, ofmt, ps_pgtok(*(u_long *)bp));
 		break;
 	default:
-		errx(1, "unknown type %d", v->type);
+		xo_errx(1, "unknown type %d", v->type);
 	}
 
 	return (str);
@@ -809,7 +810,7 @@ label(KINFO *k, VARENT *ve __unused)
 
 	string = NULL;
 	if (mac_prepare_process_label(&proclabel) == -1) {
-		warn("mac_prepare_process_label");
+		xo_warn("mac_prepare_process_label");
 		goto out;
 	}
 	error = mac_get_pid(k->ki_p->ki_pid, proclabel);
