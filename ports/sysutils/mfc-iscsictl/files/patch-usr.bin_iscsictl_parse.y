--- usr.bin/iscsictl/parse.y.orig	2015-12-29 02:29:49 UTC
+++ usr.bin/iscsictl/parse.y
@@ -27,19 +27,20 @@
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
- * $FreeBSD: releng/10.1/usr.bin/iscsictl/parse.y 262841 2014-03-06 11:07:51Z trasz $
+ * $FreeBSD: head/usr.bin/iscsictl/parse.y 281461 2015-04-12 09:36:50Z trasz $
  */
 
 #include <sys/queue.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <assert.h>
-#include <err.h>
 #include <stdio.h>
 #include <stdint.h>
 #include <stdlib.h>
 #include <string.h>
 
+#include <libxo/xo.h>
+
 #include "iscsictl.h"
 
 extern FILE *yyin;
@@ -57,8 +58,8 @@ extern void	yyrestart(FILE *);
 
 %token AUTH_METHOD HEADER_DIGEST DATA_DIGEST TARGET_NAME TARGET_ADDRESS
 %token INITIATOR_NAME INITIATOR_ADDRESS INITIATOR_ALIAS USER SECRET
-%token MUTUAL_USER MUTUAL_SECRET SEMICOLON SESSION_TYPE PROTOCOL IGNORED
-%token EQUALS OPENING_BRACKET CLOSING_BRACKET
+%token MUTUAL_USER MUTUAL_SECRET SEMICOLON SESSION_TYPE PROTOCOL OFFLOAD
+%token IGNORED EQUALS OPENING_BRACKET CLOSING_BRACKET
 
 %union
 {
@@ -77,7 +78,7 @@ targets:
 target:		STR OPENING_BRACKET target_entries CLOSING_BRACKET
 	{
 		if (target_find(conf, $1) != NULL)
-			errx(1, "duplicated target %s", $1);
+			xo_errx(1, "duplicated target %s", $1);
 		target->t_nickname = $1;
 		target = target_new(conf);
 	}
@@ -117,6 +118,8 @@ target_entry:
 	|
 	session_type
 	|
+	offload
+	|
 	protocol
 	|
 	ignored
@@ -125,7 +128,7 @@ target_entry:
 target_name:	TARGET_NAME EQUALS STR
 	{
 		if (target->t_name != NULL)
-			errx(1, "duplicated TargetName at line %d", lineno);
+			xo_errx(1, "duplicated TargetName at line %d", lineno);
 		target->t_name = $3;
 	}
 	;
@@ -133,7 +136,7 @@ target_name:	TARGET_NAME EQUALS STR
 target_address:	TARGET_ADDRESS EQUALS STR
 	{
 		if (target->t_address != NULL)
-			errx(1, "duplicated TargetAddress at line %d", lineno);
+			xo_errx(1, "duplicated TargetAddress at line %d", lineno);
 		target->t_address = $3;
 	}
 	;
@@ -141,7 +144,7 @@ target_address:	TARGET_ADDRESS EQUALS ST
 initiator_name:	INITIATOR_NAME EQUALS STR
 	{
 		if (target->t_initiator_name != NULL)
-			errx(1, "duplicated InitiatorName at line %d", lineno);
+			xo_errx(1, "duplicated InitiatorName at line %d", lineno);
 		target->t_initiator_name = $3;
 	}
 	;
@@ -149,7 +152,7 @@ initiator_name:	INITIATOR_NAME EQUALS ST
 initiator_address:	INITIATOR_ADDRESS EQUALS STR
 	{
 		if (target->t_initiator_address != NULL)
-			errx(1, "duplicated InitiatorAddress at line %d", lineno);
+			xo_errx(1, "duplicated InitiatorAddress at line %d", lineno);
 		target->t_initiator_address = $3;
 	}
 	;
@@ -157,7 +160,7 @@ initiator_address:	INITIATOR_ADDRESS EQU
 initiator_alias:	INITIATOR_ALIAS EQUALS STR
 	{
 		if (target->t_initiator_alias != NULL)
-			errx(1, "duplicated InitiatorAlias at line %d", lineno);
+			xo_errx(1, "duplicated InitiatorAlias at line %d", lineno);
 		target->t_initiator_alias = $3;
 	}
 	;
@@ -165,7 +168,7 @@ initiator_alias:	INITIATOR_ALIAS EQUALS 
 user:		USER EQUALS STR
 	{
 		if (target->t_user != NULL)
-			errx(1, "duplicated chapIName at line %d", lineno);
+			xo_errx(1, "duplicated chapIName at line %d", lineno);
 		target->t_user = $3;
 	}
 	;
@@ -173,7 +176,7 @@ user:		USER EQUALS STR
 secret:		SECRET EQUALS STR
 	{
 		if (target->t_secret != NULL)
-			errx(1, "duplicated chapSecret at line %d", lineno);
+			xo_errx(1, "duplicated chapSecret at line %d", lineno);
 		target->t_secret = $3;
 	}
 	;
@@ -181,7 +184,7 @@ secret:		SECRET EQUALS STR
 mutual_user:	MUTUAL_USER EQUALS STR
 	{
 		if (target->t_mutual_user != NULL)
-			errx(1, "duplicated tgtChapName at line %d", lineno);
+			xo_errx(1, "duplicated tgtChapName at line %d", lineno);
 		target->t_mutual_user = $3;
 	}
 	;
@@ -189,7 +192,7 @@ mutual_user:	MUTUAL_USER EQUALS STR
 mutual_secret:	MUTUAL_SECRET EQUALS STR
 	{
 		if (target->t_mutual_secret != NULL)
-			errx(1, "duplicated tgtChapSecret at line %d", lineno);
+			xo_errx(1, "duplicated tgtChapSecret at line %d", lineno);
 		target->t_mutual_secret = $3;
 	}
 	;
@@ -197,13 +200,13 @@ mutual_secret:	MUTUAL_SECRET EQUALS STR
 auth_method:	AUTH_METHOD EQUALS STR
 	{
 		if (target->t_auth_method != AUTH_METHOD_UNSPECIFIED)
-			errx(1, "duplicated AuthMethod at line %d", lineno);
+			xo_errx(1, "duplicated AuthMethod at line %d", lineno);
 		if (strcasecmp($3, "none") == 0)
 			target->t_auth_method = AUTH_METHOD_NONE;
 		else if (strcasecmp($3, "chap") == 0)
 			target->t_auth_method = AUTH_METHOD_CHAP;
 		else
-			errx(1, "invalid AuthMethod at line %d; "
+			xo_errx(1, "invalid AuthMethod at line %d; "
 			    "must be either \"none\" or \"CHAP\"", lineno);
 	}
 	;
@@ -211,13 +214,13 @@ auth_method:	AUTH_METHOD EQUALS STR
 header_digest:	HEADER_DIGEST EQUALS STR
 	{
 		if (target->t_header_digest != DIGEST_UNSPECIFIED)
-			errx(1, "duplicated HeaderDigest at line %d", lineno);
+			xo_errx(1, "duplicated HeaderDigest at line %d", lineno);
 		if (strcasecmp($3, "none") == 0)
 			target->t_header_digest = DIGEST_NONE;
 		else if (strcasecmp($3, "CRC32C") == 0)
 			target->t_header_digest = DIGEST_CRC32C;
 		else
-			errx(1, "invalid HeaderDigest at line %d; "
+			xo_errx(1, "invalid HeaderDigest at line %d; "
 			    "must be either \"none\" or \"CRC32C\"", lineno);
 	}
 	;
@@ -225,13 +228,13 @@ header_digest:	HEADER_DIGEST EQUALS STR
 data_digest:	DATA_DIGEST EQUALS STR
 	{
 		if (target->t_data_digest != DIGEST_UNSPECIFIED)
-			errx(1, "duplicated DataDigest at line %d", lineno);
+			xo_errx(1, "duplicated DataDigest at line %d", lineno);
 		if (strcasecmp($3, "none") == 0)
 			target->t_data_digest = DIGEST_NONE;
 		else if (strcasecmp($3, "CRC32C") == 0)
 			target->t_data_digest = DIGEST_CRC32C;
 		else
-			errx(1, "invalid DataDigest at line %d; "
+			xo_errx(1, "invalid DataDigest at line %d; "
 			    "must be either \"none\" or \"CRC32C\"", lineno);
 	}
 	;
@@ -239,34 +242,42 @@ data_digest:	DATA_DIGEST EQUALS STR
 session_type:	SESSION_TYPE EQUALS STR
 	{
 		if (target->t_session_type != SESSION_TYPE_UNSPECIFIED)
-			errx(1, "duplicated SessionType at line %d", lineno);
+			xo_errx(1, "duplicated SessionType at line %d", lineno);
 		if (strcasecmp($3, "normal") == 0)
 			target->t_session_type = SESSION_TYPE_NORMAL;
 		else if (strcasecmp($3, "discovery") == 0)
 			target->t_session_type = SESSION_TYPE_DISCOVERY;
 		else
-			errx(1, "invalid SessionType at line %d; "
+			xo_errx(1, "invalid SessionType at line %d; "
 			    "must be either \"normal\" or \"discovery\"", lineno);
 	}
 	;
 
+offload:	OFFLOAD EQUALS STR
+	{
+		if (target->t_offload != NULL)
+			xo_errx(1, "duplicated offload at line %d", lineno);
+		target->t_offload = $3;
+	}
+	;
+
 protocol:	PROTOCOL EQUALS STR
 	{
 		if (target->t_protocol != PROTOCOL_UNSPECIFIED)
-			errx(1, "duplicated protocol at line %d", lineno);
+			xo_errx(1, "duplicated protocol at line %d", lineno);
 		if (strcasecmp($3, "iscsi") == 0)
 			target->t_protocol = PROTOCOL_ISCSI;
 		else if (strcasecmp($3, "iser") == 0)
 			target->t_protocol = PROTOCOL_ISER;
 		else
-			errx(1, "invalid protocol at line %d; "
+			xo_errx(1, "invalid protocol at line %d; "
 			    "must be either \"iscsi\" or \"iser\"", lineno);
 	}
 	;
 
 ignored:	IGNORED EQUALS STR
 	{
-		warnx("obsolete statement ignored at line %d", lineno);
+		xo_warnx("obsolete statement ignored at line %d", lineno);
 	}
 	;
 
@@ -276,7 +287,7 @@ void
 yyerror(const char *str)
 {
 
-	errx(1, "error in configuration file at line %d near '%s': %s",
+	xo_errx(1, "error in configuration file at line %d near '%s': %s",
 	    lineno, yytext, str);
 }
 
@@ -288,19 +299,19 @@ check_perms(const char *path)
 
 	error = stat(path, &sb);
 	if (error != 0) {
-		warn("stat");
+		xo_warn("stat");
 		return;
 	}
 	if (sb.st_mode & S_IWOTH) {
-		warnx("%s is world-writable", path);
+		xo_warnx("%s is world-writable", path);
 	} else if (sb.st_mode & S_IROTH) {
-		warnx("%s is world-readable", path);
+		xo_warnx("%s is world-readable", path);
 	} else if (sb.st_mode & S_IXOTH) {
 		/*
 		 * Ok, this one doesn't matter, but still do it,
 		 * just for consistency.
 		 */
-		warnx("%s is world-executable", path);
+		xo_warnx("%s is world-executable", path);
 	}
 
 	/*
@@ -318,7 +329,7 @@ conf_new_from_file(const char *path)
 
 	yyin = fopen(path, "r");
 	if (yyin == NULL)
-		err(1, "unable to open configuration file %s", path);
+		xo_err(1, "unable to open configuration file %s", path);
 	check_perms(path);
 	lineno = 1;
 	yyrestart(yyin);
