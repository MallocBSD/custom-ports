--- usr.bin/iscsictl/token.l.orig	2015-12-29 02:29:49 UTC
+++ usr.bin/iscsictl/token.l
@@ -27,7 +27,7 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/iscsictl/token.l 270907 2014-09-01 01:39:20Z ngie $
+ * $FreeBSD: head/usr.bin/iscsictl/token.l 278232 2015-02-05 06:37:59Z trasz $
  */
 
 #include <stdio.h>
@@ -63,6 +63,7 @@ tgtChapSecret		{ return MUTUAL_SECRET; }
 AuthMethod		{ return AUTH_METHOD; }
 SessionType		{ return SESSION_TYPE; }
 protocol		{ return PROTOCOL; }
+offload			{ return OFFLOAD; }
 port			{ return IGNORED; }
 MaxConnections		{ return IGNORED; }
 TargetAlias		{ return IGNORED; }
@@ -90,6 +91,7 @@ chapDigest		{ return IGNORED; }
 =			{ return EQUALS; }
 ;			{ return SEMICOLON; }
 #.*$			/* ignore comments */;
+\r\n			{ lineno++; }
 \n			{ lineno++; }
 [ \t]+			/* ignore whitespace */;
 .			{ yylval.str = strdup(yytext); return STR; }
