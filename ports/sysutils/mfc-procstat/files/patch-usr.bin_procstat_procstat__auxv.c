--- usr.bin/procstat/procstat_auxv.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_auxv.c
@@ -1,5 +1,6 @@
 /*-
  * Copyright (c) 2011 Mikolaj Golub
+ * Copyright (c) 2015 Allan Jude <allanjude@freebsd.org>
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
@@ -23,7 +24,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat_auxv.c 249683 2013-04-20 08:15:43Z trociny $
+ * $FreeBSD: head/usr.bin/procstat/procstat_auxv.c 287509 2015-09-06 17:47:03Z allanjude $
  */
 
 #include <sys/param.h>
@@ -43,11 +44,6 @@
 
 #include "procstat.h"
 
-#define	PRINT(name, spec, val)		\
-	printf("%s %-16s " #spec "\n", prefix, #name, (val))
-#define	PRINT_UNKNOWN(type, val)	\
-	printf("%s %16ld %#lx\n", prefix, (long)type, (u_long)(val))
-
 void
 procstat_auxv(struct procstat *procstat, struct kinfo_proc *kipp)
 {
@@ -56,12 +52,18 @@ procstat_auxv(struct procstat *procstat,
 	static char prefix[256];
 
 	if (!hflag)
-		printf("%5s %-16s %-16s %-16s\n", "PID", "COMM", "AUXV", "VALUE");
+		xo_emit("{T:/%5s %-16s %-16s %-16s}\n", "PID", "COMM", "AUXV",
+		    "VALUE");
+
 	auxv = procstat_getauxv(procstat, kipp, &count);
 	if (auxv == NULL)
 		return;
-	snprintf(prefix, sizeof(prefix), "%5d %-16s", kipp->ki_pid,
+        snprintf(prefix, sizeof(prefix), "%5d %-16s", kipp->ki_pid,
+            kipp->ki_comm);
+
+	xo_emit("{e:process_id/%5d/%d}{e:command/%-16s/%s}", kipp->ki_pid,
 	    kipp->ki_comm);
+
 	for (i = 0; i < count; i++) {
 		switch(auxv[i].a_type) {
 		case AT_NULL:
@@ -69,92 +71,119 @@ procstat_auxv(struct procstat *procstat,
 		case AT_IGNORE:
 			break;
 		case AT_EXECFD:
-			PRINT(AT_EXECFD, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_EXECFD/%ld}\n",
+			    prefix, "AT_EXECFD", (long)auxv[i].a_un.a_val);
 			break;
 		case AT_PHDR:
-			PRINT(AT_PHDR, %p, auxv[i].a_un.a_ptr);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_PHDR/%p}\n",
+			    prefix, "AT_PHDR", auxv[i].a_un.a_ptr);
 			break;
 		case AT_PHENT:
-			PRINT(AT_PHENT, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_PHENT/%ld}\n",
+			    prefix, "AT_PHENT", (long)auxv[i].a_un.a_val);
 			break;
 		case AT_PHNUM:
-			PRINT(AT_PHNUM, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_PHNUM/%ld}\n",
+			    prefix, "AT_PHNUM", (long)auxv[i].a_un.a_val);
 			break;
 		case AT_PAGESZ:
-			PRINT(AT_PAGESZ, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_PAGESZ/%ld}\n",
+			    prefix, "AT_PAGESZ", (long)auxv[i].a_un.a_val);
 			break;
 		case AT_BASE:
-			PRINT(AT_BASE, %p, auxv[i].a_un.a_ptr);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_BASE/%p}\n",
+			    prefix, "AT_BASE", auxv[i].a_un.a_ptr);
 			break;
 		case AT_FLAGS:
-			PRINT(AT_FLAGS, %#lx, (u_long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_FLAGS/%#lx}\n",
+			    prefix, "AT_FLAGS", (u_long)auxv[i].a_un.a_val);
 			break;
 		case AT_ENTRY:
-			PRINT(AT_ENTRY, %p, auxv[i].a_un.a_ptr);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_ENTRY/%p}\n",
+			    prefix, "AT_ENTRY", auxv[i].a_un.a_ptr);
 			break;
 #ifdef AT_NOTELF
 		case AT_NOTELF:
-			PRINT(AT_NOTELF, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_NOTELF/%ld}\n",
+			    prefix, "AT_NOTELF", (long)auxv[i].a_un.a_val);
 			break;
 #endif
 #ifdef AT_UID
 		case AT_UID:
