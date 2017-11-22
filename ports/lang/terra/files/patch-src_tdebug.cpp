--- src/tdebug.cpp.orig	2016-03-26 01:30:20 UTC
+++ src/tdebug.cpp
@@ -151,6 +151,9 @@ static void printstacktrace(void * uap, 
 #ifdef __linux__
         rip = (void*) uc->uc_mcontext.gregs[REG_RIP];
         rbp = (void*) uc->uc_mcontext.gregs[REG_RBP];
+#elif  __FreeBSD__
+        rip = (void*) uc->uc_mcontext.mc_rip;
+        rbp = (void*) uc->uc_mcontext.mc_rbp;
 #else
         rip = (void*)uc->uc_mcontext->__ss.__rip;
         rbp = (void*)uc->uc_mcontext->__ss.__rbp;
