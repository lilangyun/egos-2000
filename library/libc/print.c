/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: formatted printing
 * format_to_str() converts a format into a C string:
 * e.g., converts ("%s-%d", "egos", 2000) to "egos-2000".
 * term_write() prints the converted C string to the screen.
 */

#include "egos.h"
#include "servers.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

void terminal_write(const char *str, int len) {
    for (int i = 0; i < len; i++) {
        *(char*)(0x10000000) = str[i];
    }
}

/* Uncomment the code block below when implementing formatted output.
 */

#include <stdlib.h>  // for itoa() and utoa()
#include <string.h>  // for strlen() and strcat()
#include <stdarg.h>  // for va_start(), va_end(), va_arg() and va_copy()

char* ulltoa(unsigned long long value, char* str, int base){
    unsigned long long x = value;
    if(x==0){
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    char res[32];
    int len = 0, idx = 0;
    while(x>0){
        unsigned int digit = (unsigned int)(x % base);
        res[len++] = (digit < 10)? '0'+digit: 'a'+digit-10;
        x /= base;
    }
    while(idx < len){
        str[idx++] = res[len-idx-1];
    }
    str[idx] = 0;
    return str;
}

unsigned int format_to_str_len(const char* fmt, va_list args) {
    unsigned int len = 0;
    for(; *fmt != '\0'; fmt++) {
        if (*fmt != '%') {
            len++;
        } else {
            fmt++;
            if (*fmt == 's') {
                len += strlen(va_arg(args, char*));
            } else if (*fmt == 'd') {
                char tmp[32];
                len += strlen(itoa(va_arg(args, int), tmp, 10));
            } else if (*fmt == 'u') {
                char tmp[32];
                len += strlen(utoa(va_arg(args, unsigned int), tmp, 10));
            } else if (*fmt == 'c') {
                len++;
            } else if (*fmt == 'x') {
                char tmp[32];
                len += strlen(utoa(va_arg(args, unsigned int), tmp, 16));
            } else if (*fmt == 'p') {
                char tmp[32];
                len += 2 + strlen(utoa(va_arg(args, unsigned int), tmp, 16));
            } else if (*fmt == 'l' && *(fmt+1) == 'l' && *(fmt+2) == 'u') {
                char tmp[32];
                len += strlen(ulltoa(va_arg(args, unsigned long long), tmp, 10));
                fmt+=2;
            }
        }
    }
    return len;
}

void format_to_str(char* out, const char* fmt, va_list args) {
    for(out[0] = 0; *fmt != '\0'; fmt++) {
        if (*fmt != '%') {
            strncat(out, fmt, 1);
        } else {
            fmt++;
            if (*fmt == 's') {
                strcat(out, va_arg(args, char*));
            } else if (*fmt == 'd') {
                itoa(va_arg(args, int), out + strlen(out), 10);
            } else if (*fmt == 'u') {
                utoa(va_arg(args, unsigned int), out + strlen(out), 10);
            } else if (*fmt == 'c') {
                size_t len = strlen(out);
                out[len] = va_arg(args, int);
                out[len+1] = '\0';
            } else if (*fmt == 'x') {
                utoa(va_arg(args, unsigned int), out + strlen(out), 16);
            } else if (*fmt == 'p') {
                strcat(out, "0x");
                utoa(va_arg(args, unsigned int), out + strlen(out), 16);
            } else if (*fmt == 'l' && *(fmt+1) == 'l' && *(fmt+2) == 'u') {
                ulltoa(va_arg(args, unsigned long long), out + strlen(out), 10);
                fmt+=2;
            }
        }
    }
}

#define LOG(prefix, suffix)                                                    \
    char buf[512];                                                             \
    strcpy(buf, prefix);                                                       \
    va_list args;                                                              \
    va_start(args, format);                                                    \
    format_to_str(buf + strlen(prefix), format, args);                         \
    va_end(args);                                                              \
    strcat(buf, suffix);                                                       \
    term_write(buf, strlen(buf));

int my_printf(const char* format, ...) { LOG("", ""); }

int INFO(const char* format, ...) { LOG("[INFO] ", "\n\r") }

int FATAL(const char* format, ...) {
    LOG("\x1B[1;31m[FATAL] ", "\x1B[1;0m\n\r") /* \x1B[1;31m means red. */
    while (1);
}

int SUCCESS(const char* format, ...) {
    LOG("\x1B[1;32m[SUCCESS] ", "\x1B[1;0m\n\r") /* \x1B[1;32m means green. */
}

int CRITICAL(const char* format, ...) {
    LOG("\x1B[1;33m[CRITICAL] ", "\x1B[1;0m\n\r") /* \x1B[1;33m means yellow. */
}
