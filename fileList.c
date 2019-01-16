/* 
 * File:   fileList.c
 * Author: Alexander Schurman (alexander.schurman@yale.edu)
 *
 * Created on September 4, 2012
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "fileList.h"

#define INIT_FILELIST_SIZE (10)
#define FILELIST_GROWTH_FACTOR (2)

////////////////////////////// Errors /////////////////////////////////////

/* Called if the file named filename cannot be opened. Prints a message to
 * stderr. */
void cannotOpenError(const char* filename)
{
    fprintf(stderr, "Cannot open file: %s\n", filename);
}

/* Called if the file named filename is unsupported by Far. Prints a message to
 * stderr. */
void unsupportedError(const char* filename)
{
    fprintf(stderr, "Unsupported file type: %s\n", filename);
}


//////////////////////////// Private functions ///////////////////////////////

/* Returns 0 if there is no file with the given filename, or we don't have
 *      permision to access it.
 * Returns 1 if the filename is a valid regular file name.
 * Returns 2 if the filename is a valid directory name.
 * Returns 3 if the file called filename is unsupported (like sockets) */
char checkFileType(const char* filename)
{
    struct stat fileStat;
    
    if(lstat(filename, &fileStat) < 0)
    {
        // failure; no file found with given name
        return 0;
    }
    
    mode_t mode = fileStat.st_mode;
    
    if(S_ISCHR(mode) || S_ISBLK(mode) || S_ISFIFO(mode) || S_ISLNK(mode) ||
       S_ISSOCK(mode))
    {
        return 3;
    }
    else if(S_ISREG(mode))
    {
        FILE* file = fopen(filename, "rb");
        if(file)
        {
            fclose(file);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if(S_ISDIR(mode))
    {
        DIR* dir = opendir(filename);
        if(dir)
        {
            closedir(dir);
            return 2;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 3;
    }
}

/* Grows files->names by FILELIST_GROWTH_FACTOR if it's needed to add another
 * filename */
void fileListGrow(fileList* files)
{
    if(files->numNames == files->sizeNames)
    {
        files->sizeNames *= FILELIST_GROWTH_FACTOR;
        files->names = realloc(files->names, sizeof(char*) * files->sizeNames);
    }
}

/* Adds the filename to files, growing files->names first if needed */
void fileListAddName(fileList* files, const char* filename)
{
    char* str = malloc(sizeof(char) * (strlen(filename) + 1));
    strcpy(str, filename);
    
    fileListGrow(files); // only grows if it's needed to add another filename
    files->names[files->numNames] = str;
    files->numNames++;
}

/* malloc's a new char* that contains dirExtension appended to dirBase */
char* appendDir(const char* dirBase, const char* dirExtension)
{
    unsigned int origLen = strlen(dirBase);
    unsigned int extLen = strlen(dirExtension);
    
    char* output = malloc(sizeof(char) * (origLen + extLen + 1));
    strcpy(output, dirBase);
    strcpy(&(output[origLen]), dirExtension);
    return output;
}

/* fileListCheckAndAddName is defined later, but it's needed for fileListAddDir,
 * which is called by fileListCheckAndAddName */
void fileListCheckAndAddName(fileList* files, const char* filename);

/* Adds the directory named dirname to files, including the contents of
 * dirname */
void fileListAddDir(fileList* files, const char* dirname)
{
    DIR* dir; // the directory named dirname
    struct dirent* dirEntry; // the dirent for dir
    
    // slashedDirname is added to files and is thus freed when files is freed
    char* slashedDirname = ensureSingleSlash(dirname);
    fileListAddName(files, slashedDirname);
    
    dir = opendir(dirname);
    
    while((dirEntry = readdir(dir)) != NULL)
    {
        // we don't want to add the .. or . directories within dir
        if(strcmp(dirEntry->d_name, ".") != 0 &&
           strcmp(dirEntry->d_name, "..") != 0)
        {
            char* childDirname = appendDir(slashedDirname, dirEntry->d_name);
            fileListCheckAndAddName(files, childDirname);
            free(childDirname);
        }
    }
}

/* Checks filename for validity and either adds it (and its contents if a dir)
 * to files or prints a message to stderr. */
void fileListCheckAndAddName(fileList* files, const char* filename)
{
    char fileType = checkFileType(filename);

    switch(fileType)
    {
        case 0:
            cannotOpenError(filename);
            break;
        case 3:
            //unsupportedError(filename);
            break;

        case 1: // regular file
            fileListAddName(files, filename);
            break;

        case 2: // directory
            fileListAddDir(files, filename);
            break;
    }
}


///////////////////////////// Public functions ///////////////////////////////

char* ensureSingleSlash(const char* dirname)
{
    char* output;
    int dirnameLen = strlen(dirname); // the strlen of dirname
    int outputSize = dirnameLen + 2; /* the malloc'd space of output.
                                      * initialized to the input len + '/' +
                                      * nul. Trailing slashes in the input len
                                      * will be subtracted out */
    
    // subtract the trailing slashes from outputSize
    for(int i = dirnameLen - 1;
        i >= 0 && dirname[i] == '/';
        i--)
    {
        outputSize--;
    }
    
    output = malloc(sizeof(char) * outputSize);
    
    // copy the chars in dirname before trailing slashes and nul
    strncpy(output, dirname, outputSize - 2);
    
    // add final slash and null
    output[outputSize - 2] = '/';
    output[outputSize - 1] = '\0';
    return output;
}

fileList* fileListNew(char** initNames, unsigned int numInitNames)
{
    fileList* files = malloc(sizeof(fileList));
    files->sizeNames = INIT_FILELIST_SIZE;
    files->names = malloc(sizeof(char*) * INIT_FILELIST_SIZE);
    files->numNames = 0;
    
    for(unsigned int i = 0; i < numInitNames; i++)
    {
        fileListCheckAndAddName(files, initNames[i]);
    }
    
    return files;
}

void fileListDelete(fileList* files)
{
    for(unsigned int i = 0; i < files->numNames; i++)
    {
        free(files->names[i]);
    }
    free(files->names);
    free(files);
}

void fileListContract(fileList* files)
{
    files->names = realloc(files->names, sizeof(char*) * files->numNames);
    files->sizeNames = files->numNames;
}