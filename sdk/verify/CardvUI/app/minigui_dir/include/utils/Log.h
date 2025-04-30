#ifndef _UTILS_LOG_H_
#define _UTILS_LOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_ANDROID_LOG
#include <android/log.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifdef USE_ANDROID_LOG_PRIORITY
extern android_LogPriority global_android_log_priority;
#endif

#ifndef LOG_TAG
#define LOG_TAG "zkgui"
#endif

inline const char* __this_file_shortname(const char* filename) {
  const char* p;
  if ((p = strrchr(filename, '\\')) != NULL) {
    return p + 1;
  } else if ((p = strrchr(filename, '/')) != NULL) {
    return p + 1;
  } else {
    return filename;
  }
}

#define CONSOLE_COLOR_NONE          "\033[0m"
#define CONSOLE_COLOR_RED           "\033[31m"
#define CONSOLE_COLOR_YELLOW        "\033[33m"

#ifdef USE_ANDROID_LOG_PRIORITY
  #define APP_LOG(level, TAG, fmt, args...)         \
  {                                                 \
      if (level >= global_android_log_priority) {                 \
        __android_log_print(level,TAG,              \
            fmt,##args);                            \
      }                                             \
  }
#else
  #define APP_LOG(level, TAG, fmt, args...)         \
  {                                                 \
        __android_log_print(level,TAG,              \
            fmt,##args);                            \
  }
#endif

#define LOGV(fmt, args...)  APP_LOG(ANDROID_LOG_VERBOSE, LOG_TAG, fmt,##args);
#define LOGD(fmt, args...)  APP_LOG(ANDROID_LOG_DEBUG, LOG_TAG, fmt,##args);
#define LOGI(fmt, args...)  APP_LOG(ANDROID_LOG_INFO, LOG_TAG, fmt,##args);
#define LOGW(fmt, args...)  APP_LOG(ANDROID_LOG_WARN, LOG_TAG, \
    CONSOLE_COLOR_YELLOW fmt CONSOLE_COLOR_NONE,##args);
#define LOGE(fmt, args...)  APP_LOG(ANDROID_LOG_ERROR, LOG_TAG, \
    CONSOLE_COLOR_RED fmt CONSOLE_COLOR_NONE,##args);

#else
#define LOGV(fmt,...)   fprintf(stderr, fmt, ##__VA_ARGS__)
#define LOGD(fmt,...)   fprintf(stderr, fmt, ##__VA_ARGS__)
#define LOGI(fmt,...)   fprintf(stderr, fmt, ##__VA_ARGS__)
#define LOGW(fmt,...)   fprintf(stderr, fmt, ##__VA_ARGS__)
#define LOGE(fmt,...)   fprintf(stderr, fmt, ##__VA_ARGS__)
#endif

#endif /* _UTILS_LOG_H_ */
