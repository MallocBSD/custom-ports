--- src/tcompiler.cpp.orig	2017-10-17 06:05:35 UTC
+++ src/tcompiler.cpp
@@ -2730,13 +2730,13 @@ static int terra_disassemble(lua_State *
 static bool FindLinker(terra_State * T, LLVM_PATH_TYPE * linker) {
 #ifndef _WIN32
     #if LLVM_VERSION >= 36
-        *linker = *sys::findProgramByName("gcc");
+        *linker = *sys::findProgramByName("cc");
         return *linker == "";
     #elif LLVM_VERSION >= 34
-        *linker = sys::FindProgramByName("gcc");
+        *linker = sys::FindProgramByName("cc");
         return *linker == "";
     #else
-        *linker = sys::Program::FindProgramByName("gcc");
+        *linker = sys::Program::FindProgramByName("cc");
         return linker->isEmpty();
     #endif
 #else
@@ -2832,7 +2832,9 @@ static bool SaveSharedObject(TerraCompil
 	cmd.push_back("-g");
     cmd.push_back("-shared");
     cmd.push_back("-Wl,-export-dynamic");
+#ifdef __linux__
     cmd.push_back("-ldl");
+#endif
 	cmd.push_back("-fPIC");
 #endif
 
