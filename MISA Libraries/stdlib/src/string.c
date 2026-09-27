#include "../include/string.h"
#include "../include/strbuf.h"

#define SWAR_SS 4
#define SWAR_ALIGN (SWAR_SS - 1)
#define SWAR_ONES ((u32t)0x01010101UL)
#define SWAR_HIGHS ((u32t)0x80808080UL)
#define SWAR_HASZERO(x) (((x) - SWAR_ONES) & ~(x) & SWAR_HIGHS)

extern void* malloc(u32t size);
extern void free(void* ptr);

extern void mem_copy(void *dst, void *src, int size);
extern void mem_set(void *dst, int value, int size);

void* memcpy(void *dest, const void *src, u32t num) {
    mem_copy(dest, (void*)src, (int)num);
    return dest;
}

void* memset(void *ptr, i16t value, u32t num) {
    mem_set(ptr, (int)value, (int)num);
    return ptr;
}

void* memmove(void *dest, const void *src, u32t num) {
    mem_copy(dest, (void*)src, (int)num);
    return dest;
}

void* memchr(const void *ptr, i16t value, u32t num) {
    const u8t *p = (const u8t *)ptr;
    u8t v = (u8t)value;
    u32t k;
    const u32t *w;

    /* walk one byte at a time until p is 4 byte aligned (or out of bytes)) */
    while (((u32t)p & SWAR_ALIGN) && num && *p != v) {
        p++;
        num--;
    }

    /* scan 4 bytes at once while a full word remains */
    if (num && *p != v) {
        k = SWAR_ONES * (u32t)v;
        w = (const u32t *)p;
        while (num >= SWAR_SS && !SWAR_HASZERO(*w ^ k)) {
            w += 1;
            num -= SWAR_SS;
        }
        p = (const u8t *)w;
    }

    /* finish the last <4 leftover bytes one at a time */
    while (num && *p != v) {
        p++;
        num--;
    }

    return num ? (void *)p : 0;
}

void* memrchr(const void *ptr, i16t value, u32t num) {
    const u8t *p = (const u8t *)ptr;
    u8t v = (u8t)value;
    u32t k;
    const u32t *w;

    p += num;

    /* process bytes until p reaches a 4-byte-aligned address */
    while (((u32t)p & SWAR_ALIGN) && num) {
        p--;
        num--;
        if (*p == v) return (void *)p;
    }

    /* scan whole words backward while any remain */
    if (num >= SWAR_SS) {
        k = SWAR_ONES * (u32t)v;
        w = (const u32t *)p;
        while (num >= SWAR_SS) {
            w--;
            num -= SWAR_SS;
            if (SWAR_HASZERO(*w ^ k)) {
                /* this word contains the rightmost match */
                const u8t *bp = (const u8t *)w;
                i16t j;
                for (j = SWAR_SS - 1; j >= 0; j--) {
                    if (bp[j] == v) return (void *)(bp + j);
                }
            }
        }
        p = (const u8t *)w;
    }

    /* check the remaining bytes at the beginning */
    while (num--) {
        p--;
        if (*p == v) return (void *)p;
    }

    return 0;
}

i16t memcmp(const void *ptr1, const void *ptr2, u32t num) {
    const u8t *p1 = (const u8t *)ptr1;
    const u8t *p2 = (const u8t *)ptr2;
    const u32t *w1, *w2;

    /* use word comparisons only when both pointers have the same align */
    if (((u32t)p1 & SWAR_ALIGN) == ((u32t)p2 & SWAR_ALIGN)) {

        /* process bytes until both pointers are 4-byte aligned */
        while (((u32t)p1 & SWAR_ALIGN) && num) {
            if (*p1 != *p2)
                return (i16t)(*p1 - *p2);

            p1++;
            p2++;
            num--;
        }

        /* compare 4 bytes at once while full words remain */
        w1 = (const u32t *)p1;
        w2 = (const u32t *)p2;

        while (num >= SWAR_SS && *w1 == *w2) {
            w1++;
            w2++;
            num -= SWAR_SS;
        }

        p1 = (const u8t *)w1;
        p2 = (const u8t *)w2;
    }

    /* compare the remaining bytes */
    while (num && *p1 == *p2) {
        p1++;
        p2++;
        num--;
    }

    return num ? (i16t)(*p1 - *p2) : 0;
}

