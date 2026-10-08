#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

/**
 * @brief strlen function calculates the length of the string s, excluding the terminating null byte ('\0')
 * @param s pointer to the string
 * @return the number of bytes in the string s
 */
size_t strlen(const char *s) {
  const char *p = s;
  while (*p) {
    p++;
  }
  return p - s;
}

/**
 * @brief strcpy function copies the string pointed to by src , into a string at the buffer pointed to by dst
 * @param dst pointer to the destination buffer
 * @param src pointer to the source string
 * @return pointer to the destination buffer dst
 */
char *strcpy(char *dst, const char *src) {
  // You should make sure that the destination buffer is large enough -- strlen(src) + 1
  // Besides, src and dst can't overlap ...

  const char *p = src;
  char *q = dst;
  while (*p) {
    *q++ = *p++;
  }
  *q = '\0';
  return dst;
}

/**
 * @brief like strcpy, but copies at most n bytes. If src is less than n bytes long, the remainder of dst is filled with '\0' characters.
 * @param dst pointer to the destination buffer
 * @param src pointer to the source string
 * @param n maximum number of bytes to copy
 * @return pointer to the destination buffer dst
 */
char *strncpy(char *dst, const char *src, size_t n) {
  const char *p = src;
  char *q = dst;
  size_t i;
  for (i = 0; i < n && *p; i++) {
    *q++ = *p++;
  }
  for (; i < n; i++) {
    *q++ = '\0';
  }
  *q = '\0'; // for safety, ensure the destination string is null-terminated
  return dst;
}

/**
 * @brief strcat function catenates the string pointed to by src, after the string pointed to by dst (overwriting the terminating null byte)
 * @param dst pointer to the destination buffer
 * @param src pointer to the src string 
 * @return pointer to the destination buffer dst 
 */
char *strcat(char *dst, const char *src) {
  // Same, you should make sure dst and src not overlap
  // And, the dst buffer is big enough to hold itself and src 

  char *p = dst;
  const char *q = src;
  p += strlen(dst);

  while(*q) {
    *p++ = *q++;
  }
  *p = '\0';
  return dst;
}

/**
 * @brief strcmp function compares two strings 
 * @param s1 first string
 * @param s2 second string
 * @return 0 if the two strings are equal, a negative value if s1 < s2, and a positive value if s1 > s2
 */
int strcmp(const char *s1, const char *s2) {
  // It is equivalent to memcmp(s1, s2, min(strlen(s1), strlen(s2)) + 1)
  while (*s1 && *s2 && *s1 == *s2) {
    s1++;
    s2++;
  }
  // In strcmp function, we treat the s1 and s2 as unsigned char
  return (unsigned char)*s1 - (unsigned char)*s2;
}

/**
 * @brief like strcmp, but compares at most n bytes. It is equivalent to memcmp(s1, s2, n)
 * @param s1 first string
 * @param s2 second string
 * @param n maximum number of characters to compare
 * @return 0 if the two strings are equal, a negative value if s1 < s2, and a positive value if s1 > s2
 */
int strncmp(const char *s1, const char *s2, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (s1[i] != s2[i]) {
      return (unsigned char)s1[i] - (unsigned char)s2[i];
    }
    if (s1[i] == '\0') {
      return 0;
    }
  }
  return 0;
}

/**
 * @brief memset function fills the first n bytes of the memory area pointed to by s with the constant byte c 
 * @param s pointer to the memory area
 * @param c constant byte to fill the memory area
 * @param n number of bytes to fill
 * @return pointer to the memory area s
 */
void *memset(void *s, int c, size_t n) {
  char *p = (char *)s;
  for (size_t i = 0; i < n; i++) {
    p[i] = (char)c;
  }
  return s;
}

/**
 * @brief memmove is similar to memcpy, but the memory areas may overlap.
 * @param dst pointer to the destination memory area
 * @param src pointer to the source memory area
 * @param n number of bytes to copy
 * @return pointer to the destination memory area dst
 */
void *memmove(void *dst, const void *src, size_t n) {
  // How to solve the overlap problem? By order !
  const char *p = (char *)src;
  char *q = (char *)dst;

  // if not overlap 
  if (p + n <= q || q + n <= p) {
    return memcpy(dst, src, n);

  } else if (p == q) {
    // do nothing
  } else if (p < q) {
    // copy from the end to the beginning
    for (size_t i = n; i > 0; i--) {
      q[i - 1] = p[i - 1];
    }
  } else {
    // p > q, copy from the beginning to the end 
    for (size_t i = 0; i < n; i++) {
      q[i] = p[i];
    }
  }
  return dst;

}

/**
 * @brief memcpy function copies n bytes from memory area src to memory area dst. The memory areas must not overlap.
 * @param out pointer to the destination memory area
 * @param in pointer to the source memory area
 * @param n number of bytes to copy
 * @return pointer to the destination memory area dst
 */
void *memcpy(void *out, const void *in, size_t n) {
  const char *p = (const char *)in;
  char *q = (char *)out;

  for(size_t i = 0; i < n; i++) {
    q[i] = p[i];
  }
  return out;
}

/**
 * @brief memcmp function compares the first n bytes (each interpreted as unsigned char) of the memory areas s1 and s2
 * @param s1 pointer to the first memory area
 * @param s2 pointer to the second memory area
 * @param n number of bytes to compare 
 * @return an integer less than, equal to, or greater than zero 
 */
int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p = (unsigned char *)s1;
  const unsigned char *q = (unsigned char *)s2;

  for (size_t i = 0; i < n; i++) {
    if (p[i] != q[i]) {
      return p[i] - q[i];
    }
  }
  return 0; // if n is 0, or all bytes are equal, return 0
}

/**
 * @brief itoa function converts an integer value to a null-terminated string using the specified base and stores the result in the array given by str parameter.
 * @param value integer value to convert
 * @param str pointer to the buffer where the resulting C-string is stored
 * @param base numerical base used to represent the value as a string
 * @return pointer to the resulting C-string
 */
char *itoa(int value, char *str, int base) {
  // assume base is 10 ?
  char *p = str;
  while (value) {
    *p++ = (value % base) + '0';
    value /= base;
  }
  // reverse the string
  char *start = str;
  char *end = p - 1;
  while (start < end) {
    char temp = *start;
    *start++ = *end;
    *end-- = temp;
  }
  *p = '\0';
  return str;
}

#endif
