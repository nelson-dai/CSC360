#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"
#include "uvfsutils.h"

int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i;
    char *imagename = NULL;
    FILE  *f;
    int   *fat_data;
	int   fat_free=0, fat_alloc=0, fat_resv=0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL)
    {
        fprintf(stderr, "usage: statuvfs --image <imagename>\n");
        exit(1);
    }

	// Open the image file for reading.
	f = fopen(imagename, "r");
	if (f == NULL) {
		fprintf(stderr, "Disk image %s could not be opened.", imagename);
		exit(1);
	}
	
	// Load the superblock and FAT.
	loadSuperblock(&sb, f);
	fat_data = loadFAT(f, &sb);
	
	// Done with the image, so we can close it now.
	fclose(f);
	
	// Get FAT summary data.
	for (i = 0; i < sb.num_blocks; i++) {
		if (fat_data[i] == 0) fat_free++;
		else if (fat_data[i] == 1) fat_resv++;
		else fat_alloc++;
	}
	
	// Clean memory.
	freeFAT(fat_data);
	
	// Output
	printf("%s (%s)\n\n", sb.magic, imagename);
	printf("-------------------------------------------------\n");
	printf("  Bsz   Bcnt   FATst FATcnt  DIRst DIRcnt\n");
	printf("%5d%7d%8d%7d%7d%7d\n\n", sb.block_size, sb.num_blocks, sb.fat_start, sb.fat_blocks, sb.dir_start, sb.dir_blocks);
	printf("-------------------------------------------------\n");
	printf(" Free   Resv  Alloc\n");
	printf("%5d%7d%7d\n", fat_free, fat_resv, fat_alloc);
	
    return 0; 
}
