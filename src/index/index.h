#ifndef INDEX_H
#define INDEX_H

#include <stdio.h> //size_t

typedef struct Index
{
	size_t *buf;
	int offset;
	size_t size;
} Index;


Index *
index_new(const int size);

void
index_del(Index *index);

void
index_append(Index *index, const size_t data);

int
index_get(Index *index, size_t *val, const int i);


#endif /*INDEX_H*/
