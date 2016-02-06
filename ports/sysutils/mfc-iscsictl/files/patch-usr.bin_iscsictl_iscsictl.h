--- usr.bin/iscsictl/iscsictl.h.orig	2015-12-29 02:29:49 UTC
+++ usr.bin/iscsictl/iscsictl.h
@@ -26,7 +26,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/iscsictl/iscsictl.h 255570 2013-09-14 15:29:06Z trasz $
+ * $FreeBSD: head/usr.bin/iscsictl/iscsictl.h 278232 2015-02-05 06:37:59Z trasz $
  */
 
 #ifndef ISCSICTL_H
@@ -72,6 +72,7 @@ struct target {
 	int			t_auth_method;
 	int			t_session_type;
 	int			t_protocol;
+	char			*t_offload;
 	char			*t_user;
 	char			*t_secret;
 	char			*t_mutual_user;
