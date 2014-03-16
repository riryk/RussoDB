
#ifndef NODES_H
#define NODES_H

typedef enum ENodeType
{
    T_Invalid = 0,

    T_IndexInfo = 10,
	T_ExprContext,
	T_ProjectionInfo,
	T_JunkFilter,
	T_List
} ENodeType;

typedef struct SNode
{
	ENodeType	 type;
} SNode, *Node;

#define nodeType(node) (((Node)(node))->type)
#define isOfType(node,_type_) (nodeTag(node) == T_##_type_)

#endif 