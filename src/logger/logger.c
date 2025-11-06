#include "logger.h"
#include <stdarg.h>

bool print_time_in_log = false;
bool print_where_in_log = false;

static log_level global_min_level = DEBUG;

typedef struct {
    log_handler_fn handler;
    void* user_data;
    log_level min_level;
    bool active;
} handler_entry;

static handler_entry handlers[MAX_LOG_HANDLERS] = {0};
static int handler_count = 0;
static bool default_handler_registered = false;

void default_log_handler(const log_message* message, void* user_data) {
    (void)user_data;     
    printf("%s[%s]%s ", message->color, message->level_str, RESET);
    
    if (print_time_in_log) {
        printf("[%s] ", message->time_str);
    }
    
    if (print_where_in_log) {
        printf("[%s:%d] ", message->file, message->line);
    }
    
    printf(": %s\n", message->formatted_message);
}

static void init_logger() {
    if (!default_handler_registered) {
        register_log_handler(default_log_handler, NULL, DEBUG);
        default_handler_registered = true;
    }
}

const char *log_level_to_str(log_level level) {
    switch (level) {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case WARN:
        return "WARN";
    case ERR:
        return "ERROR";
    case NONE:
        return "NONE";
    default:
        return "UNKNOWN";
    }
}

const char *log_level_to_color(log_level level) {
    switch (level) {
    case DEBUG:
        return BLUE;
    case INFO:
        return GREEN;
    case WARN:
        return YELLOW;
    case ERR:
        return RED;
    default:
        return RESET;
    }
}

char *current_time_str(void) {
    static char time_str[64];
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    strftime(time_str, sizeof(time_str), LOG_TIME_PATTERN, time_info);
    return time_str;
}

void logger(const char *file, int line, log_level level, const char *format,
            ...) {
    init_logger();
    
    if (level < global_min_level) {
        return;
    }
    
    log_message message = {
        .file = file,
        .line = line,
        .level = level,
        .level_str = log_level_to_str(level),
        .color = log_level_to_color(level),
        .time_str = current_time_str()
    };
    
    char formatted_message[1024]; 
    va_list args;
    va_start(args, format);
    vsnprintf(formatted_message, sizeof(formatted_message), format, args);
    va_end(args);
    message.formatted_message = formatted_message;
    
    for (int i = 0; i < handler_count; i++) {
        if (handlers[i].active && level >= handlers[i].min_level) {
            handlers[i].handler(&message, handlers[i].user_data);
        }
    }
}

bool register_log_handler(log_handler_fn handler, void* user_data, log_level min_level) {
    if (!handler || handler_count >= MAX_LOG_HANDLERS) {
        return false;
    }
    
    for (int i = 0; i < handler_count; i++) {
        if (handlers[i].handler == handler) {
            handlers[i].user_data = user_data;
            handlers[i].min_level = min_level;
            handlers[i].active = true;
            return true;
        }
    }
    
    handlers[handler_count].handler = handler;
    handlers[handler_count].user_data = user_data;
    handlers[handler_count].min_level = min_level;
    handlers[handler_count].active = true;
    handler_count++;
    
    return true;
}

bool unregister_log_handler(log_handler_fn handler) {
    if (!handler) {
        return false;
    }
    
    for (int i = 0; i < handler_count; i++) {
        if (handlers[i].handler == handler) {
            handlers[i].active = false;
            return true;
        }
    }
    
    return false;
}

void clear_log_handlers(void) {
    for (int i = 0; i < handler_count; i++) {
        handlers[i].active = false;
    }
    handler_count = 0;
    default_handler_registered = false;
}

void set_log_level(log_level level) {
    global_min_level = level;
}

log_level get_log_level(void) {
    return global_min_level;
}
