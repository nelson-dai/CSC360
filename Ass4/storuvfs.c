#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "disk.h"
#include "uvfsutils.h"


/*
 * Based on http://bit.ly/2vniWNb
 */
void pack_current_datetime(unsigned char *entry) {
    assert(entry);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    unsigned short year   = tm.tm_year + 1900;
    unsigned char  month  = (unsigned char)(tm.tm_mon + 1);
    unsigned char  day    = (unsigned char)(tm.tm_mday);
    unsigned char  hour   = (unsigned char)(tm.tm_hour);
    unsigned char  minute = (unsigned char)(tm.tm_min);
    unsigned char  second = (unsigned char)(tm.tm_sec);

    year = htons(year);

    memcpy(entry, &year, 2);
    entry[2] = month;
    entry[3] = day;
    entry[4] = hour;
    entry[5] = minute;
    entry[6] = second; 
}


int next_free_block(int *FAT, int max_blocks) {
    assert(FAT != NULL);

    int i;

    for (i = 0; i < max_blocks; i++) {
        if (FAT[i] == FAT_AVAILABLE) {
            return i;
        }
    }

    return -1;
}


int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i;
    char *imagename  = NULL;
    char *filename   = NULL;
    char *sourcename = NULL;
    FILE *f;
	int * fat;
	struct stat file_stat;
	off_t filesize;
	int freeSpace = 0;
	int currentBlock = -1;
	directory_entry_t dir;
	int dirIndex = -1;
	FILE* src;
	char * buffer;
	int numEntries;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--file") == 0 && i+1 < argc) {
            filename = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--source") == 0 && i+1 < argc) {
            sourcename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL || filename == NULL || sourcename == NULL) {
        fprintf(stderr, "usage: storuvfs --image <imagename> " \
            "--file <filename in image> " \
            "--source <filename on host>\n");
        exit(1);
    }
	
	// Open the image file for reading.
	f = fopen(imagename, "r+");
	if (f == NULL) {
		fprintf(stderr, "Disk image %s could not be opened.\n", imagename);
		exit(1);
	}
	
	// Load the superblock and FAT.
	loadSuperblock(&sb, f);
	fat = loadFAT(f, &sb);
	
	// Get the size of the file.
	stat(sourcename, &file_stat);
	filesize = file_stat.st_size;
	
	// Get info from the FAT: how much free space and the first free block index
	for (i = 0; i < sb.num_blocks; i++) {
		if (fat[i] == FAT_AVAILABLE) {
			if (currentBlock < 0) currentBlock = i;
			freeSpace += sb.block_size;
		}
	}
	
	// Print error if not enough space.
	if (freeSpace < filesize) {
		fprintf(stderr, "Not enough space: need %jd bytes, %d available.\n", (intmax_t)filesize, freeSpace);
		
		// Release FAT array.
		freeFAT(fat);
		
		exit(2);
	}
	
	// Check directory for existing filename and also find next available directory entry.
	// Loop through directory entries, print result.
	numEntries = (sb.dir_blocks * sb.block_size) / sizeof(directory_entry_t);
	for (i = 0; i < numEntries; i++) {
		loadRootDirEntry(f, &sb, i, &dir);
		if ( (dir.status & DIR_ENTRY_NORMALFILE) == DIR_ENTRY_NORMALFILE) {
			// entry is valid. Get the filename to see if it conflicts.
			if (strcmp(dir.filename, filename) == 0) {
				// Conflict. Print error and exit.
				//fprintf(stderr, "File %s already exists!\n", filename);
				fprintf(stderr,"file already exists in the image\n");
				freeFAT(fat);
				exit(3);
			}
		} else if ( (dir.status & DIR_ENTRY_DIRECTORY) != DIR_ENTRY_DIRECTORY && (dirIndex < 0) ) {
			// Free directory entry
			dirIndex = i;
		}
	}
	
	// Make sure it's' a valid directory entry
	if (dirIndex < 0) {
		// Print error and exit.
		printf("Directory is full.\n");
		freeFAT(fat);
		exit(4);
	}
	
	// Start writing the file. Open the image file for reading.
	src = fopen(sourcename, "r");
	if (src == NULL) {
		fprintf(stderr, "Source file %s could not be opened.\n", sourcename);
		freeFAT(fat);
		exit(5);
	}
	
	// Write the directory entry.
	dir.status = DIR_ENTRY_NORMALFILE;
	dir.start_block = currentBlock;
	dir.num_blocks = filesize / sb.block_size;
	if (filesize % sb.block_size > 0) dir.num_blocks++;
	dir.file_size = filesize;
	pack_current_datetime(dir.create_time);
	pack_current_datetime(dir.modify_time);
	strncpy(dir.filename, filename, DIR_FILENAME_MAX);
	
	writeRootDirEntry(f, &sb, dirIndex, &dir);
	
	// Read the file, one block at a time. For each block, find the index of the
	// next free block using the FAT, set the current FAT entry to that block, and
	// write the block with the file's data.
	buffer = (char*) malloc(sb.block_size);
	while (filesize > 0) {
		int nextBlock;
		size_t bytesRead = fread(buffer, 1, sb.block_size, src);
		
		fseek(f, currentBlock * sb.block_size, SEEK_SET);
		fwrite(buffer, 1, bytesRead, f);
		
		// Set the FAT entry, have to find a new free block if there's
		// more file data, or if this was the last block, then set to FAT_LASTBLOCK.
		filesize -= bytesRead;
		fat[currentBlock] = FAT_LASTBLOCK;
		if (filesize > 0) {
			nextBlock = next_free_block(fat, sb.num_blocks);
			fat[currentBlock] = nextBlock;
			currentBlock = nextBlock;
		}
	}
	
	// Close the source file on the host.
	fclose(src);
	
	// Free data buffer.
	free(buffer);
	
	// Now sync up the FAT.
	writeFAT(f, &sb, fat);

	// Release FAT array.
	freeFAT(fat);
	
	// Close the disk image file.
	fclose(f);
	
    return 0; 
}
