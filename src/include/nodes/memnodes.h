#ifndef MEMNODES_H
#define MEMNODES_H

#include <stdio.h>

typedef struct MemoryContextData *MemoryContext;

typedef struct MemoryContextMethods
{
	void	   *(*alloc) (MemoryContext context, size_t size);
	void		(*free_p) (MemoryContext context, void *pointer);
	void	   *(*realloc) (MemoryContext context, void *pointer, size_t size);
	void		(*reset) (MemoryContext context);
	void		(*delete_context) (MemoryContext context);
	size_t		(*get_chunk_space) (MemoryContext context, void *pointer);
	int		(*is_empty) (MemoryContext context);
} MemoryContextMethods;

typedef struct MemoryContextData
{
	int		isReset;		
	int		allowInCritSection; 
	size_t		mem_allocated;	
	const MemoryContextMethods *methods;	
	MemoryContext parent;		
	MemoryContext firstchild;	
	MemoryContext prevchild;	
	MemoryContext nextchild;	
	const char *name;			
	const char *ident;			
} MemoryContextData;

#endif