#include "hashfunctions.h"


/* This hashfunction is copied from here: 
 * http://burtleburtle.net/bob/hash/doobs.html
 * 
 * The main particularity of any hash function is randomness
 * A hash function generates some 16 bits number, for example.
 * A probability that k-th bit is set to 1/2. A probability of 1
 * for every other bits should bw the same otherwise some hash values
 * will be generated more frequentry and it will cause perfomance issues.
 *
 * Uniform Distribution
 * All good hash functions share the property of collision avoidance, 
 * which means that the desired behavior is 
 * for unique inputs to provide unique outputs.
 * 
 * Collisions are guaranteed not to happen 
 * if the mixing function is reversible, that is, 
 * it's possible to determine the input of the mixing function by examining the output. 
 * If the function is not reversible, it implies 
 * that there are at least two inputs that result in the same output. 
 * By definition, that is a collision.
 * A mixing function is guaranteed to be reversible 
 * if it consists of steps that are each reversible.
 * 
 * Reversible operations:
 *  1. ^= repeat it and you will get the original value. 
 *  2. +=, -= operations can be reversed with -= and +=.
 *  3. ^= operation combined with << or >> shift is an interesting one. 
 *     To reverse hash ^= hash >> 9 for example, 
 *     you would do hash ^= (hash >> 9) ^ (hash >> 18) ^ (hash >> 27).
 *     We need to prove it:
 *     Let's take some 32 bit integer: 
 *     0100 0000 0000 1110 0000 0000 0000 0000 = hash
 *     0000 0000 0010 0000 0000 0111 0000 0000 = hash >> 9
 *     ---------------------------------------
 *     0100 0000 0010 1110 0000 0111 0000 0000 = hash ^ (hash >> 9) = hash1     
 *
 *     After that we want to apply reversible algorithm:
 *     0100 0000 0010 1110 0000 0111 0000 0000 = hash1   
 *     0000 0000 0010 0000 0001 0111 0000 0011 = hash1 >> 9
 *     0000 0000 0000 0000 0001 0000 0000 1011 = hash1 >> 18
 *     0000 0000 0000 0000 0000 0000 0000 1000 = hash1 >> 27
 *     ---------------------------------------
 *     0100 0000 0010 1110 0000 0000 0000 0000 = hash                                      
 * 
 *  4. += and -= operations combined with << or >> shifts 
 *     are equivalent to *= and can be restated that way.
 *  
 *  5. The *= operation is the most difficult to reverse. 
 *     Look up modular inverse and Euclidean algorithm 
 *     in your favorite search engine for details. 
 *     Because of this difficulty, it's tempting to use this type of mixing step 
 *     for one-way hash functions. 
 *     Whether or not this is a good choice largely depends on the performance of the CPU. 
 *     Some CPU's perform multiplication quickly 
 *     and some take much longer than addition and subtraction.
 * 
 * Avalanche:
 *   A function is said to satisfy the strict avalanche criterion if, 
 *   whenever a single input bit is complemented (toggled between 0 and 1), 
 *   each of the output bits should change with a probability 
 *   of one half for an arbitrary selection of the remaining input bits. 
 *  
 * Calculating Avalanche:
 *   hash += hash << 1

	Input         Output         Bit changes
	0   0 0 0 0   0   0 0 0 0    - - - -
	1   0 0 0 1   3   0 0 1 1    - - 1 -
	2   0 0 1 0   6   0 1 1 0    - 1 - -
	3   0 0 1 1   9   1 0 0 1    1 - 2 -
	4   0 1 0 0   12  1 1 0 0    2 - - -
	5   0 1 0 1   15  1 1 1 1    3 - 3 -
	6   0 1 1 0   2   0 0 1 0    - 2 - -
	7   0 1 1 1   5   0 1 0 1    - - 4 -
	8   1 0 0 0   8   1 0 0 0    - - - -
	9   1 0 0 1   11  1 0 1 1    - - 5 -
	10  1 0 1 0   14  1 1 1 0    - 3 - -
	11  1 0 1 1   1   0 0 0 1    4 - 6 -
	12  1 1 0 0   4   0 1 0 0    5 - - -
	13  1 1 0 1   7   0 1 1 1    6 - 7 -
	14  1 1 1 0   10  1 0 1 0    - 4 - -
	15  1 1 1 1   13  1 1 0 1    - - 8 -

	The probability that bit 0 changes is 6/15 = 0.4
    The probability that bit 1 changes is 4/15 = 0.25
    The probability that bit 2 changes is 4/15 = 0.53
	The probability that bit 3 changes is 0/15 = 0
 *  
 * Therefore this mixing step by itself does not come close 
 * to satisfying the strict avalanche criterion. 
 * We want to see about 50% for each output bit position for each input bit.  
 * 
 * 
 */