-			PRINT(AT_UID, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_UID/%ld}\n",
+			    prefix, "AT_UID", (long)auxv[i].a_un.a_val);
 			break;
 #endif
 #ifdef AT_EUID
 		case AT_EUID:
-			PRINT(AT_EUID, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_EUID/%ld}\n",
+			    prefix, "AT_EUID", (long)auxv[i].a_un.a_val);
 			break;
 #endif
 #ifdef AT_GID
 		case AT_GID:
-			PRINT(AT_GID, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_GID/%ld}\n",
+			    prefix, "AT_GID", (long)auxv[i].a_un.a_val);
 			break;
 #endif
 #ifdef AT_EGID
 		case AT_EGID:
-			PRINT(AT_EGID, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_EGID/%ld}\n",
+			    prefix, "AT_EGID", (long)auxv[i].a_un.a_val);
 			break;
 #endif
 		case AT_EXECPATH:
-			PRINT(AT_EXECPATH, %p, auxv[i].a_un.a_ptr);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_EXECPATH/%p}\n",
+			    prefix, "AT_EXECPATH", auxv[i].a_un.a_ptr);
 			break;
 		case AT_CANARY:
-			PRINT(AT_CANARY, %p, auxv[i].a_un.a_ptr);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_CANARY/%p}\n",
+			    prefix, "AT_CANARY", auxv[i].a_un.a_ptr);
 			break;
 		case AT_CANARYLEN:
-			PRINT(AT_CANARYLEN, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_CANARYLEN/%ld}\n",
+			    prefix, "AT_CANARYLEN", (long)auxv[i].a_un.a_val);
 			break;
 		case AT_OSRELDATE:
-			PRINT(AT_OSRELDATE, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_OSRELDATE/%ld}\n",
+			    prefix, "AT_OSRELDATE", (long)auxv[i].a_un.a_val);
 			break;
 		case AT_NCPUS:
-			PRINT(AT_NCPUS, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_NCPUS/%ld}\n",
+			    prefix, "AT_NCPUS", (long)auxv[i].a_un.a_val);
 			break;
 		case AT_PAGESIZES:
-			PRINT(AT_PAGESIZES, %p, auxv[i].a_un.a_ptr);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_PAGESIZES/%p}\n",
+			    prefix, "AT_PAGESIZES", auxv[i].a_un.a_ptr);
 			break;
 		case AT_PAGESIZESLEN:
-			PRINT(AT_PAGESIZESLEN, %ld, (long)auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}"
+			    "{:AT_PAGESIZESLEN/%ld}\n", prefix,
+			    "AT_PAGESIZESLEN", (long)auxv[i].a_un.a_val);
 			break;
 		case AT_STACKPROT:
 			if ((auxv[i].a_un.a_val & VM_PROT_EXECUTE) != 0)
-				PRINT(AT_STACKPROT, %s, "NONEXECUTABLE");
+				xo_emit("{dw:/%s}{Lw:/%-16s/%s}"
+				    "{:AT_STACKPROT/%s}\n", prefix,
+				    "AT_STACKPROT", "EXECUTABLE");
 			else
-				PRINT(AT_STACKPROT, %s, "EXECUTABLE");
+				xo_emit("{dw:/%s}{Lw:/%-16s/%s}"
+				    "{:AT_STACKPROT/%s}\n", prefix,
+				    "AT_STACKPROT", "NONEXECUTABLE");
 			break;
 #ifdef AT_TIMEKEEP
 		case AT_TIMEKEEP:
-			PRINT(AT_TIMEKEEP, %p, auxv[i].a_un.a_ptr);
+			xo_emit("{dw:/%s}{Lw:/%-16s/%s}{:AT_TIMEKEEP/%p}\n",
+			    prefix, "AT_TIMEKEEP", auxv[i].a_un.a_ptr);
 			break;
 #endif
 		default:
-			PRINT_UNKNOWN(auxv[i].a_type, auxv[i].a_un.a_val);
+			xo_emit("{dw:/%s}{Lw:/%16ld/%ld}{:UNKNOWN/%#lx}\n",
+			    prefix, auxv[i].a_type, auxv[i].a_un.a_val);
 			break;
 		}
 	}
-	printf("\n");
+	xo_emit("\n");
 	procstat_freeauxv(procstat, auxv);
 }
 
