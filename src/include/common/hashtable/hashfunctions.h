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

uint getHashFast(register const unsigned char *key, register int keylen);
uint getHashFastInt(uint num);
uint getHashId(void* key, ulong keySize);

double** AvalancheMatrix(int trials, int repetitions, int size, uint (*mix)(uint hash));

#endif

