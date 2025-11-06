#ifndef LOGGER_H
#define LOGGER_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"

#define LOG_TIME_PATTERN "%d.%m.%Y %H:%M:%S"
#define MAX_LOG_HANDLERS 10

extern bool print_time_in_log;
extern bool print_where_in_log;

typedef enum log_level {
   DEBUG,
   INFO,
   WARN,
   ERR,
   NONE
} log_level;

typedef struct log_message {
    const char* file;
    int line;
    log_level level;
    const char* level_str;
    const char* color;
    char* time_str;
    char* formatted_message;
} log_message;

typedef void (*log_handler_fn)(const log_message* message, void* user_data);

void logger(const char* file, int line, log_level level, const char* format, ...);
const char* log_level_to_str(log_level level);
const char* log_level_to_color(log_level level);
char* current_time_str(void);

bool register_log_handler(log_handler_fn handler, void* user_data, log_level min_level);
bool unregister_log_handler(log_handler_fn handler);
void clear_log_handlers(void);

void set_log_level(log_level level);
log_level get_log_level(void);

#define dlog(format, ...) logger(__FILE__, __LINE__, DEBUG, format, ##__VA_ARGS__)
#define ilog(format, ...) logger(__FILE__, __LINE__, INFO, format, ##__VA_ARGS__)
#define wlog(format, ...) logger(__FILE__, __LINE__, WARN, format, ##__VA_ARGS__)
#define elog(format, ...) do { \
    logger(__FILE__, __LINE__, ERR, format, ##__VA_ARGS__); \
    exit(1); \
} while(0)

#pragma GCC diagnostic pop
#endif
