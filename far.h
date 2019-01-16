/* 
 * File:   Far.h
 * Author: Alexander Schurman
 *
 * Created on August 30, 2012
 * 
 * Provides the interface for Far
 */

#ifndef FAR_H
#define FAR_H

// return codes for Far functions
typedef enum
{
    SUCCESS = 0,
    OPEN_ERROR, // failed to open the archive file
    CORRUPTED_ARCH, // the archive file is corrupted
    TEMP_FILE_ERROR // failed to create the temporary archive file
} FAR_RTRN;

/* Executes Far's 'r' command to add to an archive.
 * Returns a code as described above. */
FAR_RTRN farAdd(char* archiveName, char** fileArgs, unsigned char numFileArgs);

/* Executes Far's 'x' command to extract from an archive.
 * Returns a code as described above. */
FAR_RTRN farExtract(char* archiveName,
                    char** fileArgs,
                    unsigned char numFileArgs);

/* Executes Far's 'd' command to delete files from an archive.
 * Returns a code as described above. */
FAR_RTRN farDelete(char* archiveName,
                   char** fileArgs,
                   unsigned char numFileArgs);

/* Executes Far's 't' command to print the contents of an archive.
 * Returns a code as described above. */
FAR_RTRN farPrint(char* archiveName);

#endif
