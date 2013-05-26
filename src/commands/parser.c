
enum ListNodeType
{
	L_Invalid = 0
};

struct ListItem
{
	union
	{
		void*       pointer;
		int			integer;
		Oid			oid_value;
	}            data;
	ListItem*    next;
};


struct List
{
	ListNodeType		type;		
	int			        length;
	ListItem*           head;
	ListItem*           tail;
};

struct Node
{
	ListNodeType		type;		
};

enum InheritanceInfo
{
	INHERIT_NO,						
	INHERIT_YES,					
	INHERIT_DEFAULT					
};

struct RelationAlias
{
	struct ListNodeType		type;		
	char*                   aliasNames;	
	struct List*            columnNames;
};

struct RelationParseInfo
{
	struct ListNodeType	  	     type;
	char*                        catalogName;	
	char*                        schemaName;
	char*                        relationName;	
	struct InheritanceInfo	     inheritanceInfo;
	unsigned int			     relationId;		
	char		                 relationKind;
	struct RelationAlias*        relationDetails;
	int		                     isLateral;
	int		                     isInheritance;
	int		                     inFromStatement;	
};

struct InsertStatement
{
	struct ListNodeType		     type;
	struct RelationParseInfo*    relation;
	struct List*                 columns;	
	struct Node*                 selectStatement;		
	struct List*                 returningList;
};


struct SelectStatement
{
	struct ListNodeType		     type;
	struct List*                 distinctClause; 
	struct List*                 targetList;
	struct List*	             fromClause;
	struct Node*                 whereClause;
	struct List*                 groupClause;
	struct Node*                 havingClause;
	struct List*                 windowClause;

	struct List*	             valuesLists;
	struct List*                 sortClause;
	struct Node*                 limitOffset;
	struct Node*                 limitCount;
	struct List*                 lockingClause;

	int		                     all;	
	struct SelectStatement*      leftChild;
	struct SelectStatement*      rightChild;	
};

enum CommandType
{
	CMD_UNKNOWN,
	CMD_SELECT,	
	CMD_UPDATE,	
	CMD_INSERT,	
	CMD_DELETE,
	CMD_UTILITY,	
	CMD_NOTHING	
};

struct ParseState
{
    int         isInsert;
	Relation	relation;
}

struct Query
{
    CommandType		commandType;	
	int			    resultRelation; 
}

struct Query* parse(struct ParseState* parseState, struct Node* parseTree)
{
	int	                       hasSelect;
    struct Query*              query = malloc(sizeof(struct Query));
    struct InsertStatement*    insertStatement = (struct InsertStatement*)parseTree;
	struct SelectStatement*    selectStatement = (struct SelectStatement*)insertStatement->selectStatement;
	struct List*               valuesLists = selectStatement->valuesLists;
    struct RelationParseInfo*  relationParseInfo;

	query->commandType = CMD_INSERT;
	parseState->isInsert = 1;
    
	hasSelect = selectStatement && (selectStatement->valuesLists == (List*)NULL);
    relationParseInfo = (struct RelationParseInfo*)malloc(sizeof(struct RelationParseInfo));

    return query;
}