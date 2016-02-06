--- sbin/savecore/savecore.c.orig	2015-12-29 09:25:09 UTC
+++ sbin/savecore/savecore.c
@@ -61,7 +61,7 @@
  */
 
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/sbin/savecore/savecore.c 272434 2014-10-02 18:11:13Z bdrewery $");
+__FBSDID("$FreeBSD: head/sbin/savecore/savecore.c 290520 2015-11-07 23:27:03Z cem $");
 
 #include <sys/param.h>
 #include <sys/disk.h>
@@ -80,6 +80,7 @@ __FBSDID("$FreeBSD: releng/10.1/sbin/sav
 #include <syslog.h>
 #include <time.h>
 #include <unistd.h>
+#include <libxo/xo.h>
 
 /* The size of the buffer used for I/O. */
 #define	BUFFERSIZE	(1024*1024)
@@ -98,29 +99,28 @@ static sig_atomic_t got_siginfo;
 static void infohandler(int);
 
 static void
-printheader(FILE *f, const struct kerneldumpheader *h, const char *device,
+printheader(xo_handle_t *xo, const struct kerneldumpheader *h, const char *device,
     int bounds, const int status)
 {
 	uint64_t dumplen;
 	time_t t;
 	const char *stat_str;
 
-	fprintf(f, "Dump header from device %s\n", device);
-	fprintf(f, "  Architecture: %s\n", h->architecture);
-	fprintf(f, "  Architecture Version: %u\n",
-	    dtoh32(h->architectureversion));
+	xo_flush_h(xo);
+	xo_emit_h(xo, "{Lwc:Dump header from device}{:dump_device/%s}\n", device);
+	xo_emit_h(xo, "{P:  }{Lwc:Architecture}{:architecture/%s}\n", h->architecture);
+	xo_emit_h(xo, "{P:  }{Lwc:Architecture Version}{:architecture_version/%u}\n", dtoh32(h->architectureversion));
 	dumplen = dtoh64(h->dumplength);
-	fprintf(f, "  Dump Length: %lldB (%lld MB)\n", (long long)dumplen,
-	    (long long)(dumplen >> 20));
-	fprintf(f, "  Blocksize: %d\n", dtoh32(h->blocksize));
+	xo_emit_h(xo, "{P:  }{Lwc:Dump Length}{:dump_length_bytes/%lld}\n", (long long)dumplen);
+	xo_emit_h(xo, "{P:  }{Lwc:Blocksize}{:blocksize/%d}\n", dtoh32(h->blocksize));
 	t = dtoh64(h->dumptime);
-	fprintf(f, "  Dumptime: %s", ctime(&t));
-	fprintf(f, "  Hostname: %s\n", h->hostname);
-	fprintf(f, "  Magic: %s\n", h->magic);
-	fprintf(f, "  Version String: %s", h->versionstring);
-	fprintf(f, "  Panic String: %s\n", h->panicstring);
-	fprintf(f, "  Dump Parity: %u\n", h->parity);
-	fprintf(f, "  Bounds: %d\n", bounds);
+	xo_emit_h(xo, "{P:  }{Lwc:Dumptime}{:dumptime/%s}", ctime(&t));
+	xo_emit_h(xo, "{P:  }{Lwc:Hostname}{:hostname/%s}\n", h->hostname);
+	xo_emit_h(xo, "{P:  }{Lwc:Magic}{:magic/%s}\n", h->magic);
+	xo_emit_h(xo, "{P:  }{Lwc:Version String}{:version_string/%s}", h->versionstring);
+	xo_emit_h(xo, "{P:  }{Lwc:Panic String}{:panic_string/%s}\n", h->panicstring);
+	xo_emit_h(xo, "{P:  }{Lwc:Dump Parity}{:dump_parity/%u}\n", h->parity);
+	xo_emit_h(xo, "{P:  }{Lwc:Bounds}{:bounds/%d}\n", bounds);
 
 	switch(status) {
 	case STATUS_BAD:
@@ -132,8 +132,8 @@ printheader(FILE *f, const struct kernel
 	default:
 		stat_str = "unknown";
 	}
-	fprintf(f, "  Dump Status: %s\n", stat_str);
-	fflush(f);
+	xo_emit_h(xo, "{P:  }{Lwc:Dump Status}{:dump_status/%s}\n", stat_str);
+	xo_flush_h(xo);
 }
 
 static int
@@ -434,6 +434,7 @@ DoTextdumpFile(int fd, off_t dumpsize, o
 static void
 DoFile(const char *savedir, const char *device)
 {
+	xo_handle_t *xostdout, *xoinfo;
 	static char infoname[PATH_MAX], corename[PATH_MAX], linkname[PATH_MAX];
 	static char *buf = NULL;
 	struct kerneldumpheader kdhf, kdhl;
@@ -442,13 +443,19 @@ DoFile(const char *savedir, const char *
 	mode_t oumask;
 	int fd, fdinfo, error;
 	int bounds, status;
-	u_int sectorsize;
+	u_int sectorsize, xostyle;
 	int istextdump;
 
 	bounds = getbounds();
 	mediasize = 0;
 	status = STATUS_UNKNOWN;
 
+	xostdout = xo_create_to_file(stdout, XO_STYLE_TEXT, 0);
+	if (xostdout == NULL) {
+		syslog(LOG_ERR, "%s: %m", infoname);
+		return;
+	}
+
 	if (maxdumps > 0 && bounds == maxdumps)
 		bounds = 0;
 
@@ -484,9 +491,8 @@ DoFile(const char *savedir, const char *
 	}
 
 	lasthd = mediasize - sectorsize;
-	lseek(fd, lasthd, SEEK_SET);
-	error = read(fd, &kdhl, sizeof kdhl);
-	if (error != sizeof kdhl) {
+	if (lseek(fd, lasthd, SEEK_SET) != lasthd ||
+	    read(fd, &kdhl, sizeof(kdhl)) != sizeof(kdhl)) {
 		syslog(LOG_ERR,
 		    "error reading last dump header at offset %lld in %s: %m",
 		    (long long)lasthd, device);
@@ -562,9 +568,8 @@ DoFile(const char *savedir, const char *
 	}
 	dumpsize = dtoh64(kdhl.dumplength);
 	firsthd = lasthd - dumpsize - sizeof kdhf;
-	lseek(fd, firsthd, SEEK_SET);
-	error = read(fd, &kdhf, sizeof kdhf);
-	if (error != sizeof kdhf) {
+	if (lseek(fd, firsthd, SEEK_SET) != firsthd ||
+	    read(fd, &kdhf, sizeof(kdhf)) != sizeof(kdhf)) {
 		syslog(LOG_ERR,
 		    "error reading first dump header at offset %lld in %s: %m",
 		    (long long)firsthd, device);
@@ -574,10 +579,10 @@ DoFile(const char *savedir, const char *
 
 	if (verbose >= 2) {
 		printf("First dump headers:\n");
-		printheader(stdout, &kdhf, device, bounds, -1);
+		printheader(xostdout, &kdhf, device, bounds, -1);
 
 		printf("\nLast dump headers:\n");
-		printheader(stdout, &kdhl, device, bounds, -1);
+		printheader(xostdout, &kdhl, device, bounds, -1);
 		printf("\n");
 	}
 
@@ -599,7 +604,8 @@ DoFile(const char *savedir, const char *
 	}
 
 	if (kdhl.panicstring[0])
-		syslog(LOG_ALERT, "reboot after panic: %s", kdhl.panicstring);
+		syslog(LOG_ALERT, "reboot after panic: %*s",
+		    (int)sizeof(kdhl.panicstring), kdhl.panicstring);
 	else
 		syslog(LOG_ALERT, "reboot");
 
@@ -626,6 +632,7 @@ DoFile(const char *savedir, const char *
 		nerr++;
 		goto closefd;
 	}
+
 	oumask = umask(S_IRWXG|S_IRWXO); /* Restrict access to the core file.*/
 	if (compress) {
 		snprintf(corename, sizeof(corename), "%s.%d.gz",
@@ -649,13 +656,25 @@ DoFile(const char *savedir, const char *
 	if (info == NULL) {
 		syslog(LOG_ERR, "fdopen failed: %m");
 		nerr++;
-		goto closefd;
+		goto closeall;
+	}
+
+	xostyle = xo_get_style(NULL);
+	xoinfo = xo_create_to_file(info, xostyle, 0);
+	if (xoinfo == NULL) {
+		syslog(LOG_ERR, "%s: %m", infoname);
+		nerr++;
+		goto closeall;
 	}
+	xo_open_container_h(xoinfo, "crashdump");
 
 	if (verbose)
-		printheader(stdout, &kdhl, device, bounds, status);
+		printheader(xostdout, &kdhl, device, bounds, status);
 
-	printheader(info, &kdhl, device, bounds, status);
+	printheader(xoinfo, &kdhl, device, bounds, status);
+	xo_close_container_h(xoinfo, "crashdump");
+	xo_flush_h(xoinfo);
+	xo_finish_h(xoinfo);
 	fclose(info);
 
 	syslog(LOG_NOTICE, "writing %score to %s/%s",
@@ -706,12 +725,13 @@ nuke:
 		if (verbose)
 			printf("clearing dump header\n");
 		memcpy(kdhl.magic, KERNELDUMPMAGIC_CLEARED, sizeof kdhl.magic);
-		lseek(fd, lasthd, SEEK_SET);
-		error = write(fd, &kdhl, sizeof kdhl);
-		if (error != sizeof kdhl)
+		if (lseek(fd, lasthd, SEEK_SET) != lasthd ||
+		    write(fd, &kdhl, sizeof(kdhl)) != sizeof(kdhl))
 			syslog(LOG_ERR,
 			    "error while clearing the dump header: %m");
 	}
+	xo_close_container_h(xostdout, "crashdump");
+	xo_finish_h(xostdout);
 	close(fd);
 	return;
 
@@ -725,7 +745,7 @@ closefd:
 static void
 usage(void)
 {
-	fprintf(stderr, "%s\n%s\n%s\n",
+	xo_error("%s\n%s\n%s\n",
 	    "usage: savecore -c [-v] [device ...]",
 	    "       savecore -C [-v] [device ...]",
 	    "       savecore [-fkvz] [-m maxdumps] [directory [device ...]]");
@@ -745,6 +765,10 @@ main(int argc, char **argv)
 	openlog("savecore", LOG_PERROR, LOG_DAEMON);
 	signal(SIGINFO, infohandler);
 
+	argc = xo_parse_args(argc, argv);
+	if (argc < 0)
+		exit(1);
+
 	while ((ch = getopt(argc, argv, "Ccfkm:vz")) != -1)
 		switch(ch) {
 		case 'C':
@@ -812,15 +836,18 @@ main(int argc, char **argv)
 	/* Emit minimal output. */
 	if (nfound == 0) {
 		if (checkfor) {
-			printf("No dump exists\n");
+			if (verbose)
+				printf("No dump exists\n");
 			exit(1);
 		}
-		syslog(LOG_WARNING, "no dumps found");
-	}
-	else if (nsaved == 0) {
-		if (nerr != 0)
-			syslog(LOG_WARNING, "unsaved dumps found but not saved");
-		else
+		if (verbose)
+			syslog(LOG_WARNING, "no dumps found");
+	} else if (nsaved == 0) {
+		if (nerr != 0) {
+			if (verbose)
+				syslog(LOG_WARNING, "unsaved dumps found but not saved");
+			exit(1);
+		} else if (verbose)
 			syslog(LOG_WARNING, "no unsaved dumps found");
 	}
 
