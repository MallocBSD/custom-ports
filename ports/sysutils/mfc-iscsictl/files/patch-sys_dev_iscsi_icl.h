--- sys/dev/iscsi/icl.h.orig	2015-12-29 02:29:49 UTC
+++ sys/dev/iscsi/icl.h
@@ -26,7 +26,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/sys/dev/iscsi/icl.h 265513 2014-05-07 07:37:55Z trasz $
+ * $FreeBSD: head/sys/dev/iscsi/icl.h 278232 2015-02-05 06:37:59Z trasz $
  */
 
 #ifndef ICL_H
@@ -37,7 +37,32 @@
  * and receive iSCSI PDUs.
  */
 
+#include <sys/types.h>
+#include <sys/kobj.h>
+#include <sys/condvar.h>
+#include <sys/sysctl.h>
+
+SYSCTL_DECL(_kern_icl);
+
+extern int icl_debug;
+
+#define	ICL_DEBUG(X, ...)						\
+	do {								\
+		if (icl_debug > 1)					\
+			printf("%s: " X "\n", __func__, ## __VA_ARGS__);\
+	} while (0)
+
+#define	ICL_WARN(X, ...)						\
+	do {								\
+		if (icl_debug > 0) {					\
+			printf("WARNING: %s: " X "\n",			\
+			    __func__, ## __VA_ARGS__);			\
+		}							\
+	} while (0)
+
 struct icl_conn;
+struct ccb_scsiio;
+union ctl_io;
 
 struct icl_pdu {
 	STAILQ_ENTRY(icl_pdu)	ip_next;
@@ -57,13 +82,6 @@ struct icl_pdu {
 	uint32_t		ip_prv2;
 };
 
-struct icl_pdu		*icl_pdu_new_bhs(struct icl_conn *ic, int flags);
-size_t			icl_pdu_data_segment_length(const struct icl_pdu *ip);
-int			icl_pdu_append_data(struct icl_pdu *ip, const void *addr, size_t len, int flags);
-void			icl_pdu_get_data(struct icl_pdu *ip, size_t off, void *addr, size_t len);
-void			icl_pdu_queue(struct icl_pdu *ip);
-void			icl_pdu_free(struct icl_pdu *ip);
-
 #define ICL_CONN_STATE_INVALID		0
 #define ICL_CONN_STATE_BHS		1
 #define ICL_CONN_STATE_AHS		2
@@ -74,6 +92,7 @@ void			icl_pdu_free(struct icl_pdu *ip);
 #define	ICL_MAX_DATA_SEGMENT_LENGTH	(128 * 1024)
 
 struct icl_conn {
+	KOBJ_FIELDS;
 	struct mtx		*ic_lock;
 	struct socket		*ic_socket;
 #ifdef DIAGNOSTIC
@@ -94,6 +113,7 @@ struct icl_conn {
 	bool			ic_disconnecting;
 	bool			ic_iser;
 	const char		*ic_name;
+	const char		*ic_offload;
 
 	void			(*ic_receive)(struct icl_pdu *);
 	void			(*ic_error)(struct icl_conn *);
@@ -104,12 +124,14 @@ struct icl_conn {
 	void			*ic_prv0;
 };
 
-struct icl_conn		*icl_conn_new(const char *name, struct mtx *lock);
-void			icl_conn_free(struct icl_conn *ic);
-int			icl_conn_handoff(struct icl_conn *ic, int fd);
-void			icl_conn_shutdown(struct icl_conn *ic);
-void			icl_conn_close(struct icl_conn *ic);
-bool			icl_conn_connected(struct icl_conn *ic);
+struct icl_conn	*icl_new_conn(const char *offload, const char *name,
+		    struct mtx *lock);
+int		icl_limits(const char *offload, size_t *limitp);
+
+int		icl_register(const char *offload, int priority,
+		    int (*limits)(size_t *),
+		    struct icl_conn *(*new_conn)(const char *, struct mtx *));
+int		icl_unregister(const char *offload);
 
 #ifdef ICL_KERNEL_PROXY
 
