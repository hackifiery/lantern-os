#ifndef ATA_H
#define ATA_H

int ataIdentify(void);
void ataRead(unsigned int lba, unsigned short *buf);
void ataWrite(unsigned int lba, unsigned short *buf);

#endif