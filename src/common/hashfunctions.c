#include "hashfunctions.h"


/* The main particularity of any hash function is randomness
 * A hash function generates some 16 bits number, for example.
 * A probability that k-th bit is set to 1/2. A probability of 1
 * for every other bits should bw the same otherwise some hash values
 * will be generated more frequentry and it will cause perfomance issues.
 */

uint hashFuncFake  (void* key, ulong keySize) { return 1; }
uint hashFuncForIds(void* key, ulong keySize) { return 1; }
uint hashFuncStr   (void* key, ulong keySize) { return 1; }
uint hashFuncTag   (void* key, ulong keySize) { return -1; }

int hashCmpFuncStr (char* key1, char* key2, ulong keySize) 
{ 
	return strncmp(key1, key2, keySize - 1); 
}

/* 1. Why do we use 'register' keyword here?
 *    'Register' keyword requests that the variable 
 *    be accessed as quickly as possible. This request is not guaranteed. 
 *    Normally, the variable’s value is kept within a CPU register for maximum speed.
 *    For example: 
 *    We have some C code:
 *    {
 *       int a = 233;
 *       int b = 345;
 *       int c = a - b;
 *    }
 *    generated assembler code:
 *    ;
 *       mov    DWORD PTR [ebp-12], 999
 *       mov    DWORD PTR [ebp-8], 888 
 *       mov    eax, DWORD PTR [ebp-8]
 *       mov    edx, DWORD PTR [ebp-12]
 *       mov    ecx, edx
 *       sub    ecx, eax
 *       mov    eax, ecx
 *       mov    DWORD PTR [ebp-4], eax
 *    ;
 *    As we can notice compiler use memory to keep variables there.
 *    Then we load values from memory to registers, computes division
 *    and loads the result back to memory so that registers are free.
 *    Now we have some C code:
 *    {
 *       register int a = 233;
 *       register int b = 345;
 *       int c = a - b;
 *    }
 *    ;
 *       push    esi
 *       push    ebx
 *       mov     esi, 999 
 *       mov     ebx, 888
 *       mov     eax, esi
 *       sub     eax, ebx  
 *       mov     DWORD PTR [ebp-12], eax
 *       pop     ebx
 *       pop     esi
 *    ;
 *    In this case we load numbers directly to registers, perform
 *    substraction and the result load into memory
 *    
 */   
uint genHash(register const unsigned char *k, register int keylen)
{
   register uint a, b, c, len;

   len = keylen;
   a = b = c = 0x9e3779b9 + len + 3923095;

   if (((int)k & (sizeof(uint) - 1)) == 0)
   {

   }
}

