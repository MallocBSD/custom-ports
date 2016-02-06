--- usr.bin/procstat/procstat_files.c.orig	2015-12-29 08:36:41 UTC
+++ usr.bin/procstat/procstat_files.c
@@ -1,5 +1,5 @@
 /*-
- * Copyright (c) 2007-2011 Robert N. M. Watson
+ * Copyright (c) 2015 Allan Jude <allanjude@freebsd.org>
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
@@ -23,7 +23,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/procstat/procstat_files.c 268517 2014-07-11 00:11:24Z delphij $
+ * $FreeBSD: head/usr.bin/procstat/procstat_files.c 287486 2015-09-05 17:02:01Z allanjude $
  */
 
 #include <sys/param.h>
@@ -271,19 +271,22 @@ print_capability(cap_rights_t *rightsp, 
 	width = 0;
 	for (i = width_capability(rightsp); i < capwidth; i++) {
 		if (i != 0)
-			printf(" ");
+			xo_emit(" ");
 		else
-			printf("-");
+			xo_emit("-");
 	}
+	xo_open_list("capabilities");
 	for (i = 0; i < cap_desc_count; i++) {
 		if (cap_rights_is_set(rightsp, cap_desc[i].cd_right)) {
-			printf("%s%s", count ? "," : "", cap_desc[i].cd_desc);
+			xo_emit("{D:/%s}{l:capabilities/%s}", count ? "," : "",
+			    cap_desc[i].cd_desc);
 			width += strlen(cap_desc[i].cd_desc);
 			if (count)
 				width++;
 			count++;
 		}
 	}
+	xo_close_list("capabilities");
 }
 
 void
