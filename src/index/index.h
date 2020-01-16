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

int
index_append(Index *index, const size_t data);

size_t
index_get(Index *index, const int i);


#endif /*INDEX_H*/
