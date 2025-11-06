#include "b.h"
#define PROG_NAME "main"
#define BIN_FOLDER "./bin"
#define BUILD_FOLDER "./build"
#define SRC_FOLDER "./src"
#define RAYLIB_SRC "./raylib/src"
#define GC "gcc"
#define GC_FLAGS "-Wall -Wextra -g -O2"

// Определяем платформу
#ifdef __APPLE__
    #define PLATFORM_MACOS
#elif defined(__linux__)
    #define PLATFORM_LINUX
#endif

void make_dirs(void) {
    if (!dir_exists(BIN_FOLDER))
        make_dir(BIN_FOLDER, 0755);
    if (!dir_exists(BUILD_FOLDER))
        make_dir(BUILD_FOLDER, 0755);
    if (!dir_exists(SRC_FOLDER))
        make_dir(SRC_FOLDER, 0755);
}

void build_raylib(void) {
    char *raylib_lib = RAYLIB_SRC"/libraylib.a";
    
    if (!file_exists(raylib_lib)) {
        INFO("Building raylib...\n");
        
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        
        chdir(RAYLIB_SRC);
        RUN("make", "PLATFORM=PLATFORM_DESKTOP");
        
        chdir(cwd);
        
        INFO("Raylib built successfully\n");
    } else {
        VERBOSE("Raylib already built\n");
    }
}

Array* make_o_files(Array *c_files) {
    Array *o_files = array_new(c_files->count);
    
    for (size_t i = 0; i < c_files->count; i++) {
        char* name = path_basename((char *)c_files->items[i]);
        char* obj_file_path = pathjoin(BIN_FOLDER, name);
        obj_file_path = change_extension(obj_file_path, "o");
        
        RUN(GC, "-c", (char *)c_files->items[i], "-o", obj_file_path, 
            GC_FLAGS, "-I"RAYLIB_SRC, "-I./src/ttf", "-I./src/logger");
        
        array_add(o_files, obj_file_path);
    }
    return o_files;
}

void link_o(Array *o_files) {
    char *objs = o_files->items[0];
    for(size_t i = 1; i < o_files->count; i++){
        objs = strcat_new(objs, " ");
        objs = strcat_new(objs, o_files->items[i]); 
    }
    
    #ifdef PLATFORM_MACOS
        RUN(GC, objs, "-o", BUILD_FOLDER"/"PROG_NAME,
            "-L"RAYLIB_SRC, "-lraylib",
            "-framework", "OpenGL",
            "-framework", "Cocoa",
            "-framework", "IOKit",
            "-framework", "CoreVideo",
            "-framework", "CoreAudio",
            "-framework", "CoreFoundation");
    #elif defined(PLATFORM_LINUX)
        RUN(GC, objs, "-o", BUILD_FOLDER"/"PROG_NAME,
            "-L"RAYLIB_SRC, "-lraylib",
            "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11");
    #else
        RUN(GC, objs, "-o", BUILD_FOLDER"/"PROG_NAME,
            "-L"RAYLIB_SRC, "-lraylib");
    #endif
}

int main(void) {
    INFO("Start building\n");
    make_dirs();
    
    build_raylib();
    
    Array *c_files = find_all_files("./src", "c");
    Array *o_files = make_o_files(c_files);
    link_o(o_files);
    
    return 0;
}
