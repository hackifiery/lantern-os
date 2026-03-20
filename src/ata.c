#include "io.h"
#include "ata.h"
#include "sys.h"
typedef unsigned char uc;
/*
Error Register
Bit     Abbreviation    Function
--------------------------------------------
0       AMNF          Address mark not found.
1       TKZNF         Track zero not found.
2       ABRT          Aborted command.
3       MCR               Media change request.
4       IDNF          ID not found.
5       MC                Media changed.
6       UNC               Uncorrectable data error.
7       BBK               Bad Block detected.

Drive / Head Register (I/O base + 6)
Bit     Abbreviation    Function
-------------------------------------------
0 - 3                   In CHS addressing, bits 0 to 3 of the head. In LBA addressing, bits 24 to 27 of the block number.
4       DRV                 Selects the drive number.
5       1                   Always set.
6       LBA                 Uses CHS addressing if clear or LBA addressing if set.
7       1                   Always set.

Status Register (I/O base + 7)
Bit     Abbreviation    Function
------------------------------------------------
0       ERR                 Indicates an error occurred. Send a new command to clear it (or nuke it with a Software Reset).
1       IDX                 Index. Always set to zero.
2       CORR            Corrected data. Always set to zero.
3       DRQ                 Set when the drive has PIO data to transfer, or is ready to accept PIO data.
4       SRV                 Overlapped Mode Service Request.
5       DF                  Drive Fault Error (does not set ERR).
6       RDY                 Bit is clear when drive is spun down, or after an error. Set otherwise.
7       BSY                 Indicates the drive is preparing to send/receive data (wait for it to clear). In case of 'hang' (it never clears), do a software reset.
*/

static int waitDrive(void) {
    // Wait for BSY to clear
    for (int i = 0; i < 10000; i++) {
        uc status = inb(ATA_SR);
        if (!(status & ATA_SR_BSY)) break;
        for (volatile int j = 0; j < 100; j++);
        if (i == 9999) return 0;  // Timeout
    }
    
    // Wait for DRQ to set
    for (int i = 0; i < 10000; i++) {
        uc status = inb(ATA_SR);
        if (status & ATA_SR_DRQ) return 1;
        if (status & ATA_SR_ERR) return 0;  // Error occurred
        for (volatile int j = 0; j < 100; j++);
    }
    return 0;  // Timeout
}

int ataWait(unsigned char mask, unsigned char value, int timeout) {
    for (int i = 0; i < timeout; i++) {
        uc status = inb(ATA_SR);
        if ((status & mask) == value) return 1;
        for (volatile int j = 0; j < 100; j++);
    }
    return 0;
}
int ataIdentify(void) {
    outb(ATA_HEAD, 0xA0);
    //fmtWrite("%x", *((int*)ATA_COMM_IDENT));
    //for(;;);
    outb(ATA_LBA_L, 0);
    outb(ATA_LBA_M, 0);
    outb(ATA_LBA_H, 0);
    waitDrive();
    // unsigned char x = inb(ATA_COMM);
    outb(ATA_COMM, (uc)ATA_COMM_IDENT);
    uc status = inb(ATA_SR);
    {unsigned short _[256]; ataRead(0, _);} // prime the ata

    if (status == 0) {
        fmtWrite("ata: no drive\n");
    }
    if (!ataWait(1 << ATA_SR_BSY, 0, 10000)) {
        fmtWrite("ATA Timeout!\n");
    }
    if (!ataWait(1 << ATA_SR_BSY, 0, 10000)) return 0;
    if (!ataWait(1 << ATA_SR_DRQ, 1 << ATA_SR_DRQ, 10000)) return 0;
}
void ataRead(unsigned int lba, unsigned short *buf) {
    outb(ATA_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_ERR, 0);
    outb(ATA_SEC_COUNT, 1);
    outb(ATA_LBA_L, (uc)lba);
    outb(ATA_LBA_M, (uc)(lba >> 8));
    outb(ATA_LBA_H, (uc)(lba >> 16));
    outb(ATA_COMM, ATA_COMM_READ);
    waitDrive();
    // 256 w (512 b)
    for (unsigned int i = 0; i < 256; i++) buf[i] = inw(ATA_DATA);
}
void ataWrite(unsigned int lba, unsigned short *buf) {
    outb(ATA_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_ERR, 0);
    outb(ATA_SEC_COUNT, 1);
    outb(ATA_LBA_L, (uc)lba);
    outb(ATA_LBA_M, (uc)(lba >> 8));
    outb(ATA_LBA_H, (uc)(lba >> 16));
    outb(ATA_COMM, ATA_COMM_WRITE);
    waitDrive();
    // 256 w (512 b)
    for (unsigned int i = 0; i < 256; i++) outw(ATA_DATA, buf[i]);
    outb(ATA_COMM, ATA_COMM_FLUSH);
    ataWait(1 << ATA_SR_BSY, 0, 10000);
}

unsigned int ataGetSectorCount(void) {
    outb(ATA_HEAD, 0xA0);
    outb(ATA_LBA_L, 0);
    outb(ATA_LBA_M, 0);
    outb(ATA_LBA_H, 0);
    outb(ATA_SEC_COUNT, 0);
    outb(ATA_COMM, ATA_COMM_IDENT);

    uc status = inb(ATA_SR);
    if (status == 0 || status == 0xFF) return 0;  // No drive

    if (!ataWait(ATA_SR_BSY, 0, 10000)) return 0;
    if (!ataWait(ATA_SR_DRQ, ATA_SR_DRQ, 10000)) return 0;

    unsigned short buf[256];
    for (int i = 0; i < 256; i++) buf[i] = inw(ATA_DATA);

    // 48-bit
    if (buf[83] & (1 << 10)) {
        return ((unsigned int)buf[101] << 16) | buf[100];
    } else {
        // 28-bit
        return ((unsigned int)buf[61] << 16) | buf[60];
    }
}