@@ -296,6 +299,8 @@ procstat_files(struct procstat *procstat
 	struct vnstat vn;
 	u_int capwidth, width;
 	int error;
+	char src_addr[PATH_MAX];
+	char dst_addr[PATH_MAX];
 
 	/*
 	 * To print the header in capability mode, we need to know the width
@@ -317,84 +322,103 @@ procstat_files(struct procstat *procstat
 
 	if (!hflag) {
 		if (Cflag)
-			printf("%5s %-16s %4s %1s %-9s %-*s "
-			    "%-3s %-12s\n", "PID", "COMM", "FD", "T",
+			xo_emit("{T:/%5s %-16s %5s %1s %-8s %-*s "
+			    "%-3s %-12s}\n", "PID", "COMM", "FD", "T",
 			    "FLAGS", capwidth, "CAPABILITIES", "PRO",
 			    "NAME");
 		else
-			printf("%5s %-16s %4s %1s %1s %-9s "
-			    "%3s %7s %-3s %-12s\n", "PID", "COMM", "FD", "T",
+			xo_emit("{T:/%5s %-16s %5s %1s %1s %-8s "
+			    "%3s %7s %-3s %-12s}\n", "PID", "COMM", "FD", "T",
 			    "V", "FLAGS", "REF", "OFFSET", "PRO", "NAME");
 	}
 
 	if (head == NULL)
 		return;
+	xo_emit("{ek:process_id/%5d/%d}", kipp->ki_pid);
+	xo_emit("{e:command/%-16s/%s}", kipp->ki_comm);
+	xo_open_list("files");
 	STAILQ_FOREACH(fst, head, next) {
-		printf("%5d ", kipp->ki_pid);
-		printf("%-16s ", kipp->ki_comm);
+		xo_open_instance("files");
+		xo_emit("{dk:process_id/%5d/%d} ", kipp->ki_pid);
+		xo_emit("{d:command/%-16s/%s} ", kipp->ki_comm);
 		if (fst->fs_uflags & PS_FST_UFLAG_CTTY)
-			printf(" ctty ");
+			xo_emit("{P: }{:fd/%s} ", "ctty");
 		else if (fst->fs_uflags & PS_FST_UFLAG_CDIR)
-			printf("  cwd ");
+			xo_emit("{P:  }{:fd/%s} ", "cwd");
 		else if (fst->fs_uflags & PS_FST_UFLAG_JAIL)
-			printf(" jail ");
+			xo_emit("{P: }{:fd/%s} ", "jail");
 		else if (fst->fs_uflags & PS_FST_UFLAG_RDIR)
-			printf(" root ");
+			xo_emit("{P: }{:fd/%s} ", "root");
 		else if (fst->fs_uflags & PS_FST_UFLAG_TEXT)
-			printf(" text ");
+			xo_emit("{P: }{:fd/%s} ", "text");
 		else if (fst->fs_uflags & PS_FST_UFLAG_TRACE)
-			printf("trace ");
+			xo_emit("{:fd/%s} ", "trace");
 		else
-			printf("%5d ", fst->fs_fd);
+			xo_emit("{:fd/%5d} ", fst->fs_fd);
 
 		switch (fst->fs_type) {
 		case PS_FST_TYPE_VNODE:
 			str = "v";
+			xo_emit("{eq:fd_type/vnode}");
 			break;
 
 		case PS_FST_TYPE_SOCKET:
 			str = "s";
+			xo_emit("{eq:fd_type/socket}");
 			break;
 
 		case PS_FST_TYPE_PIPE:
 			str = "p";
+			xo_emit("{eq:fd_type/pipe}");
 			break;
 
 		case PS_FST_TYPE_FIFO:
 			str = "f";
+			xo_emit("{eq:fd_type/fifo}");
 			break;
 
 		case PS_FST_TYPE_KQUEUE:
 			str = "k";
+			xo_emit("{eq:fd_type/kqueue}");
 			break;
 
 		case PS_FST_TYPE_CRYPTO:
 			str = "c";
+			xo_emit("{eq:fd_type/crypto}");
 			break;
 
 		case PS_FST_TYPE_MQUEUE:
 			str = "m";
+			xo_emit("{eq:fd_type/mqueue}");
 			break;
 
 		case PS_FST_TYPE_SHM:
 			str = "h";
+			xo_emit("{eq:fd_type/shm}");
 			break;
 
 		case PS_FST_TYPE_PTS:
 			str = "t";
+			xo_emit("{eq:fd_type/pts}");
 			break;
 
 		case PS_FST_TYPE_SEM:
 			str = "e";
+			xo_emit("{eq:fd_type/sem}");
 			break;
 
 		case PS_FST_TYPE_NONE:
+			str = "?";
+			xo_emit("{eq:fd_type/none}");
+			break;
+
 		case PS_FST_TYPE_UNKNOWN:
 		default:
 			str = "?";
+			xo_emit("{eq:fd_type/unknown}");
 			break;
 		}
-		printf("%1s ", str);
+		xo_emit("{d:fd_type/%1s/%s} ", str);
 		if (!Cflag) {
 			str = "-";
 			if (fst->fs_type == PS_FST_TYPE_VNODE) {
@@ -403,73 +427,118 @@ procstat_files(struct procstat *procstat
 				switch (vn.vn_type) {
 				case PS_FST_VTYPE_VREG:
 					str = "r";
+					xo_emit("{eq:vode_type/regular}");
 					break;
 
 				case PS_FST_VTYPE_VDIR:
 					str = "d";
+					xo_emit("{eq:vode_type/directory}");
 					break;
 
 				case PS_FST_VTYPE_VBLK:
 					str = "b";
+					xo_emit("{eq:vode_type/block}");
 					break;
 
 				case PS_FST_VTYPE_VCHR:
 					str = "c";
+					xo_emit("{eq:vode_type/character}");
 					break;
 
 				case PS_FST_VTYPE_VLNK:
 					str = "l";
+					xo_emit("{eq:vode_type/link}");
 					break;
 
 				case PS_FST_VTYPE_VSOCK:
 					str = "s";
+					xo_emit("{eq:vode_type/socket}");
 					break;
 
 				case PS_FST_VTYPE_VFIFO:
 					str = "f";
+					xo_emit("{eq:vode_type/fifo}");
 					break;
 
 				case PS_FST_VTYPE_VBAD:
 					str = "x";
+					xo_emit("{eq:vode_type/revoked_device}");
 					break;
 
 				case PS_FST_VTYPE_VNON:
+					str = "?";
+					xo_emit("{eq:vode_type/non}");
+					break;
+
 				case PS_FST_VTYPE_UNKNOWN:
 				default:
 					str = "?";
+					xo_emit("{eq:vode_type/unknown}");
 					break;
 				}
 			}
-			printf("%1s ", str);
+			xo_emit("{d:vnode_type/%1s/%s} ", str);
 		}
-		printf("%s", fst->fs_fflags & PS_FST_FFLAG_READ ? "r" : "-");
-		printf("%s", fst->fs_fflags & PS_FST_FFLAG_WRITE ? "w" : "-");
-		printf("%s", fst->fs_fflags & PS_FST_FFLAG_APPEND ? "a" : "-");
-		printf("%s", fst->fs_fflags & PS_FST_FFLAG_ASYNC ? "s" : "-");
-		printf("%s", fst->fs_fflags & PS_FST_FFLAG_SYNC ? "f" : "-");
-		printf("%s", fst->fs_fflags & PS_FST_FFLAG_NONBLOCK ? "n" : "-");
-		printf("%s", fst->fs_fflags & PS_FST_FFLAG_DIRECT ? "d" : "-");
-		printf("%s", fst->fs_fflags & PS_FST_FFLAG_HASLOCK ? "l" : "-");
+		
+		xo_emit("{d:/%s}", fst->fs_fflags & PS_FST_FFLAG_READ ?
+		    "r" : "-");
+		xo_emit("{d:/%s}", fst->fs_fflags & PS_FST_FFLAG_WRITE ?
+		    "w" : "-");
+		xo_emit("{d:/%s}", fst->fs_fflags & PS_FST_FFLAG_APPEND ?
+		    "a" : "-");
+		xo_emit("{d:/%s}", fst->fs_fflags & PS_FST_FFLAG_ASYNC ?
+		    "s" : "-");
+		xo_emit("{d:/%s}", fst->fs_fflags & PS_FST_FFLAG_SYNC ?
+		    "f" : "-");
+		xo_emit("{d:/%s}", fst->fs_fflags & PS_FST_FFLAG_NONBLOCK ?
+		    "n" : "-");
+		xo_emit("{d:/%s}", fst->fs_fflags & PS_FST_FFLAG_DIRECT ?
+		    "d" : "-");
+		xo_emit("{d:/%s}", fst->fs_fflags & PS_FST_FFLAG_HASLOCK ?
+		    "l" : "-");
+		xo_emit(" ");
+		xo_open_list("fd_flags");
+		if (fst->fs_fflags & PS_FST_FFLAG_READ)
+			xo_emit("{elq:fd_flags/read}");
+		if (fst->fs_fflags & PS_FST_FFLAG_WRITE)
+			xo_emit("{elq:fd_flags/write}");
+		if (fst->fs_fflags & PS_FST_FFLAG_APPEND)
+			xo_emit("{elq:fd_flags/append}");
+		if (fst->fs_fflags & PS_FST_FFLAG_ASYNC)
+			xo_emit("{elq:fd_flags/async}");
+		if (fst->fs_fflags & PS_FST_FFLAG_SYNC)
+			xo_emit("{elq:fd_flags/fsync}");
+		if (fst->fs_fflags & PS_FST_FFLAG_NONBLOCK)
+			xo_emit("{elq:fd_flags/nonblocking}");
+		if (fst->fs_fflags & PS_FST_FFLAG_DIRECT)
+			xo_emit("{elq:fd_flags/direct_io}");
+		if (fst->fs_fflags & PS_FST_FFLAG_HASLOCK)
+			xo_emit("{elq:fd_flags/lock_held}");
+		xo_close_list("fd_flags");
+
 		if (!Cflag) {
 			if (fst->fs_ref_count > -1)
-				printf("%3d ", fst->fs_ref_count);
+				xo_emit("{:ref_count/%3d/%d} ",
+				    fst->fs_ref_count);
 			else
-				printf("%3c ", '-');
+				xo_emit("{q:ref_count/%3c/%c} ", '-');
 			if (fst->fs_offset > -1)
-				printf("%7jd ", (intmax_t)fst->fs_offset);
+				xo_emit("{:offset/%7jd/%jd} ",
+				    (intmax_t)fst->fs_offset);
 			else
-				printf("%7c ", '-');
+				xo_emit("{q:offset/%7c/%c} ", '-');
 		}
 		if (Cflag) {
 			print_capability(&fst->fs_cap_rights, capwidth);
-			printf(" ");
+			xo_emit(" ");
 		}
 		switch (fst->fs_type) {
 		case PS_FST_TYPE_SOCKET:
-			error = procstat_get_socket_info(procstat, fst, &sock, NULL);
+			error = procstat_get_socket_info(procstat, fst, &sock,
+			    NULL);
 			if (error != 0)
 				break;
-			printf("%-3s ",
+			xo_emit("{:protocol/%-3s/%s} ",
 			    protocol_to_string(sock.dom_family,
 			    sock.type, sock.proto));
 			/*
@@ -484,22 +553,30 @@ procstat_files(struct procstat *procstat
 				    (struct sockaddr_un *)&sock.sa_local;
 
 				if (sun->sun_path[0] != 0)
-					print_address(&sock.sa_local);
+					addr_to_string(&sock.sa_local,
+					    src_addr, sizeof(src_addr));
 				else
-					print_address(&sock.sa_peer);
+					addr_to_string(&sock.sa_peer,
+					    src_addr, sizeof(src_addr));
+				xo_emit("{:path/%s}", src_addr);
 			} else {
-				print_address(&sock.sa_local);
-				printf(" ");
-				print_address(&sock.sa_peer);
+				addr_to_string(&sock.sa_local, src_addr,
+				    sizeof(src_addr));
+				addr_to_string(&sock.sa_peer, dst_addr,
+				    sizeof(dst_addr));
+				xo_emit("{:path/%s %s}", src_addr, dst_addr);
 			}
 			break;
 
 		default:
-			printf("%-3s ", "-");
-			printf("%-18s", fst->fs_path != NULL ? fst->fs_path : "-");
+			xo_emit("{:protocol/%-3s/%s} ", "-");
+			xo_emit("{:path/%-18s/%s}", fst->fs_path != NULL ?
+			    fst->fs_path : "-");
 		}
 
-		printf("\n");
+		xo_emit("\n");
+		xo_close_instance("files");
 	}
+	xo_close_list("files");
 	procstat_freefiles(procstat, head);
 }
