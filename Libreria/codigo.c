#ifndef FMU_MCX153_LIB_H
#define FMU_MCX153_LIB_H

#include <stdint.h>

/********************************************************************
 * 1. Declaración de registros FMU como uniones con campos de bits
 ********************************************************************/

/**
 * \typedef __FMU_FSTAT_t
 * Flash Status Register
 */
typedef union {
    uint32_t WORD;
    struct {
        uint32_t FAIL:1;          // Bit 0: Command Fail Flag
        uint32_t RESERVED0:1;     // Bit 1: Reserved
        uint32_t CMDABT:1;        // Bit 2: Command Abort Flag
        uint32_t RESERVED1:1;     // Bit 3: Reserved
        uint32_t PVIOL:1;         // Bit 4: Command Protection Violation Flag
        uint32_t ACCERR:1;        // Bit 5: Command Access Error Flag
        uint32_t CWSABT:1;        // Bit 6: Command Write Sequence Abort Flag
        uint32_t CCIF:1;          // Bit 7: Command Complete Interrupt Flag
        uint32_t CMDPRT:2;        // Bits 8-9: Command Protection Level
        uint32_t CMDP:1;          // Bit 10: Command Protection Status Flag
        uint32_t RESERVED2:1;     // Bit 11: Reserved
        uint32_t CMDDID:4;        // Bits 12-15: Command Domain ID
        uint32_t DFDIF:1;         // Bit 16: Double Bit Fault Detect Interrupt Flag
        uint32_t SALV_USED:1;     // Bit 17: Salvage Used for Erase Operation
        uint32_t RESERVED3:13;    // Bits 18-30: Reserved
        uint32_t PERDY:1;         // Bit 31: Program-Erase Ready Control/Status Flag
    } BITS;
} __FMU_FSTAT_t;

/* Máscaras para FSTAT */
#define mFMU_FSTAT_FAIL        0x00000001UL
#define mFMU_FSTAT_CMDABT      0x00000004UL
#define mFMU_FSTAT_PVIOL       0x00000010UL
#define mFMU_FSTAT_ACCERR      0x00000020UL
#define mFMU_FSTAT_CWSABT      0x00000040UL
#define mFMU_FSTAT_CCIF        0x00000080UL
#define mFMU_FSTAT_CMDPRT      0x00000300UL
#define mFMU_FSTAT_CMDP        0x00000400UL
#define mFMU_FSTAT_CMDDID      0x0000F000UL
#define mFMU_FSTAT_DFDIF       0x00010000UL
#define mFMU_FSTAT_SALV_USED   0x00020000UL
#define mFMU_FSTAT_PERDY       0x80000000UL

/* Constantes para CMDPRT */
#define kFMU_FSTAT_CMDPRT_SecureNormal       0x0
#define kFMU_FSTAT_CMDPRT_SecurePrivileged   0x1
#define kFMU_FSTAT_CMDPRT_NonsecureNormal    0x2
#define kFMU_FSTAT_CMDPRT_NonsecurePrivileged 0x3

/**
 * \typedef __FMU_FCNFG_t
 * Flash Configuration Register
 */
typedef union {
    uint32_t WORD;
    struct {
        uint32_t RESERVED0:7;     // Bits 0-6: Reserved
        uint32_t CCIE:1;          // Bit 7: Command Complete Interrupt Enable
        uint32_t ERSREQ:1;        // Bit 8: Mass Erase Request (Read Only)
        uint32_t RESERVED1:7;     // Bits 9-15: Reserved
        uint32_t DFDIE:1;         // Bit 16: Double Bit Fault Detect Interrupt Enable
        uint32_t RESERVED2:7;     // Bits 17-23: Reserved
        uint32_t ERSIEN0:4;       // Bits 24-27: Erase IFR Sector Enable Block 0
        uint32_t ERSIEN1:4;       // Bits 28-31: Erase IFR Sector Enable Block 1
    } BITS;
} __FMU_FCNFG_t;

/* Máscaras para FCNFG */
#define mFMU_FCNFG_CCIE      0x00000080UL
#define mFMU_FCNFG_ERSREQ    0x00000100UL
#define mFMU_FCNFG_DFDIE     0x00010000UL
#define mFMU_FCNFG_ERSIEN0   0x0F000000UL
#define mFMU_FCNFG_ERSIEN1   0xF0000000UL

/**
 * \typedef __FMU_FCTRL_t
 * Flash Control Register
 */
typedef union {
    uint32_t WORD;
    struct {
        uint32_t RWSC:4;         // Bits 0-3: Read Wait-State Control
        uint32_t RESERVED0:4;    // Bits 4-7: Reserved
        uint32_t LSACTIVE:1;     // Bit 8: Low speed active mode
        uint32_t RESERVED1:7;    // Bits 9-15: Reserved
        uint32_t FDFD:1;         // Bit 16: Force Double Bit Fault Detect
        uint32_t RESERVED2:7;    // Bits 17-23: Reserved
        uint32_t ABTREQ:1;       // Bit 24: Abort Request
        uint32_t RESERVED3:7;    // Bits 25-31: Reserved
    } BITS;
} __FMU_FCTRL_t;

