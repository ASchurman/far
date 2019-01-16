/* 
 * File:   Far.c
 * Author: Alexander Schurman (alexander.schurman@yale.edu)
 *
 * Created on August 30, 2012
 * 
 * Provides the implementation for Far
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "far.h"
#include "charBuffer.h"
#include "fileList.h"

#define TEMP_ARCHIVE_NAME "ARCHIVE.bak"

/*******************************************************************************
********************************** Errors **************************************
*******************************************************************************/

/* Called when the given archive name can't be opened. Prints a message to
 * stderr. Returns an error code. */
FAR_RTRN invalidArchiveNameError()
{
    fprintf(stderr, "Cannot open/create archive.\n");
    return OPEN_ERROR;
}

/* Called when the archive file is of an unexpected format. Prints a message to
 * stderr. Returns an error code. */
FAR_RTRN corruptedArchiveError()
{
    fprintf(stderr, "The archive is corrupted.\n");
    return CORRUPTED_ARCH;
}

/* Called when Far fails to open a non-archive file with fopen. Prints a
 * message to stderr. The argument filename is the name of the file that
 * failed to open. */
void fileOpenError(const char* filename)
{
    fprintf(stderr, "Cannot open file: %s\n", filename);
}

/* Called when Far fails to open/create a directory, likely due to lack of
 * permission. Prints a message to stderr. The argument is the name of the
 * directory. */
void dirOpenError(const char* dirname)
{
    fprintf(stderr, "Cannot open directory: %s\n", dirname);
}

/* Called when Far fails to create ARCHIVE.bak. Prints a message to stderr. */
FAR_RTRN openTempArchiveError()
{
    fprintf(stderr, "Failed to create temporary file.\n");
    return TEMP_FILE_ERROR;
}

/* Called when a file argument passed to Far can't be found in the given archive
 * file. Prints a message to stderr. */
void cannotFindArgError(const char* filename)
{
    fprintf(stderr, "Cannot find file: %s\n", filename);
}


/*******************************************************************************
***************************** Helper Functions *********************************
*******************************************************************************/

/* Copies a nul-terminated string starting at the current file position in
 * archive into the charBuffer named filename. Returns 0 if successful,
 * 1 if failed. */
int getFileName(FILE* archive, charBuffer* filename)
{
    charBufferClear(filename);
    
    int c;
    while((c = getc(archive)) != '\0')
    {
        if(c == EOF)
        {
            return 1; // failure
        }
        charBufferAppend(filename, c);
    }
    charBufferAppend(filename, c); // add final \0 to charBuffer
    
    return 0; // success
}

/* Determines if name is in nameArray (which has length numNames). Returns the
 * index of the first match in nameArray, or returns -1 if there is no match */
