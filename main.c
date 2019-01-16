/* 
 * File:   main.c
 * Author: Alexander Schurman (alexander.schurman@yale.edu)
 *
 * Created on August 29, 2012
 * 
 * Contains the main method, which interprets the arguments passed to Far and
 * calls the appropriate functions in other files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "far.h"

/* Called if Far isn't passed valid arguments. Prints a message to stderr. */
void invalidArgsError()
{
    fprintf(stderr, "Invalid arguments; Far r|x|d|t archive [filename]*\n");
}

/* Returns a malloc'd array of strings that is identical to names with trailing
 * '/' characters removed */
char** stripTrailingSlashes(char** names, unsigned int numNames)
{
    char** slashlessNames = malloc(sizeof(char*) * numNames);
    
    for(unsigned int i = 0; i < numNames; i++)
    {
        int nameLen = strlen(names[i]);
        int slashlessSize = nameLen + 1; /* trailing slashes in names[i] will be
                                          * subtracted out of slashlessSize
                                          * before malloc'ing */
        
        // subtract the trailing slashes
        for(int j = nameLen - 1; j >= 0 && names[i][j] == '/'; j--)
        {
            slashlessSize--;
        }
        
        if(slashlessSize == 1) // only slashes
        {
            slashlessNames[i] = malloc(sizeof(char) * 2);
            slashlessNames[i][0] = '/';
            slashlessNames[i][1] = '\0';
        }
        else
        {
            slashlessNames[i] = malloc(sizeof(char) * slashlessSize);
            strncpy(slashlessNames[i], names[i], slashlessSize - 1);
            slashlessNames[i][slashlessSize - 1] = '\0';
        }
    }
    
    return slashlessNames;
}

// Frees a malloc'd array of numNames strings
void freeCharArray(char** names, unsigned int numNames)
{
    for(unsigned int i = 0; i < numNames; i++)
    {
        free(names[i]);
    }
    free(names);
}

/* Interprets the arguments passed from the command line and calls
 * the appropriate function in far.h.
 * Returns one of the return codes defined in far.h, or 4 for invalid command
 * line invocation of Far */
int main(int argc, char** argv)
{
    char* archiveName; // The name of the archive passed to Far
    char** filenames; // Pointer to the beginning of the filenames in argv,
                      // or NULL if there are no filenames.
    unsigned char numFiles; // The number of filenames passed to Far
    FAR_RTRN returnCode;
    
    if(argc < 3) // less than "Far" plus a KEY plus an ARCHIVE
    {
        invalidArgsError();
        return 1;
    }
    
    archiveName = argv[2];
    
    if(argc > 3) // if Far was passed at least one filename
    {
        numFiles = argc - 3;
        filenames = stripTrailingSlashes(&(argv[3]), numFiles);
    }
    else
    {
        filenames = NULL;
        numFiles = 0;
    }
    
    if(strcmp(argv[1], "r") == 0)
    {
        returnCode = farAdd(archiveName, filenames, numFiles);
    }
    else if(strcmp(argv[1], "x") == 0)
    {
        returnCode = farExtract(archiveName, filenames, numFiles);
    }
    else if(strcmp(argv[1], "d") == 0)
    {
        returnCode = farDelete(archiveName, filenames, numFiles);
    }
    else if(strcmp(argv[1], "t") == 0)
    {
        returnCode = farPrint(archiveName);
    }
    else // first arg is not a valid key
    {
        invalidArgsError();
        returnCode = 4;
    }
    
    if(filenames) freeCharArray(filenames, numFiles);
    return returnCode;
}