/* Máscaras para FCTRL */
#define mFMU_FCTRL_RWSC       0x0000000FUL
#define mFMU_FCTRL_LSACTIVE   0x00000100UL
#define mFMU_FCTRL_FDFD       0x00010000UL
#define mFMU_FCTRL_ABTREQ     0x01000000UL

/**
 * \typedef __FMU_FCCOB_t
 * Flash Common Command Object Registers (FCCOB0 - FCCOB7)
 */
typedef union {
    uint32_t WORD;
    struct {
        uint32_t CCOBn:32;       // Bits 0-31: Command or parameter value
    } BITS;
} __FMU_FCCOB_t;

/********************************************************************
 * 2. Declaración de la estructura del módulo FMU
 ********************************************************************/
typedef struct {
    volatile __FMU_FSTAT_t FSTAT;       // Offset 0x00
    volatile __FMU_FCNFG_t FCNFG;       // Offset 0x04
    volatile __FMU_FCTRL_t FCTRL;       // Offset 0x08
    volatile __FMU_FCCOB_t FCCOB0;      // Offset 0x0C
    volatile __FMU_FCCOB_t FCCOB1;      // Offset 0x10
    volatile __FMU_FCCOB_t FCCOB2;      // Offset 0x14
    volatile __FMU_FCCOB_t FCCOB3;      // Offset 0x18
    volatile __FMU_FCCOB_t FCCOB4;      // Offset 0x1C
    volatile __FMU_FCCOB_t FCCOB5;      // Offset 0x20
    volatile __FMU_FCCOB_t FCCOB6;      // Offset 0x24
    volatile __FMU_FCCOB_t FCCOB7;      // Offset 0x28
} __FMU__t;

/********************************************************************
 * 3. Macros de acceso corto
 ********************************************************************/

/* Base address del módulo FMU (según manual) */
#define kFMU_BASE_ADDR 0x40095000UL

/* Macro acceso a estructura FMU */
#define sFMU (*((__FMU__t *) kFMU_BASE_ADDR))

/* Macros para acceso a registros */
#define rFMU_FSTAT       sFMU.FSTAT.WORD
#define rFMU_FCNFG       sFMU.FCNFG.WORD
#define rFMU_FCTRL       sFMU.FCTRL.WORD
#define rFMU_FCCOB0      sFMU.FCCOB0.WORD
#define rFMU_FCCOB1      sFMU.FCCOB1.WORD
#define rFMU_FCCOB2      sFMU.FCCOB2.WORD
#define rFMU_FCCOB3      sFMU.FCCOB3.WORD
#define rFMU_FCCOB4      sFMU.FCCOB4.WORD
#define rFMU_FCCOB5      sFMU.FCCOB5.WORD
#define rFMU_FCCOB6      sFMU.FCCOB6.WORD
#define rFMU_FCCOB7      sFMU.FCCOB7.WORD

/* Macros para acceso a bits individuales o campos de bits */

/* FSTAT bits */
#define bFMU_FAIL        sFMU.FSTAT.BITS.FAIL
#define bFMU_CMDABT      sFMU.FSTAT.BITS.CMDABT
#define bFMU_PVIOL       sFMU.FSTAT.BITS.PVIOL
#define bFMU_ACCERR      sFMU.FSTAT.BITS.ACCERR
#define bFMU_CWSABT      sFMU.FSTAT.BITS.CWSABT
#define bFMU_CCIF        sFMU.FSTAT.BITS.CCIF
#define bFMU_CMDPRT      sFMU.FSTAT.BITS.CMDPRT
#define bFMU_CMDP        sFMU.FSTAT.BITS.CMDP
#define bFMU_CMDDID      sFMU.FSTAT.BITS.CMDDID
#define bFMU_DFDIF       sFMU.FSTAT.BITS.DFDIF
#define bFMU_SALV_USED   sFMU.FSTAT.BITS.SALV_USED
#define bFMU_PERDY       sFMU.FSTAT.BITS.PERDY

/* FCNFG bits */
#define bFMU_CCIE        sFMU.FCNFG.BITS.CCIE
#define bFMU_ERSREQ      sFMU.FCNFG.BITS.ERSREQ
#define bFMU_DFDIE       sFMU.FCNFG.BITS.DFDIE
#define bFMU_ERSIEN0     sFMU.FCNFG.BITS.ERSIEN0
#define bFMU_ERSIEN1     sFMU.FCNFG.BITS.ERSIEN1

/* FCTRL bits */
#define bFMU_RWSC        sFMU.FCTRL.BITS.RWSC
#define bFMU_LSACTIVE    sFMU.FCTRL.BITS.LSACTIVE
#define bFMU_FDFD        sFMU.FCTRL.BITS.FDFD
#define bFMU_ABTREQ      sFMU.FCTRL.BITS.ABTREQ

#endif /* FMU_MCX153_LIB_H */
