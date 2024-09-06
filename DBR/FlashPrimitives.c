//-----------------------------------------------------------------------------
// F120_FlashPrimitives.c
//-----------------------------------------------------------------------------
// Copyright 2004 Silicon Laboratories, Inc.
//
// This program contains several useful utilities for writing and updating
// FLASH memory.
//
// AUTH: BW & GP
// DATE: 21 JUL 04
//
// Target: C8051F12x
// Tool chain: KEIL C51 7.06
//
// Release 1.1
// -Upgrading release version due to change in FlashPrimitives.h
// -07 FEB 2006 (GP)
//
// Release 1.0
//

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "FlashPrimitives.h"
#include <c8051F120.h>

//-----------------------------------------------------------------------------
// Structures, Unions, Enumerations, and Type Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

// FLASH read/write/erase routines
void FLASH_ByteWrite (FLADDR addr, char byte, bit SFLE);
unsigned char FLASH_ByteRead (FLADDR addr, bit SFLE);
void FLASH_PageErase (FLADDR addr, bit SFLE);
void FLASH_Write (FLADDR dest, char *src, unsigned numbytes, bit SFLE);
void FLASH_Write_UI (FLADDR dest, unsigned int *src, unsigned numbytes, bit SFLE);
char * FLASH_Read (char *dest, FLADDR src, unsigned numbytes, bit SFLE);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FLASH Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FLASH_ByteWrite
//-----------------------------------------------------------------------------
//
// This routine writes <byte> to the linear FLASH address <addr>.
// Linear map is decoded as follows:
// Linear Address       Bank     Bank Address
// ------------------------------------------------
// 0x00000 - 0x07FFF    0        0x0000 - 0x7FFF
// 0x08000 - 0x0FFFF    1        0x8000 - 0xFFFF
// 0x10000 - 0x17FFF    2        0x8000 - 0xFFFF
// 0x18000 - 0x1FFFF    3        0x8000 - 0xFFFF
//
void FLASH_ByteWrite (FLADDR addr, char byte, bit SFLE)
{
   char SFRPAGE_SAVE = SFRPAGE;        // preserve SFRPAGE
   bit EA_SAVE = EA;                   // preserve EA
   char PSBANK_SAVE = PSBANK;          // preserve PSBANK
   char xdata * data pwrite;           // FLASH write pointer

   EA = 0;                             // disable interrupts

   SFRPAGE = LEGACY_PAGE;

   if (addr < 0x10000) {               // 64K linear address
      pwrite = (char xdata *) addr;
   } else if (addr < 0x18000) {        // BANK 2
      addr |= 0x8000;
      pwrite = (char xdata *) addr;
      PSBANK &= ~0x30;                 // COBANK = 0x2
      PSBANK |=  0x20;
   } else {                            // BANK 3
      pwrite = (char xdata *) addr;
      PSBANK &= ~0x30;                 // COBANK = 0x3
      PSBANK |=  0x30;
   }

   FLSCL |= 0x01;                      // enable FLASH writes/erases
   PSCTL |= 0x01;                      // PSWE = 1

   if (SFLE) {
      PSCTL |= 0x04;                   // set SFLE
   }

   RSTSRC = 0x02;                      // enable VDDMON as reset source
   *pwrite = byte;                     // write the byte

   if (SFLE) {
      PSCTL &= ~0x04;                  // clear SFLE
   }
   PSCTL &= ~0x01;                     // PSWE = 0
   FLSCL &= ~0x01;                     // disable FLASH writes/erases

   PSBANK = PSBANK_SAVE;               // restore PSBANK
   SFRPAGE = SFRPAGE_SAVE;             // restore SFRPAGE
   EA = EA_SAVE;                       // restore interrupts
}

