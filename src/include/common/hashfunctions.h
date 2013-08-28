#include "common.h"


#ifndef HASH_FUNC_H
#define HASH_FUNC_H


typedef uint  (*hashFunc)        (void* key, ulong keySize);

typedef int   (*hashCmpFunc) (void* key1, void* key2, ulong keySize);

typedef void* (*hashCpyFunc)    (void* dest, void* source, ulong keySize);

typedef void* (*hashAllocFunc)   (uint size);


uint hashFuncFake  (void* key, ulong keySize);
uint hashFuncForIds(void* key, ulong keySize);
uint hashFuncStr   (void* key, ulong keySize);
uint hashFuncTag   (void* key, ulong keySize);

int hashCmpFuncStr (char* Key1, char* Key2, ulong keySize);


#endif

