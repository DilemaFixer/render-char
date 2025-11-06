#ifndef BUILDER_H
#define BUILDER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

char* run_command(const char* cmd, ...);
char* path_dirname(const char* path);
char* strcat_new(const char* str1, const char* str2);

#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"

#define LOG(level, color, fmt, ...) \
    do { \
        time_t t = time(NULL); \
        struct tm *tm_info = localtime(&t); \
        char time_buf[20]; \
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info); \
        fprintf(stderr, "%s[%s] [%s][%s:%d]: " fmt "\n" RESET, color, level, time_buf, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while(0)

#define ERROR(fmt, ...)\
    do { \
        LOG("ERROR", RED, fmt, ##__VA_ARGS__); \
        exit(1); \
    } while(0)

#define WARN(fmt, ...) LOG("WARN", YELLOW, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) LOG("INFO", GREEN, fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...) LOG("DEBUG", BLUE, fmt, ##__VA_ARGS__)
#define VERBOSE(fmt, ...) LOG("VERBOSE", CYAN, fmt, ##__VA_ARGS__)

#define TODO(note) LOG("TODO", MAGENTA, "%s", note)

char* change_extension(const char* file_path, const char* new_ext) {
    if (!file_path || !new_ext) {
        ERROR("Invalid parameters for changing extension");
    }

    char* base_path = strdup(file_path);
    if (!base_path) {
        ERROR("Failed to allocate memory for path");
    }

    char* last_dot = strrchr(base_path, '.');
    if (last_dot) {
        *last_dot = '\0';
    }

    char* new_path = strcat_new(base_path, ".");
    free(base_path);

    if (!new_path) {
        ERROR("Failed to allocate memory for new path");
    }

    char* result = strcat_new(new_path, new_ext);
    free(new_path);

    return result;
}

char* joinstr(int count, ...) {
    va_list args;
    va_start(args, count);

    size_t total_len = 0;
    va_list args_copy;
    va_copy(args_copy, args);

    for (int i = 0; i < count; i++) {
        char* str = va_arg(args_copy, char*);
        if (str) total_len += strlen(str);
        if (i < count - 1) total_len += 1;
    }
    va_end(args_copy);

    char* result = malloc(total_len + 1);
    if (!result) {
        va_end(args);
        ERROR("Failed to allocate memory for joined string");
    }

    char* current = result;
    for (int i = 0; i < count; i++) {
        char* str = va_arg(args, char*);
        if (str) {
            size_t len = strlen(str);
            memcpy(current, str, len);
            current += len;
        }
        if (i < count - 1) *current++ = ' ';
    }
    *current = '\0';
    va_end(args);
    return result;
}

#define RUN(...) do { \
    int arg_count = sizeof((char*[]){__VA_ARGS__})/sizeof(char*); \
    char* cmd = joinstr(arg_count, __VA_ARGS__); \
    if (cmd) { \
        int result = system(cmd); \
        if (result != 0) { \
            ERROR("Command failed with code %d: %s", result, cmd); \
        } else { \
            VERBOSE("%s", cmd); \
        } \
        free(cmd); \
    } else { \
        ERROR("Failed to allocate memory for command"); \
    } \
} while(0)

#define MKDIR_CMD "mkdir -p"
#define RM_CMD "rm"
#define RMDIR_CMD "rm -rf"
#define TOUCH_CMD "touch"
#define CP_CMD "cp"
#define MV_CMD "mv"
#define LS_CMD "ls -la"
#define CAT_CMD "cat"
#define CLEAR_CMD "clear"
#define CHMOD_CMD "chmod"
#define WHICH_CMD "which"
#define LN_CMD "ln -s"
#define GREP_CMD "grep"
#define TAR_CMD "tar"
#define WGET_CMD "wget"

char* exec_path() {
    char* path = malloc(PATH_MAX);
    if (!path) ERROR("Failed to allocate memory for executable path");

#ifdef __linux__
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1) {
        free(path);
        ERROR("Failed to get executable path on Linux");
    }
    path[count] = '\0';
#elif defined(__APPLE__)
    uint32_t size = PATH_MAX;
    if (_NSGetExecutablePath(path, &size) != 0) {
        free(path);
        ERROR("Failed to get executable path on macOS");
    }
    char* real_path = realpath(path, NULL);
    if (real_path) {
        free(path);
        path = real_path;
    }
#else
    free(path);
    ERROR("Unsupported platform for executable path");
#endif

    return path;
}

