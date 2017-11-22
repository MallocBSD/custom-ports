--- src/terralib.lua.orig	2017-11-21 07:00:21 UTC
+++ src/terralib.lua
@@ -3341,8 +3341,8 @@ function terra.includecstring(code,cargs
     local args = terra.newlist {"-O3","-Wno-deprecated","-resource-dir",clangresourcedirectory}
     target = target or terra.nativetarget
 
-    if (target == terra.nativetarget and ffi.os == "Linux") or (target.Triple and target.Triple:match("linux")) then
-        args:insert("-internal-isystem")
+    if (target == terra.nativetarget and (ffi.os == "Linux" or ffi.os == "BSD")) or (target.Triple and target.Triple:match("linux")) then
+    	args:insert( ffi.os == "BSD" and "-isystem" or "-internal-isystem")
         args:insert(clangresourcedirectory.."/include")
     end
     
