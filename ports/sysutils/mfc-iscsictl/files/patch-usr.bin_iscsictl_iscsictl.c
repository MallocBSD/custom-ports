--- usr.bin/iscsictl/iscsictl.c.orig	2015-12-29 02:29:49 UTC
+++ usr.bin/iscsictl/iscsictl.c
@@ -29,14 +29,13 @@
  */
 
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/usr.bin/iscsictl/iscsictl.c 270888 2014-08-31 20:21:08Z trasz $");
+__FBSDID("$FreeBSD: head/usr.bin/iscsictl/iscsictl.c 289459 2015-10-17 16:05:42Z trasz $");
 
 #include <sys/ioctl.h>
 #include <sys/param.h>
 #include <sys/linker.h>
 #include <assert.h>
 #include <ctype.h>
-#include <err.h>
 #include <errno.h>
 #include <fcntl.h>
 #include <limits.h>
@@ -44,6 +43,7 @@ __FBSDID("$FreeBSD: releng/10.1/usr.bin/
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
+#include <libxo/xo.h>
 
 #include <iscsi_ioctl.h>
 #include "iscsictl.h"
@@ -55,7 +55,7 @@ conf_new(void)
 
 	conf = calloc(1, sizeof(*conf));
 	if (conf == NULL)
-		err(1, "calloc");
+		xo_err(1, "calloc");
 
 	TAILQ_INIT(&conf->conf_targets);
 
@@ -83,7 +83,7 @@ target_new(struct conf *conf)
 
 	targ = calloc(1, sizeof(*targ));
 	if (targ == NULL)
-		err(1, "calloc");
+		xo_err(1, "calloc");
 	targ->t_conf = conf;
 	TAILQ_INSERT_TAIL(&conf->conf_targets, targ, t_next);
 
@@ -110,12 +110,12 @@ default_initiator_name(void)
 
 	name = calloc(1, namelen + 1);
 	if (name == NULL)
-		err(1, "calloc");
+		xo_err(1, "calloc");
 	strcpy(name, DEFAULT_IQN);
 	error = gethostname(name + strlen(DEFAULT_IQN),
 	    namelen - strlen(DEFAULT_IQN));
 	if (error != 0)
-		err(1, "gethostname");
+		xo_err(1, "gethostname");
 
 	return (name);
 }
