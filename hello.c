/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: starter code for implementing printf()
 */

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

/* int printf(const char* format, ...) {
    char buf[512];
    va_list args;
    va_start(args, format);
    format_to_str(buf, format, args);
    va_end(args);
    terminal_write(buf, strlen(buf));

    return 0;
} */

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    unsigned int len = format_to_str_len(format, args_copy);
    va_end(args_copy);

    char *buf = malloc(len+1); // Remeber the '\0'
    format_to_str(buf, format, args);
    va_end(args);
    terminal_write(buf, strlen(buf));

    free(buf);

    return 0;
}


/* Uncomment the code block below when implementing dynamic memory allocation.
 */

extern char __heap_start, __heap_end;
static char* brk = &__heap_start;
char* _sbrk(int size) {
    if (brk + size > (char*)&__heap_end) {
        terminal_write("_sbrk: heap grows too large\r\n", 29);
        return NULL;
    }

    char* old_brk = brk;
    brk += size;
    return old_brk;
}


int main() {
    char* msg = "Hello, World!\n\r";
    terminal_write(msg, 15);

    /* Uncomment this line of code when implementing formatted output. */
    printf("%s-%d is awesome!\n\r", "egos", 2000);
    printf("%c is character $\n", '$');
    printf("%c is character 0\n", (char)48);
    printf("%x is integer 1234 in hexadecimal\n", 1234);
    printf("%u is the maximum of unsigned int\n", (unsigned int)0xFFFFFFFF);
    printf("%p is the hexadecimal address of the hello-world string\n", msg);
    printf("%llu is the maximum of unsigned long long\n", 0xFFFFFFFFFFFFFFFFULL);

    return 0;
}
