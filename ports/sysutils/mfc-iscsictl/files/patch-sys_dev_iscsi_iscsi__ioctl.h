--- sys/dev/iscsi/iscsi_ioctl.h.orig	2015-12-29 02:29:49 UTC
+++ sys/dev/iscsi/iscsi_ioctl.h
@@ -26,7 +26,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/sys/dev/iscsi/iscsi_ioctl.h 269065 2014-07-24 15:31:45Z mav $
+ * $FreeBSD: head/sys/dev/iscsi/iscsi_ioctl.h 278232 2015-02-05 06:37:59Z trasz $
  */
 
 #ifndef ISCSI_IOCTL_H
@@ -43,6 +43,7 @@
 #define	ISCSI_ADDR_LEN		47	/* INET6_ADDRSTRLEN + '\0' */
 #define	ISCSI_ALIAS_LEN		256	/* XXX: Where did it come from? */
 #define	ISCSI_SECRET_LEN	17	/* 16 + '\0' */
+#define	ISCSI_OFFLOAD_LEN	8
 #define	ISCSI_REASON_LEN	64
 
 #define	ISCSI_DIGEST_NONE	0
@@ -65,7 +66,16 @@ struct iscsi_session_conf {
 	int		isc_header_digest;
 	int		isc_data_digest;
 	int		isc_iser;
-	int		isc_spare[4];
+	char		isc_offload[ISCSI_OFFLOAD_LEN];
+	int		isc_spare[2];
+};
+
+/*
+ * Additional constraints imposed by chosen ICL offload module;
+ * iscsid(8) must obey those when negotiating operational parameters.
+ */
+struct iscsi_session_limits {
+	size_t		isl_max_data_segment_length;
 };
 
 /*
@@ -81,20 +91,21 @@ struct iscsi_session_state {
 	int		iss_immediate_data;
 	int		iss_connected;
 	char		iss_reason[ISCSI_REASON_LEN];
-	int		iss_spare[4];
+	char		iss_offload[ISCSI_OFFLOAD_LEN];
+	int		iss_spare[2];
 };
 
 /*
- * For use with iscsid(8).
+ * The following ioctls are used by iscsid(8).
  */
-
 struct iscsi_daemon_request {
 	unsigned int			idr_session_id;
 	struct iscsi_session_conf	idr_conf;
 	uint8_t				idr_isid[6];
 	uint16_t			idr_tsih;
 	uint16_t			idr_spare_cid;
-	int				idr_spare[4];
+	struct iscsi_session_limits	idr_limits;
+	int				idr_spare[2];
 };
 
 struct iscsi_daemon_handoff {
@@ -182,9 +193,8 @@ struct iscsi_daemon_receive {
 #endif /* ICL_KERNEL_PROXY */
 
 /*
- * For use with iscsictl(8).
+ * The following ioctls are used by iscsictl(8).
  */
-
 struct iscsi_session_add {
 	struct iscsi_session_conf	isa_conf;
 	int				isa_spare[4];