uint hashFuncFake  (void* key, ulong keySize) { return 1; }
uint hashFuncForIds(void* key, ulong keySize) { return 1; }
uint hashFuncStr   (void* key, ulong keySize) { return 1; }

int hashCmpFuncStr (char* key1, char* key2, ulong keySize) 
{ 
	return strncmp(key1, key2, keySize - 1); 
}

/* Rotate a 32-bit integer value left by k bits. 
 * Suppose we have some 32 bit integer:
 * 0011 0101 0000 0000  0000 0000 0000 1011
 * We rotate it on 5 position left:
 * x << 5 = 
 * 1010 0000 0000 0000  0000 0001 0110 0000
 * x >> 32 - 5 =
 * 0000 0000 0000 0000  0000 0000 0000 0110
 * (x << 5) | (x >> 28)
 * 1010 0000 0000 0000  0000 0001 0110 0110
 */
#define rotate(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

/* Test1. Test mix when numbers are almost the same and
 *        differs only in one or two bits
 * a =         0000 0000 0000 0000  1000 0000 0000 0000
 * b =         0000 0000 0000 0000  0100 0000 0000 0000
 * c =         0000 0000 0000 0000  0010 0000 0000 0000
 * a -= c  =   0000 0000 0000 0000  0100 0000 0000 0000
 * rot(c,4)=   0000 0000 0000 0100  0000 0000 0000 0000
 * a^=rot(c,4)=0000 0000 0000 0100  0100 0000 0000 0000 = a
 * c =         0000 0000 0000 0000  0110 0000 0000 0000 = 
 * b -= a  =   
 */
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rotate(c, 4);	c += b; \
  b -= a;  b ^= rotate(a, 6);	a += c; \
  c -= b;  c ^= rotate(b, 8);	b += a; \
  a -= c;  a ^= rotate(c,16);	c += b; \
  b -= a;  b ^= rotate(a,19);	a += c; \
  c -= b;  c ^= rotate(b, 4);	b += a; \
}

