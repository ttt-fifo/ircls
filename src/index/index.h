/*
 * Indexing the newlines in the given file.
 * The newlines bytes positions are written in the buf array.
 * On buffer overflow, the first newline is deleted and the last is written
 * at its place. (circular buffering)
 * Accessing the array with positive or negative indexes, e.g:
 * index[1], index[-1]
 * is possible.
 */
#ifndef INDEX_H
#define INDEX_H

#include <stdio.h>                               //size_t


/*
 * The buffer datatype implementation
 */
typedef struct Index
{
	size_t *buf;                             //buffer to hold bytes
	int offset;                              //showing where buf[0]
	                                         //is currently positioned
	size_t size;                             //size of buf in bytes
} Index;


/*
 * Constructor
 * size: what buffer size do we need?
 * Returns: pointer to the created Index type variable
 */
Index *
index_new(const int size);

/*
 * Frees memory
 * index: the Index type variable to be freed
 */
void
index_del(Index *index);

/*
 * Appends one value to the end of the buffer
 * index: Index type variable to manipulate
 * val: what to save at the end of the buffer
 */
void
index_append(Index *index, const size_t val);

/*
 * Gets the value of the given index
 * index: the Index type variable
 * val: the value is saved in this size_t variable
 * i: index of the value to get (can be positive or negative)
 *    example: index_get(...., 3) or index_get(...., -2)
 * Returns: 1 on success, 0 on non-existent value
 */
int
index_get(Index *index, size_t *val, const int i);


#endif /*INDEX_H*/
