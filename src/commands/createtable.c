
#include "parser.h"



typedef struct CreateStatement
{
	int	  	     type;
	struct RelationParseInfo*    relation;
	struct List*                 tableIngredients;
	struct List*                 constraints;	
	struct List*                 options;
	char*                        tablespace;
};