#ifndef __UVFSUTILS_H__
#define __UVFSUTILS_H__

#include "disk.h"

void loadSuperblock(superblock_entry_t * sb, FILE * f);

void loadFATEntry(FILE * f, superblock_entry_t * sb, int index, int * entry);
	
int * loadFAT(FILE * f, superblock_entry_t * sb);

void writeFAT(FILE * f, superblock_entry_t * sb, int * fat);

void freeFAT(int * fat);

void loadRootDirEntry(FILE * f, superblock_entry_t *sb, int index, directory_entry_t * entry);

void writeRootDirEntry(FILE * f, superblock_entry_t *sb, int index, directory_entry_t * entry);

void printDataBlock(FILE * f, superblock_entry_t * sb, int block, int bytes_remaining);

#endif