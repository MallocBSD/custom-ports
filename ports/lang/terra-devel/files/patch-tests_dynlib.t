--- tests/dynlib.t.orig	2017-10-17 06:05:35 UTC
+++ tests/dynlib.t
@@ -29,7 +29,9 @@ end
 
 if ffi.os ~= "Windows" then
     print(libpath)
-    local flags = terralib.newlist {"-Wl,-rpath,"..libpath,libpath.."/terra.so"}
+    local flags = ffi.os == "Linux" and
+        terralib.newlist {"-Wl,-rpath,"..libpath,libpath.."/terra.so"} or
+        terralib.newlist {"-Wl,-rpath,"..libpath, libpath.."/libterra.so", "/usr/local/lib/libluajit-5.1.so"}
     if require("ffi").os == "OSX" then
         flags:insertall {"-pagezero_size","10000", "-image_base", "100000000"}
     end
@@ -44,4 +46,4 @@ else
     terralib.saveobj("dynlib.exe",{main = main},flags)
     putenv("Path="..os.getenv("Path")..";"..terralib.terrahome.."\\bin") --make dll search happy
     assert(0 == os.execute(".\\dynlib.exe"))
-end
\ No newline at end of file
+end
