#ifndef TYPES_H
#define TYPES_H

typedef unsigned long u32t;
typedef long i32t;
typedef unsigned int u16t;
typedef int i16t;
typedef unsigned char u8t;
typedef char i8t;
typedef double f32t; // no smaller size than f32t in MISA
typedef char* string;
typedef const char* cstring;

#define NULL ((void*)0)

#endif /* TYPES_H */