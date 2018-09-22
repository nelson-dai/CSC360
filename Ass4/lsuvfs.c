#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"
#include "uvfsutils.h"


char *month_to_string(short m) {
    switch(m) {
    case 1: return "Jan";
    case 2: return "Feb";
    case 3: return "Mar";
    case 4: return "Apr";
    case 5: return "May";
    case 6: return "Jun";
    case 7: return "Jul";
    case 8: return "Aug";
    case 9: return "Sep";
    case 10: return "Oct";
    case 11: return "Nov";
    case 12: return "Dec";
    default: return "?!?";
    }
}


void unpack_datetime(unsigned char *time, short *year, short *month, 
    short *day, short *hour, short *minute, short *second)
{
    assert(time != NULL);

    memcpy(year, time, 2);
    *year = htons(*year);

    *month = (unsigned short)(time[2]);
    *day = (unsigned short)(time[3]);
    *hour = (unsigned short)(time[4]);
    *minute = (unsigned short)(time[5]);
    *second = (unsigned short)(time[6]);
}


int main(int argc, char *argv[]) {
    superblock_entry_t sb;
	directory_entry_t dir;
    int  i;
	int  numEntries;
    char *imagename = NULL;
    FILE *f;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL)
    {
        fprintf(stderr, "usage: lsuvfs --image <imagename>\n");
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
	
	// Loop through directory entries, printing results.
	numEntries = (sb.dir_blocks * sb.block_size) / sizeof(directory_entry_t);
	for (i = 0; i < numEntries; i++) {
		loadRootDirEntry(f, &sb, i, &dir);
		if ( (dir.status & 0x1) == 1) {
			// entry is valid. Get the information to print.
			short year, month, day, hour, minute, second;
			unpack_datetime(dir.modify_time, &year, &month, &day, &hour, &minute, &second);
			printf("%8d %4d-%3s-%02d %02d:%02d:%02d %s\n", dir.file_size, year, month_to_string(month), day, hour, minute, second, dir.filename);
		}
	}
	
	// File image is done, we can close it now.
	fclose(f);

    return 0; 
}
