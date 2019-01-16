/* 
 * File:   fileList.h
 * Author: Alexander Schurman
 *
 * Created on September 4, 2012
 * 
 * Handles a list of valid filenames.
 */

#ifndef FILELIST_H
#define FILELIST_H

typedef struct
{
    char** names; // a list of nul-terminated filenames
    unsigned int numNames; // the number of filenames in names
    unsigned int sizeNames; // the malloc'd size of names
} fileList;

/* Creates a fileList and returns a pointer to it. This fileList's names
 * will be initialized using initNames; only valid filenames in initNames are
 * added, and directories are expanded to also include their contents.
 * 
 * Prints messages to stderr regarding invalid filenames in initNames. */
fileList* fileListNew(char** initNames, unsigned int numInitNames);

// Frees a fileList
void fileListDelete(fileList* files);

// Frees any excess space in files->names and updates files->sizeNames
void fileListContract(fileList* files);

/* Returns a malloc'd string equal to dirname with a single / at the end
 * before nul if it's not already there. */
char* ensureSingleSlash(const char* dirname);

#endif