char* exec_dir() {
    char* path = exec_path();
    if (!path) ERROR("Failed to get executable path");

    char* dir = path_dirname(path);
    if (!dir) {
        free(path);
        ERROR("Failed to get directory from executable path");
    }

    free(path);
    return dir;
}

char* pwd() {
    char* path = malloc(PATH_MAX);
    if (!path) {
        ERROR("Failed to allocate memory for current directory");
    }

    if (getcwd(path, PATH_MAX) == NULL) {
        ERROR("Failed to get current directory");
    }

    return path;
}

bool cd(const char* path) {
    if (!path) {
        WARN("Can't change to NULL directory");
        return false;
    }

    if (chdir(path) != 0) {
        ERROR("Failed to change directory to %s", path);
    }

    VERBOSE("Changed directory to %s", path);
    return true;
}

bool file_exists(const char* path) {
    if (!path) return false;

    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

bool dir_exists(const char* path) {
    if (!path) return false;

    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

bool is_exec(const char* path) {
    if (!path) return false;

    struct stat st;
    return (stat(path, &st) == 0 && (st.st_mode & S_IXUSR));
}

off_t file_size(const char* path) {
    if (!path) return -1;

    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return st.st_size;
}

mode_t file_mode(const char* path) {
    if (!path) return 0;

    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return st.st_mode & 0777;
}

bool make_dir(const char* path, mode_t mode) {
    if (!path || !*path) {
        WARN("Invalid directory path");
        return false;
    }

    if (dir_exists(path)) {
        INFO("Directory %s already exists", path);
        return true;
    }

    if (mkdir(path, mode) == 0) {
        VERBOSE("Created directory %s", path);
        return true;
    }

    char cmd[PATH_MAX + 20];
    snprintf(cmd, sizeof(cmd), "%s %s", MKDIR_CMD, path);
    if (system(cmd) == 0) {
        VERBOSE("Created directory %s with %s", path, MKDIR_CMD);
        return dir_exists(path);
    }

    ERROR("Failed to create directory %s", path);
    return false;
}

bool touch_file(const char* path) {
    if (!path || !*path) {
        WARN("Invalid file path");
        return false;
    }

    if (file_exists(path)) {
        INFO("File %s already exists", path);
        return true;
    }

    int fd = open(path, O_CREAT|O_WRONLY, 0644);
    if (fd >= 0) {
        close(fd);
        VERBOSE("Created file %s", path);
        return true;
    }

    char cmd[PATH_MAX + 10];
    snprintf(cmd, sizeof(cmd), "%s %s", TOUCH_CMD, path);
    if (system(cmd) == 0) {
        VERBOSE("Created file %s with %s", path, TOUCH_CMD);
        return file_exists(path);
    }

    ERROR("Failed to create file %s", path);
    return false;
}

bool touch_with_ext(const char* name, const char* ext) {
    if (!name || !ext) {
        WARN("Invalid parameters");
        return false;
    }

    char* full_name = NULL;
    char* dot = strrchr(name, '.');

    if (dot && strcmp(dot + 1, ext) == 0) {
        full_name = strdup(name);
    } else {
        size_t name_len = strlen(name);
        size_t ext_len = strlen(ext);
        full_name = malloc(name_len + 1 + ext_len + 1);

        if (!full_name) {
            ERROR("Failed to allocate memory for filename");
        }

        strcpy(full_name, name);
        full_name[name_len] = '.';
        strcpy(full_name + name_len + 1, ext);
    }

    bool result = touch_file(full_name);
    free(full_name);
    return result;
}

bool remove_file(const char* path) {
    if (!path || !*path) {
        WARN("Invalid file path");
        return false;
    }

    if (!file_exists(path)) {
        INFO("File %s doesn't exist", path);
        return true;
    }

    if (unlink(path) == 0) {
        VERBOSE("Deleted file %s", path);
        return true;
    }

    char cmd[PATH_MAX + 5];
    snprintf(cmd, sizeof(cmd), "%s %s", RM_CMD, path);
    if (system(cmd) == 0) {
        VERBOSE("Deleted file %s with %s", path, RM_CMD);
        return !file_exists(path);
    }

    ERROR("Failed to delete file %s", path);
    return false;
}

bool remove_dir(const char* path) {
    if (!path || !*path) {
        WARN("Invalid directory path");
        return false;
    }

    if (!dir_exists(path)) {
        INFO("Directory %s doesn't exist", path);
        return true;
    }

    char cmd[PATH_MAX + 10];
    snprintf(cmd, sizeof(cmd), "%s %s", RMDIR_CMD, path);
    if (system(cmd) == 0) {
        VERBOSE("Removed directory %s", path);
        return !dir_exists(path);
    }

    ERROR("Failed to remove directory %s", path);
    return false;
}

bool copy_file(const char* src, const char* dst) {
    if (!src || !dst) {
        WARN("Invalid parameters");
        return false;
    }

    if (!file_exists(src)) {
        WARN("Source file %s doesn't exist", src);
        return false;
    }

    FILE *source = fopen(src, "rb");
    if (!source) {
        ERROR("Cannot open source file %s", src);
    }

    FILE *dest = fopen(dst, "wb");
    if (!dest) {
        fclose(source);
        ERROR("Cannot open destination file %s", dst);
    }

    char buffer[8192];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        if (fwrite(buffer, 1, bytesRead, dest) != bytesRead) {
            fclose(source);
            fclose(dest);
            ERROR("Write error");
        }
    }

    fclose(source);
    fclose(dest);

    if (ferror(source)) {
        ERROR("Read error");
    }

    VERBOSE("Copied %s to %s", src, dst);
    return file_exists(dst);
}

bool move_file(const char* src, const char* dst) {
    if (!src || !dst) {
        WARN("Invalid parameters");
        return false;
    }

    if (!file_exists(src)) {
        WARN("Source file %s doesn't exist", src);
        return false;
    }

    if (rename(src, dst) == 0) {
        VERBOSE("Moved %s to %s", src, dst);
        return true;
    }

    if (copy_file(src, dst)) {
        return remove_file(src);
    }

    char cmd[2 * PATH_MAX + 10];
    snprintf(cmd, sizeof(cmd), "%s %s %s", MV_CMD, src, dst);
    if (system(cmd) == 0) {
        VERBOSE("Moved %s to %s with %s", src, dst, MV_CMD);
        return file_exists(dst) && !file_exists(src);
    }

    ERROR("Failed to move %s to %s", src, dst);
    return false;
}

bool rename_file(const char* old_path, const char* new_path) {
    return move_file(old_path, new_path);
}

void list_dir(const char* path) {
    char cmd[PATH_MAX + 10];
    snprintf(cmd, sizeof(cmd), "%s %s", LS_CMD, path ? path : ".");
    system(cmd);
}

bool write_to_file(const char* path, const char* content) {
    if (!path || !content) {
        WARN("Invalid parameters");
        return false;
    }

    FILE* file = fopen(path, "w");
    if (!file) {
        ERROR("Failed to open file %s for writing", path);
    }

    size_t content_len = strlen(content);
    size_t written = fwrite(content, 1, content_len, file);
    fclose(file);

    if (written != content_len) {
        ERROR("Failed to write all content to file %s", path);
    }

    VERBOSE("Wrote %zu bytes to file %s", written, path);
    return true;
}

char* read_from_file(const char* path) {
    if (!path) {
        WARN("Invalid file path");
        ERROR("Cannot read from NULL path");
    }

    FILE* file = fopen(path, "r");
    if (!file) {
        ERROR("Failed to open file %s for reading", path);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        INFO("File %s is empty", path);
        fclose(file);
        char* empty_str = malloc(1);
        if (empty_str) empty_str[0] = '\0';
        else ERROR("Failed to allocate memory for empty string");
        return empty_str;
    }

    char* content = malloc(file_size + 1);
    if (!content) {
        ERROR("Failed to allocate memory for file content");
    }

    size_t read_size = fread(content, 1, file_size, file);
    fclose(file);

    if (read_size != file_size) {
        ERROR("Failed to read entire file %s", path);
    }

    content[file_size] = '\0';
    VERBOSE("Read %zu bytes from file %s", read_size, path);
    return content;
}

void cat_file(const char* path) {
    if (!path || !file_exists(path)) {
        WARN("Invalid file path or file doesn't exist: %s", path ? path : "(null)");
        return;
    }

    char cmd[PATH_MAX + 10];
    snprintf(cmd, sizeof(cmd), "%s %s", CAT_CMD, path);
    system(cmd);
}

void clear_screen() {
    system(CLEAR_CMD);
}

bool change_mode(const char* path, mode_t mode) {
    if (!path || !file_exists(path)) {
        WARN("Invalid file path or file doesn't exist: %s", path ? path : "(null)");
        return false;
    }

    if (chmod(path, mode) == 0) {
        VERBOSE("Changed mode of %s to %o", path, mode);
        return true;
    }

    char cmd[PATH_MAX + 20];
    char mode_str[5];
    snprintf(mode_str, sizeof(mode_str), "%o", mode);
    snprintf(cmd, sizeof(cmd), "%s %s %s", CHMOD_CMD, mode_str, path);
    if (system(cmd) == 0) {
        VERBOSE("Changed mode of %s to %s with %s", path, mode_str, CHMOD_CMD);
        return (file_mode(path) == mode);
    }

    ERROR("Failed to change mode of %s to %o", path, mode);
    return false;
}

bool make_symlink(const char* target, const char* link_path) {
    if (!target || !link_path) {
        WARN("Invalid parameters");
        return false;
    }

    if (symlink(target, link_path) == 0) {
        VERBOSE("Created symlink from %s to %s", link_path, target);
        return true;
    }

    char cmd[2 * PATH_MAX + 10];
    snprintf(cmd, sizeof(cmd), "%s %s %s", LN_CMD, target, link_path);
    if (system(cmd) == 0) {
        VERBOSE("Created symlink from %s to %s with %s", link_path, target, LN_CMD);

        char buf[PATH_MAX];
        ssize_t len = readlink(link_path, buf, sizeof(buf) - 1);
        if (len != -1) {
            buf[len] = '\0';
            return (strcmp(buf, target) == 0);
        }
    }

    ERROR("Failed to create symlink from %s to %s", link_path, target);
    return false;
}

char* find_command(const char* cmd) {
    if (!cmd) {
        WARN("Invalid command name");
        ERROR("Cannot find NULL command");
    }

    if (strchr(cmd, '/')) {
        if (is_exec(cmd)) {
            return strdup(cmd);
        }
        return NULL;
    }

    char cmd_buf[PATH_MAX];
    snprintf(cmd_buf, sizeof(cmd_buf), "%s %s", WHICH_CMD, cmd);

    FILE* pipe = popen(cmd_buf, "r");
    if (!pipe) {
        ERROR("Failed to execute which command");
    }

    char buffer[PATH_MAX];
    char* result = NULL;

    if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (buffer[0] != '\0') {
            result = strdup(buffer);
        }
    }

    pclose(pipe);
    return result;
}

