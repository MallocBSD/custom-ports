--- src/terralib.lua.orig	2017-10-17 06:05:35 UTC
+++ src/terralib.lua
@@ -3366,12 +3366,12 @@ function terra.includecstring(code,cargs
     local args = terra.newlist {"-O3","-Wno-deprecated","-resource-dir",clangresourcedirectory}
     target = target or terra.nativetarget
 
-    if (target == terra.nativetarget and ffi.os == "Linux") or (target.Triple and target.Triple:match("linux")) then
-        args:insert("-internal-isystem")
+    if (target == terra.nativetarget and (ffi.os == "Linux" or ffi.os == "BSD")) or (target.Triple and target.Triple:match("linux")) then
+    	args:insert( ffi.os == "BSD" and "-isystem" or "-internal-isystem")
         args:insert(clangresourcedirectory.."/include")
     end
     for _,path in ipairs(terra.systemincludes) do
-    	args:insert("-internal-isystem")
+    	args:insert( ffi.os == "BSD" and "-isystem" or "-internal-isystem")
     	args:insert(path)
     end
     
@@ -3397,6 +3397,7 @@ function terra.includecstring(code,cargs
     addtogeneral(macros)
     setmetatable(general,mt)
     setmetatable(tagged,mt)
+    for _,v in ipairs(args) do print( v ) end
     return general,tagged,macros
 end
 function terra.includec(fname,cargs,target)
@@ -4015,6 +4016,7 @@ end
 terra.cudahome = os.getenv("CUDA_HOME") or (ffi.os == "Windows" and os.getenv("CUDA_PATH")) or "/usr/local/cuda"
 terra.cudalibpaths = ({ OSX = {driver = "/usr/local/cuda/lib/libcuda.dylib", runtime = "$CUDA_HOME/lib/libcudart.dylib", nvvm =  "$CUDA_HOME/nvvm/lib/libnvvm.dylib"}; 
                        Linux =  {driver = "libcuda.so", runtime = "$CUDA_HOME/lib64/libcudart.so", nvvm = "$CUDA_HOME/nvvm/lib64/libnvvm.so"}; 
+                       BSD =  {driver = "libcuda.so", runtime = "$CUDA_HOME/lib64/libcudart.so", nvvm = "$CUDA_HOME/nvvm/lib64/libnvvm.so"}; 
                        Windows = {driver = "nvcuda.dll", runtime = "$CUDA_HOME\\bin\\cudart64_*.dll", nvvm = "$CUDA_HOME\\nvvm\\bin\\nvvm64_*.dll"}; })[ffi.os]
 for name,path in pairs(terra.cudalibpaths) do
 	path = path:gsub("%$CUDA_HOME",terra.cudahome)