#define mixJerkins(state) \
{ \
  state += (state << 12); \
  state ^= (state >> 22); \
  state += (state << 4);  \
  state ^= (state >> 9);  \
  state += (state << 10); \
  state ^= (state >> 2);  \
  state += (state << 7);  \
  state ^= (state >> 12); \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rotate(b,14); \
  a ^= c; a -= rotate(c,11); \
  b ^= a; b -= rotate(a,25); \
  c ^= b; c -= rotate(b,16); \
  a ^= c; a -= rotate(c, 4); \
  b ^= a; b -= rotate(a,14); \
  c ^= b; c -= rotate(b,24); \
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
 * 2. The key parameter is an unaligned variable-length char array.  
 *    
 */   
uint getHashFast(register const unsigned char *key, register int keylen)
{
   register uint a, b, c, len;

   len = keylen;
   a = b = c = 0x9e3779b9 /* the golden ratio; an arbitrary value */
	         + len 
			 + 3923095;   /* the previous hash, or an arbitrary value 
						   * in our case we choose an arbitrary value. */
   
   /* 1. Converting char* to int
    *    char* is a pointer and it is actually a hex number: 0x002323ed2
	*    when we convert char* to int we convert this hex number to int number
	*
	* 2. Logically ANDing it with sizeof(uint)-1 (4-1=3)
	*    4 = 00...00100, 3 = 00...00011
	*    We receive 0 in the following operation only if the last 2 digits are 0
	*    For example: 110110011100 & 00000000011 = 0 when Num = 4*n, 
	*    where n is an arbitrary number.
	* 
	* 3. Why key pointer word alignment is so important?
	*    Word is a 4-byte variable. 
	*    Let's look at the following example:
	*    {
	*       unsigned char* ptr    = "<Some char array>";
	*       double*        dpoint = (double*)ptr;
	*       double         dval   = *dpoint;
	*    }
	*    Suppose that ptr is not aligned on 4-byte boundary.
	*    On ARM-based systems you cannot address a 32-bit word 
	*    that is not aligned to a 4-byte boundary. 
	*    Doing so will result in an access violation exception. 
	*    On x86 you can access such non-aligned data, 
	*    though the performance suffers a little since two words 
	*    have to be fetched from memory instead of just one.
    */
   if (((int)key & (sizeof(uint) - 1)) == 0)
   {
	   /* Here we also need some explanation how to convert 
	    * a char pointer to an unsigned int pointer. 
		* If we have string: "axcv"
		* Let's see what the appropriate binary representation is:
		* 
		* 0111 0110 | 0110 0011 | 0111 1000 | 0110 0001
        *   118     |   99      |    120    |   97
        *    v          c             x         a 
		* 
		* we receive a binary representation of this number, then reverse it and so
		* we get a binary int32 number and convert it to decimal system.
		* So we divide char* key array on 4-byte parts, each consists of 4 chars
		* and convert every part to an integer.
		*/
	   register uint* keyCodes = (uint*)key;

	   /* handle most of the key 
	    * Four symbols will be converted to one int32
		* We need to take 3 integers, thus we need 
		* to have at least 12 symbols
	    */
	   while (len >= 12)
	   {
		  a += keyCodes[0];
		  b += keyCodes[1];
		  c += keyCodes[2];
		  mix(a, b, c);
		  keyCodes += 3;
		  len -= 12;
	   }

	   key = (unsigned char*)keyCodes;

	   switch (len)
	   {
			case 11: c += ((uint)key[10] << 24);
			case 10: c += ((uint)key[9] << 16);
			case 9:  c += ((uint)key[8] << 8);
			case 8:  
				b += keyCodes[1];
		        a += keyCodes[0];
				break;
			case 7:  b += ((uint)key[6] << 16);
			case 6:  b += ((uint)key[5] << 8);	
			case 5:  b += key[4];
			case 4:
				a += keyCodes[0];
				break;
			case 3:  a += ((uint)key[2] << 16);
			case 2:  a += ((uint)key[1] << 8);
			case 1:  a += key[0];
	   }

       final(a, b, c);
	   return c;
   }

   /* If the pointer to the key string is not alighned
    * we need to parse each symbol and convert them to integers.
    * key = "axcvweerhitr"
    * One char takes 8 bit memory. 
	*  
	*  key[0] = 'a' = 97  = 0110 0001
	*  key[1] = 'x' = 120 = 0111 1000
	*  key[2] = 'c' = 99  = 0110 0011
	*  key[3] = 'v' = 118 = 0111 0110
	*
	*  0000 0000 0000 0000 0000 0000 0110 0001 + 
	*  0000 0000 0000 0000 0111 1000 0000 0000 + 
	*  0000 0000 0110 0011 0000 0000 0000 0000 +
	*  0111 0110 0000 0000 0000 0000 0000 0000 
	*  --------------------------------------- =
	*  0111 0110 0110 0011 0111 1000 0110 0001
	*  
    */
   while (len >= 12)
   {
	   a += (key[0] + ((uint)key[1] << 8) + ((uint)key[2] << 16) + ((uint)key[3] << 24));
	   b += (key[4] + ((uint)key[5] << 8) + ((uint)key[6] << 16) + ((uint)key[7] << 24));
	   c += (key[8] + ((uint)key[9] << 8) + ((uint)key[10] << 16) + ((uint)key[11] << 24));

	   mix(a, b, c);
	   key += 12;
	   len -= 12;
   }

   /* Suppose that 11 symbols are left:
    * key     = "axcvweerhit"
    * key[10] = t | 116 = 0111 0100
	* key[9]  = i | 105 = 0110 1001
	* key[8]  = h | 104 = 0110 1000
	*
	* 0111 0100 0000 0000 0000 0000 0000 0000 
	* 0000 0000 0110 1001 0000 0000 0000 0000 
	* 0000 0000 0000 0000 0110 1000 0000 0000 
	* --------------------------------------- =
	* 0111 0100 0110 1001 0110 1000 0000 0000
	* add this number to a number c:
	*
	* Do the same for key chars: 
	* [7,6,5,4] and add them to number b.
	* 
    * Do the same for key chars: 
	* [3,2,1,0] and add them to number a.
    */
   switch (len)	
   {
	   case 11: c += ((uint)key[10] << 24);
	   case 10: c += ((uint)key[9] << 16);
	   case 9:  c += ((uint)key[8] << 8);		
	   case 8:  b += ((uint)key[7] << 24);
	   case 7:  b += ((uint)key[6] << 16);
	   case 6:  b += ((uint)key[5] << 8);
	   case 5:  b += key[4];
	   case 4:  a += ((uint)key[3] << 24);
	   case 3:  a += ((uint)key[2] << 16);
	   case 2:  a += ((uint)key[1] << 8);
	   case 1:  a += key[0];
   }

   final(a, b, c);
   return c;
}

uint getHashFastInt(uint num)
{
	register uint a, b, c;

	a = b = c = 0x9e3779b9 + (uint) sizeof(uint) + 3923095;
	a += num;

	final(a, b, c);
	return c;
}

uint hashFuncTag(void* key, ulong keySize)
{
    return getHashFast((const unsigned char*)key, (int)keySize);
}

uint getHashId(void* key, ulong keySize)
{
	return getHashFastInt((uint) *((uint*)key));
}

/* Mixing function:
 *  The multiplication by 33 is called the mixing step 
 *  and the function f : uint → uint defined by f(x) = x * 33 
 *  where * is the multiply-without-overflow operator 
 *  is called the mixing function. 
 *  The constant 33 is chosen arbitrarily.
 * 
 * Combining function:
 *  The addition of the Unicode code point value of the character 
 *  is called the combining step and the function 
 *  g : uint × uint → uint defined by g(x, y) = x + y 
 *  where + is the add-without-overflow operator is called the combining function.
 */
uint getHashSimple(char* key, uint len)
{
    uint hash, i;
    for (hash=0, i=0; i<len; ++i) 
      hash = hash * 33 + (uint)key[i];
    return hash;
}

uint Mix1(int hash)
{
    hash += hash << 11;
    hash ^= ~hash >> 5;
    hash -= hash << 13;
    return hash;
}

uint getHashCompl1(char* key, uint len)
{
    /* initialization */
    int n = len;
    uint hash = 0x79FEF6E8;
    int i = 0;
	
    /* process each block
	 * For each block, the combining function is applied 
	 * to the prior state and the bits from the current block, 
	 * and then the mixing function is called. 
	 * This step is repeated for each full-sized block in the message.*/
    while (i < n - 2)
    {
        /* combining step */
        hash += (uint)key[i++] + ((uint)key[i++]) << 16;
        /* mixing step */
        hash = Mix1(hash);
    }
	
    /* process partial block
	 * Then the final, partial block is processed, if necessary. 
	 * The combining step is modified to accomodate the incomplete block.
	 */
    if (i < n) 
    {
        /* combining step */
	    hash += (uint)key[i++];
        /* mixing step */
        hash = Mix1(hash);
    }
	
    /* post-processing step
	 * Finally, some final processing is done to
	 * further randomize the internal state. 
	 * In this case, the mixing step is applied two more times. */
    hash = Mix1(Mix1(hash));
    return hash;
}

/* The table length must be prime, 
 * and can't be much bigger than one byte 
 * because the value of variable hash is never much bigger than one byte. 
 */
uint getHashAdd(char* key, uint len, uint prime)
{
   uint hash, i;
   for (hash=len, i=0; i<len; ++i) 
     hash += key[i];
   return (hash % prime);
}

/* This takes 8n+3 instructions. 
 * This is the same as the additive hash, 
 * except it has a mixing step (a circular shift by 4) 
 * and the combining step is exclusive-or instead of addition. 
 * The table size is a prime, but the prime can be any size.  
 * On machines with a rotate (such as the Intel x86 line) this is 6n+2 instructions. 
 * I have seen the (hash % prime) replaced with   
 *   hash = (hash ^ (hash>>10) ^ (hash>>20)) & mask;
 * eliminating the % and allowing the table size to be a power of 2, 
 * making this 6n+6 instructions. 
 * % can be very slow, for example it is 230 times slower than addition on a Sparc 20.
 */
uint getHashRot(char* key, uint len, uint prime)
{
   uint hash, i;
   for (hash=len, i=0; i<len; ++i)
     hash = (hash<<4)^(hash>>28)^key[i];
   return (hash % prime);
}

/* This is similar to the rotating hash, 
 * but it actually mixes the internal state. 
 * It takes 9n+9 instructions and produces a full 4-byte result. 
 * Preliminary analysis suggests there are no funnels. 
 */
uint get_one_at_a_time(char* key, uint len)
{
   uint   hash, i, mask;
   mask = 123;
   for (hash=0, i=0; i<len; ++i)
   {
      hash += key[i];
      hash += (hash << 10);
      hash ^= (hash >> 6);
   }
   hash += (hash << 3);
   hash ^= (hash >> 11);
   hash += (hash << 15);
   return (hash & mask);
} 

uint bernstein(char* key, uint len, uint level)
{
  uint hash = level;
  uint i;
  for (i=0; i<len; ++i) 
	 hash = 33*hash + key[i];
  return hash;
}

/* We'll create a function for calculating the avalanche matrix. 
 * Each element i,j of this 32x32 matrix of floating-point values 
 * will tell us the probability that bit i of the input 
 * will affect bit j of the output. 
 * We want to see a value of 0.5 for every combination.
 */
double** AvalancheMatrix(int trials, int repetitions, int size, uint (*mixFunc)(uint hash))
{
    uint save, state, inb, outb;
	int i, r, j;
	int nBytes = 4;
    double dTrials = trials;
    double **matrix, **result;
    int* bytes;
    int** t;

    matrix = (double**)malloc(size*sizeof(double*));
    result = (double**)malloc(size*sizeof(double*));
    bytes = (int*)malloc(nBytes*sizeof(int));
	t = (int**)malloc(size*sizeof(int*));

    for (i = 0; i < size; i++)  
	{
       matrix[i] = (double*)malloc(size*sizeof(double)); 
       result[i] = (double*)malloc(size*sizeof(double)); 
	   t[i] = (int*)malloc(size*sizeof(int)); 
	}

    if (trials <= 0 || repetitions <= 0)
       return matrix;

    while (trials-- > 0)
    {
		for (i = 0; i < nBytes; i++)
           bytes[i] = rand();
         
        save = state = (uint) (bytes[0] 
            + 256 * bytes[1] 
            + 65536 * bytes[2] 
            + 16777216 * bytes[3]);

        for (r = 0; r < repetitions; r++) 
            state = mixFunc(state);
		
        inb = state;
        for (i = 0; i < size; i++)
        {
            state = save ^ (1 << i);
            for (r = 0; r < repetitions; r++) 
                state = mixFunc(state);

            outb = state ^ inb;
            for (j = 0; j < size; j++)
            {
				/* We toggle bit i change it to the opposite one
				 * After that we see what bits were affected
				 * and calculate the matrix
				 */
                if ((outb & 1) != 0)
                    t[i, j]++;
                outb >>= 1;
            }
        }
    }

    for (i=0; i<size; i++)
    for (j=0; j<size; j++)
        result[i][j] = t[i][j] / dTrials;

    for (i = 0; i < size; i++)  
	{
       free(matrix[i]); 
	   free(t[i]);
	}

	free(matrix);
    free(bytes);
	free(t);

    return result;
}
