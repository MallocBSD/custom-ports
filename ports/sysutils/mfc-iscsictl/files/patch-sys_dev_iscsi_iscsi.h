--- sys/dev/iscsi/iscsi.h.orig	2015-12-29 02:29:49 UTC
+++ sys/dev/iscsi/iscsi.h
@@ -26,7 +26,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/sys/dev/iscsi/iscsi.h 268703 2014-07-15 18:21:26Z mav $
+ * $FreeBSD: head/sys/dev/iscsi/iscsi.h 278397 2015-02-08 19:15:14Z trasz $
  */
 
 #ifndef ISCSI_H
@@ -45,6 +45,7 @@ struct iscsi_outstanding {
 	size_t				io_received;
 	uint32_t			io_initiator_task_tag;
 	uint32_t			io_datasn;
+	void				*io_icl_prv;
 };
 
 struct iscsi_session {
