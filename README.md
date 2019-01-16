# Far

A GNU/Linux file archiver offering a limited subset of the functionality of ar
and tar.

## Compiling

To compile Far, simply invoke the make utility with no arguments. Running
`make DEBUG=1` compiles in debug mode. See the Makefile for more details.
It is important to note that this project adheres to the C99 standard and may
not compile under other C standards. Additionally, `_GNU_SOURCE` is defined in
fileList.c.

## Running

A command line invocation of Far is of the form

`Far KEY ARCHIVE [filename]*`

where KEY indicates the action for Far to execute (described below), `ARCHIVE`
is the name of the archive file, and `[filename]*` is a list of zero or more
files upon which to act.

For example, `./Far r archiveFile fileA fileB directoryC/fileD` is a valid
invocation of Far, where r is the key, archiveFile is the name of the archive,
and fileA, fileB, and directoryC/fileD are the files upon which to act.

### KEY Arguments

#### Add/Replace

The `r` key tells Far to add the specified files to the archive. If a file name
argument is already in the archive, Far replaces the old file with the current
version in the file system. If a file name argument specifies a directory, its
contents are added recursively to the archive. If the archive file does not
exist, Far creates an empty archive with the specified name before acting on the
list of file names.

#### Extract

The `x` key tells Far to extract the specified files from the archive. If a file
from the archive already exists in the file system, Far overwrites the file. If
no file name arguments are passed to Far, the entirety of the archive is
extracted.

#### Delete

The `d` key tells Far to delete each file name argument from the archive. If
a file name argument specifies a directory, the directory and its contents are
deleted.

#### Print

The `t` key tells Far to print to the standard output the name and size of each
file in the archive. File name arguments are ignored.

## Limitations

Far only handles regular files and directories, meaning that soft links,
sockets, FIFOs, and devices are ignored. Hard links to the same inode are
treated as distinct files.