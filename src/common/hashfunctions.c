#include "hashfunctions.h"


uint hashFuncFake  (void* key, ulong keySize) { return 1; }
uint hashFuncForIds(void* key, ulong keySize) { return 1; }
uint hashFuncStr   (void* key, ulong keySize) { return 1; }
uint hashFuncTag   (void* key, ulong keySize) { return -1; }

int hashCmpFuncStr (char* key1, char* key2, ulong keySize) 
{ 
	return strncmp(key1, key2, keySize - 1); 
}


