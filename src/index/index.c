/* Diagram to understand the code
 *
 *                  --------------------------
 *                  | 0 | val[5] or val[-2]  / <-----------
 *                  --------------------------            |
 *                  | 1 | val[6] or val[-1]  /            |
 *                  --------------------------            |
 *   offset ------> | 2 | val[0] or ...      /            | wrap
 *            |     --------------------------            | around
 *            |     | 3 | val[1] or ...      /            ^
 *       write|     --------------------------            |
 *            |     | 4 | val[2] or ...      /            |
 *           \|/    --------------------------            |
 *                  | 5 | val[3] or ...      /            |
 *                  --------------------------            |
 *                  | 6 | val[4] or val[-3]  / ----->-----|
 *                  --------------------------
 *
 * NOTE: this is simple implementation of python-like array, accessible by
 *       index[1], index[-2] - like approach.
 */
#include <index/index.h>

#include <stdlib.h>                              //malloc()
#include <stdio.h>                               //size_t


/*
 * Constructor
 * size: what buffer size do we need?
 * Returns: pointer to the created Index type variable
 */
Index *
index_new(const int size)
{
	/*reserve index memory*/
	Index *index = (Index *)malloc(sizeof(Index));
	if(index == NULL) return NULL;

	/*reserve and clear memory for the data buffer*/
	index->buf = (size_t *)calloc(size, sizeof(size_t));
	if(index->buf == NULL)
	{
		index_del(index);
		return NULL;
	}

	/*initialize buffer*/
	index->size = size;
	index->buf[0] = 0;                       //0 byte is always indexed
	index->offset = 1;                       //and the offset is on [1]

	return index;
} /*index_new()*/


/*
 * Frees memory
 * index: the Index type variable to be freed
 */
void
index_del(Index *index)
{
	if(index->buf != NULL) free(index->buf);
	if(index != NULL) free(index);
} /*index_del()*/


/*
 * Appends one value to the end of the buffer
 * index: Index type variable to manipulate
 * val: what to save at the end of the buffer
 */
void
index_append(Index *index, const size_t val)
{
	index->buf[index->offset++] = val;
	if(index->offset == index->size) index->offset = 0; //wrap over
} /*index_append()*/


/*
 * Gets the value of the given index
 * index: the Index type variable
 * val: the value is saved in this size_t variable
 * i: index of the value to get (can be positive or negative)
 *    example: index_get(...., 3) or index_get(...., -2)
 * Returns: 1 on success, 0 on non-existent value
 */
int
index_get(Index *index, size_t *val, const int i)
{
	int i_;                                  //i_ is calculated from i

	if(abs(i) > index->size - 1) return 0;   //check for nonexistent val

	i_ = index->offset + i;                  //initialize i_

	if(i_ < 0) i_ = index->size + i_;        //calculate for negative i
	if(i_ > index->size - 1) i_ = i_ - index->size; //wrap over

	/* assign value and return OK*/
	*val = index->buf[i_];
	return 1;
} /*index_get()*/
