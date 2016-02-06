--- sys/dev/iscsi/icl_proxy.c.orig	2015-12-29 02:29:49 UTC
+++ sys/dev/iscsi/icl_proxy.c
@@ -68,10 +68,10 @@
 #ifdef ICL_KERNEL_PROXY
 
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/sys/dev/iscsi/icl_proxy.c 270891 2014-08-31 20:47:10Z trasz $");
+__FBSDID("$FreeBSD: head/sys/dev/iscsi/icl_proxy.c 270282 2014-08-21 16:08:17Z trasz $");
 
 #include <sys/param.h>
-#include <sys/capability.h>
+#include <sys/capsicum.h>
 #include <sys/condvar.h>
 #include <sys/conf.h>
 #include <sys/kernel.h>