u32t strlen(cstring str) {
    cstring a = str;
    const u32t *w;

    /* process bytes until str is 4-byte aligned */
    while ((u32t)str & SWAR_ALIGN) {
        if (!*str) 
            return (u32t)(str - a);
        str++;
    }

    /* scan 4 bytes at once until '\0' is found */
    w = (const u32t *)str;
    while (!SWAR_HASZERO(*w)) 
        w++;
    str = (cstring)w;

    /* find the '\0' within the matching word */
    while (*str) 
        str++;

    return (u32t)(str - a);
}

string stpcpy(string restrict dest, cstring restrict src) {
    string p = dest;

    /* use word copies only when src and dest have the same alignment */
    if (((u32t)src & SWAR_ALIGN) == ((u32t)dest & SWAR_ALIGN)) {

        /* process bytes until src and dest are 4 byte aligned */
        while ((u32t)src & SWAR_ALIGN) {
            if (!(*p = *src))
                return p;

            p++;
            src++;
        }

        {
            const u32t *ws = (const u32t *)src;
            u32t *wd = (u32t *)p;

            /* scan 4 bytes at once until '\0' is found */
            while (!SWAR_HASZERO(*ws)) {
                *wd++ = *ws++;
            }

            src = (cstring)ws;
            p = (string)wd;
        }
    }

    /* copy the remaining bytes until '\0' */
    while ((*p = *src)) {
        p++;
        src++;
    }

    return p;
}

string strcpy(string dest, cstring src) {
    stpcpy(dest, src);
    return dest;
}

string stpncpy(string restrict dest, cstring restrict src, u32t num) {
    string p = dest;

    /* use word copies only when src and dest have the same alignment */
    if (((u32t)src & SWAR_ALIGN) == ((u32t)dest & SWAR_ALIGN)) {

        /* process bytes until src and dest are 4-byte aligned */
        while (((u32t)src & SWAR_ALIGN) && num && *src) {
            *p++ = *src++;
            num--;
        }

        /* scan 4 bytes at once while a full word remains */
        if (num >= SWAR_SS) {
            const u32t *ws = (const u32t *)src;
            u32t *wd = (u32t *)p;

            /* stop when a word contains the terminating '\0' */
            while (num >= SWAR_SS && !SWAR_HASZERO(*ws)) {
                *wd++ = *ws++;
                num -= SWAR_SS;
            }

            src = (cstring)ws;
            p = (string)wd;
        }
    }

    /* copy the remaining bytes until '\0' or num is exhausted */
    while (num && *src) {
        *p++ = *src++;
        num--;
    }

    /* pad the remaining destination bytes with '\0' */
    while (num--)
        *p++ = '\0';

    return p;
}

string strncpy(string dest, cstring src, u32t num) {
    stpncpy(dest, src, num);
    return dest;
}

string strchrnul(string str, i16t character) {
    u8t c = (u8t)character;

    /* find character or terminating '\0' */
    while ((u8t)*str != c && *str != '\0')
        str++;

    return str;
}

string strchr(string str, i16t character) {
    string p = strchrnul(str, character);

    /* return NULL if character not found from strchrnul */
    if ((u8t)*p != (u8t)character)
        return NULL;

    return p;
}

string strrchr(string str, i16t character) {
    /* include the terminator in the search */
    return memrchr(str, character, strlen(str) + 1);
}

string strcat(string restrict dest, string restrict src) {
    strcpy(dest + strlen(dest), src);
    return dest;
}

string strncat(string restrict dest, string restrict src, u32t num) {
    /* move p to the end of dest */
    string p = dest + strlen(dest);

    /* while num > 0 and src != NULL */
    while(num && *src) { 
        /* copy character and advance both pointers */
        *p++ = *src++;
        num--;
    }
    
    /* terminate the resulting string */
    *p = '\0';

    return dest;
}

i16t strncmp(string str1, string str2, u32t num) {
    const u8t *l = (const u8t *)str1;
    const u8t *r = (const u8t *)str2;

    /* no characters to compare */
    if (num == 0)
        return 0;

    /* compare until a difference, null terminator, or limit */
    while (--num && *l && *r) {
        if (*l != *r)
            return *l - *r;

        l++;
        r++;
    }

    /* compare final character */
    return *l - *r;
}

