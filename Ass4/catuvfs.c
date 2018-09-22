#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"
#include "uvfsutils.h"

int main(int argc, char *argv[]) {
    superblock_entry_t sb;
	directory_entry_t dir;
    int  i;
    char *imagename = NULL;
    char *filename  = NULL;
	int numEntries;
    FILE *f;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--file") == 0 && i+1 < argc) {
            filename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL || filename == NULL) {
        fprintf(stderr, "usage: catuvfs --image <imagename> " \
            "--file <filename in image>\n");
        exit(1);
    }
	
	// Open the image file for reading.
	f = fopen(imagename, "r");
	if (f == NULL) {
		fprintf(stderr, "Disk image %s could not be opened.\n", imagename);
		exit(1);
	}
	
	// Load the superblock.
	loadSuperblock(&sb, f);
	
	// Loop through directory entries, looking for the target.
	numEntries = (sb.dir_blocks * sb.block_size) / sizeof(directory_entry_t);
	for (i = 0; i < numEntries; i++) {
		loadRootDirEntry(f, &sb, i, &dir);
		if ( (dir.status & DIR_ENTRY_NORMALFILE) == DIR_ENTRY_NORMALFILE) {
			// entry is valid. Check the filename.
			if (strcmp(dir.filename, filename) == 0) {
				break;
			} else {
				dir.status = 0;
			}
		}
	}
	
	// After the loop, if the current dir has a valid status, then the file was found.
	if ( (dir.status & DIR_ENTRY_NORMALFILE) == DIR_ENTRY_NORMALFILE) {
		int block = dir.start_block;
		int fat_data = 0;
		int bytes_remaining = dir.file_size;

		while (bytes_remaining > 0) {
			printDataBlock(f, &sb, block, bytes_remaining);
			bytes_remaining -= sb.block_size;
			loadFATEntry(f, &sb, block, &fat_data);
			block = fat_data;
		}
	} else {
		printf("File not found: %s\n", filename);
	}
	
	// File image is done, we can close it now.
	fclose(f);
	
	
    return 0; 
}