@@ -158,7 +158,7 @@ valid_iscsi_name(const char *name)
 	int i;
 
 	if (strlen(name) >= MAX_NAME_LEN) {
-		warnx("overlong name for \"%s\"; max length allowed "
+		xo_warnx("overlong name for \"%s\"; max length allowed "
 		    "by iSCSI specification is %d characters",
 		    name, MAX_NAME_LEN);
 		return (false);
@@ -172,13 +172,13 @@ valid_iscsi_name(const char *name)
 		for (i = strlen("iqn."); name[i] != '\0'; i++) {
 			/*
 			 * XXX: We should verify UTF-8 normalisation, as defined
-			 * 	by 3.2.6.2: iSCSI Name Encoding.
+			 *      by 3.2.6.2: iSCSI Name Encoding.
 			 */
 			if (isalnum(name[i]))
 				continue;
 			if (name[i] == '-' || name[i] == '.' || name[i] == ':')
 				continue;
-			warnx("invalid character \"%c\" in iSCSI name "
+			xo_warnx("invalid character \"%c\" in iSCSI name "
 			    "\"%s\"; allowed characters are letters, digits, "
 			    "'-', '.', and ':'", name[i], name);
 			break;
@@ -188,12 +188,12 @@ valid_iscsi_name(const char *name)
 		 */
 	} else if (strncasecmp(name, "eui.", strlen("eui.")) == 0) {
 		if (strlen(name) != strlen("eui.") + 16)
-			warnx("invalid iSCSI name \"%s\"; the \"eui.\" "
+			xo_warnx("invalid iSCSI name \"%s\"; the \"eui.\" "
 			    "should be followed by exactly 16 hexadecimal "
 			    "digits", name);
 		for (i = strlen("eui."); name[i] != '\0'; i++) {
 			if (!valid_hex(name[i])) {
-				warnx("invalid character \"%c\" in iSCSI "
+				xo_warnx("invalid character \"%c\" in iSCSI "
 				    "name \"%s\"; allowed characters are 1-9 "
 				    "and A-F", name[i], name);
 				break;
@@ -201,19 +201,19 @@ valid_iscsi_name(const char *name)
 		}
 	} else if (strncasecmp(name, "naa.", strlen("naa.")) == 0) {
 		if (strlen(name) > strlen("naa.") + 32)
-			warnx("invalid iSCSI name \"%s\"; the \"naa.\" "
+			xo_warnx("invalid iSCSI name \"%s\"; the \"naa.\" "
 			    "should be followed by at most 32 hexadecimal "
 			    "digits", name);
 		for (i = strlen("naa."); name[i] != '\0'; i++) {
 			if (!valid_hex(name[i])) {
-				warnx("invalid character \"%c\" in ISCSI "
+				xo_warnx("invalid character \"%c\" in ISCSI "
 				    "name \"%s\"; allowed characters are 1-9 "
 				    "and A-F", name[i], name);
 				break;
 			}
 		}
 	} else {
-		warnx("invalid iSCSI name \"%s\"; should start with "
+		xo_warnx("invalid iSCSI name \"%s\"; should start with "
 		    "either \".iqn\", \"eui.\", or \"naa.\"",
 		    name);
 	}
@@ -231,26 +231,26 @@ conf_verify(struct conf *conf)
 			targ->t_session_type = SESSION_TYPE_NORMAL;
 		if (targ->t_session_type == SESSION_TYPE_NORMAL &&
 		    targ->t_name == NULL)
-			errx(1, "missing TargetName for target \"%s\"",
+			xo_errx(1, "missing TargetName for target \"%s\"",
 			    targ->t_nickname);
 		if (targ->t_session_type == SESSION_TYPE_DISCOVERY &&
 		    targ->t_name != NULL)
-			errx(1, "cannot specify TargetName for discovery "
+			xo_errx(1, "cannot specify TargetName for discovery "
 			    "sessions for target \"%s\"", targ->t_nickname);
 		if (targ->t_name != NULL) {
 			if (valid_iscsi_name(targ->t_name) == false)
-				errx(1, "invalid target name \"%s\"",
+				xo_errx(1, "invalid target name \"%s\"",
 				    targ->t_name);
 		}
 		if (targ->t_protocol == PROTOCOL_UNSPECIFIED)
 			targ->t_protocol = PROTOCOL_ISCSI;
 		if (targ->t_address == NULL)
-			errx(1, "missing TargetAddress for target \"%s\"",
+			xo_errx(1, "missing TargetAddress for target \"%s\"",
 			    targ->t_nickname);
 		if (targ->t_initiator_name == NULL)
 			targ->t_initiator_name = default_initiator_name();
 		if (valid_iscsi_name(targ->t_initiator_name) == false)
-			errx(1, "invalid initiator name \"%s\"",
+			xo_errx(1, "invalid initiator name \"%s\"",
 			    targ->t_initiator_name);
 		if (targ->t_header_digest == DIGEST_UNSPECIFIED)
 			targ->t_header_digest = DIGEST_NONE;
@@ -268,19 +268,19 @@ conf_verify(struct conf *conf)
 		}
 		if (targ->t_auth_method == AUTH_METHOD_CHAP) {
 			if (targ->t_user == NULL) {
-				errx(1, "missing chapIName for target \"%s\"",
+				xo_errx(1, "missing chapIName for target \"%s\"",
 				    targ->t_nickname);
 			}
 			if (targ->t_secret == NULL)
-				errx(1, "missing chapSecret for target \"%s\"",
+				xo_errx(1, "missing chapSecret for target \"%s\"",
 				    targ->t_nickname);
 			if (targ->t_mutual_user != NULL ||
 			    targ->t_mutual_secret != NULL) {
 				if (targ->t_mutual_user == NULL)
-					errx(1, "missing tgtChapName for "
+					xo_errx(1, "missing tgtChapName for "
 					    "target \"%s\"", targ->t_nickname);
 				if (targ->t_mutual_secret == NULL)
-					errx(1, "missing tgtChapSecret for "
+					xo_errx(1, "missing tgtChapSecret for "
 					    "target \"%s\"", targ->t_nickname);
 			}
 		}
@@ -327,6 +327,9 @@ conf_from_target(struct iscsi_session_co
 		conf->isc_discovery = 1;
 	if (targ->t_protocol == PROTOCOL_ISER)
 		conf->isc_iser = 1;
+	if (targ->t_offload != NULL)
+		strlcpy(conf->isc_offload, targ->t_offload,
+		    sizeof(conf->isc_offload));
 	if (targ->t_header_digest == DIGEST_CRC32C)
 		conf->isc_header_digest = ISCSI_DIGEST_CRC32C;
 	else
@@ -347,7 +350,7 @@ kernel_add(int iscsi_fd, const struct ta
 	conf_from_target(&isa.isa_conf, targ);
 	error = ioctl(iscsi_fd, ISCSISADD, &isa);
 	if (error != 0)
-		warn("ISCSISADD");
+		xo_warn("ISCSISADD");
 	return (error);
 }
 
@@ -362,7 +365,7 @@ kernel_modify(int iscsi_fd, unsigned int
 	conf_from_target(&ism.ism_conf, targ);
 	error = ioctl(iscsi_fd, ISCSISMODIFY, &ism);
 	if (error != 0)
-		warn("ISCSISMODIFY");
+		xo_warn("ISCSISMODIFY");
 	return (error);
 }
 
@@ -382,7 +385,7 @@ kernel_modify_some(int iscsi_fd, unsigne
 		states = realloc(states,
 		    nentries * sizeof(struct iscsi_session_state));
 		if (states == NULL)
-			err(1, "realloc");
+			xo_err(1, "realloc");
 
 		memset(&isl, 0, sizeof(isl));
 		isl.isl_nentries = nentries;
@@ -396,7 +399,7 @@ kernel_modify_some(int iscsi_fd, unsigne
 		break;
 	}
 	if (error != 0)
-		errx(1, "ISCSISLIST");
+		xo_errx(1, "ISCSISLIST");
 
 	for (i = 0; i < isl.isl_nentries; i++) {
 		state = &states[i];
@@ -405,7 +408,7 @@ kernel_modify_some(int iscsi_fd, unsigne
 			break;
 	}
 	if (i == isl.isl_nentries)
-		errx(1, "session-id %u not found", session_id);
+		xo_errx(1, "session-id %u not found", session_id);
 
 	conf = &state->iss_conf;
 
@@ -424,7 +427,7 @@ kernel_modify_some(int iscsi_fd, unsigne
 	memcpy(&ism.ism_conf, conf, sizeof(ism.ism_conf));
 	error = ioctl(iscsi_fd, ISCSISMODIFY, &ism);
 	if (error != 0)
-		warn("ISCSISMODIFY");
+		xo_warn("ISCSISMODIFY");
 }
 
 static int
@@ -437,7 +440,7 @@ kernel_remove(int iscsi_fd, const struct
 	conf_from_target(&isr.isr_conf, targ);
 	error = ioctl(iscsi_fd, ISCSISREMOVE, &isr);
 	if (error != 0)
-		warn("ISCSISREMOVE");
+		xo_warn("ISCSISREMOVE");
 	return (error);
 }
 
@@ -459,7 +462,7 @@ kernel_list(int iscsi_fd, const struct t
 		states = realloc(states,
 		    nentries * sizeof(struct iscsi_session_state));
 		if (states == NULL)
-			err(1, "realloc");
+			xo_err(1, "realloc");
 
 		memset(&isl, 0, sizeof(isl));
 		isl.isl_nentries = nentries;
@@ -473,89 +476,184 @@ kernel_list(int iscsi_fd, const struct t
 		break;
 	}
 	if (error != 0) {
-		warn("ISCSISLIST");
+		xo_warn("ISCSISLIST");
 		return (error);
 	}
 
 	if (verbose != 0) {
+		xo_open_list("session");
 		for (i = 0; i < isl.isl_nentries; i++) {
 			state = &states[i];
 			conf = &state->iss_conf;
 
-			printf("Session ID:       %u\n", state->iss_id);
-			printf("Initiator name:   %s\n", conf->isc_initiator);
-			printf("Initiator portal: %s\n",
-			    conf->isc_initiator_addr);
-			printf("Initiator alias:  %s\n",
-			    conf->isc_initiator_alias);
-			printf("Target name:      %s\n", conf->isc_target);
-			printf("Target portal:    %s\n",
-			    conf->isc_target_addr);
-			printf("Target alias:     %s\n",
-			    state->iss_target_alias);
-			printf("User:             %s\n", conf->isc_user);
-			printf("Secret:           %s\n", conf->isc_secret);
-			printf("Mutual user:      %s\n",
-			    conf->isc_mutual_user);
-			printf("Mutual secret:    %s\n",
-			    conf->isc_mutual_secret);
-			printf("Session type:     %s\n",
+			xo_open_instance("session");
+
+			/*
+			 * Display-only modifier as this information
+			 * is also present within the 'session' container
+			 */
+			xo_emit("{L:/%-18s}{V:sessionId/%u}\n",
+			    "Session ID:", state->iss_id);
+
+			xo_open_container("initiator");
+			xo_emit("{L:/%-18s}{V:name/%s}\n",
+			    "Initiator name:", conf->isc_initiator);
+			xo_emit("{L:/%-18s}{V:portal/%s}\n",
+			    "Initiator portal:", conf->isc_initiator_addr);
+			xo_emit("{L:/%-18s}{V:alias/%s}\n",
+			    "Initiator alias:", conf->isc_initiator_alias);
+			xo_close_container("initiator");
+
+			xo_open_container("target");
+			xo_emit("{L:/%-18s}{V:name/%s}\n",
+			    "Target name:", conf->isc_target);
+			xo_emit("{L:/%-18s}{V:portal/%s}\n",
+			    "Target portal:", conf->isc_target_addr);
+			xo_emit("{L:/%-18s}{V:alias/%s}\n",
+			    "Target alias:", state->iss_target_alias);
+			xo_close_container("target");
+
+			xo_open_container("auth");
+			xo_emit("{L:/%-18s}{V:user/%s}\n",
+			    "User:", conf->isc_user);
+			xo_emit("{L:/%-18s}{V:secret/%s}\n",
+			    "Secret:", conf->isc_secret);
+			xo_emit("{L:/%-18s}{V:mutualUser/%s}\n",
+			    "Mutual user:", conf->isc_mutual_user);
+			xo_emit("{L:/%-18s}{V:mutualSecret/%s}\n",
+			    "Mutual secret:", conf->isc_mutual_secret);
+			xo_close_container("auth");
+
+			xo_emit("{L:/%-18s}{V:type/%s}\n",
+			    "Session type:",
 			    conf->isc_discovery ? "Discovery" : "Normal");
-			printf("Session state:    %s\n",
-			    state->iss_connected ?
-			    "Connected" : "Disconnected");
-			printf("Failure reason:   %s\n", state->iss_reason);
-			printf("Header digest:    %s\n",
+			xo_emit("{L:/%-18s}{V:state/%s}\n",
+			    "Session state:",
+			    state->iss_connected ? "Connected" : "Disconnected");
+			xo_emit("{L:/%-18s}{V:failureReason/%s}\n",
+			    "Failure reason:", state->iss_reason);
+			xo_emit("{L:/%-18s}{V:headerDigest/%s}\n",
+			    "Header digest:",
 			    state->iss_header_digest == ISCSI_DIGEST_CRC32C ?
 			    "CRC32C" : "None");
-			printf("Data digest:      %s\n",
+			xo_emit("{L:/%-18s}{V:dataDigest/%s}\n",
+			    "Data digest:",
 			    state->iss_data_digest == ISCSI_DIGEST_CRC32C ?
 			    "CRC32C" : "None");
-			printf("DataSegmentLen:   %d\n",
-			    state->iss_max_data_segment_length);
-			printf("ImmediateData:    %s\n",
-			    state->iss_immediate_data ? "Yes" : "No");
-			printf("iSER (RDMA):      %s\n",
-			    conf->isc_iser ? "Yes" : "No");
-			printf("Device nodes:     ");
+			xo_emit("{L:/%-18s}{V:dataSegmentLen/%d}\n",
+			    "DataSegmentLen:", state->iss_max_data_segment_length);
+			xo_emit("{L:/%-18s}{V:immediateData/%s}\n",
+			    "ImmediateData:", state->iss_immediate_data ? "Yes" : "No");
+			xo_emit("{L:/%-18s}{V:iSER/%s}\n",
+			    "iSER (RDMA):", conf->isc_iser ? "Yes" : "No");
+			xo_emit("{L:/%-18s}{V:offloadDriver/%s}\n",
+			    "Offload driver:", state->iss_offload);
+			xo_emit("{L:/%-18s}",
+			    "Device nodes:");
 			print_periphs(state->iss_id);
-			printf("\n\n");
+			xo_emit("\n\n");
+			xo_close_instance("session");
 		}
+		xo_close_list("session");
 	} else {
-		printf("%-36s %-16s %s\n",
+		xo_emit("{T:/%-36s} {T:/%-16s} {T:/%s}\n",
 		    "Target name", "Target portal", "State");
+
+		if (isl.isl_nentries != 0)
+			xo_open_list("session");
 		for (i = 0; i < isl.isl_nentries; i++) {
+
 			state = &states[i];
 			conf = &state->iss_conf;
 
-			printf("%-36s %-16s ",
+			xo_open_instance("session");
+			xo_emit("{V:name/%-36s/%s} {V:portal/%-16s/%s} ",
 			    conf->isc_target, conf->isc_target_addr);
 
 			if (state->iss_reason[0] != '\0') {
-				printf("%s\n", state->iss_reason);
+				xo_emit("{V:state/%s}\n", state->iss_reason);
 			} else {
 				if (conf->isc_discovery) {
-					printf("Discovery\n");
+					xo_emit("{V:state}\n", "Discovery");
 				} else if (state->iss_connected) {
-					printf("Connected: ");
+					xo_emit("{V:state}: ", "Connected");
 					print_periphs(state->iss_id);
-					printf("\n");
+					xo_emit("\n");
 				} else {
-					printf("Disconnected\n");
+					xo_emit("{V:state}\n", "Disconnected");
 				}
 			}
+			xo_close_instance("session");
 		}
+		if (isl.isl_nentries != 0)
+			xo_close_list("session");
 	}
 
 	return (0);
 }
 
+static int
+kernel_wait(int iscsi_fd, int timeout)
+{
+	struct iscsi_session_state *states = NULL;
+	const struct iscsi_session_state *state;
+	struct iscsi_session_list isl;
+	unsigned int i, nentries = 1;
+	bool all_connected;
+	int error;
+
+	for (;;) {
+		for (;;) {
+			states = realloc(states,
+			    nentries * sizeof(struct iscsi_session_state));
+			if (states == NULL)
+				xo_err(1, "realloc");
+
+			memset(&isl, 0, sizeof(isl));
+			isl.isl_nentries = nentries;
+			isl.isl_pstates = states;
+
+			error = ioctl(iscsi_fd, ISCSISLIST, &isl);
+			if (error != 0 && errno == EMSGSIZE) {
+				nentries *= 4;
+				continue;
+			}
+			break;
+		}
+		if (error != 0) {
+			xo_warn("ISCSISLIST");
+			return (error);
+		}
+
+		all_connected = true;
+		for (i = 0; i < isl.isl_nentries; i++) {
+			state = &states[i];
+
+			if (!state->iss_connected) {
+				all_connected = false;
+				break;
+			}
+		}
+
+		if (all_connected)
+			return (0);
+
+		sleep(1);
+
+		if (timeout > 0) {
+			timeout--;
+			if (timeout == 0)
+				return (1);
+		}
+	}
+}
+
 static void
 usage(void)
 {
 
 	fprintf(stderr, "usage: iscsictl -A -p portal -t target "
-	    "[-u user -s secret]\n");
+	    "[-u user -s secret] [-w timeout]\n");
 	fprintf(stderr, "       iscsictl -A -d discovery-host "
 	    "[-u user -s secret]\n");
 	fprintf(stderr, "       iscsictl -A -a [-c path]\n");
@@ -567,7 +665,7 @@ usage(void)
 	fprintf(stderr, "       iscsictl -R [-p portal] [-t target]\n");
 	fprintf(stderr, "       iscsictl -R -a\n");
 	fprintf(stderr, "       iscsictl -R -n nickname [-c path]\n");
-	fprintf(stderr, "       iscsictl -L [-v]\n");
+	fprintf(stderr, "       iscsictl -L [-v] [-w timeout]\n");
 	exit(1);
 }
 
@@ -578,7 +676,7 @@ checked_strdup(const char *s)
 
 	c = strdup(s);
 	if (c == NULL)
-		err(1, "strdup");
+		xo_err(1, "strdup");
 	return (c);
 }
 
@@ -589,6 +687,7 @@ main(int argc, char **argv)
 	const char *conf_path = DEFAULT_CONFIG_PATH;
 	char *nickname = NULL, *discovery_host = NULL, *portal = NULL,
 	     *target = NULL, *user = NULL, *secret = NULL;
+	int timeout = -1;
 	long long session_id = -1;
 	char *end;
 	int ch, error, iscsi_fd, retval, saved_errno;
@@ -596,7 +695,10 @@ main(int argc, char **argv)
 	struct conf *conf;
 	struct target *targ;
 
-	while ((ch = getopt(argc, argv, "AMRLac:d:i:n:p:t:u:s:v")) != -1) {
+	argc = xo_parse_args(argc, argv);
+	xo_open_container("iscsictl");
+
+	while ((ch = getopt(argc, argv, "AMRLac:d:i:n:p:t:u:s:vw:")) != -1) {
 		switch (ch) {
 		case 'A':
 			Aflag = 1;
@@ -622,11 +724,11 @@ main(int argc, char **argv)
 		case 'i':
 			session_id = strtol(optarg, &end, 10);
 			if ((size_t)(end - optarg) != strlen(optarg))
-				errx(1, "trailing characters after session-id");
+				xo_errx(1, "trailing characters after session-id");
 			if (session_id < 0)
-				errx(1, "session-id cannot be negative");
+				xo_errx(1, "session-id cannot be negative");
 			if (session_id > UINT_MAX)
-				errx(1, "session-id cannot be greater than %u",
+				xo_errx(1, "session-id cannot be greater than %u",
 				    UINT_MAX);
 			break;
 		case 'n':
@@ -647,6 +749,13 @@ main(int argc, char **argv)
 		case 'v':
 			vflag = 1;
 			break;
+		case 'w':
+			timeout = strtol(optarg, &end, 10);
+			if ((size_t)(end - optarg) != strlen(optarg))
+				xo_errx(1, "trailing characters after timeout");
+			if (timeout < 0)
+				xo_errx(1, "timeout cannot be negative");
+			break;
 		case '?':
 		default:
 			usage();
@@ -659,7 +768,7 @@ main(int argc, char **argv)
 	if (Aflag + Mflag + Rflag + Lflag == 0)
 		Lflag = 1;
 	if (Aflag + Mflag + Rflag + Lflag > 1)
-		errx(1, "at most one of -A, -M, -R, or -L may be specified");
+		xo_errx(1, "at most one of -A, -M, -R, or -L may be specified");
 
 	/*
 	 * Note that we ignore unneccessary/inapplicable "-c" flag; so that
@@ -669,127 +778,126 @@ main(int argc, char **argv)
 	if (Aflag != 0) {
 		if (aflag != 0) {
 			if (portal != NULL)
-				errx(1, "-a and -p and mutually exclusive");
+				xo_errx(1, "-a and -p and mutually exclusive");
 			if (target != NULL)
-				errx(1, "-a and -t and mutually exclusive");
+				xo_errx(1, "-a and -t and mutually exclusive");
 			if (user != NULL)
-				errx(1, "-a and -u and mutually exclusive");
+				xo_errx(1, "-a and -u and mutually exclusive");
 			if (secret != NULL)
-				errx(1, "-a and -s and mutually exclusive");
+				xo_errx(1, "-a and -s and mutually exclusive");
 			if (nickname != NULL)
-				errx(1, "-a and -n and mutually exclusive");
+				xo_errx(1, "-a and -n and mutually exclusive");
 			if (discovery_host != NULL)
-				errx(1, "-a and -d and mutually exclusive");
+				xo_errx(1, "-a and -d and mutually exclusive");
 		} else if (nickname != NULL) {
 			if (portal != NULL)
-				errx(1, "-n and -p and mutually exclusive");
+				xo_errx(1, "-n and -p and mutually exclusive");
 			if (target != NULL)
-				errx(1, "-n and -t and mutually exclusive");
+				xo_errx(1, "-n and -t and mutually exclusive");
 			if (user != NULL)
-				errx(1, "-n and -u and mutually exclusive");
+				xo_errx(1, "-n and -u and mutually exclusive");
 			if (secret != NULL)
-				errx(1, "-n and -s and mutually exclusive");
+				xo_errx(1, "-n and -s and mutually exclusive");
 			if (discovery_host != NULL)
-				errx(1, "-n and -d and mutually exclusive");
+				xo_errx(1, "-n and -d and mutually exclusive");
 		} else if (discovery_host != NULL) {
 			if (portal != NULL)
-				errx(1, "-d and -p and mutually exclusive");
+				xo_errx(1, "-d and -p and mutually exclusive");
 			if (target != NULL)
-				errx(1, "-d and -t and mutually exclusive");
+				xo_errx(1, "-d and -t and mutually exclusive");
 		} else {
 			if (target == NULL && portal == NULL)
-				errx(1, "must specify -a, -n or -t/-p");
+				xo_errx(1, "must specify -a, -n or -t/-p");
 
 			if (target != NULL && portal == NULL)
-				errx(1, "-t must always be used with -p");
+				xo_errx(1, "-t must always be used with -p");
 			if (portal != NULL && target == NULL)
-				errx(1, "-p must always be used with -t");
+				xo_errx(1, "-p must always be used with -t");
 		}
 
 		if (user != NULL && secret == NULL)
-			errx(1, "-u must always be used with -s");
+			xo_errx(1, "-u must always be used with -s");
 		if (secret != NULL && user == NULL)
-			errx(1, "-s must always be used with -u");
+			xo_errx(1, "-s must always be used with -u");
 
 		if (session_id != -1)
-			errx(1, "-i cannot be used with -A");
+			xo_errx(1, "-i cannot be used with -A");
 		if (vflag != 0)
-			errx(1, "-v cannot be used with -A");
+			xo_errx(1, "-v cannot be used with -A");
 
 	} else if (Mflag != 0) {
 		if (session_id == -1)
-			errx(1, "-M requires -i");
+			xo_errx(1, "-M requires -i");
 
 		if (discovery_host != NULL)
-			errx(1, "-M and -d are mutually exclusive");
+			xo_errx(1, "-M and -d are mutually exclusive");
 		if (aflag != 0)
-			errx(1, "-M and -a are mutually exclusive");
+			xo_errx(1, "-M and -a are mutually exclusive");
 		if (nickname != NULL) {
 			if (portal != NULL)
-				errx(1, "-n and -p and mutually exclusive");
+				xo_errx(1, "-n and -p and mutually exclusive");
 			if (target != NULL)
-				errx(1, "-n and -t and mutually exclusive");
+				xo_errx(1, "-n and -t and mutually exclusive");
 			if (user != NULL)
-				errx(1, "-n and -u and mutually exclusive");
+				xo_errx(1, "-n and -u and mutually exclusive");
 			if (secret != NULL)
-				errx(1, "-n and -s and mutually exclusive");
+				xo_errx(1, "-n and -s and mutually exclusive");
 		}
 
 		if (vflag != 0)
-			errx(1, "-v cannot be used with -M");
+			xo_errx(1, "-v cannot be used with -M");
+		if (timeout != -1)
+			xo_errx(1, "-w cannot be used with -M");
 
 	} else if (Rflag != 0) {
 		if (user != NULL)
-			errx(1, "-R and -u are mutually exclusive");
+			xo_errx(1, "-R and -u are mutually exclusive");
 		if (secret != NULL)
-			errx(1, "-R and -s are mutually exclusive");
+			xo_errx(1, "-R and -s are mutually exclusive");
 		if (discovery_host != NULL)
-			errx(1, "-R and -d are mutually exclusive");
+			xo_errx(1, "-R and -d are mutually exclusive");
 
 		if (aflag != 0) {
 			if (portal != NULL)
-				errx(1, "-a and -p and mutually exclusive");
+				xo_errx(1, "-a and -p and mutually exclusive");
 			if (target != NULL)
-				errx(1, "-a and -t and mutually exclusive");
+				xo_errx(1, "-a and -t and mutually exclusive");
 			if (nickname != NULL)
-				errx(1, "-a and -n and mutually exclusive");
+				xo_errx(1, "-a and -n and mutually exclusive");
 		} else if (nickname != NULL) {
 			if (portal != NULL)
-				errx(1, "-n and -p and mutually exclusive");
-			if (target != NULL)
-				errx(1, "-n and -t and mutually exclusive");
-		} else if (portal != NULL) {
+				xo_errx(1, "-n and -p and mutually exclusive");
 			if (target != NULL)
-				errx(1, "-p and -t and mutually exclusive");
-		} else if (target != NULL) {
-			if (portal != NULL)
-				errx(1, "-t and -p and mutually exclusive");
-		} else
-			errx(1, "must specify either -a, -n, -t, or -p");
+				xo_errx(1, "-n and -t and mutually exclusive");
+		} else if (target == NULL && portal == NULL) {
+			xo_errx(1, "must specify either -a, -n, -t, or -p");
+		}
 
 		if (session_id != -1)
-			errx(1, "-i cannot be used with -R");
+			xo_errx(1, "-i cannot be used with -R");
 		if (vflag != 0)
-			errx(1, "-v cannot be used with -R");
+			xo_errx(1, "-v cannot be used with -R");
+		if (timeout != -1)
+			xo_errx(1, "-w cannot be used with -R");
 
 	} else {
 		assert(Lflag != 0);
 
 		if (portal != NULL)
-			errx(1, "-L and -p and mutually exclusive");
+			xo_errx(1, "-L and -p and mutually exclusive");
 		if (target != NULL)
-			errx(1, "-L and -t and mutually exclusive");
+			xo_errx(1, "-L and -t and mutually exclusive");
 		if (user != NULL)
-			errx(1, "-L and -u and mutually exclusive");
+			xo_errx(1, "-L and -u and mutually exclusive");
 		if (secret != NULL)
-			errx(1, "-L and -s and mutually exclusive");
+			xo_errx(1, "-L and -s and mutually exclusive");
 		if (nickname != NULL)
-			errx(1, "-L and -n and mutually exclusive");
+			xo_errx(1, "-L and -n and mutually exclusive");
 		if (discovery_host != NULL)
-			errx(1, "-L and -d and mutually exclusive");
+			xo_errx(1, "-L and -d and mutually exclusive");
 
 		if (session_id != -1)
-			errx(1, "-i cannot be used with -L");
+			xo_errx(1, "-i cannot be used with -L");
 	}
 
 	iscsi_fd = open(ISCSI_PATH, O_RDWR);
@@ -802,7 +910,7 @@ main(int argc, char **argv)
 			errno = saved_errno;
 	}
 	if (iscsi_fd < 0)
-		err(1, "failed to open %s", ISCSI_PATH);
+		xo_err(1, "failed to open %s", ISCSI_PATH);
 
 	if (Aflag != 0 && aflag != 0) {
 		conf = conf_new_from_file(conf_path);
@@ -813,7 +921,7 @@ main(int argc, char **argv)
 		conf = conf_new_from_file(conf_path);
 		targ = target_find(conf, nickname);
 		if (targ == NULL)
-			errx(1, "target %s not found in %s",
+			xo_errx(1, "target %s not found in %s",
 			    nickname, conf_path);
 
 		if (Aflag != 0)
@@ -830,7 +938,7 @@ main(int argc, char **argv)
 	} else {
 		if (Aflag != 0 && target != NULL) {
 			if (valid_iscsi_name(target) == false)
-				errx(1, "invalid target name \"%s\"", target);
+				xo_errx(1, "invalid target name \"%s\"", target);
 		}
 		conf = conf_new();
 		targ = target_new(conf);
@@ -856,11 +964,17 @@ main(int argc, char **argv)
 			failed += kernel_list(iscsi_fd, targ, vflag);
 	}
 
+	if (timeout != -1)
+		failed += kernel_wait(iscsi_fd, timeout);
+
 	error = close(iscsi_fd);
 	if (error != 0)
-		err(1, "close");
+		xo_err(1, "close");
 
 	if (failed > 0)
 		return (1);
+
+	xo_close_container("iscsictl");
+	xo_finish();
 	return (0);
 }
