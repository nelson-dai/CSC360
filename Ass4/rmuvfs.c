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
	int * fat;
	int dirIndex;

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
        fprintf(stderr, "usage: rmuvfs --image <imagename> " \
            "--file <filename in image>");
        exit(1);
    }
	
	// Open the image file for reading.
	f = fopen(imagename, "r+");
	if (f == NULL) {
		fprintf(stderr, "Disk image %s could not be opened.", imagename);
		exit(1);
	}
	
	// Load the superblock.
	loadSuperblock(&sb, f);
	fat = loadFAT(f, &sb);
	
	// Loop through directory entries, looking for the target.
	numEntries = (sb.dir_blocks * sb.block_size) / sizeof(directory_entry_t);
	for (dirIndex = 0; dirIndex < numEntries; dirIndex++) {
		loadRootDirEntry(f, &sb, dirIndex, &dir);
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
		int currentBlock = dir.start_block;
		
		// Loop through the FAT clearing out entries associated with this file.
		do {
			int nextBlock = fat[currentBlock];
			fat[currentBlock] = FAT_AVAILABLE;
			currentBlock = nextBlock;
		} while (currentBlock != FAT_LASTBLOCK);
		
		// Write the FAT back to the disk image.
		writeFAT(f, &sb, fat);
		
		// Now clear out the directory entry. All we have to do is set
		// the status to DIR_ENTRY_AVAILABLE.
		dir.status = DIR_ENTRY_AVAILABLE;
		writeRootDirEntry(f, &sb, dirIndex, &dir);

	} else {
		printf("File not found: %s\n", filename);
	}
	
	// Free the FAT array in memory.
	freeFAT(fat);
	
	// File image is done, we can close it now.
	fclose(f);
	
	
    return 0; 
}