char* timestamp() {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    char* ts = malloc(20);
    if (!ts) {
        ERROR("Failed to allocate memory for timestamp");
    }

    strftime(ts, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    return ts;
}

char* datestamp() {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    char* date = malloc(11);
    if (!date) {
        ERROR("Failed to allocate memory for date");
    }

    strftime(date, 11, "%Y-%m-%d", tm_info);
    return date;
}

char* strcat_new(const char* str1, const char* str2) {
    if (!str1) str1 = "";
    if (!str2) str2 = "";

    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    char* result = malloc(len1 + len2 + 1);
    if (!result) {
        ERROR("Failed to allocate memory for string concatenation");
    }

    strcpy(result, str1);
    strcat(result, str2);
    return result;
}

char* strcat_with_space(const char* str1, const char* str2) {
    if (!str1) str1 = "";
    if (!str2) str2 = "";

    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    char* result = malloc(len1 + 1 + len2 + 1);
    if (!result) {
        ERROR("Failed to allocate memory for string concatenation with space");
    }

    strcpy(result, str1);

    if (len1 > 0 && len2 > 0) {
        strcat(result, " ");
    }

    strcat(result, str2);
    return result;
}

char* strreplace(const char* str, const char* old_sub, const char* new_sub) {
    if (!str || !old_sub || !new_sub) {
        WARN("Invalid parameters");
        ERROR("Invalid parameters for string replacement");
    }

    size_t old_len = strlen(old_sub);
    if (old_len == 0) return strdup(str);

    size_t new_len = strlen(new_sub);
    size_t count = 0;

    const char* tmp = str;
    while ((tmp = strstr(tmp, old_sub)) != NULL) {
        count++;
        tmp += old_len;
    }

    if (count == 0) return strdup(str);

    size_t str_len = strlen(str);
    size_t result_len = str_len + count * (new_len - old_len);

    char* result = malloc(result_len + 1);
    if (!result) {
        ERROR("Failed to allocate memory for string replacement");
    }

    char* dest = result;
    const char* src = str;
    const char* match;

    while ((match = strstr(src, old_sub)) != NULL) {
        size_t prefix_len = match - src;
        memcpy(dest, src, prefix_len);
        dest += prefix_len;

        memcpy(dest, new_sub, new_len);
        dest += new_len;

        src = match + old_len;
    }

    strcpy(dest, src);
    return result;
}

char* pathjoin(const char* path1, const char* path2) {
    if (!path1) path1 = "";
    if (!path2) path2 = "";

    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);

    bool has_separator = (len1 > 0 && path1[len1-1] == '/');
    bool needs_separator = (len1 > 0 && len2 > 0 && !has_separator);

    size_t result_len = len1 + (needs_separator ? 1 : 0) + len2;
    char* result = malloc(result_len + 1);

    if (!result) {
        ERROR("Failed to allocate memory for path joining");
    }

    strcpy(result, path1);

    if (needs_separator) {
        result[len1] = '/';
        result[len1 + 1] = '\0';
    }

    strcat(result, path2);
    return result;
}