//-----------------------------------------------------------------------------
// FLASH_ByteRead
//-----------------------------------------------------------------------------
//
// This routine reads a <byte> from the linear FLASH address <addr>.
//
unsigned char FLASH_ByteRead (FLADDR addr, bit SFLE)
{
   char SFRPAGE_SAVE = SFRPAGE;        // preserve SFRPAGE
   bit EA_SAVE = EA;                   // preserve EA
   char PSBANK_SAVE = PSBANK;          // preserve PSBANK
   char code * data pread;             // FLASH read pointer
   unsigned char byte;

   EA = 0;                             // disable interrupts

   SFRPAGE = LEGACY_PAGE;

   if (addr < 0x10000) {               // 64K linear address
      pread = (char code *) addr;
   } else if (addr < 0x18000) {        // BANK 2
      addr |= 0x8000;
      pread = (char code *) addr;
      PSBANK &= ~0x30;                 // COBANK = 0x2
      PSBANK |=  0x20;
   } else {                            // BANK 3
      pread = (char code *) addr;
      PSBANK &= ~0x30;                 // COBANK = 0x3
      PSBANK |=  0x30;
   }

   if (SFLE) {
      PSCTL |= 0x04;                   // set SFLE
   }

   byte = *pread;                      // read the byte

   if (SFLE) {
      PSCTL &= ~0x04;                  // clear SFLE
   }

   PSBANK = PSBANK_SAVE;               // restore PSBANK
   SFRPAGE = SFRPAGE_SAVE;             // restore SFRPAGE
   EA = EA_SAVE;                       // restore interrupts

   return byte;
}

//-----------------------------------------------------------------------------
// FLASH_PageErase
//-----------------------------------------------------------------------------
//
// This routine erases the FLASH page containing the linear FLASH address
// <addr>.
//
void FLASH_PageErase (FLADDR addr, bit SFLE)
{
   char SFRPAGE_SAVE = SFRPAGE;        // preserve SFRPAGE
   bit EA_SAVE = EA;                   // preserve EA
   char PSBANK_SAVE = PSBANK;          // preserve PSBANK
   char xdata * data pwrite;           // FLASH write pointer

   EA = 0;                             // disable interrupts

   SFRPAGE = LEGACY_PAGE;

   if (addr < 0x10000) {               // 64K linear address
      pwrite = (char xdata *) addr;
   } else if (addr < 0x18000) {        // BANK 2
      addr |= 0x8000;
      pwrite = (char xdata *) addr;
      PSBANK &= ~0x30;                 // COBANK = 0x2
      PSBANK |=  0x20;
   } else {                            // BANK 3
      pwrite = (char xdata *) addr;
      PSBANK &= ~0x30;                 // COBANK = 0x3
      PSBANK |=  0x30;
   }

   FLSCL |= 0x01;                      // enable FLASH writes/erases
   PSCTL |= 0x03;                      // PSWE = 1; PSEE = 1

   if (SFLE) {
      PSCTL |= 0x04;                   // set SFLE
   }

   RSTSRC = 0x02;                      // enable VDDMON as reset source
   *pwrite = 0;                        // initiate page erase

   if (SFLE) {
      PSCTL &= ~0x04;                  // clear SFLE
   }

   PSCTL &= ~0x03;                     // PSWE = 0; PSEE = 0
   FLSCL &= ~0x01;                     // disable FLASH writes/erases

   PSBANK = PSBANK_SAVE;               // restore PSBANK
   SFRPAGE = SFRPAGE_SAVE;             // restore SFRPAGE
   EA = EA_SAVE;                       // restore interrupts
}

//-----------------------------------------------------------------------------
// FLASH_Write
//-----------------------------------------------------------------------------
//
// This routine copies <numbytes> from <src> to the linear FLASH address
// <dest>.
//
/*
void FLASH_Write (FLADDR dest, char *src, unsigned numbytes, bit SFLE)
{
   FLADDR i;

   for (i = dest; i < dest+numbytes; i++) {
      FLASH_ByteWrite (i, *src++, SFLE);
   }
}
*/
//-----------------------------------------------------------------------------
// FLASH_Write_UI
//-----------------------------------------------------------------------------

void FLASH_Write_UI (FLADDR dest, unsigned int *src, unsigned numbytes, bit SFLE)
{
   FLADDR i;
   unsigned char *ptr;

   ptr=((char *)src);
   for (i = dest; i < dest+numbytes; i++)
   {
      FLASH_ByteWrite (i, *ptr++, SFLE);
   }
}

//-----------------------------------------------------------------------------
// FLASH_Read
//-----------------------------------------------------------------------------
//
// This routine copies <numbytes> from the linear FLASH address <src> to
// <dest>.
//
char * FLASH_Read (char *dest, FLADDR src, unsigned numbytes, bit SFLE)
{
   FLADDR i;

   for (i = 0; i < numbytes; i++) {
      *dest++ = FLASH_ByteRead (src+i, SFLE);
   }
   return dest;
}
