#ifndef _MACROS_
#define _MACROS_

#define print_error(stream, text, args...)                                   \
  fprintf(stream, "error in function %s (%s:%d): " text, __func__, __FILE__, \
          __LINE__, ##args)
#define print_warning(stream, text, args...)                                   \
  fprintf(stream, "warning in function %s (%s:%d): " text, __func__, __FILE__, \
          __LINE__, ##args)
#define print_notification(stream, text, args...)                         \
  fprintf(stream, "NOTIFICATION in function %s (%s:%d): " text, __func__, \
          __FILE__, __LINE__, ##args)

#define _pi_ 3.1415926535897932384

#endif /* _MACROS_ */