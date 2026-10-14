#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

/**
 * @brief printf function write output to stdout according to the format string fmt
 * @param fmt format string 
 * @param ... variable arguments 
 * @return If successful, the total number of bytes printed (excluding the null byte used to end output to strings).
 * On error, a negative value is returned, and errno is set to indicate the error.
 */
int printf(const char *fmt, ...) {

  va_list ap;
  int d;
  char *s;
  int total = 0;

  va_start(ap, fmt);
  while(*fmt) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt++)
      {
      case 's':
        s = va_arg(ap, char *);
        for (; *s; s++) {
          putch(*s);
          total++;
        }
        break;
      case 'd':
        d = va_arg(ap, int);
        char buf[32];
        itoa(d, buf, 10);
        for (char *q = buf; *q; q++) {
          putch(*q);
          total++;
        }
        break;
      default:
        panic("Not implemented");
        break;
      }
    } else {
      putch(*fmt++);
      total++;
    }
  }
  va_end(ap);
  return total;
}

/**
 * @brief vsprintf like sprintf, but takes a va_list instead of a variable number of arguments.
 * @param out pointer to the output buffer
 * @param fmt format string
 * @param ap variable arguments list
 * @return On success, the number of characters written to the buffer, excluding the null byte ('\0')
 * On error, a negative value is returned, and errno is set to indicate the error.
 */
int vsprintf(char *out, const char *fmt, va_list ap) {
  int d;
  char *s;

  char *p = out;

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
  return p - out;
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

/**
 * @brief snprintf function like sprintf, but writes at most n bytes (including the terminating null byte ('\0')) to the output buffer.
 * @param out pointer to the output buffer
 * @param n maximum number of bytes to write
 * @param fmt format string
 * @param ... variable arguments
 * @return the number of bytes printed (excluding the null byte used to end output to strings)
 * On truncation, the number of bytes (excluding the terminating null byte) which would have been written to the final string if enough space had been available is returned. Thus, a return value of n or more means that the output was truncated.  
 * On error, a negative value is returned, and errno is set to indicate the error.
 */
int snprintf(char *out, size_t n, const char *fmt, ...) {

  va_list ap;
  int d;
  char *s;

  char *p = out;

  va_start(ap, fmt);
  while (*fmt && (p - out) < n - 1) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt++)
      {
      case 's':
        s = va_arg(ap, char *);
        for (; *s && (p - out) < n - 1; s++) {
          *p++ = *s;
        }
        break;
      case 'd':
        d = va_arg(ap, int);
        char buf[32];
        itoa(d, buf, 10);
        for (char *q = buf; *q && (p - out) < n - 1; q++) {
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

/**
 * @brief vsnprintf like snprintf, but takes a va_list instead of a variable number of arguments.
 * @param out pointer to the output buffer
 * @param n maximum number of bytes to write
 * @param fmt format string
 * @param ap variable arguments list
 * @return the number of bytes printed (excluding the null byte used to end output to strings) 
 * On truncation, the number of bytes (excluding the terminating null byte) which would have been written to the final string if enough space had been available is returned. Thus, a return value of n or more means that the output was truncated.
 * On error, a negative value is returned, and errno is set to indicate the error.
 */
int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  int d;
  char *s;

  char *p = out;

  while (*fmt && (p - out) < n - 1) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt++)
      {
      case 's':
        s = va_arg(ap, char *);
        for (; *s && (p - out) < n - 1; s++) {
          *p++ = *s;
        }
        break;
      case 'd':
        d = va_arg(ap, int);
        char buf[32];
        itoa(d, buf, 10);
        for (char *q = buf; *q && (p - out) < n - 1; q++) {
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
  return p - out;
}

#endif