int isDuplicateFile(char* name, char** nameArray, unsigned int numNames)
{
    for(unsigned int i = 0; i < numNames; i++)
    {
        if(strcmp(name, nameArray[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

/* Determines if dirnames (which contains numDirnames elements) contains a
 * prefix for filename. Returns the index of the first element of dirnames that
 * is a prefix, or -1 if there's no prefix. */
int isDirectoryMatch(char* filename, char** dirnames, unsigned int numDirnames)
{
    char* substringPtr;
    
    for(unsigned int i = 0; i < numDirnames; i++)
    {
        substringPtr = strstr(filename, dirnames[i]);
        
        // see if substringPtr points to the beginning of filename, in which
        // case dirnames[i] is a prefix
        if(substringPtr == filename)
        {
            return i;
        }
    }
    return -1;
}



/* Finalizes tempArchive, closes and deletes oldArchive, and renames tempArchive
 * to archiveName. numFiles is the number of files stored in tempArchive.
 * Returns 0 on success. */
int finalizeArchive(FILE* oldArchive,
                    char* archiveName,
                    FILE* tempArchive,
                    unsigned int numFiles)
{
    // write the updated numFiles to tempArchive
    fseek(tempArchive, 0, SEEK_SET);
    fwrite(&numFiles, sizeof(unsigned int), 1, tempArchive);
    
    // close and delete oldArchive, and rename tempArchive to it
    if(oldArchive)
    {
        fclose(oldArchive);
    }
    fclose(tempArchive);
    rename(TEMP_ARCHIVE_NAME, archiveName);
    return 0;
}

// Frees the given array with numElts elements in it
void charArrayDelete(char** array, unsigned int numElts)
{
    for(unsigned int i = 0; i < numElts; i++)
    {
        free(array[i]);
    }
    free(array);
}

/* Reallocs array to hold numElts+1 uints and adds newValue to the end. Returns
 * a pointer to the new uint array. */
unsigned int* uintArrayAdd(unsigned int* array,
                           unsigned int numElts,
                           unsigned int newValue)
{
    numElts++;
    array = realloc(array, sizeof(unsigned int) * numElts);
    array[numElts-1] = newValue;
    return array;
}

// Used to pass to qsort() in printUnusedArgs. Compares 2 unsigned ints cast to
// void*
int compareUints(const void* a, const void* b)
{
    unsigned int eltA = *(unsigned int*)a;
    unsigned int eltB = *(unsigned int*)b;
    
    if(eltA < eltB) return -1;
    else if(eltA == eltB) return 0;
    else return 1;
}

/* Prints messages to stderr about filename arguments that didn't cause some
 * action. fileArgs (length numFileArgs) contains the original arguments passed
 * to Far. usedArgs (length numUsedArgs) contains indicies of arguments in
 * fileArgs that DID cause some action.
 * WARNING: This function will sort usedArgs. */
void printUnusedArgs(char** fileArgs,
                     unsigned int numFileArgs,
                     unsigned int* usedArgs,
                     unsigned int numUsedArgs)
{
    if(usedArgs && numUsedArgs > 0)
    {
        qsort(usedArgs, numUsedArgs, sizeof(unsigned int), compareUints);
    
        // find unused fileArgs before usedArgs[0]
        for(unsigned int i = 0; i < usedArgs[0]; i++)
        {
            cannotFindArgError(fileArgs[i]);
        }

        // find unused fileArgs in between elements of usedArgs
        for(unsigned int i = 1; i < numUsedArgs; i++)
        {
            for(unsigned int j = usedArgs[i-1] + 1; j < usedArgs[i]; j++)
            {
                cannotFindArgError(fileArgs[j]);
            }
        }

        // find unused fileArgs after the last of usedArgs
        for(unsigned int i = usedArgs[numUsedArgs-1] + 1; i < numFileArgs; i++)
        {
            cannotFindArgError(fileArgs[i]);
        }
    }
    else // none of fileArgs were used, so print an error message for them all
    {
        for(unsigned int i = 0; i < numFileArgs; i++)
        {
            cannotFindArgError(fileArgs[i]);
        }
    }
}

/*******************************************************************************
********************************** farAdd **************************************
*******************************************************************************/

/* Writes the contents of the open file 'fileToAdd' with the name 'filename'
 * and size in bytes 'fileSize' to the open file 'archive'.
 * fileToAdd: the file to write to the archive
 * filename: the name of the file to add
 * fileSize: the size in bytes of the file to add
 * archive: the archive to which we should write fileToAdd
 *
 * Returns -1 on failure, 0 on success. */
int writeFileToArchive(FILE* fileToAdd,
                       char* filename,
                       unsigned int fileSize,
                       FILE* archive)
{
    fwrite(filename, sizeof(char), strlen(filename) + 1, archive);
    fwrite(&fileSize, sizeof(unsigned int), 1, archive);

    int c;
    for(unsigned int i = 0; i < fileSize; i++)
    {
        c = fgetc(fileToAdd);
        if(c == EOF)
        {
            return -1;
        }
        else
        {
            fputc(c, archive);
        }
    }
    
    return 0;
}

FAR_RTRN farAdd(char* archiveName, char** fileArgs, unsigned char numFileArgs)
{
    FILE* oldArchive; // the archive file named archiveName
    FILE* tempArchive; // the temp archive with name TEMP_ARCHIVE_NAME
    FILE* fileToAdd = NULL; // a file with name from fileArgs to add to the
                            // archive
    
    unsigned int newNumFiles = 0; // the total number of files in the NEW
                                  // archive
    unsigned int oldNumFiles; // the total number of files in the OLD archive
    
    charBuffer* filename; // the name of a file being copied from oldArchive
    unsigned int fileSize; // the size of the current file in oldArchive
    
    struct stat fileStat; // holds data from any stat() calls
    DIR* dir; // pointer to a directory-type file with name from fileArgs
    
    // check for no-args
    if(numFileArgs == 0)
    {
        return SUCCESS;
    }
    
    /* eliminates invalid args (and prints errors) and expands directories to
     * include their contents */
    fileList* validArgs = fileListNew(fileArgs, numFileArgs);
    
    oldArchive = fopen(archiveName, "rb+");
    
    // read the number of files in oldArchive
    if(oldArchive)
    {
        if(fread(&oldNumFiles, sizeof(unsigned int), 1, oldArchive) < 1)
        {
            fclose(oldArchive);
            fileListDelete(validArgs);
            return corruptedArchiveError();
        }
    }
    else
    {
        oldNumFiles = 0;
    }
    
    // open the temp archive and check for error
    tempArchive = fopen(TEMP_ARCHIVE_NAME, "wb+");
    if(!tempArchive)
    {
        fclose(oldArchive);
        fileListDelete(validArgs);
        return openTempArchiveError();
    }
    
    // write numFiles simply to reserve the uint space;
    // it will be overwritten at the end
    fwrite(&newNumFiles, sizeof(unsigned int), 1, tempArchive);
    
    filename = charBufferNew();
    
    // copy oldArchive to tempArchive, not copying any entries that appear in
    // validArgs
    for(unsigned int i = 0; i < oldNumFiles; i++)
    {
        // get filename and fileSize
        if(getFileName(oldArchive, filename) ||
           fread(&fileSize, sizeof(unsigned int), 1, oldArchive) < 1)
        {
            fclose(oldArchive);
            fclose(tempArchive);
            unlink(TEMP_ARCHIVE_NAME);
            charBufferDelete(filename);
            fileListDelete(validArgs);
            return corruptedArchiveError();
        }
        
        // check if filename appears in validArgs
        char shouldCopy = (isDuplicateFile(filename->str,
                           validArgs->names,
                           validArgs->numNames) < 0);
        
        // write filename and file size
        if(shouldCopy)
        {
            fwrite(filename->str,
                   sizeof(char),
                   filename->len,
                   tempArchive);
            fwrite(&fileSize, sizeof(unsigned int), 1, tempArchive);
            newNumFiles++;
        }
        
        // read file contents and write to tempArchive if shouldCopy
        for(unsigned int j = 0; j < fileSize; j++)
        {
            int c = fgetc(oldArchive);
            if(c == EOF)
            {
                fclose(oldArchive);
                fclose(tempArchive);
                unlink(TEMP_ARCHIVE_NAME);
                charBufferDelete(filename);
                fileListDelete(validArgs);
                return corruptedArchiveError();
            }
            else if(shouldCopy)
            {
                fputc(c, tempArchive);
            }
        }
    }
    
    // append new files to the end of tempArchive
    for(unsigned int i = 0; i < validArgs->numNames; i++)
    {
        if(isDuplicateFile(validArgs->names[i], validArgs->names, i) >= 0)
        {
            continue;
        }
        
        // call stat on the file
        if(stat(validArgs->names[i], &fileStat) < 0)
        {
            fileOpenError(validArgs->names[i]);
            continue;
        }
        
        // check if directory
        dir = opendir(validArgs->names[i]);
        if(dir)
        {            
            // write directory name to archive
            fwrite(validArgs->names[i],
                   sizeof(char),
                   strlen(validArgs->names[i]) + 1,
                   tempArchive);
            
            // write directory size (zero) to archive
            fileSize = 0;
            fwrite(&fileSize, sizeof(unsigned int), 1, tempArchive);
            
            newNumFiles++;
            
            closedir(dir);
        }
        else
        {
            // add this regular file to tempArchive
            fileToAdd = fopen(validArgs->names[i], "rb");
            fileSize = fileStat.st_size;
            
            writeFileToArchive(fileToAdd,
                               validArgs->names[i],
                               fileSize,
                               tempArchive);
            
            fclose(fileToAdd);
            
            // update numFiles
            newNumFiles++;
        }
    }
    
    finalizeArchive(oldArchive, archiveName, tempArchive, newNumFiles);
    
    // clean-up
    charBufferDelete(filename);
    fileListDelete(validArgs);
    return SUCCESS;
}

/*******************************************************************************
******************************** farExtract ************************************
*******************************************************************************/

/* Checks to see if the directory named dirname exists and creates it if it
 * doesn't. Prints message to stderr upon failure. */
void ensureDirExists(char* dirname)
{
    DIR* dir = opendir(dirname);
    
    if(dir == NULL)
    {
        if(errno == EACCES) // if permission denied
        {
            dirOpenError(dirname);
        }
        else if(mkdir(dirname, 0777) < 0) // try to make the directory
        {
            dirOpenError(dirname);
        }
    }
    else
    {
        closedir(dir);
    }
}

/* Extracts the file named filename with size fileSize from archive. The file
 * pointer in archive must be pointing to the beginning of the body of the file
 * to extract. Prints a message to stderr if the extraction cannot be done.
 * Moves the file pointer to the end of the body of the file extracted.
 * Returns -1 if the archive is corrupted, else returns 0. */
char extractFile(FILE* archive, char* filename, unsigned int fileSize)
{
    int currentLen = 0;
    char* currentStr = NULL;
    
    while(currentLen < strlen(filename))
    {
        // count chars in filename up to the next slash, and include the slash
        currentLen += strcspn(&(filename[currentLen]), "/") + 1;
        
        currentStr = malloc(sizeof(char) * (currentLen + 1));
        
        strncpy(currentStr, filename, currentLen);
        currentStr[currentLen] = '\0';
        
        if(currentStr[currentLen - 1] == '/') // if it's a directory
        {
            ensureDirExists(currentStr);
        }
        else // it's a regular file
        {
            FILE* extractedFile = fopen(filename, "wb+");
            
            if(extractedFile)
            {
                for(unsigned int i = 0; i < fileSize; i++)
                {
                    int c = fgetc(archive);
                    if(c == EOF)
                    {
                        return -1;
                    }
                    else
                    {
                        fputc(c, extractedFile);
                    }
                }
                fclose(extractedFile);
            }
            else
            {
                fileOpenError(filename);
            }
        }
    }
    
    if(currentStr) free(currentStr);
    return 0;
}

FAR_RTRN farExtract(char* archiveName,
                    char** fileArgs,
                    unsigned char numFileArgs)
{
    FILE* archive; // the archive from which we are extracting
    unsigned int numFiles; // the number of files in archive
    
    charBuffer* filename; // the name of a file read from archive
    unsigned int fileSize; // the size of a file read from archive
    
    char** slashedFileArgs = NULL; /* holds the strings of fileArgs with a '/'
                                    * added to the end if it's not already
                                    * there. Used to compare directory paths */
    
    unsigned int* usedArgs = NULL; /* holds the indicies of entries in fileArgs
                                    * and slashedFileArgs that caused
                                    * extractions */
    unsigned int numUsedArgs = 0; // the number of elements in usedArgs
    
    archive = fopen(archiveName, "rb");
    if(!archive)
    {
        return invalidArchiveNameError();
    }
    else if(fread(&numFiles, sizeof(unsigned int), 1, archive) < 1)
    {
        fclose(archive);
        return corruptedArchiveError();
    }
    
    if(numFileArgs > 0)
    {
        // initialize slashedFileArgs
        slashedFileArgs = malloc(sizeof(char*) * numFileArgs);
        for(unsigned int i = 0; i < numFileArgs; i++)
        {
            slashedFileArgs[i] = ensureSingleSlash(fileArgs[i]);
        }
    }
    
    filename = charBufferNew();
    
    // move through archive, extracting all files that should be extracted
    for(unsigned int i = 0; i < numFiles; i++)
    {
        // get filename and fileSize
        if(getFileName(archive, filename) ||
           fread(&fileSize, sizeof(unsigned int), 1, archive) < 1)
        {
            // failure
            fclose(archive);
            charBufferDelete(filename);
            if(slashedFileArgs) charArrayDelete(slashedFileArgs, numFileArgs);
            return corruptedArchiveError();
        }
        
        // extract all files if we weren't passed any fileArgs
        if(numFileArgs == 0)
        {
            if(extractFile(archive, filename->str, fileSize) < 0)
            {
                // corrupted archive
                fclose(archive);
                charBufferDelete(filename);
                if(slashedFileArgs) charArrayDelete(slashedFileArgs,
                                                    numFileArgs);
                return corruptedArchiveError();
            }
            continue;
        }
        
        // compare filename to the arguments passed to 'x'
        int exactMatchIndex = isDuplicateFile(filename->str,
                                              fileArgs,
                                              numFileArgs);
        int directoryMatchIndex = isDirectoryMatch(filename->str,
                                                   slashedFileArgs,
                                                   numFileArgs);
        
        // remember which file argument caused this extraction, if one is
        // to occur
        if(exactMatchIndex >= 0)
        {
            usedArgs = uintArrayAdd(usedArgs, numUsedArgs, exactMatchIndex);
            numUsedArgs++;
        }
        else if(directoryMatchIndex >= 0)
        {
            usedArgs = uintArrayAdd(usedArgs, numUsedArgs, directoryMatchIndex);
            numUsedArgs++;
        }
        
        if(exactMatchIndex >= 0 || directoryMatchIndex >= 0)
        {
            if(extractFile(archive, filename->str, fileSize) < 0)
            {
                // corrupted archive
                fclose(archive);
                charBufferDelete(filename);
                if(slashedFileArgs) charArrayDelete(slashedFileArgs,
                                                    numFileArgs);
                return corruptedArchiveError();
            }
        }
        else
        {
            // move past file body without extracting
            for(unsigned int j = 0; j < fileSize; j++)
            {
                int c = fgetc(archive);
                if(c == EOF)
                {
                    // unexpected EOF; corrupted archive
                    fclose(archive);
                    charBufferDelete(filename);
                    if(slashedFileArgs) charArrayDelete(slashedFileArgs,
                                                        numFileArgs);
                    return corruptedArchiveError();
                }
            }
        }
    }
    
    // print messages to stderr about unused filename arguments
    printUnusedArgs(fileArgs, numFileArgs, usedArgs, numUsedArgs);
    
    // clean-up
    fclose(archive);
    charBufferDelete(filename);
    if(slashedFileArgs) charArrayDelete(slashedFileArgs, numFileArgs);
    return SUCCESS;
}

/*******************************************************************************
******************************** farDelete *************************************
*******************************************************************************/

FAR_RTRN farDelete(char* archiveName,
                   char** fileArgs,
                   unsigned char numFileArgs)
{
    FILE* oldArchive; // the old archive named archiveName
    FILE* tempArchive; // temporary archive that's renamed to archiveName
    
    unsigned int oldNumFiles; // number of files in oldArchive
    unsigned int newNumFiles = 0; // number of files in tempArchive
    
    charBuffer* filename; // the name of a file being copied from oldArchive
    unsigned int fileSize; // the size of the current file in oldArchive
    
    char** slashedFileArgs; /* holds the strings of fileArgs with a '/' added
                             * to the end if it's not already there. Used to
                             * compare directory names */
    
    unsigned int* usedArgs = NULL; /* holds indicies of entries in fileArgs and
                                    * slashedFileArgs that caused deletions */
    unsigned int numUsedArgs = 0; // the number of elements in usedArgs
    
    // check for no-args
    if(numFileArgs == 0)
    {
        return SUCCESS;
    }
    
    // initialize slashedFileArgs
    slashedFileArgs = malloc(sizeof(char*) * numFileArgs);
    for(unsigned int i = 0; i < numFileArgs; i++)
    {
        slashedFileArgs[i] = ensureSingleSlash(fileArgs[i]);
    }
    
    oldArchive = fopen(archiveName, "rb+");
    
    // read the number of files in oldArchive
    if(oldArchive)
    {
        if(fread(&oldNumFiles, sizeof(unsigned int), 1, oldArchive) < 1)
        {
            fclose(oldArchive);
            charArrayDelete(slashedFileArgs, numFileArgs);
            return corruptedArchiveError();
        }
    }
    else
    {
        charArrayDelete(slashedFileArgs, numFileArgs);
        return invalidArchiveNameError();
    }
    
    // open the temp archive and check for error
    tempArchive = fopen(TEMP_ARCHIVE_NAME, "wb+");
    if(!tempArchive)
    {
        fclose(oldArchive);
        charArrayDelete(slashedFileArgs, numFileArgs);
        return openTempArchiveError();
    }
    
    // write newNumFiles simply to reserve the uint space;
    // it will be overwritten at the end
    fwrite(&newNumFiles, sizeof(unsigned int), 1, tempArchive);
    
    filename = charBufferNew();
    
    // copy oldArchive to tempArchive, not copying any entries that we're
    // supposed to delete
    for(unsigned int i = 0; i < oldNumFiles; i++)
    {
        // get filename and fileSize
        if(getFileName(oldArchive, filename) ||
           fread(&fileSize, sizeof(unsigned int), 1, oldArchive) < 1)
        {
            fclose(oldArchive);
            fclose(tempArchive);
            unlink(TEMP_ARCHIVE_NAME);
            charBufferDelete(filename);
            if(usedArgs) free(usedArgs);
            charArrayDelete(slashedFileArgs, numFileArgs);
            return corruptedArchiveError();
        }
        
        // compare filename to the arguments passed to 'd'
        int exactMatchIndex = isDuplicateFile(filename->str,
                                              fileArgs,
                                              numFileArgs);
        int directoryMatchIndex = isDirectoryMatch(filename->str,
                                                   slashedFileArgs,
                                                   numFileArgs);
        
        // remember which file argument caused this deletion, if a deletion is
        // to occur
        if(exactMatchIndex >= 0)
        {
            usedArgs = uintArrayAdd(usedArgs, numUsedArgs, exactMatchIndex);
            numUsedArgs++;
        }
        else if(directoryMatchIndex >= 0)
        {
            usedArgs = uintArrayAdd(usedArgs, numUsedArgs, directoryMatchIndex);
            numUsedArgs++;
        }
        
        char shouldCopy = exactMatchIndex < 0 && directoryMatchIndex < 0;
        
        // write filename and file size
        if(shouldCopy)
        {
            fwrite(filename->str,
                   sizeof(char),
                   filename->len,
                   tempArchive);
            fwrite(&fileSize, sizeof(unsigned int), 1, tempArchive);
            newNumFiles++;
        }
        
        // read file body (and copy to tempArchive if shouldCopy)
        for(unsigned int j = 0; j < fileSize; j++)
        {
            int c = fgetc(oldArchive);
            if(c == EOF)
            {
                fclose(oldArchive);
                fclose(tempArchive);
                unlink(TEMP_ARCHIVE_NAME);
                charBufferDelete(filename);
                if(usedArgs) free(usedArgs);
                charArrayDelete(slashedFileArgs, numFileArgs);
                return corruptedArchiveError();
            }
            else if(shouldCopy)
            {
                fputc(c, tempArchive);
            }
        }
    }
    
    // determine which elements of fileArgs didn't cause a deletion
    printUnusedArgs(fileArgs, numFileArgs, usedArgs, numUsedArgs);
    
    // finish and clean-up
    finalizeArchive(oldArchive, archiveName, tempArchive, newNumFiles);
    charBufferDelete(filename);
    if(usedArgs) free(usedArgs);
    charArrayDelete(slashedFileArgs, numFileArgs);
    return SUCCESS;
}

/*******************************************************************************
********************************* farPrint *************************************
*******************************************************************************/

FAR_RTRN farPrint(char* archiveName)
{
    FILE* archive; // the archive file named archiveName
    unsigned int numFiles; // the number of files in the archive
    charBuffer* filename; // the current filename being read from archive
    unsigned int fileSize; // the size of the current file in archive
    
    archive = fopen(archiveName, "rb");
    
    // check for open file error
    if(!archive)
    {
        return invalidArchiveNameError();
    }
    
    // get number of files in archive
    if(fread(&numFiles, sizeof(unsigned int), 1, archive) < 1)
    {
        fclose(archive);
        return corruptedArchiveError();
    }
    
    // print name and size of each file to stdout
    filename = charBufferNew();
    for(unsigned int i = 0; i < numFiles; i++)
    {
        // copy the filename into the filename charBuffer
        if(getFileName(archive, filename))
        {
            fclose(archive);
            charBufferDelete(filename);
            return corruptedArchiveError();
        }
        
        // get size of current file
        if(fread(&fileSize, sizeof(unsigned int), 1, archive) < 1)
        {
            fclose(archive);
            charBufferDelete(filename);
            return corruptedArchiveError();
        }
        
        printf("%8d %s\n", fileSize, filename->str);
        
        // skip the file body
        for(unsigned int j = 0; j < fileSize; j++)
        {
            int c = fgetc(archive);
            if(c == EOF)
            {
                fclose(archive);
                charBufferDelete(filename);
                return corruptedArchiveError();
            }
        }
    }
    
    // clean-up and return success
    charBufferDelete(filename);
    fclose(archive);
    return SUCCESS;
}