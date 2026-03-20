#ifndef ATA_H
#define ATA_H

#define ATA_PRIM_OFFSET 0x1F0
#define ATA_DATA        (ATA_PRIM_OFFSET)
#define ATA_ERR         (ATA_PRIM_OFFSET + 1)
#define ATA_SEC_COUNT   (ATA_PRIM_OFFSET + 2)
#define ATA_LBA_L       (ATA_PRIM_OFFSET + 3)
#define ATA_LBA_M       (ATA_PRIM_OFFSET + 4)
#define ATA_LBA_H       (ATA_PRIM_OFFSET + 5)
#define ATA_HEAD        (ATA_PRIM_OFFSET + 6)
#define ATA_COMM        (ATA_PRIM_OFFSET + 7)
#define ATA_SR          (ATA_PRIM_OFFSET + 7)
#define ATA_SR_ERR      (1 << 0)
#define ATA_SR_IDX      (1 << 1)
#define ATA_SR_CORR     (1 << 2)
#define ATA_SR_DRQ      (1 << 3)
#define ATA_SR_SRV      (1 << 4)
#define ATA_SR_DF       (1 << 5)
#define ATA_SR_RDY      (1 << 6)
#define ATA_SR_BSY      (1 << 7)
#define ATA_COMM_IDENT  0xEC
#define ATA_COMM_READ   0x20
#define ATA_COMM_WRITE  0x30
#define ATA_COMM_FLUSH  0xE7

int ataIdentify(void);
void ataRead(unsigned int lba, unsigned short *buf);
void ataWrite(unsigned int lba, unsigned short *buf);
unsigned int ataGetSectorCount(void);
int ataWait(unsigned char mask, unsigned char value, int timeout);

#endif