u32t strspn(cstring str, cstring accept) {
    u32t i;
    u8t byteset[32] = {0};

    /* no accepted characters */
    if (!accept[0])
        return 0;

    /* scan directly when only one character is accepted */
    if (!accept[1]) {
        cstring s = str;

        for (; *s == *accept; s++);

        return (u32t)(s - str);
    }

    /* store each accept char as a bit in 256-bit lookup table */
    for (i = 0; accept[i] != '\0'; i++) {
        u8t c = (u8t)accept[i];
        byteset[c / 8] |= (u8t)1 << (c % 8);
    }

    /* scan str while char is found in lookup table */
    for (i = 0; str[i] != '\0'; i++) {
        u8t c = (u8t)str[i];

        if (!(byteset[c / 8] & ((u8t)1 << (c % 8))))
            return i;
    }

    /* all chars were accepted */
    return i;
}

u32t strcspn(cstring str, cstring reject) {
    u32t i;
    u8t byteset[32] = {0};

    /* no rejected characters, or only one */
    if (!reject[0] || !reject[1]) {
        string p = strchrnul((string)str, *reject);
        return (u32t)(p - str);
    }

    /* store each reject char as a bit in 256-bit lookup table */
    /* each byte holds flags for 8 possible char values */
    for (i = 0; reject[i] != '\0'; i++) {
        u8t c = (u8t)reject[i];
        byteset[c / 8] |= (u8t)1 << (c % 8);
    }

    /* scan str until char is found in lookup table */
    for (i = 0; str[i] != '\0'; i++) {
        u8t c = (u8t)str[i];
        if (byteset[c / 8] & ((u8t)1 << (c % 8)))
            return i;
    }

    /* no reject char */
    return i;
}

string strpbrk(cstring str, cstring accept) {
    str += strcspn(str, accept);

    if (*str == '\0')
        return NULL;

    return (string)str;
}

string strstr(string str, string substr) {
    u32t i, j;

    /* empty substring always matches at the start */
    if (substr[0] == '\0')
        return str;

    /* single char substr can use strchr() */
    if (substr[1] == '\0')
        return strchr(str, substr[0]);

    /* search for the substr */
    for (i = 0; str[i] != '\0'; i++) {
        /* skip pos that don't match first char */
        if (str[i] != substr[0])
            continue;

        /* check rest of substr */
        for (j = 1; substr[j] != '\0'; j++) {
            if (str[i + j] != substr[j])
                break;
        }

        /* entire substr matched */
        if (substr[j] == '\0')
            return &str[i];
    }

    /* substr not found, return NULL ptr */
    return NULL;
}

string strtok(string restrict s, string restrict sep) {
	static string p; // where tokenizer should continue btwn calls
	if (!s && !(s = p)) 
        return NULL; /* if s null, return null, if s = p and still null, return null */
	s += strspn(s, sep); /* skip over leading delimiters */
	if (!*s) 
        return p = 0; /* if character at s is '\0', set p to 0 AND return 0 */
	p = s + strcspn(s, sep); /* p points at next delimiter */
	if (*p) 
        *p++ = 0; /* equivalent to if (*p != '\0') { *p = '\0'; p++; } */
	else 
        p = 0;
	return s;
}

i16t strcmp(cstring str1, cstring str2) {
    const u32t *w1, *w2;

    if (((u32t)str1 & SWAR_ALIGN) == ((u32t)str2 & SWAR_ALIGN)) {
        while ((u32t)str1 & SWAR_ALIGN) {
            if (*str1 != *str2 || !*str1)
                return (i16t)((u8t)*str1 - (u8t)*str2);
            str1++;
            str2++;
        }

        w1 = (const u32t *)str1;
        w2 = (const u32t *)str2;
        while (*w1 == *w2 && !SWAR_HASZERO(*w1)) {
            w1++;
            w2++;
        }
        str1 = (cstring)w1;
        str2 = (cstring)w2;
    }

    while (*str1 && *str1 == *str2) {
        str1++;
        str2++;
    }

    return (i16t)((u8t)*str1 - (u8t)*str2);
}