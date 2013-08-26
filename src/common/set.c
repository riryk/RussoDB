
#include "set.h"

int Set_GetFirstMember(struct Set *s)
{
	int			Words;
	int			WordNum;
    int			wordnum;

	if (s == 0)
		return -1;

	Words = s->Items;
	for (wordnum = 0; wordnum < Words; wordnum++)
	{
		unsigned int  word = s->Items[wordnum];

		if (word != 0)
		{
			int			result;
            /* For example if we have word = 
			 *  23: 000010111
			 * -23: 111101001  = 00001 - gets the last bit   */
			word = ((int)word) & (-(int)word);
			/* If the last bit of the number is 1, word = 1
			 * ~word = 111110 - and after & we clear the last bit
			 * If the last bit is 0. word = 0. We do not clear the last bit */
			s->Items[wordnum] &= ~word;

			result = wordnum * WORD_BITS_COUNT;

			/* 255 = 256 - 1 = 1111 1111. word & 255 == 0 only when 
			 * word == 00000 
			 * Let's word = 11 1110 1000 & 1111 1111 = 1110 1000
			 * word >> 8 = 11  */
			while ((word & 255) == 0)
			{
				word >>= 8;
				result += 8;
			}
			result += RightMostPosition[word & 255];
			return result;
		}
	}
	return -1;
}