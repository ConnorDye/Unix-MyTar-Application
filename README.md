# Unix MyTar Application

This application is a version of the standard utility tar(1). The purpose of this program is to build and restore archives in a way that is interoperable with
GNU tar.

Created by Connor Dye as a California Polytechnic University Project.

## Features
- Usage: mytar [ctxvS]f tarfile [ path [...] ]
- Basic commands: 1.) c - creates an archive 2.) t - prints the table of contents for an archive 3.) x - extracts the contents of an archive 4.) x - increases verbosity 5.) f - specifies archive filename 6.) S - strict about standards compliance with GNU tar
- createArchive.c holds the functionalitily to create a tar header and tar file. Functionality is as follows: 1.) arguments on the command line are taken as paths to be added to the archive 2.) if a given path is a directory, that directory and all the files and directories below it are added to the archive 3.) header format is in GNU tar format so that it is interoperable 4.) If the verbose (v) option is set, mytar lists files as they are added, one per line.
- extractArchive.c holds the functionality to extract files from a new archive based on the header. Functionality is as follows: 1.) If no names are given on the command mytar will extract all files in the archive. If a name or names are given on the command line, mytar will extract the path and its descendents. 2.) Restores the modification time of the extracted files 3.) If the verbose (v) option is set, mytar lists files as they are extracted






