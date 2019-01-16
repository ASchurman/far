/* 
 * File:   charBuffer.c
 * Author: Alexander Schurman (alexander.schurman@yale.edu)
 *
 * Created on August 30, 2012
 * 
 * Provides an array of chars that remembers its size and can be grown.
 */

#include <stdio.h>
#include <stdlib.h>
#include "charBuffer.h"

#define CHARBUFFER_INIT_SIZE (10)
#define CHARBUFFER_GROWTH_FACTOR (2)

charBuffer* charBufferNew()
{
    charBuffer* buf = malloc(sizeof(charBuffer));
    
    if(!buf)
    {
        return NULL;
    }
    else
    {
        buf->size = CHARBUFFER_INIT_SIZE;
        buf->str = malloc(sizeof(char) * CHARBUFFER_INIT_SIZE);
        buf->len = 0;
        return buf;
    }
}

charBuffer* charBufferNewInit(char* initString)
{
    charBuffer* buf = charBufferNew();
    
    if(buf)
    {
        for(int i = 0; initString[i] != '\0'; i++)
        {
            charBufferAppend(buf, initString[i]);
        }
        // append final \0
        charBufferAppend(buf, '\0');
    }
    
    return buf;
}

void charBufferDelete(charBuffer* buf)
{
    free(buf->str);
    free(buf);
}

charBuffer* charBufferAppend(charBuffer* buf, char c)
{
    if(buf->len == buf->size)
    {
        charBufferGrow(buf);
    }
    
    buf->str[buf->len] = c;
    buf->len++;
    
    return buf;
}

charBuffer* charBufferGrow(charBuffer* buf)
{
    buf->size *= CHARBUFFER_GROWTH_FACTOR;
    buf->str = realloc(buf->str, buf->size);
    
    return buf;
}

charBuffer* charBufferClear(charBuffer* buf)
{
    buf->len = 0;
    buf->str[0] = '\0';
    return buf;
}