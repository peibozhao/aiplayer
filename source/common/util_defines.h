
#include "glog/logging.h"
#include <stdio.h>

#define LOG_TYPE(type, format, ...) \
{ \
    char string_log[1024] = {0}; \
    sprintf(string_log, format, ##__VA_ARGS__); \
    LOG(type) << string_log; \
}


#define LOG_INFO(format, ...) LOG_TYPE(INFO, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) LOG_TYPE(ERROR, format, ##__VA_ARGS__)

