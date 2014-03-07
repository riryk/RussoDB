
#ifndef NODES_H
#define NODES_H

typedef enum EListNodeType
{
    T_Invalid = 0,

    T_IndexInfo = 10,
	T_ExprContext,
	T_ProjectionInfo,
	T_JunkFilter
} EListNodeType;

#endif 