char* path_basename(const char* path) {
    if (!path) {
        WARN("Invalid path");
        ERROR("Invalid path for basename");
    }

    const char* last_slash = strrchr(path, '/');

    if (!last_slash) {
        return strdup(path);
    }

    return strdup(last_slash + 1);
}

char* path_dirname(const char* path) {
    if (!path) {
        WARN("Invalid path");
        ERROR("Invalid path for dirname");
    }

    char* path_copy = strdup(path);
    if (!path_copy) {
        ERROR("Failed to allocate memory");
    }

    char* last_slash = strrchr(path_copy, '/');

    if (!last_slash) {
        free(path_copy);
        return strdup(".");
    }

    *last_slash = '\0';

    if (path_copy[0] == '\0') {
        free(path_copy);
        return strdup("/");
    }

    return path_copy;
}

char* extname(const char* path) {
    if (!path) {
        WARN("Invalid path");
        ERROR("Invalid path for extname");
    }

    char* base = path_basename(path);
    if (!base) ERROR("Failed to get basename for extname");

    char* dot = strrchr(base, '.');
    char* ext;

    if (!dot || dot == base) {
            free(base);
            return strdup("");
        }

        ext = strdup(dot + 1);
        free(base);
        return ext;
    }

    char* run_command(const char* cmd, ...) {
        if (!cmd) {
            WARN("Invalid command");
            ERROR("Cannot run NULL command");
        }

        va_list args;
        va_start(args, cmd);

        va_list args_copy;
        va_copy(args_copy, args);
        int arg_count = 0;
        while (va_arg(args_copy, const char*) != NULL) {
            arg_count++;
        }
        va_end(args_copy);

        size_t cmd_len = strlen(cmd);
        size_t total_len = cmd_len;
        char** arg_array = malloc((arg_count + 1) * sizeof(char*));

        if (!arg_array) {
            ERROR("Failed to allocate memory for command arguments");
        }

        arg_array[0] = (char*)cmd;

        for (int i = 0; i < arg_count; i++) {
            const char* arg = va_arg(args, const char*);
            arg_array[i+1] = (char*)arg;
            total_len += strlen(arg) + 1;
        }

        va_end(args);

        char* full_cmd = malloc(total_len + 1);
        if (!full_cmd) {
            free(arg_array);
            ERROR("Failed to allocate memory for command");
        }

        strcpy(full_cmd, cmd);
        size_t pos = cmd_len;

        for (int i = 1; i <= arg_count; i++) {
            full_cmd[pos++] = ' ';
            strcpy(full_cmd + pos, arg_array[i]);
            pos += strlen(arg_array[i]);
        }

        free(arg_array);

        FILE* pipe = popen(full_cmd, "r");
        free(full_cmd);

        if (!pipe) {
            ERROR("Failed to execute command");
        }

        char buffer[128];
        char* result = NULL;
        size_t total_size = 0;

        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            size_t buffer_len = strlen(buffer);

            if (!result) {
                result = malloc(buffer_len + 1);
                if (!result) {
                    pclose(pipe);
                    ERROR("Failed to allocate memory");
                }
                strcpy(result, buffer);
                total_size = buffer_len;
            } else {
                char* new_result = realloc(result, total_size + buffer_len + 1);
                if (!new_result) {
                    free(result);
                    pclose(pipe);
                    ERROR("Failed to reallocate memory");
                }
                result = new_result;
                strcpy(result + total_size, buffer);
                total_size += buffer_len;
            }
        }

        int status = pclose(pipe);
        if (status != 0) {
            WARN("Command exited with status %d", status);
        }

        if (!result) {
            result = malloc(1);
            if (!result) {
                ERROR("Failed to allocate memory for empty result");
            }
            result[0] = '\0';
        }

        return result;
    }

    char* grep(const char* pattern, const char* file) {
        if (!pattern || !file) {
            WARN("Invalid parameters");
            ERROR("Invalid parameters for grep");
        }

        char cmd[PATH_MAX + 100];
        snprintf(cmd, sizeof(cmd), "%s %s %s", GREP_CMD, pattern, file);
        system(cmd);

        return run_command(GREP_CMD, pattern, file, NULL);
    }

    char* find_files(const char* dir, const char* pattern) {
        if (!dir || !pattern) {
            WARN("Invalid parameters");
            ERROR("Invalid parameters for find_files");
        }

        return run_command("find", dir, "-name", pattern, NULL);
    }

    bool tar_create(const char* archive, const char* files) {
        if (!archive || !files) {
            WARN("Invalid parameters");
            ERROR("Invalid parameters for tar_create");
        }

        char cmd[PATH_MAX * 2 + 20];
        snprintf(cmd, sizeof(cmd), "%s -czf %s %s", TAR_CMD, archive, files);
        if (system(cmd) == 0) {
            VERBOSE("Created archive %s from %s", archive, files);
            return file_exists(archive);
        }

        ERROR("Failed to create archive %s", archive);
        return false;
    }

    bool tar_extract(const char* archive, const char* dest) {
        if (!archive || !file_exists(archive)) {
            WARN("Archive doesn't exist: %s", archive ? archive : "(null)");
            ERROR("Invalid archive for extraction");
        }

        if (dest && !dir_exists(dest)) {
            make_dir(dest, 0755);
        }

        char cmd[PATH_MAX * 2 + 20];

        if (dest) {
            char* old_dir = pwd();
            cd(dest);
            snprintf(cmd, sizeof(cmd), "%s -xzf %s", TAR_CMD, archive);
            int result = system(cmd);
            cd(old_dir);
            free(old_dir);

            if (result == 0) {
                VERBOSE("Extracted archive %s to %s", archive, dest);
                return true;
            }
        } else {
            snprintf(cmd, sizeof(cmd), "%s -xzf %s", TAR_CMD, archive);
            if (system(cmd) == 0) {
                VERBOSE("Extracted archive %s", archive);
                return true;
            }
        }

        ERROR("Failed to extract archive %s", archive);
        return false;
    }

    bool wget_file(const char* url, const char* output) {
        if (!url) {
            WARN("Invalid URL");
            ERROR("Invalid URL for download");
        }

        char cmd[PATH_MAX + 1024];

        if (output) {
            snprintf(cmd, sizeof(cmd), "%s -O %s %s", WGET_CMD, output, url);
            if (system(cmd) == 0) {
                VERBOSE("Downloaded %s to %s", url, output);
                return file_exists(output);
            }
        } else {
            snprintf(cmd, sizeof(cmd), "%s %s", WGET_CMD, url);
            if (system(cmd) == 0) {
                VERBOSE("Downloaded %s", url);
                return true;
            }
        }

        ERROR("Failed to download %s", url);
        return false;
    }

    char* tmp_file() {
        char* template = strdup("/tmp/tmp.XXXXXX");
        if (!template) {
            ERROR("Failed to allocate memory");
        }

        int fd = mkstemp(template);
        if (fd == -1) {
            ERROR("Failed to create temporary file");
        }

        close(fd);
        return template;
    }

    char* tmp_dir() {
        char* template = strdup("/tmp/tmp.XXXXXX");
        if (!template) {
            ERROR("Failed to allocate memory");
        }

        char* result = mkdtemp(template);
        if (!result) {
            ERROR("Failed to create temporary directory");
        }

        return template;
    }

    typedef struct {
        char** items;
        int count;
        int capacity;
    } Array;

    Array* array_new(int initial_capacity) {
        Array* arr = malloc(sizeof(Array));
        if (!arr) {
            ERROR("Failed to allocate memory for array");
        }

        arr->items = malloc(initial_capacity * sizeof(char*));
        if (!arr->items) {
            free(arr);
            ERROR("Failed to allocate memory for array items");
        }

        arr->count = 0;
        arr->capacity = initial_capacity;
        return arr;
    }

    void array_add(Array* arr, const char* item) {
        if (!arr || !item) return;

        if (arr->count >= arr->capacity) {
            int new_capacity = arr->capacity * 2;
            char** new_items = realloc(arr->items, new_capacity * sizeof(char*));

            if (!new_items) {
                ERROR("Failed to resize array");
            }

            arr->items = new_items;
            arr->capacity = new_capacity;
        }

        arr->items[arr->count] = strdup(item);
        if (!arr->items[arr->count]) {
            ERROR("Failed to duplicate string");
        }

        arr->count++;
    }

    void array_free(Array* arr) {
        if (!arr) return;

        for (int i = 0; i < arr->count; i++) {
            free(arr->items[i]);
        }

        free(arr->items);
        free(arr);
    }

    void shift(int* argc, char*** argv) {
        if (!argc || !argv || *argc <= 0) return;

        (*argc)--;
        (*argv)++;
    }

    char* get_arg(int argc, char** argv, int index) {
        if (index < 0 || index >= argc) return NULL;
        return argv[index];
    }

    bool has_arg(int argc, char** argv, const char* arg) {
        if (!arg) return false;

        for (int i = 0; i < argc; i++) {
            if (argv[i] && strcmp(argv[i], arg) == 0)
                return true;
        }

        return false;
    }

    char* get_arg_value(int argc, char** argv, const char* arg) {
        if (!arg) return NULL;

        for (int i = 0; i < argc - 1; i++) {
            if (argv[i] && strcmp(argv[i], arg) == 0)
                return argv[i+1];
        }

        return NULL;
    }

    Array* find_all_files(const char* dir, const char* ext) {
        DIR* d = opendir(dir);
        if (!d) {
            ERROR("Cannot open directory: %s", dir);
        }

        Array* result = array_new(10);
        if (!result) {
            closedir(d);
            ERROR("Failed to create array for files");
        }

        struct dirent* entry;
        while ((entry = readdir(d)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char* path = pathjoin(dir, entry->d_name);
            if (!path) continue;

            struct stat st;
            if (stat(path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    Array* subdir_files = find_all_files(path, ext);
                    if (subdir_files) {
                        for (int i = 0; i < subdir_files->count; i++) {
                            array_add(result, subdir_files->items[i]);
                        }
                        array_free(subdir_files);
                    }
                } else if (S_ISREG(st.st_mode)) {
                    char* file_ext = extname(path);
                    if (file_ext && strcmp(file_ext, ext) == 0) {
                        array_add(result, path);
                        VERBOSE("Found file: %s", path);
                    }
                    free(file_ext);
                }
            }

            free(path);
        }

        closedir(d);
        return result;
    }

    char* change_folder_name(const char* path, const char* folder_name, size_t index) {
        if (!path || !folder_name) {
            ERROR("Invalid parameters for changing folder name");
        }

        char* path_copy = strdup(path);
        if (!path_copy) {
            ERROR("Failed to allocate memory for path copy");
        }

        char** components = NULL;
        size_t comp_count = 0;

        char* token = strtok(path_copy, "/\\");
        while (token) {
            components = realloc(components, (comp_count + 1) * sizeof(char*));
            if (!components) {
                free(path_copy);
                ERROR("Failed to allocate memory for components array");
            }

            components[comp_count] = strdup(token);
            comp_count++;

            token = strtok(NULL, "/\\");
        }

        free(path_copy);

        if (index >= comp_count) {
            for (size_t i = 0; i < comp_count; i++) {
                free(components[i]);
            }
            free(components);
            return strdup(path);
        }

        free(components[index]);
        components[index] = strdup(folder_name);

        size_t result_len = 1;
        for (size_t i = 0; i < comp_count; i++) {
            result_len += strlen(components[i]) + 1;
        }

        char* result = malloc(result_len);
        if (!result) {
            for (size_t i = 0; i < comp_count; i++) {
                free(components[i]);
            }
            free(components);
            ERROR("Failed to allocate memory for result path");
        }

        result[0] = '\0';

        for (size_t i = 0; i < comp_count; i++) {
            strcat(result, components[i]);
            if (i < comp_count - 1) {
                strcat(result, "/");
            }
        }

        for (size_t i = 0; i < comp_count; i++) {
            free(components[i]);
        }
        free(components);

        return result;
    }

    #endif // BUILDER_H
