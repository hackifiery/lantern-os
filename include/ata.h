#ifndef ATA_H
#define ATA_H

int ataInit(void);
int ataReadSectors(unsigned int lba, unsigned char count, void* buffer);
int ataWriteSectors(unsigned int lba, unsigned char count, const void* buffer);

#endif