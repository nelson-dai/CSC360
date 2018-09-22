#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "uvfsutils.h"
#include "disk.h"

void loadSuperblock(superblock_entry_t * sb, FILE * f) {
	// Read the superblock.
	fseek(f, 0, SEEK_SET);
	fread(sb, sizeof(superblock_entry_t), 1, f);
	
	// Fix the integer byte orders.
	sb->block_size = ntohs(sb->block_size);
	sb->num_blocks = ntohl(sb->num_blocks);
	sb->fat_start = ntohl(sb->fat_start);
	sb->fat_blocks = ntohl(sb->fat_blocks);
	sb->dir_start = ntohl(sb->dir_start);
	sb->dir_blocks = ntohl(sb->dir_blocks);
}

void loadFATEntry(FILE * f, superblock_entry_t * sb, int index, int * entry) {
	fseek(f, sb->fat_start * sb->block_size + index * sizeof(int), SEEK_SET);
	fread(entry, sizeof(int), 1, f);
	*entry = ntohl(*entry);
}

void writeFAT(FILE * f, superblock_entry_t * sb, int * fat) {
	int i;
	fseek(f, sb->fat_start * sb->block_size, SEEK_SET);
	for (i = 0 ; i < sb->num_blocks; i++) {
		int value = htonl(fat[i]);
		fwrite(&value, sizeof(int), 1, f);
	}
}


int * loadFAT(FILE * f, superblock_entry_t * sb) {
	int i;
	
	// Allocate space for the FAT array.
	int * fat = (int*) malloc(sb->num_blocks*sizeof(int));
	
	// Move the file pointer to the first byte of the FAT and read the entire FAT
	// into the array.
	fseek(f, sb->fat_start * sb->block_size, SEEK_SET);
	
	// Read the FAT into the array.
	fread(fat, sizeof(int), sb->num_blocks, f);
	
	
	// Walk through the array converting each value to host format.
	for (i = 0; i < sb->num_blocks; i++) {
		
		// Read the entry and convert to host format.
		fat[i] = ntohl(fat[i]);
	}
	
	return fat;
	
}

void freeFAT(int * fat) {
	free(fat);
}

void loadRootDirEntry(FILE * f, superblock_entry_t * sb, int index, directory_entry_t * entry) {
	fseek(f, sb->dir_start * sb->block_size + index * sizeof(directory_entry_t), SEEK_SET);
	fread(entry, sizeof(directory_entry_t), 1, f);
	entry->start_block = ntohl(entry->start_block);
	entry->num_blocks = ntohl(entry->num_blocks);
	entry->file_size = ntohl(entry->file_size);
}

void writeRootDirEntry(FILE * f, superblock_entry_t *sb, int index, directory_entry_t * entry) {
	fseek(f, sb->dir_start * sb->block_size + index * sizeof(directory_entry_t), SEEK_SET);
	entry->start_block = htonl(entry->start_block);
	entry->num_blocks = htonl(entry->num_blocks);
	entry->file_size = htonl(entry->file_size);
	fwrite(entry, sizeof(directory_entry_t), 1, f);
}



void printDataBlock(FILE * f, superblock_entry_t * sb, int block, int bytes_remaining) {
	char data;
	fseek(f, block * sb->block_size, SEEK_SET);
	if (bytes_remaining > sb->block_size) bytes_remaining = sb->block_size;
	while (bytes_remaining > 0) {
		fread(&data, sizeof(char), 1, f);
		printf("%c", data);
		bytes_remaining--;
	}
}