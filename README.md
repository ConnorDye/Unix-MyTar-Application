# Unix MyTar Application

This application is a version of the standard utility tar(1). The purpose of this program is to build and restore archives in a way that is interoperable with
GNU tar.

Created by Connor Dye as a California Polytechnic University Project.

## Features
- Usage: mytar [ctxvS]f tarfile [ path [...] ]
- Basic commands: 1.) c - creates an archive 2.) t - prints the table of contents for an archive 3.) x - extracts the contents of an archive 4.) x - increases verbosity 5.) f - specifies archive filename 6.) S - strict about standards compliance with GNU tar
- createArchive.c holds the functionalitily to create a tar header and tar file. Functionality is as follows: 1.) arguments on the command line are taken as paths to be added to the archive 2.) if a given path is a directory, that directory and all the files and directories below it are added to the archive 3.) header format is in GNU tar format so that it is interoperable 4.) If the verbose (v) option is set, mytar lists files as they are added, one per line.
- extractArchive.c holds the functionality to extract files from a new archive based on the header. Functionality is as follows: 1.) If no names are given on the command mytar will extract all files in the archive. If a name or names are given on the command line, mytar will extract the path and its descendents. 2.) Restores the modification time of the extracted files 3.) If the verbose (v) option is set, mytar lists files as they are extracted
- listarch.c holds the functionality to list (Table of contents) the contents of the given archive file, in order, one per line. Functionality is as follows: 1.) If no names are given on the command line, mytar, lists all the files in the archive 2.)  If a name or names are given on the command line, mytar will list the path and its descendents 3.) If the verbose (’v’) option is set, mytar gives expanded information about each file as it lists (e.g $mytar tvf archive.tar would yield something like drwx------ user/user 0 2010-11-02 13:49 Testdir/
- convertOctal.c - holds functionality for inserting and removing binary integers from non-conforming headers.
- mytar.c handles the parsing of command line usage using getopt()
- Error handling: 1.) stops at the first corrupt record, skips unreadable files 2.) displays error message on bad header 3.) reports other errors with error messages and continues if possible 4.) max path of 256 characters 5.) Names can only be broken on a ’/’. If a name can not be partitioned error is printed


##Notes
-The elements of each listing are the permissions, the owner/group, the size, the last modified date (mtime) and the filename.
- File types supported: Regular files (Regular and alternate markings), directories, symbolic links
- Mytar implements the POSIX-specified USTAR archive format available at http://www.unix.org/single unix specification/1
- A USTAR archive is a list of records each of which consists of a header block followed by the file as data blocks. The end of the file is marked by two blocks of all zero bytes (end of archive marker). All blocks are 512 bytes and filled with zero bytes if they are not completely filled with data


 













