
typedef struct CreateStatement
{
	struct ListNodeType	  	     type;
	struct RelationParseInfo*    relation;
	struct List*                 tableIngredients;
	struct List*                 constraints;	
	struct List*                 options;
	char*                        tablespace;
};