#include <index/index.h>

#include <stdlib.h> //malloc()
#include <stdio.h> //size_t


Index *
index_new(const int size)
{
	Index *index = (Index *)malloc(sizeof(Index));
	if(index == NULL) return NULL;

	index->buf = (size_t *)calloc(size, sizeof(size_t));
	if(index->buf == NULL)
	{
		index_del(index);
		return NULL;
	}

	index->offset = 0;
	index->size = size;

	return index;
} /*index_new()*/


void
index_del(Index *index)
{
	if(index->buf != NULL) free(index->buf);
	if(index != NULL) free(index);
} /*index_del()*/


int
index_append(Index *index, const size_t data)
{
	if(data == 0) return 0;
	index->buf[index->offset++] = data;
	if(index->offset == index->size) index->offset = 0;
	return 1;
} /*index_append()*/


size_t
index_get(Index *index, const int i)
{
	int i_;

	if(abs(i) > index->size - 1) return 0;

	i_ = index->offset + i;

	if(i_ < 0) i_ = index->size + i_;
	if(i_ > index->size - 1) i_ = i_ - index->size;

	return index->buf[i_];
} /*index_get()*/
