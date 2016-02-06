--- sys/dev/iscsi/iscsi_proto.h.orig	2015-12-29 02:29:49 UTC
+++ sys/dev/iscsi/iscsi_proto.h
@@ -26,7 +26,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/sys/dev/iscsi/iscsi_proto.h 255570 2013-09-14 15:29:06Z trasz $
+ * $FreeBSD: head/sys/dev/iscsi/iscsi_proto.h 278099 2015-02-02 16:06:23Z mav $
  */
 
 #ifndef ISCSI_PROTO_H
@@ -38,6 +38,9 @@
 #define __CTASSERT(x, y)	typedef char __assert_ ## y [(x) ? 1 : -1]
 #endif
 
+#define	ISCSI_SNGT(x, y)	((int32_t)(x) - (int32_t)(y) > 0)
+#define	ISCSI_SNLT(x, y)	((int32_t)(x) - (int32_t)(y) < 0)
+
 #define	ISCSI_BHS_SIZE			48
 #define	ISCSI_HEADER_DIGEST_SIZE	4
 #define	ISCSI_DATA_DIGEST_SIZE		4
@@ -112,7 +115,9 @@ struct iscsi_bhs_scsi_response {
 	uint8_t		bhssr_status;
 	uint8_t		bhssr_total_ahs_len;
 	uint8_t		bhssr_data_segment_len[3];
-	uint64_t	bhssr_reserved;
+	uint16_t	bhssr_status_qualifier;
+	uint16_t	bhssr_reserved;
+	uint32_t	bhssr_reserved2;
 	uint32_t	bhssr_initiator_task_tag;
 	uint32_t	bhssr_snack_tag;
 	uint32_t	bhssr_statsn;
@@ -132,6 +137,10 @@ CTASSERT(sizeof(struct iscsi_bhs_scsi_re
 #define	BHSTMR_FUNCTION_TARGET_WARM_RESET	6
 #define	BHSTMR_FUNCTION_TARGET_COLD_RESET	7
 #define	BHSTMR_FUNCTION_TASK_REASSIGN		8
+#define	BHSTMR_FUNCTION_QUERY_TASK		9
+#define	BHSTMR_FUNCTION_QUERY_TASK_SET		10
+#define	BHSTMR_FUNCTION_I_T_NEXUS_RESET		11
+#define	BHSTMR_FUNCTION_QUERY_ASYNC_EVENT	12
 
 struct iscsi_bhs_task_management_request {
 	uint8_t		bhstmr_opcode;
@@ -151,7 +160,14 @@ struct iscsi_bhs_task_management_request
 CTASSERT(sizeof(struct iscsi_bhs_task_management_request) == ISCSI_BHS_SIZE);
 
 #define	BHSTMR_RESPONSE_FUNCTION_COMPLETE	0
+#define	BHSTMR_RESPONSE_TASK_DOES_NOT_EXIST	1
+#define	BHSTMR_RESPONSE_LUN_DOES_NOT_EXIST	2
+#define	BHSTMR_RESPONSE_TASK_STILL_ALLEGIANT	3
+#define	BHSTMR_RESPONSE_TASK_ALL_REASS_NOT_SUPP	4
 #define	BHSTMR_RESPONSE_FUNCTION_NOT_SUPPORTED	5
+#define	BHSTMR_RESPONSE_FUNCTION_AUTH_FAIL	6
+#define	BHSTMR_RESPONSE_FUNCTION_SUCCEEDED	7
+#define	BHSTMR_RESPONSE_FUNCTION_REJECTED	255
 
 struct iscsi_bhs_task_management_response {
 	uint8_t		bhstmr_opcode;
@@ -160,7 +176,8 @@ struct iscsi_bhs_task_management_respons
 	uint8_t		bhstmr_reserved;
 	uint8_t		bhstmr_total_ahs_len;
 	uint8_t		bhstmr_data_segment_len[3];
-	uint64_t	bhstmr_reserved2;
+	uint8_t		bhstmr_additional_reponse_information[3];
+	uint8_t		bhstmr_reserved2[5];
 	uint32_t	bhstmr_initiator_task_tag;
 	uint32_t	bhstmr_reserved3;
 	uint32_t	bhstmr_statsn;
