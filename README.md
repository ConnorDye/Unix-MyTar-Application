# Unix MyTar Application

This application is a version of the standard utility tar(1). The purpose of this program is to build and restore archives in a way that is interoperable with
GNU tar.

Created by Connor Dye as a California Polytechnic University Project.

## Features
- Usage: mytar [ctxvS]f tarfile [ path [...] ]
- Basic commands: 1.) c - creates an archive 2.) t - prints the table of contents for an archive 3.) x - extracts the contents of an archive 4.) x - increases verbosity 5.) f - specifies archive filename 6.) S - strict about standards compliance with GNU tar
- createArchive.c holds the functionalitily to create a tar header and tar file
- hash.c and hash.h is a custom hash data structure which includes a hash item struct to represent each word and its count, and a hash table where each word can be  mapped and quickly accessed
- uniq.c contains the parsing and main functionality
- Accepts an optional argument -n <number of words> which allows the user to specify number of words to display
- Utilizes getopt() to parse command line arguments
- Words are displayed with their count followed by the word in all lowercase
- Includes appropriate error checking for files that cannot be opened, invalid command line arguments, etc.
- Includes proper C memory management (malloc() to designate memory and free() for memory which is no longer being used)




