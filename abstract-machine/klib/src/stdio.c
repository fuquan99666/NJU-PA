#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

/**
 * @brief like snprintf, but no check for the buffer size. Compared with printf, it writes the output to the buffer instead of the stdout.
 * @param out pointer to the output buffer
 * @param fmt format string
 * @param ... variable arguments
 * @return the number of characters written to the buffer, excluding the null byte ('\0
 */
int sprintf(char *out, const char *fmt, ...) {

  va_list ap;
  int d;
  char *s;

  char *p = out;

  va_start(ap, fmt);
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt++)
      {
      case 's':
        s = va_arg(ap, char *);
        for (; *s; s++) {
          *p++ = *s;
        }
        break;
      case 'd':
        d = va_arg(ap, int);
        char buf[32];
        itoa(d, buf, 10);
        for (char *q = buf; *q; q++) {
          *p++ = *q;
        }
        break;
    
      default:
        panic("Not implemented");
        break;
      }
    } else {
      *p++ = *fmt++;
    }
  }
  *p = '\0';
  va_end(ap);
  return p - out;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
