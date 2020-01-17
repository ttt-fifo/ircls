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

	index->size = size;
	index->buf[0] = 0;                       //0 byte is always indexed
	index->offset = 1;                       //and the offset is on [1]

	return index;
} /*index_new()*/


void
index_del(Index *index)
{
	if(index->buf != NULL) free(index->buf);
	if(index != NULL) free(index);
} /*index_del()*/


void
index_append(Index *index, const size_t data)
{
	index->buf[index->offset++] = data;
	if(index->offset == index->size) index->offset = 0;
} /*index_append()*/


int
index_get(Index *index, size_t *val, const int i)
{
	int i_;

	if(abs(i) > index->size - 1) return 0;

	i_ = index->offset + i;

	if(i_ < 0) i_ = index->size + i_;
	if(i_ > index->size - 1) i_ = i_ - index->size;

	*val = index->buf[i_];

	return 1;
} /*index_get()*/
