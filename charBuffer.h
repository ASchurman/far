/* 
 * File:   charBuffer.h
 * Author: Alexander Schurman
 *
 * Created on August 30, 2012
 * 
 * Provides an array of chars that remembers its size and can be grown.
 */

#ifndef CHARBUFFER_H
#define CHARBUFFER_H

typedef struct
{
    char* str; // The string held in this charBuffer
    unsigned int size; // The size of the malloc'd block pointed to by str
    unsigned int len; // The number of occupied elements of str
} charBuffer;

/* mallocs a charBuffer and returns a pointer to it.
 * Returns NULL upon failure */
charBuffer* charBufferNew();

/* mallocs a charBuffer with an initial nul-terminated string and returns a
 * pointer to it. Returns NULL upon failure. */
charBuffer* charBufferNewInit(char* initString);

// frees the given charBuffer
void charBufferDelete(charBuffer* buf);

/* Appends the char c to the end of the given charBuffer, growing it if
 * necessary. Returns a pointer to the modified charBuffer. */
charBuffer* charBufferAppend(charBuffer* buf, char c);

/* grows the given charBuffer by an internal constant factor and returns
 * a pointer to it. */
charBuffer* charBufferGrow(charBuffer* buf);

/* Clears the contents of the charBuffer by making buf->len be zero */
charBuffer* charBufferClear(charBuffer* buf);

#endif
