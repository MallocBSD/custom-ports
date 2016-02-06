--- sys/dev/iscsi/iscsi.c.orig	2015-12-29 02:29:49 UTC
+++ sys/dev/iscsi/iscsi.c
@@ -29,11 +29,12 @@
  */
 
 #include <sys/cdefs.h>
-__FBSDID("$FreeBSD: releng/10.1/sys/dev/iscsi/iscsi.c 270891 2014-08-31 20:47:10Z trasz $");
+__FBSDID("$FreeBSD: head/sys/dev/iscsi/iscsi.c 291911 2015-12-07 02:56:08Z smh $");
 
 #include <sys/param.h>
 #include <sys/condvar.h>
 #include <sys/conf.h>
+#include <sys/endian.h>
 #include <sys/eventhandler.h>
 #include <sys/file.h>
 #include <sys/kernel.h>
@@ -59,6 +60,7 @@ __FBSDID("$FreeBSD: releng/10.1/sys/dev/
 #include <cam/scsi/scsi_message.h>
 
 #include <dev/iscsi/icl.h>
+#include <dev/iscsi/icl_wrappers.h>
 #include <dev/iscsi/iscsi_ioctl.h>
 #include <dev/iscsi/iscsi_proto.h>
 #include <dev/iscsi/iscsi.h>
@@ -79,27 +81,21 @@ static struct iscsi_softc	*sc;
 
 SYSCTL_NODE(_kern, OID_AUTO, iscsi, CTLFLAG_RD, 0, "iSCSI initiator");
 static int debug = 1;
-TUNABLE_INT("kern.iscsi.debug", &debug);
 SYSCTL_INT(_kern_iscsi, OID_AUTO, debug, CTLFLAG_RWTUN,
     &debug, 0, "Enable debug messages");
 static int ping_timeout = 5;
-TUNABLE_INT("kern.iscsi.ping_timeout", &ping_timeout);
 SYSCTL_INT(_kern_iscsi, OID_AUTO, ping_timeout, CTLFLAG_RWTUN, &ping_timeout,
     0, "Timeout for ping (NOP-Out) requests, in seconds");
 static int iscsid_timeout = 60;
-TUNABLE_INT("kern.iscsi.iscsid_timeout", &iscsid_timeout);
 SYSCTL_INT(_kern_iscsi, OID_AUTO, iscsid_timeout, CTLFLAG_RWTUN, &iscsid_timeout,
     0, "Time to wait for iscsid(8) to handle reconnection, in seconds");
 static int login_timeout = 60;
-TUNABLE_INT("kern.iscsi.login_timeout", &login_timeout);
 SYSCTL_INT(_kern_iscsi, OID_AUTO, login_timeout, CTLFLAG_RWTUN, &login_timeout,
     0, "Time to wait for iscsid(8) to finish Login Phase, in seconds");
 static int maxtags = 255;
-TUNABLE_INT("kern.iscsi.maxtags", &maxtags);
 SYSCTL_INT(_kern_iscsi, OID_AUTO, maxtags, CTLFLAG_RWTUN, &maxtags,
     0, "Max number of IO requests queued");
 static int fail_on_disconnection = 0;
-TUNABLE_INT("kern.iscsi.fail_on_disconnection", &fail_on_disconnection);
 SYSCTL_INT(_kern_iscsi, OID_AUTO, fail_on_disconnection, CTLFLAG_RWTUN,
     &fail_on_disconnection, 0, "Destroy CAM SIM on connection failure");
 
@@ -144,6 +140,7 @@ static uma_zone_t iscsi_outstanding_zone
 #define ISCSI_SESSION_LOCK(X)		mtx_lock(&X->is_lock)
 #define ISCSI_SESSION_UNLOCK(X)		mtx_unlock(&X->is_lock)
 #define ISCSI_SESSION_LOCK_ASSERT(X)	mtx_assert(&X->is_lock, MA_OWNED)
+#define ISCSI_SESSION_LOCK_ASSERT_NOT(X) mtx_assert(&X->is_lock, MA_NOTOWNED)
 
 static int	iscsi_ioctl(struct cdev *dev, u_long cmd, caddr_t arg,
 		    int mode, struct thread *td);
@@ -172,7 +169,7 @@ static void	iscsi_poll(struct cam_sim *s
 static struct iscsi_outstanding	*iscsi_outstanding_find(struct iscsi_session *is,
 		    uint32_t initiator_task_tag);
 static struct iscsi_outstanding	*iscsi_outstanding_add(struct iscsi_session *is,
-		    uint32_t initiator_task_tag, union ccb *ccb);
+		    union ccb *ccb, uint32_t *initiator_task_tagp);
 static void	iscsi_outstanding_remove(struct iscsi_session *is,
 		    struct iscsi_outstanding *io);
 
@@ -196,7 +193,7 @@ iscsi_pdu_prepare(struct icl_pdu *reques
 	 * Data-Out PDU does not contain CmdSN.
 	 */
 	if (bhssc->bhssc_opcode != ISCSI_BHS_OPCODE_SCSI_DATA_OUT) {
-		if (is->is_cmdsn > is->is_maxcmdsn &&
+		if (ISCSI_SNGT(is->is_cmdsn, is->is_maxcmdsn) &&
 		    (bhssc->bhssc_opcode & ISCSI_BHS_OPCODE_IMMEDIATE) == 0) {
 			/*
 			 * Current MaxCmdSN prevents us from sending any more
@@ -205,8 +202,10 @@ iscsi_pdu_prepare(struct icl_pdu *reques
 			 * or by maintenance thread.
 			 */
 #if 0
-			ISCSI_SESSION_DEBUG(is, "postponing send, CmdSN %d, ExpCmdSN %d, MaxCmdSN %d, opcode 0x%x",
-			    is->is_cmdsn, is->is_expcmdsn, is->is_maxcmdsn, bhssc->bhssc_opcode);
+			ISCSI_SESSION_DEBUG(is, "postponing send, CmdSN %u, "
+			    "ExpCmdSN %u, MaxCmdSN %u, opcode 0x%x",
+			    is->is_cmdsn, is->is_expcmdsn, is->is_maxcmdsn,
+			    bhssc->bhssc_opcode);
 #endif
 			return (true);
 		}
@@ -271,7 +270,7 @@ iscsi_session_logout(struct iscsi_sessio
 	struct icl_pdu *request;
 	struct iscsi_bhs_logout_request *bhslr;
 
-	request = icl_pdu_new_bhs(is->is_conn, M_NOWAIT);
+	request = icl_pdu_new(is->is_conn, M_NOWAIT);
 	if (request == NULL)
 		return;
 
@@ -372,7 +371,6 @@ static void
 iscsi_maintenance_thread_reconnect(struct iscsi_session *is)
 {
 
-	icl_conn_shutdown(is->is_conn);
 	icl_conn_close(is->is_conn);
 
 	ISCSI_SESSION_LOCK(is);
@@ -423,6 +421,7 @@ iscsi_maintenance_thread_terminate(struc
 	sx_xunlock(&sc->sc_lock);
 
 	icl_conn_close(is->is_conn);
+	callout_drain(&is->is_callout);
 
 	ISCSI_SESSION_LOCK(is);
 
@@ -436,8 +435,6 @@ iscsi_maintenance_thread_terminate(struc
 	cv_signal(&is->is_login_cv);
 #endif
 
-	callout_drain(&is->is_callout);
-
 	iscsi_session_cleanup(is, true);
 
 	KASSERT(TAILQ_EMPTY(&is->is_outstanding),
@@ -513,6 +510,7 @@ iscsi_session_reconnect(struct iscsi_ses
 static void
 iscsi_session_terminate(struct iscsi_session *is)
 {
+
 	if (is->is_terminating)
 		return;
 
@@ -534,16 +532,18 @@ iscsi_callout(void *context)
 
 	is = context;
 
-	if (is->is_terminating)
+	ISCSI_SESSION_LOCK(is);
+	if (is->is_terminating) {
+		ISCSI_SESSION_UNLOCK(is);
 		return;
+	}
 
 	callout_schedule(&is->is_callout, 1 * hz);
 
-	ISCSI_SESSION_LOCK(is);
 	is->is_timeout++;
 
 	if (is->is_waiting_for_iscsid) {
-		if (is->is_timeout > iscsid_timeout) {
+		if (iscsid_timeout > 0 && is->is_timeout > iscsid_timeout) {
 			ISCSI_SESSION_WARN(is, "timed out waiting for iscsid(8) "
 			    "for %d seconds; reconnecting",
 			    is->is_timeout);
@@ -553,7 +553,7 @@ iscsi_callout(void *context)
 	}
 
 	if (is->is_login_phase) {
-		if (is->is_timeout > login_timeout) {
+		if (login_timeout > 0 && is->is_timeout > login_timeout) {
 			ISCSI_SESSION_WARN(is, "login timed out after %d seconds; "
 			    "reconnecting", is->is_timeout);
 			reconnect_needed = true;
@@ -561,6 +561,16 @@ iscsi_callout(void *context)
 		goto out;
 	}
 
+	if (ping_timeout <= 0) {
+		/*
+		 * Pings are disabled.  Don't send NOP-Out in this case.
+		 * Reset the timeout, to avoid triggering reconnection,
+		 * should the user decide to reenable them.
+		 */
+		is->is_timeout = 0;
+		goto out;
+	}
+
 	if (is->is_timeout >= ping_timeout) {
 		ISCSI_SESSION_WARN(is, "no ping reply (NOP-In) after %d seconds; "
 		    "reconnecting", ping_timeout);
@@ -581,7 +591,7 @@ iscsi_callout(void *context)
 	if (is->is_timeout < 2)
 		return;
 
-	request = icl_pdu_new_bhs(is->is_conn, M_NOWAIT);
+	request = icl_pdu_new(is->is_conn, M_NOWAIT);
 	if (request == NULL) {
 		ISCSI_SESSION_WARN(is, "failed to allocate PDU");
 		return;
@@ -606,7 +616,7 @@ iscsi_pdu_update_statsn(const struct icl
 {
 	const struct iscsi_bhs_data_in *bhsdi;
 	struct iscsi_session *is;
-	uint32_t expcmdsn, maxcmdsn;
+	uint32_t expcmdsn, maxcmdsn, statsn;
 
 	is = PDU_SESSION(response);
 
@@ -625,26 +635,27 @@ iscsi_pdu_update_statsn(const struct icl
 	 */
 	if (bhsdi->bhsdi_opcode != ISCSI_BHS_OPCODE_SCSI_DATA_IN ||
 	    (bhsdi->bhsdi_flags & BHSDI_FLAGS_S) != 0) {
-		if (ntohl(bhsdi->bhsdi_statsn) < is->is_statsn) {
-			ISCSI_SESSION_WARN(is,
-			    "PDU StatSN %d >= session StatSN %d, opcode 0x%x",
-			    is->is_statsn, ntohl(bhsdi->bhsdi_statsn),
-			    bhsdi->bhsdi_opcode);
+		statsn = ntohl(bhsdi->bhsdi_statsn);
+		if (statsn != is->is_statsn && statsn != (is->is_statsn + 1)) {
+			/* XXX: This is normal situation for MCS */
+			ISCSI_SESSION_WARN(is, "PDU 0x%x StatSN %u != "
+			    "session ExpStatSN %u (or + 1); reconnecting",
+			    bhsdi->bhsdi_opcode, statsn, is->is_statsn);
+			iscsi_session_reconnect(is);
 		}
-		is->is_statsn = ntohl(bhsdi->bhsdi_statsn);
+		if (ISCSI_SNGT(statsn, is->is_statsn))
+			is->is_statsn = statsn;
 	}
 
 	expcmdsn = ntohl(bhsdi->bhsdi_expcmdsn);
 	maxcmdsn = ntohl(bhsdi->bhsdi_maxcmdsn);
 
-	/*
-	 * XXX: Compare using Serial Arithmetic Sense.
-	 */
-	if (maxcmdsn + 1 < expcmdsn) {
-		ISCSI_SESSION_DEBUG(is, "PDU MaxCmdSN %d + 1 < PDU ExpCmdSN %d; ignoring",
+	if (ISCSI_SNLT(maxcmdsn + 1, expcmdsn)) {
+		ISCSI_SESSION_DEBUG(is,
+		    "PDU MaxCmdSN %u + 1 < PDU ExpCmdSN %u; ignoring",
 		    maxcmdsn, expcmdsn);
 	} else {
-		if (maxcmdsn > is->is_maxcmdsn) {
+		if (ISCSI_SNGT(maxcmdsn, is->is_maxcmdsn)) {
 			is->is_maxcmdsn = maxcmdsn;
 
 			/*
@@ -653,15 +664,19 @@ iscsi_pdu_update_statsn(const struct icl
 			 */
 			if (!STAILQ_EMPTY(&is->is_postponed))
 				cv_signal(&is->is_maintenance_cv);
-		} else if (maxcmdsn < is->is_maxcmdsn) {
-			ISCSI_SESSION_DEBUG(is, "PDU MaxCmdSN %d < session MaxCmdSN %d; ignoring",
+		} else if (ISCSI_SNLT(maxcmdsn, is->is_maxcmdsn)) {
+			/* XXX: This is normal situation for MCS */
+			ISCSI_SESSION_DEBUG(is,
+			    "PDU MaxCmdSN %u < session MaxCmdSN %u; ignoring",
 			    maxcmdsn, is->is_maxcmdsn);
 		}
 
-		if (expcmdsn > is->is_expcmdsn) {
+		if (ISCSI_SNGT(expcmdsn, is->is_expcmdsn)) {
 			is->is_expcmdsn = expcmdsn;
-		} else if (expcmdsn < is->is_expcmdsn) {
-			ISCSI_SESSION_DEBUG(is, "PDU ExpCmdSN %d < session ExpCmdSN %d; ignoring",
+		} else if (ISCSI_SNLT(expcmdsn, is->is_expcmdsn)) {
+			/* XXX: This is normal situation for MCS */
+			ISCSI_SESSION_DEBUG(is,
+			    "PDU ExpCmdSN %u < session ExpCmdSN %u; ignoring",
 			    expcmdsn, is->is_expcmdsn);
 		}
 	}
@@ -705,37 +720,46 @@ iscsi_receive_callback(struct icl_pdu *r
 	switch (response->ip_bhs->bhs_opcode) {
 	case ISCSI_BHS_OPCODE_NOP_IN:
 		iscsi_pdu_handle_nop_in(response);
+		ISCSI_SESSION_UNLOCK(is);
 		break;
 	case ISCSI_BHS_OPCODE_SCSI_RESPONSE:
 		iscsi_pdu_handle_scsi_response(response);
+		/* Session lock dropped inside. */
+		ISCSI_SESSION_LOCK_ASSERT_NOT(is);
 		break;
 	case ISCSI_BHS_OPCODE_TASK_RESPONSE:
 		iscsi_pdu_handle_task_response(response);
+		ISCSI_SESSION_UNLOCK(is);
 		break;
 	case ISCSI_BHS_OPCODE_SCSI_DATA_IN:
 		iscsi_pdu_handle_data_in(response);
+		/* Session lock dropped inside. */
+		ISCSI_SESSION_LOCK_ASSERT_NOT(is);
 		break;
 	case ISCSI_BHS_OPCODE_LOGOUT_RESPONSE:
 		iscsi_pdu_handle_logout_response(response);
+		ISCSI_SESSION_UNLOCK(is);
 		break;
 	case ISCSI_BHS_OPCODE_R2T:
 		iscsi_pdu_handle_r2t(response);
+		ISCSI_SESSION_UNLOCK(is);
 		break;
 	case ISCSI_BHS_OPCODE_ASYNC_MESSAGE:
 		iscsi_pdu_handle_async_message(response);
+		ISCSI_SESSION_UNLOCK(is);
 		break;
 	case ISCSI_BHS_OPCODE_REJECT:
 		iscsi_pdu_handle_reject(response);
+		ISCSI_SESSION_UNLOCK(is);
 		break;
 	default:
 		ISCSI_SESSION_WARN(is, "received PDU with unsupported "
 		    "opcode 0x%x; reconnecting",
 		    response->ip_bhs->bhs_opcode);
 		iscsi_session_reconnect(is);
+		ISCSI_SESSION_UNLOCK(is);
 		icl_pdu_free(response);
 	}
-
-	ISCSI_SESSION_UNLOCK(is);
 }
 
 static void
@@ -785,7 +809,7 @@ iscsi_pdu_handle_nop_in(struct icl_pdu *
 		icl_pdu_get_data(response, 0, data, datasize);
 	}
 
-	request = icl_pdu_new_bhs(response->ip_conn, M_NOWAIT);
+	request = icl_pdu_new(response->ip_conn, M_NOWAIT);
 	if (request == NULL) {
 		ISCSI_SESSION_WARN(is, "failed to allocate memory; "
 		    "reconnecting");
@@ -824,8 +848,9 @@ iscsi_pdu_handle_scsi_response(struct ic
 	struct iscsi_bhs_scsi_response *bhssr;
 	struct iscsi_outstanding *io;
 	struct iscsi_session *is;
+	union ccb *ccb;
 	struct ccb_scsiio *csio;
-	size_t data_segment_len;
+	size_t data_segment_len, received;
 	uint16_t sense_len;
 
 	is = PDU_SESSION(response);
@@ -836,46 +861,44 @@ iscsi_pdu_handle_scsi_response(struct ic
 		ISCSI_SESSION_WARN(is, "bad itt 0x%x", bhssr->bhssr_initiator_task_tag);
 		icl_pdu_free(response);
 		iscsi_session_reconnect(is);
+		ISCSI_SESSION_UNLOCK(is);
 		return;
 	}
 
+	ccb = io->io_ccb;
+	received = io->io_received;
+	iscsi_outstanding_remove(is, io);
+	ISCSI_SESSION_UNLOCK(is);
+
 	if (bhssr->bhssr_response != BHSSR_RESPONSE_COMMAND_COMPLETED) {
 		ISCSI_SESSION_WARN(is, "service response 0x%x", bhssr->bhssr_response);
- 		if ((io->io_ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
- 			xpt_freeze_devq(io->io_ccb->ccb_h.path, 1);
+ 		if ((ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
+ 			xpt_freeze_devq(ccb->ccb_h.path, 1);
 			ISCSI_SESSION_DEBUG(is, "freezing devq");
 		}
- 		io->io_ccb->ccb_h.status = CAM_REQ_CMP_ERR | CAM_DEV_QFRZN;
+ 		ccb->ccb_h.status = CAM_REQ_CMP_ERR | CAM_DEV_QFRZN;
 	} else if (bhssr->bhssr_status == 0) {
-		io->io_ccb->ccb_h.status = CAM_REQ_CMP;
+		ccb->ccb_h.status = CAM_REQ_CMP;
 	} else {
- 		if ((io->io_ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
- 			xpt_freeze_devq(io->io_ccb->ccb_h.path, 1);
+ 		if ((ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
+ 			xpt_freeze_devq(ccb->ccb_h.path, 1);
 			ISCSI_SESSION_DEBUG(is, "freezing devq");
 		}
- 		io->io_ccb->ccb_h.status = CAM_SCSI_STATUS_ERROR | CAM_DEV_QFRZN;
-		io->io_ccb->csio.scsi_status = bhssr->bhssr_status;
-	}
-
-	if (bhssr->bhssr_flags & BHSSR_FLAGS_RESIDUAL_OVERFLOW) {
-		ISCSI_SESSION_WARN(is, "target indicated residual overflow");
-		icl_pdu_free(response);
-		iscsi_session_reconnect(is);
-		return;
+ 		ccb->ccb_h.status = CAM_SCSI_STATUS_ERROR | CAM_DEV_QFRZN;
+		ccb->csio.scsi_status = bhssr->bhssr_status;
 	}
 
-	csio = &io->io_ccb->csio;
-
+	csio = &ccb->csio;
 	data_segment_len = icl_pdu_data_segment_length(response);
 	if (data_segment_len > 0) {
 		if (data_segment_len < sizeof(sense_len)) {
 			ISCSI_SESSION_WARN(is, "truncated data segment (%zd bytes)",
 			    data_segment_len);
-			if ((io->io_ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
-				xpt_freeze_devq(io->io_ccb->ccb_h.path, 1);
+			if ((ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
+				xpt_freeze_devq(ccb->ccb_h.path, 1);
 				ISCSI_SESSION_DEBUG(is, "freezing devq");
 			}
-			io->io_ccb->ccb_h.status = CAM_REQ_CMP_ERR | CAM_DEV_QFRZN;
+			ccb->ccb_h.status = CAM_REQ_CMP_ERR | CAM_DEV_QFRZN;
 			goto out;
 		}
 		icl_pdu_get_data(response, 0, &sense_len, sizeof(sense_len));
@@ -888,11 +911,11 @@ iscsi_pdu_handle_scsi_response(struct ic
 			ISCSI_SESSION_WARN(is, "truncated data segment "
 			    "(%zd bytes, should be %zd)",
 			    data_segment_len, sizeof(sense_len) + sense_len);
-			if ((io->io_ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
-				xpt_freeze_devq(io->io_ccb->ccb_h.path, 1);
+			if ((ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
+				xpt_freeze_devq(ccb->ccb_h.path, 1);
 				ISCSI_SESSION_DEBUG(is, "freezing devq");
 			}
-			io->io_ccb->ccb_h.status = CAM_REQ_CMP_ERR | CAM_DEV_QFRZN;
+			ccb->ccb_h.status = CAM_REQ_CMP_ERR | CAM_DEV_QFRZN;
 			goto out;
 		} else if (sizeof(sense_len) + sense_len < data_segment_len)
 			ISCSI_SESSION_WARN(is, "oversize data segment "
@@ -905,7 +928,7 @@ iscsi_pdu_handle_scsi_response(struct ic
 		}
 		icl_pdu_get_data(response, sizeof(sense_len), &csio->sense_data, sense_len);
 		csio->sense_resid = csio->sense_len - sense_len;
-		io->io_ccb->ccb_h.status |= CAM_AUTOSNS_VALID;
+		ccb->ccb_h.status |= CAM_AUTOSNS_VALID;
 	}
 
 out:
@@ -913,21 +936,19 @@ out:
 		csio->resid = ntohl(bhssr->bhssr_residual_count);
 
 	if ((csio->ccb_h.flags & CAM_DIR_MASK) == CAM_DIR_IN) {
-		KASSERT(io->io_received <= csio->dxfer_len,
-		    ("io->io_received > csio->dxfer_len"));
-		if (io->io_received < csio->dxfer_len) {
-			if (csio->resid != csio->dxfer_len - io->io_received) {
+		KASSERT(received <= csio->dxfer_len,
+		    ("received > csio->dxfer_len"));
+		if (received < csio->dxfer_len) {
+			if (csio->resid != csio->dxfer_len - received) {
 				ISCSI_SESSION_WARN(is, "underflow mismatch: "
 				    "target indicates %d, we calculated %zd",
-				    csio->resid,
-				    csio->dxfer_len - io->io_received);
+				    csio->resid, csio->dxfer_len - received);
 			}
-			csio->resid = csio->dxfer_len - io->io_received;
+			csio->resid = csio->dxfer_len - received;
 		}
 	}
 
-	xpt_done(io->io_ccb);
-	iscsi_outstanding_remove(is, io);
+	xpt_done(ccb);
 	icl_pdu_free(response);
 }
 
@@ -969,8 +990,9 @@ iscsi_pdu_handle_data_in(struct icl_pdu 
 	struct iscsi_bhs_data_in *bhsdi;
 	struct iscsi_outstanding *io;
 	struct iscsi_session *is;
+	union ccb *ccb;
 	struct ccb_scsiio *csio;
-	size_t data_segment_len;
+	size_t data_segment_len, received, oreceived;
 	
 	is = PDU_SESSION(response);
 	bhsdi = (struct iscsi_bhs_data_in *)response->ip_bhs;
@@ -979,6 +1001,7 @@ iscsi_pdu_handle_data_in(struct icl_pdu 
 		ISCSI_SESSION_WARN(is, "bad itt 0x%x", bhsdi->bhsdi_initiator_task_tag);
 		icl_pdu_free(response);
 		iscsi_session_reconnect(is);
+		ISCSI_SESSION_UNLOCK(is);
 		return;
 	}
 
@@ -989,6 +1012,7 @@ iscsi_pdu_handle_data_in(struct icl_pdu 
 		 * but initiators and targets MUST be able to properly receive
 		 * 0 length data segments."
 		 */
+		ISCSI_SESSION_UNLOCK(is);
 		icl_pdu_free(response);
 		return;
 	}
@@ -1003,10 +1027,12 @@ iscsi_pdu_handle_data_in(struct icl_pdu 
 		    io->io_received, (size_t)ntohl(bhsdi->bhsdi_buffer_offset));
 		icl_pdu_free(response);
 		iscsi_session_reconnect(is);
+		ISCSI_SESSION_UNLOCK(is);
 		return;
 	}
 
-	csio = &io->io_ccb->csio;
+	ccb = io->io_ccb;
+	csio = &ccb->csio;
 
 	if (io->io_received + data_segment_len > csio->dxfer_len) {
 		ISCSI_SESSION_WARN(is, "oversize data segment (%zd bytes "
@@ -1014,11 +1040,18 @@ iscsi_pdu_handle_data_in(struct icl_pdu 
 		    data_segment_len, io->io_received, csio->dxfer_len);
 		icl_pdu_free(response);
 		iscsi_session_reconnect(is);
+		ISCSI_SESSION_UNLOCK(is);
 		return;
 	}
 
-	icl_pdu_get_data(response, 0, csio->data_ptr + io->io_received, data_segment_len);
+	oreceived = io->io_received;
 	io->io_received += data_segment_len;
+	received = io->io_received;
+	if ((bhsdi->bhsdi_flags & BHSDI_FLAGS_S) != 0)
+		iscsi_outstanding_remove(is, io);
+	ISCSI_SESSION_UNLOCK(is);
+
+	icl_pdu_get_data(response, 0, csio->data_ptr + oreceived, data_segment_len);
 
 	/*
 	 * XXX: Check DataSN.
@@ -1034,33 +1067,31 @@ iscsi_pdu_handle_data_in(struct icl_pdu 
 
 	//ISCSI_SESSION_DEBUG(is, "got S flag; status 0x%x", bhsdi->bhsdi_status);
 	if (bhsdi->bhsdi_status == 0) {
-		io->io_ccb->ccb_h.status = CAM_REQ_CMP;
+		ccb->ccb_h.status = CAM_REQ_CMP;
 	} else {
-		if ((io->io_ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
-			xpt_freeze_devq(io->io_ccb->ccb_h.path, 1);
+		if ((ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
+			xpt_freeze_devq(ccb->ccb_h.path, 1);
 			ISCSI_SESSION_DEBUG(is, "freezing devq");
 		}
-		io->io_ccb->ccb_h.status = CAM_SCSI_STATUS_ERROR | CAM_DEV_QFRZN;
+		ccb->ccb_h.status = CAM_SCSI_STATUS_ERROR | CAM_DEV_QFRZN;
 		csio->scsi_status = bhsdi->bhsdi_status;
 	}
 
 	if ((csio->ccb_h.flags & CAM_DIR_MASK) == CAM_DIR_IN) {
-		KASSERT(io->io_received <= csio->dxfer_len,
-		    ("io->io_received > csio->dxfer_len"));
-		if (io->io_received < csio->dxfer_len) {
+		KASSERT(received <= csio->dxfer_len,
+		    ("received > csio->dxfer_len"));
+		if (received < csio->dxfer_len) {
 			csio->resid = ntohl(bhsdi->bhsdi_residual_count);
-			if (csio->resid != csio->dxfer_len - io->io_received) {
+			if (csio->resid != csio->dxfer_len - received) {
 				ISCSI_SESSION_WARN(is, "underflow mismatch: "
 				    "target indicates %d, we calculated %zd",
-				    csio->resid,
-				    csio->dxfer_len - io->io_received);
+				    csio->resid, csio->dxfer_len - received);
 			}
-			csio->resid = csio->dxfer_len - io->io_received;
+			csio->resid = csio->dxfer_len - received;
 		}
 	}
 
-	xpt_done(io->io_ccb);
-	iscsi_outstanding_remove(is, io);
+	xpt_done(ccb);
 	icl_pdu_free(response);
 }
 
@@ -1146,7 +1177,7 @@ iscsi_pdu_handle_r2t(struct icl_pdu *res
 			return;
 		}
 
-		request = icl_pdu_new_bhs(response->ip_conn, M_NOWAIT);
+		request = icl_pdu_new(response->ip_conn, M_NOWAIT);
 		if (request == NULL) {
 			icl_pdu_free(response);
 			iscsi_session_reconnect(is);
@@ -1277,6 +1308,16 @@ iscsi_ioctl_daemon_wait(struct iscsi_sof
 		request->idr_tsih = 0;	/* New or reinstated session. */
 		memcpy(&request->idr_conf, &is->is_conf,
 		    sizeof(request->idr_conf));
+		
+		error = icl_limits(is->is_conf.isc_offload,
+		    &request->idr_limits.isl_max_data_segment_length);
+		if (error != 0) {
+			ISCSI_SESSION_WARN(is, "icl_limits for offload \"%s\" "
+			    "failed with error %d", is->is_conf.isc_offload,
+			    error);
+			sx_sunlock(&sc->sc_lock);
+			return (error);
+		}
 
 		sx_sunlock(&sc->sc_lock);
 		return (0);
@@ -1550,7 +1591,7 @@ iscsi_ioctl_daemon_send(struct iscsi_sof
 		}
 	}
 
-	ip = icl_pdu_new_bhs(is->is_conn, M_WAITOK);
+	ip = icl_pdu_new(is->is_conn, M_WAITOK);
 	memcpy(ip->ip_bhs, ids->ids_bhs, sizeof(*ip->ip_bhs));
 	if (datalen > 0) {
 		error = icl_pdu_append_data(ip, data, datalen, M_WAITOK);
@@ -1702,7 +1743,13 @@ iscsi_ioctl_session_add(struct iscsi_sof
 		return (EBUSY);
 	}
 
-	is->is_conn = icl_conn_new("iscsi", &is->is_lock);
+	is->is_conn = icl_new_conn(is->is_conf.isc_offload,
+	    "iscsi", &is->is_lock);
+	if (is->is_conn == NULL) {
+		sx_xunlock(&sc->sc_lock);
+		free(is, M_ISCSI);
+		return (EINVAL);
+	}
 	is->is_conn->ic_receive = iscsi_receive_callback;
 	is->is_conn->ic_error = iscsi_error_callback;
 	is->is_conn->ic_prv0 = is;
@@ -1721,15 +1768,17 @@ iscsi_ioctl_session_add(struct iscsi_sof
 	arc4rand(&is->is_isid[1], 5, 0);
 	is->is_tsih = 0;
 	callout_init(&is->is_callout, 1);
-	callout_reset(&is->is_callout, 1 * hz, iscsi_callout, is);
-	TAILQ_INSERT_TAIL(&sc->sc_sessions, is, is_next);
 
 	error = kthread_add(iscsi_maintenance_thread, is, NULL, NULL, 0, 0, "iscsimt");
 	if (error != 0) {
 		ISCSI_SESSION_WARN(is, "kthread_add(9) failed with error %d", error);
+		sx_xunlock(&sc->sc_lock);
 		return (error);
 	}
 
+	callout_reset(&is->is_callout, 1 * hz, iscsi_callout, is);
+	TAILQ_INSERT_TAIL(&sc->sc_sessions, is, is_next);
+
 	/*
 	 * Trigger immediate reconnection.
 	 */
@@ -1748,18 +1797,16 @@ static bool
 iscsi_session_conf_matches(unsigned int id1, const struct iscsi_session_conf *c1,
     unsigned int id2, const struct iscsi_session_conf *c2)
 {
-	if (id2 == 0 && c2->isc_target[0] == '\0' &&
-	    c2->isc_target_addr[0] == '\0')
-		return (true);
-	if (id2 != 0 && id2 == id1)
-		return (true);
+
+	if (id2 != 0 && id2 != id1)
+		return (false);
 	if (c2->isc_target[0] != '\0' &&
-	    strcmp(c1->isc_target, c2->isc_target) == 0)
-		return (true);
+	    strcmp(c1->isc_target, c2->isc_target) != 0)
+		return (false);
 	if (c2->isc_target_addr[0] != '\0' &&
-	    strcmp(c1->isc_target_addr, c2->isc_target_addr) == 0)
-		return (true);
-	return (false);
+	    strcmp(c1->isc_target_addr, c2->isc_target_addr) != 0)
+		return (false);
+	return (true);
 }
 
 static int
@@ -1809,6 +1856,7 @@ iscsi_ioctl_session_list(struct iscsi_so
 		iss.iss_id = is->is_id;
 		strlcpy(iss.iss_target_alias, is->is_target_alias, sizeof(iss.iss_target_alias));
 		strlcpy(iss.iss_reason, is->is_reason, sizeof(iss.iss_reason));
+		strlcpy(iss.iss_offload, is->is_conn->ic_offload, sizeof(iss.iss_offload));
 
 		if (is->is_conn->ic_header_crc32c)
 			iss.iss_header_digest = ISCSI_DIGEST_CRC32C;
@@ -1915,40 +1963,6 @@ iscsi_ioctl(struct cdev *dev, u_long cmd
 	}
 }
 
-static uint64_t
-iscsi_encode_lun(uint32_t lun)
-{
-	uint8_t encoded[8];
-	uint64_t result;
-
-	memset(encoded, 0, sizeof(encoded));
-
-	if (lun < 256) {
-		/*
-		 * Peripheral device addressing.
-		 */
-		encoded[1] = lun;
-	} else if (lun < 16384) {
-		/*
-		 * Flat space addressing.
-		 */
-		encoded[0] = 0x40;
-		encoded[0] |= (lun >> 8) & 0x3f;
-		encoded[1] = lun & 0xff;
-	} else {
-		/*
-		 * Extended flat space addressing.
-		 */
-		encoded[0] = 0xd2;
-		encoded[1] = lun >> 16;
-		encoded[2] = lun >> 8;
-		encoded[3] = lun;
-	}
-
-	memcpy(&result, encoded, sizeof(result));
-	return (result);
-}
-
 static struct iscsi_outstanding *
 iscsi_outstanding_find(struct iscsi_session *is, uint32_t initiator_task_tag)
 {
@@ -1979,21 +1993,33 @@ iscsi_outstanding_find_ccb(struct iscsi_
 
 static struct iscsi_outstanding *
 iscsi_outstanding_add(struct iscsi_session *is,
-    uint32_t initiator_task_tag, union ccb *ccb)
+    union ccb *ccb, uint32_t *initiator_task_tagp)
 {
 	struct iscsi_outstanding *io;
+	int error;
 
 	ISCSI_SESSION_LOCK_ASSERT(is);
 
-	KASSERT(iscsi_outstanding_find(is, initiator_task_tag) == NULL,
-	    ("initiator_task_tag 0x%x already added", initiator_task_tag));
-
 	io = uma_zalloc(iscsi_outstanding_zone, M_NOWAIT | M_ZERO);
 	if (io == NULL) {
-		ISCSI_SESSION_WARN(is, "failed to allocate %zd bytes", sizeof(*io));
+		ISCSI_SESSION_WARN(is, "failed to allocate %zd bytes",
+		    sizeof(*io));
 		return (NULL);
 	}
-	io->io_initiator_task_tag = initiator_task_tag;
+
+	error = icl_conn_task_setup(is->is_conn, &ccb->csio,
+	    initiator_task_tagp, &io->io_icl_prv);
+	if (error != 0) {
+		ISCSI_SESSION_WARN(is,
+		    "icl_conn_task_setup() failed with error %d", error);
+		uma_zfree(iscsi_outstanding_zone, io);
+		return (NULL);
+	}
+
+	KASSERT(iscsi_outstanding_find(is, *initiator_task_tagp) == NULL,
+	    ("initiator_task_tag 0x%x already added", *initiator_task_tagp));
+
+	io->io_initiator_task_tag = *initiator_task_tagp;
 	io->io_ccb = ccb;
 	TAILQ_INSERT_TAIL(&is->is_outstanding, io, io_next);
 	return (io);
@@ -2005,6 +2031,7 @@ iscsi_outstanding_remove(struct iscsi_se
 
 	ISCSI_SESSION_LOCK_ASSERT(is);
 
+	icl_conn_task_done(is->is_conn, io->io_icl_prv);
 	TAILQ_REMOVE(&is->is_outstanding, io, io_next);
 	uma_zfree(iscsi_outstanding_zone, io);
 }
@@ -2016,6 +2043,7 @@ iscsi_action_abort(struct iscsi_session 
 	struct iscsi_bhs_task_management_request *bhstmr;
 	struct ccb_abort *cab = &ccb->cab;
 	struct iscsi_outstanding *io, *aio;
+	uint32_t initiator_task_tag;
 
 	ISCSI_SESSION_LOCK_ASSERT(is);
 
@@ -2036,23 +2064,16 @@ iscsi_action_abort(struct iscsi_session 
 		return;
 	}
 
-	request = icl_pdu_new_bhs(is->is_conn, M_NOWAIT);
+	request = icl_pdu_new(is->is_conn, M_NOWAIT);
 	if (request == NULL) {
 		ccb->ccb_h.status = CAM_RESRC_UNAVAIL;
 		xpt_done(ccb);
 		return;
 	}
 
-	bhstmr = (struct iscsi_bhs_task_management_request *)request->ip_bhs;
-	bhstmr->bhstmr_opcode = ISCSI_BHS_OPCODE_TASK_REQUEST;
-	bhstmr->bhstmr_function = 0x80 | BHSTMR_FUNCTION_ABORT_TASK;
-
-	bhstmr->bhstmr_lun = iscsi_encode_lun(ccb->ccb_h.target_lun);
-	bhstmr->bhstmr_initiator_task_tag = is->is_initiator_task_tag;
-	is->is_initiator_task_tag++;
-	bhstmr->bhstmr_referenced_task_tag = aio->io_initiator_task_tag;
+	initiator_task_tag = is->is_initiator_task_tag++;
 
-	io = iscsi_outstanding_add(is, bhstmr->bhstmr_initiator_task_tag, NULL);
+	io = iscsi_outstanding_add(is, NULL, &initiator_task_tag);
 	if (io == NULL) {
 		icl_pdu_free(request);
 		ccb->ccb_h.status = CAM_RESRC_UNAVAIL;
@@ -2060,6 +2081,14 @@ iscsi_action_abort(struct iscsi_session 
 		return;
 	}
 	io->io_datasn = aio->io_initiator_task_tag;
+
+	bhstmr = (struct iscsi_bhs_task_management_request *)request->ip_bhs;
+	bhstmr->bhstmr_opcode = ISCSI_BHS_OPCODE_TASK_REQUEST;
+	bhstmr->bhstmr_function = 0x80 | BHSTMR_FUNCTION_ABORT_TASK;
+	bhstmr->bhstmr_lun = htobe64(CAM_EXTLUN_BYTE_SWIZZLE(ccb->ccb_h.target_lun));
+	bhstmr->bhstmr_initiator_task_tag = initiator_task_tag;
+	bhstmr->bhstmr_referenced_task_tag = aio->io_initiator_task_tag;
+
 	iscsi_pdu_queue_locked(request);
 }
 
@@ -2071,6 +2100,7 @@ iscsi_action_scsiio(struct iscsi_session
 	struct ccb_scsiio *csio;
 	struct iscsi_outstanding *io;
 	size_t len;
+	uint32_t initiator_task_tag;
 	int error;
 
 	ISCSI_SESSION_LOCK_ASSERT(is);
@@ -2090,7 +2120,7 @@ iscsi_action_scsiio(struct iscsi_session
 	}
 #endif
 
-	request = icl_pdu_new_bhs(is->is_conn, M_NOWAIT);
+	request = icl_pdu_new(is->is_conn, M_NOWAIT);
 	if (request == NULL) {
 		if ((ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
 			xpt_freeze_devq(ccb->ccb_h.path, 1);
@@ -2101,6 +2131,19 @@ iscsi_action_scsiio(struct iscsi_session
 		return;
 	}
 
+	initiator_task_tag = is->is_initiator_task_tag++;
+	io = iscsi_outstanding_add(is, ccb, &initiator_task_tag);
+	if (io == NULL) {
+		icl_pdu_free(request);
+		if ((ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
+			xpt_freeze_devq(ccb->ccb_h.path, 1);
+			ISCSI_SESSION_DEBUG(is, "freezing devq");
+		}
+		ccb->ccb_h.status = CAM_RESRC_UNAVAIL | CAM_DEV_QFRZN;
+		xpt_done(ccb);
+		return;
+	}
+
 	csio = &ccb->csio;
 	bhssc = (struct iscsi_bhs_scsi_command *)request->ip_bhs;
 	bhssc->bhssc_opcode = ISCSI_BHS_OPCODE_SCSI_COMMAND;
@@ -2133,9 +2176,8 @@ iscsi_action_scsiio(struct iscsi_session
 	} else
 		bhssc->bhssc_flags |= BHSSC_FLAGS_ATTR_UNTAGGED;
 
-	bhssc->bhssc_lun = iscsi_encode_lun(csio->ccb_h.target_lun);
-	bhssc->bhssc_initiator_task_tag = is->is_initiator_task_tag;
-	is->is_initiator_task_tag++;
+	bhssc->bhssc_lun = htobe64(CAM_EXTLUN_BYTE_SWIZZLE(ccb->ccb_h.target_lun));
+	bhssc->bhssc_initiator_task_tag = initiator_task_tag;
 	bhssc->bhssc_expected_data_transfer_length = htonl(csio->dxfer_len);
 	KASSERT(csio->cdb_len <= sizeof(bhssc->bhssc_cdb),
 	    ("unsupported CDB size %zd", (size_t)csio->cdb_len));
@@ -2145,18 +2187,6 @@ iscsi_action_scsiio(struct iscsi_session
 	else
 		memcpy(&bhssc->bhssc_cdb, csio->cdb_io.cdb_bytes, csio->cdb_len);
 
-	io = iscsi_outstanding_add(is, bhssc->bhssc_initiator_task_tag, ccb);
-	if (io == NULL) {
-		icl_pdu_free(request);
-		if ((ccb->ccb_h.status & CAM_DEV_QFRZN) == 0) {
-			xpt_freeze_devq(ccb->ccb_h.path, 1);
-			ISCSI_SESSION_DEBUG(is, "freezing devq");
-		}
-		ccb->ccb_h.status = CAM_RESRC_UNAVAIL | CAM_DEV_QFRZN;
-		xpt_done(ccb);
-		return;
-	}
-
 	if (is->is_immediate_data &&
 	    (csio->ccb_h.flags & CAM_DIR_MASK) == CAM_DIR_OUT) {
 		len = csio->dxfer_len;
@@ -2165,6 +2195,10 @@ iscsi_action_scsiio(struct iscsi_session
 			ISCSI_SESSION_DEBUG(is, "len %zd -> %zd", len, is->is_first_burst_length);
 			len = is->is_first_burst_length;
 		}
+		if (len > is->is_max_data_segment_length) {
+			ISCSI_SESSION_DEBUG(is, "len %zd -> %zd", len, is->is_max_data_segment_length);
+			len = is->is_max_data_segment_length;
+		}
 
 		error = icl_pdu_append_data(request, csio->data_ptr, len, M_NOWAIT);
 		if (error != 0) {
@@ -2205,13 +2239,16 @@ iscsi_action(struct cam_sim *sim, union 
 		cpi->version_num = 1;
 		cpi->hba_inquiry = PI_TAG_ABLE;
 		cpi->target_sprt = 0;
-		//cpi->hba_misc = PIM_NOBUSRESET;
-		cpi->hba_misc = 0;
+		cpi->hba_misc = PIM_EXTLUNS;
 		cpi->hba_eng_cnt = 0;
 		cpi->max_target = 0;
+		/*
+		 * Note that the variable below is only relevant for targets
+		 * that don't claim compliance with anything above SPC2, which
+		 * means they don't support REPORT_LUNS.
+		 */
 		cpi->max_lun = 255;
-		//cpi->initiator_id = 0; /* XXX */
-		cpi->initiator_id = 64; /* XXX */
+		cpi->initiator_id = ~0;
 		strlcpy(cpi->sim_vid, "FreeBSD", SIM_IDLEN);
 		strlcpy(cpi->hba_vid, "iSCSI", HBA_IDLEN);
 		strlcpy(cpi->dev_name, cam_sim_name(sim), DEV_IDLEN);
@@ -2285,11 +2322,23 @@ iscsi_shutdown(struct iscsi_softc *sc)
 {
 	struct iscsi_session *is;
 
-	ISCSI_DEBUG("removing all sessions due to shutdown");
+	/*
+	 * Trying to reconnect during system shutdown would lead to hang.
+	 */
+	fail_on_disconnection = 1;
 
+	/*
+	 * If we have any sessions waiting for reconnection, request
+	 * maintenance thread to fail them immediately instead of waiting
+	 * for reconnect timeout.
+	 */
 	sx_slock(&sc->sc_lock);
-	TAILQ_FOREACH(is, &sc->sc_sessions, is_next)
-		iscsi_session_terminate(is);
+	TAILQ_FOREACH(is, &sc->sc_sessions, is_next) {
+		ISCSI_SESSION_LOCK(is);
+		if (is->is_waiting_for_iscsid)
+			iscsi_session_reconnect(is);
+		ISCSI_SESSION_UNLOCK(is);
+	}
 	sx_sunlock(&sc->sc_lock);
 }
 
@@ -2315,13 +2364,8 @@ iscsi_load(void)
 	}
 	sc->sc_cdev->si_drv1 = sc;
 
-	/*
-	 * Note that this needs to get run before dashutdown().  Otherwise,
-	 * when rebooting with iSCSI session with outstanding requests,
-	 * but disconnected, dashutdown() will hang on cam_periph_runccb().
-	 */
-	sc->sc_shutdown_eh = EVENTHANDLER_REGISTER(shutdown_post_sync,
-	    iscsi_shutdown, sc, SHUTDOWN_PRI_FIRST);
+	sc->sc_shutdown_eh = EVENTHANDLER_REGISTER(shutdown_pre_sync,
+	    iscsi_shutdown, sc, SHUTDOWN_PRI_DEFAULT-1);
 
 	return (0);
 }
@@ -2338,7 +2382,7 @@ iscsi_unload(void)
 	}
 
 	if (sc->sc_shutdown_eh != NULL)
-		EVENTHANDLER_DEREGISTER(shutdown_post_sync, sc->sc_shutdown_eh);
+		EVENTHANDLER_DEREGISTER(shutdown_pre_sync, sc->sc_shutdown_eh);
 
 	sx_slock(&sc->sc_lock);
 	TAILQ_FOREACH_SAFE(is, &sc->sc_sessions, is_next, tmp)
