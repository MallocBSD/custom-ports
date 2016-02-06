--- usr.bin/procstat/procstat_sigs.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_sigs.c
@@ -1,5 +1,6 @@
 /*-
  * Copyright (c) 2010 Konstantin Belousov
+ * Copyright (c) 2015 Allan Jude <allanjude@freebsd.org>
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
@@ -23,7 +24,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat_sigs.c 249668 2013-04-20 07:50:59Z trociny $
+ * $FreeBSD: head/usr.bin/procstat/procstat_sigs.c 287486 2015-09-05 17:02:01Z allanjude $
  */
 
 #include <sys/param.h>
@@ -51,70 +52,129 @@ procstat_print_signame(int sig)
 		strlcpy(name, sys_signame[sig], sizeof(name));
 		for (i = 0; name[i] != 0; i++)
 			name[i] = toupper(name[i]);
-		printf("%-7s ", name);
+		xo_emit("{d:signal/%-7s/%s} ", name);
+		xo_open_container(name);
+	} else {
+		xo_emit("{d:signal/%-7d/%d} ", sig);
+		snprintf(name, 12, "%d", sig);
+		xo_open_container(name);
+	}
+}
+
+static void
+procstat_close_signame(int sig)
+{
+	char name[12];
+	int i;
+
+	if (!nflag && sig < sys_nsig) {
+		strlcpy(name, sys_signame[sig], sizeof(name));
+		for (i = 0; name[i] != 0; i++)
+			name[i] = toupper(name[i]);
+		xo_close_container(name);
 	} else
-		printf("%-7d ", sig);
+		snprintf(name, 12, "%d", sig);
+		xo_close_container(name);
 }
 
 static void
 procstat_print_sig(const sigset_t *set, int sig, char flag)
 {
-
-	printf("%c", sigismember(set, sig) ? flag : '-');
+	xo_emit("{d:sigmember/%c}", sigismember(set, sig) ? flag : '-');
+	switch (flag) {
+		case 'B':
+			xo_emit("{en:mask/%s}", sigismember(set, sig) ?
+			    "true" : "false");
+			break;
+		case 'C':
+			xo_emit("{en:catch/%s}", sigismember(set, sig) ?
+			    "true" : "false");
+			break;
+		case 'P':
+			xo_emit("{en:list/%s}", sigismember(set, sig) ?
+			    "true" : "false");
+			break;
+		case 'I':
+			xo_emit("{en:ignore/%s}", sigismember(set, sig) ?
+			    "true" : "false");
+			break;
+		default:
+			xo_emit("{en:unknown/%s}", sigismember(set, sig) ?
+			    "true" : "false");
+			break;
+	}
 }
 
 void
 procstat_sigs(struct procstat *prstat __unused, struct kinfo_proc *kipp)
 {
 	int j;
-	pid_t pid;
 
-	pid = kipp->ki_pid;
 	if (!hflag)
-		printf("%5s %-16s %-7s %4s\n", "PID", "COMM", "SIG", "FLAGS");
+		xo_emit("{T:/%5s %-16s %-7s %4s}\n", "PID", "COMM", "SIG",
+		    "FLAGS");
 
+	xo_emit("{ek:process_id/%5d/%d}", kipp->ki_pid);
+	xo_emit("{e:command/%-16s/%s}", kipp->ki_comm);
+	xo_open_container("signals");
 	for (j = 1; j <= _SIG_MAXSIG; j++) {
-		printf("%5d ", pid);
-		printf("%-16s ", kipp->ki_comm);
+		xo_emit("{dk:process_id/%5d/%d} ", kipp->ki_pid);
+		xo_emit("{d:command/%-16s/%s} ", kipp->ki_comm);
 		procstat_print_signame(j);
-		printf(" ");
+		xo_emit(" ");
 		procstat_print_sig(&kipp->ki_siglist, j, 'P');
 		procstat_print_sig(&kipp->ki_sigignore, j, 'I');
 		procstat_print_sig(&kipp->ki_sigcatch, j, 'C');
-		printf("\n");
+		procstat_close_signame(j);
+		xo_emit("\n");
 	}
+	xo_close_container("signals");
 }
 
 void
 procstat_threads_sigs(struct procstat *procstat, struct kinfo_proc *kipp)
 {
 	struct kinfo_proc *kip;
-	pid_t pid;
 	int j;
 	unsigned int count, i;
+	char *threadid;
 
-	pid = kipp->ki_pid;
 	if (!hflag)
-		printf("%5s %6s %-16s %-7s %4s\n", "PID", "TID", "COMM",
+		xo_emit("{T:/%5s %6s %-16s %-7s %4s}\n", "PID", "TID", "COMM",
 		     "SIG", "FLAGS");
 
 	kip = procstat_getprocs(procstat, KERN_PROC_PID | KERN_PROC_INC_THREAD,
-	    pid, &count);
+	    kipp->ki_pid, &count);
 	if (kip == NULL)
 		return;
+	xo_emit("{ek:process_id/%5d/%d}", kipp->ki_pid);
+	xo_emit("{e:command/%-16s/%s}", kipp->ki_comm);
+	xo_open_container("threads");
 	kinfo_proc_sort(kip, count);
 	for (i = 0; i < count; i++) {
 		kipp = &kip[i];
+		asprintf(&threadid, "%d", kipp->ki_tid);
+		if (threadid == NULL)
+			xo_errc(1, ENOMEM, "Failed to allocate memory in "
+			    "procstat_threads_sigs()");
+		xo_open_container(threadid);
+		xo_emit("{e:thread_id/%6d/%d}", kipp->ki_tid);
+		xo_open_container("signals");
 		for (j = 1; j <= _SIG_MAXSIG; j++) {
-			printf("%5d ", pid);
-			printf("%6d ", kipp->ki_tid);
-			printf("%-16s ", kipp->ki_comm);
+			xo_emit("{dk:process_id/%5d/%d} ", kipp->ki_pid);
+			xo_emit("{d:thread_id/%6d/%d} ", kipp->ki_tid);
+			xo_emit("{d:command/%-16s/%s} ", kipp->ki_comm);
 			procstat_print_signame(j);
-			printf(" ");
+			xo_emit(" ");
 			procstat_print_sig(&kipp->ki_siglist, j, 'P');
 			procstat_print_sig(&kipp->ki_sigmask, j, 'B');
-			printf("\n");
+			procstat_close_signame(j);
+			xo_emit("\n");
 		}
+		xo_close_container("signals");
+		xo_close_container(threadid);
+		free(threadid);
 	}
+	xo_close_container("threads");
 	procstat_freeprocs(procstat, kip);
 }
