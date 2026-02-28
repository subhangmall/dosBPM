#include <stdint.h>
#include <stdbool.h>

static int cursor = 0;
volatile char* videoMemory = (volatile char*) 0xB8000;

void kclear() {
    for (int i = 0; i < 80*25; i++) {
        videoMemory[i*2] = ' ';
        videoMemory[i*2 + 1] = 0x0F;
    }
    cursor = 0;
}

void kputc(char c) {
    if (c == '\n') {
        cursor = (cursor / 80 + 1) * 80;
        return;
    }

    videoMemory[cursor*2] = c;
    videoMemory[cursor*2+1] = 0x0F;
    cursor++;
}

void kprint(const char* s) {
    while (*s)
        kputc(*s++);
}

void kprint_hex(uint32_t n) {
    char hexChars[] = "0123456789ABCDEF";
    char result[11];

    result[0] = '0';
    result[1] = 'x';
    result[10] = '\0';

    for (int i = 2; i <= 9; i++) {
        result[11-i] = hexChars[n & 0x0F];
        n >>= 4;
    }

    kprint(result);
}

void kprint_hex64(uint64_t n) {
    char hexChars[] = "0123456789ABCDEF";
    char result[19];

    result[0] = '0';
    result[1] = 'x';
    result[18] = '\0';

    for (int i = 2; i <= 17; i++) {
        result[19-i] = hexChars[n & 0x0F];
        n >>= 4;
    }

    kprint(result);
}