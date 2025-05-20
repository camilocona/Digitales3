/**
 * @file FMU_MCX153_LIB.h
 * @author Camilo Andres Anacona Anacona (camilo.anacona@udea.edu.co)
 * @brief Hardware library for MCX 153 Flash Memory Module (FMU)
 * @version 0.1
 * @date 2025-05-19
 * 
 * @copyright Unlicensed
 * 
 * This header file declares the data types, constants, macros, and the module structure
 * for controlling the memory-mapped registers of the Flash Memory Module (FMU).
 * The defined data structures and macros follow a standard naming notation
 * to ensure consistency and ease of use in register access and manipulation.
 *
 * Naming notation used in this file:
 *
 * MODULE STRUCTURE                 ==>   s + MODULEname + [MODULEnumber]
 * MMIO REGISTER                  ==>   r + MODULEname + [MODULEnumber] + _ + REGISTERname + [REGISTERnumber]
 * REGISTER BITFIELD or BIT       ==>   b + MODULEname + [MODULEnumber] + _ + BITFIELDname
 * BITFIELD CONSTANT VALUEs       ==>   k + MODULEname + _ + BITFIELDname + _ + CONSTANTname
 * MODULE MASKs                  ==>   m + MODULEname + _ + MASKname
 * REGISTER TYPE                ==>   __ + MODULEname + _ + REGISTERname + _t
 * STRUCTURE TYPE               ==>   __ + MODULEname + _ + _t
 *
 * Structure of this file:
 *
 * 1. Register definition
 *    a. Data type: typedef unions for registers (__FMU_FSTAT_t, etc.)
 *    b. Register masks: #define mFMU_<REGISTER>_<BIT> masks
 *    c. Bitfield constant values: #define kFMU_<REGISTER>_<BITFIELD>_<CONST>
 *
 * 2. Module structure definition
 *    a. Data type: typedef struct __FMU__t mapping all registers
 *    b. Module masks: None defined (not applicable or not needed)
 *    c. Structure access macro: #define sFMU (*((__FMU__t *) kFMU_BASE_ADDR))
 *
 * 3. Register access macros
 *    a. Whole register access macros: #define rFMU_<REGISTER> for 32-bit access
 *    b. Bitfield access macros: #define bFMU_<BITFIELD> for individual bits or fields
 *    c. Atomic write macros:
 *       - #define xorFMU_<REGISTER>_<NUMBER> for atomic XOR writes
 *       - #define setFMU_<REGISTER>_<NUMBER> for atomic SET writes
 *       - #define clrFMU_<REGISTER>_<NUMBER> for atomic CLR writes
 */

#ifndef FMU_MCX153_LIB_H
#define FMU_MCX153_LIB_H

#include <stdint.h> /**< Include fixed-width integer types (e.g., uint32_t) */

/**
 * @brief Flash Status Register (Flash module status register).
 *
 * Represents the status flags and control bits of the Flash Memory Module.
 */
typedef union {
    uint32_t WORD; /**< Full 32-bit register access */
    struct {
        uint32_t FAIL:1;       /**< Bit 0: Command Fail Flag (1 indicates command failure) */
        uint32_t RESERVED0:1;  /**< Bit 1: Reserved, do not use */
        uint32_t CMDABT:1;     /**< Bit 2: Command Abort Flag (indicates command was aborted) */
        uint32_t RESERVED1:1;  /**< Bit 3: Reserved, do not use */
        uint32_t PVIOL:1;      /**< Bit 4: Protection Violation Flag */
        uint32_t ACCERR:1;     /**< Bit 5: Access Error Flag (invalid access or parameters) */
        uint32_t CWSABT:1;     /**< Bit 6: Command Write Sequence Abort Flag */
        uint32_t CCIF:1;       /**< Bit 7: Command Complete Interrupt Flag */
        uint32_t CMDPRT:2;     /**< Bits 8-9: Command Protection Level */
        uint32_t CMDP:1;       /**< Bit 10: Command Protection Status Flag */
        uint32_t RESERVED2:1;  /**< Bit 11: Reserved, do not use */
        uint32_t CMDDID:4;     /**< Bits 12-15: Command Domain ID */
        uint32_t DFDIF:1;      /**< Bit 16: Double Bit Fault Detect Interrupt Flag (ECC error) */
        uint32_t SALV_USED:1;  /**< Bit 17: Salvage Used Flag (ECC salvage on erase) */
        uint32_t RESERVED3:13; /**< Bits 18-30: Reserved, do not use */
        uint32_t PERDY:1;      /**< Bit 31: Program-Erase Ready Flag */
    } BITS; /**< Bit-field access structure */
} __FMU_FSTAT_t;

/* Register bit masks */
#define mFMU_FSTAT_FAIL        0x00000001UL /**< Mask for FAIL bit (bit 0) in FSTAT register */
#define mFMU_FSTAT_CMDABT      0x00000004UL /**< Mask for CMDABT bit (bit 2) in FSTAT register */
#define mFMU_FSTAT_PVIOL       0x00000010UL /**< Mask for PVIOL bit (bit 4) in FSTAT register */
#define mFMU_FSTAT_ACCERR      0x00000020UL /**< Mask for ACCERR bit (bit 5) in FSTAT register */
#define mFMU_FSTAT_CWSABT      0x00000040UL /**< Mask for CWSABT bit (bit 6) in FSTAT register */
#define mFMU_FSTAT_CCIF        0x00000080UL /**< Mask for CCIF bit (bit 7) in FSTAT register */
#define mFMU_FSTAT_CMDPRT      0x00000300UL /**< Mask for CMDPRT bits (bits 8-9) in FSTAT register */
#define mFMU_FSTAT_CMDP        0x00000400UL /**< Mask for CMDP bit (bit 10) in FSTAT register */
#define mFMU_FSTAT_CMDDID      0x0000F000UL /**< Mask for CMDDID bits (bits 12-15) in FSTAT register */
#define mFMU_FSTAT_DFDIF       0x00010000UL /**< Mask for DFDIF bit (bit 16) in FSTAT register */
#define mFMU_FSTAT_SALV_USED   0x00020000UL /**< Mask for SALV_USED bit (bit 17) in FSTAT register */
#define mFMU_FSTAT_PERDY       0x80000000UL /**< Mask for PERDY bit (bit 31) in FSTAT register */

/* CMDPRT Protection Levels Constants */
#define kFMU_FSTAT_CMDPRT_SecureNormal         0x0 /**< Secure Normal Protection */
#define kFMU_FSTAT_CMDPRT_SecurePrivileged     0x1 /**< Secure Privileged Protection */
#define kFMU_FSTAT_CMDPRT_NonsecureNormal      0x2 /**< Nonsecure Normal Protection */
#define kFMU_FSTAT_CMDPRT_NonsecurePrivileged  0x3 /**< Nonsecure Privileged Protection */

/**
 * @brief Flash Configuration Register (Flash module configuration register).
 *
 * Controls configuration flags for the Flash Memory Module.
 */
typedef union {
    uint32_t WORD; /**< Full 32-bit access */
    struct {
        uint32_t RESERVED0:7;  /**< Bits 0-6: Reserved */
        uint32_t CCIE:1;       /**< Bit 7: Command Complete Interrupt Enable */
        uint32_t ERSREQ:1;     /**< Bit 8: Mass Erase Request (read-only) */
        uint32_t RESERVED1:7;  /**< Bits 9-15: Reserved */
        uint32_t DFDIE:1;      /**< Bit 16: Double Bit Fault Detect Interrupt Enable */
        uint32_t RESERVED2:7;  /**< Bits 17-23: Reserved */
        uint32_t ERSIEN0:4;    /**< Bits 24-27: Erase IFR Sector Enable Block 0 */
        uint32_t ERSIEN1:4;    /**< Bits 28-31: Erase IFR Sector Enable Block 1 */
    } BITS; /**< Bit-field access structure */
} __FMU_FCNFG_t;

/* FCNFG Register Masks */
#define mFMU_FCNFG_CCIE      0x00000080UL /**< Mask for CCIE bit (bit 7) in FCNFG register */
#define mFMU_FCNFG_ERSREQ    0x00000100UL /**< Mask for ERSREQ bit (bit 8) in FCNFG register */
#define mFMU_FCNFG_DFDIE     0x00010000UL /**< Mask for DFDIE bit (bit 16) in FCNFG register */
#define mFMU_FCNFG_ERSIEN0   0x0F000000UL /**< Mask for ERSIEN0 bits (bits 24-27) in FCNFG register */
#define mFMU_FCNFG_ERSIEN1   0xF0000000UL /**< Mask for ERSIEN1 bits (bits 28-31) in FCNFG register */

/**
 * @brief Flash Control Register (Flash module control register).
 *
 * Provides control signals and flags for Flash operation.
 */
typedef union {
    uint32_t WORD; /**< Full 32-bit access */
    struct {
        uint32_t RWSC:4;       /**< Bits 0-3: Read Wait-State Control */
        uint32_t RESERVED0:4;  /**< Bits 4-7: Reserved */
        uint32_t LSACTIVE:1;   /**< Bit 8: Low Speed Active mode enable */
        uint32_t RESERVED1:7;  /**< Bits 9-15: Reserved */
        uint32_t FDFD:1;       /**< Bit 16: Force Double Bit Fault Detect */
        uint32_t RESERVED2:7;  /**< Bits 17-23: Reserved */
        uint32_t ABTREQ:1;     /**< Bit 24: Abort Request */
        uint32_t RESERVED3:7;  /**< Bits 25-31: Reserved */
    } BITS; /**< Bit-field access structure */
} __FMU_FCTRL_t;

/* FCTRL Register Masks */
#define mFMU_FCTRL_RWSC       0x0000000FUL /**< Mask for RWSC bits (bits 0-3) in FCTRL register */
#define mFMU_FCTRL_LSACTIVE   0x00000100UL /**< Mask for LSACTIVE bit (bit 8) in FCTRL register */
#define mFMU_FCTRL_FDFD       0x00010000UL /**< Mask for FDFD bit (bit 16) in FCTRL register */
#define mFMU_FCTRL_ABTREQ     0x01000000UL /**< Mask for ABTREQ bit (bit 24) in FCTRL register */

/**
 * @brief Flash Common Command Object Registers (FCCOB0 - FCCOB7).
 *
 * Used for command parameters and communication with the Flash module.
 */
typedef union {
    uint32_t WORD; /**< Full 32-bit access to each FCCOB register */
    struct {
        uint32_t CCOBn:32; /**< Bits 0-31: Command or parameter value */
    } BITS; /**< Bit-field access structure */
} __FMU_FCCOB_t;

/**
 * @brief FMU module structure mapping all registers.
 */
typedef struct {
    volatile __FMU_FSTAT_t FSTAT;   /**< Offset 0x00: Flash Status Register */
    volatile __FMU_FCNFG_t FCNFG;   /**< Offset 0x04: Flash Configuration Register */
    volatile __FMU_FCTRL_t FCTRL;   /**< Offset 0x08: Flash Control Register */
    volatile __FMU_FCCOB_t FCCOB0;  /**< Offset 0x0C: Flash Common Command Object Register 0 */
    volatile __FMU_FCCOB_t FCCOB1;  /**< Offset 0x10: Flash Common Command Object Register 1 */
    volatile __FMU_FCCOB_t FCCOB2;  /**< Offset 0x14: Flash Common Command Object Register 2 */
    volatile __FMU_FCCOB_t FCCOB3;  /**< Offset 0x18: Flash Common Command Object Register 3 */
    volatile __FMU_FCCOB_t FCCOB4;  /**< Offset 0x1C: Flash Common Command Object Register 4 */
    volatile __FMU_FCCOB_t FCCOB5;  /**< Offset 0x20: Flash Common Command Object Register 5 */
    volatile __FMU_FCCOB_t FCCOB6;  /**< Offset 0x24: Flash Common Command Object Register 6 */
    volatile __FMU_FCCOB_t FCCOB7;  /**< Offset 0x28: Flash Common Command Object Register 7 */
} __FMU__t;

/**
 * @brief FMU base address as defined in datasheet.
 */
#define kFMU_BASE_ADDR 0x40095000UL

/**
 * @brief Macro that casts the FMU base address to a pointer to FMU registers structure.
 */
#define sFMU (*((__FMU__t *) kFMU_BASE_ADDR))

/* Macros for direct access to full 32-bit registers */
#define rFMU_FSTAT       sFMU.FSTAT.WORD  /**< Macro for FSTAT register (Flash Status) */
#define rFMU_FCNFG       sFMU.FCNFG.WORD  /**< Macro for FCNFG register (Flash Configuration) */
#define rFMU_FCTRL       sFMU.FCTRL.WORD  /**< Macro for FCTRL register (Flash Control) */
#define rFMU_FCCOB0      sFMU.FCCOB0.WORD /**< Macro for FCCOB0 register */
#define rFMU_FCCOB1      sFMU.FCCOB1.WORD /**< Macro for FCCOB1 register */
#define rFMU_FCCOB2      sFMU.FCCOB2.WORD /**< Macro for FCCOB2 register */
#define rFMU_FCCOB3      sFMU.FCCOB3.WORD /**< Macro for FCCOB3 register */
#define rFMU_FCCOB4      sFMU.FCCOB4.WORD /**< Macro for FCCOB4 register */
#define rFMU_FCCOB5      sFMU.FCCOB5.WORD /**< Macro for FCCOB5 register */
#define rFMU_FCCOB6      sFMU.FCCOB6.WORD /**< Macro for FCCOB6 register */
#define rFMU_FCCOB7      sFMU.FCCOB7.WORD /**< Macro for FCCOB7 register */

/* Macros for access to individual bits or bit-fields */
#define bFMU_FAIL        sFMU.FSTAT.BITS.FAIL           /**< Command Fail Flag */
#define bFMU_CMDABT      sFMU.FSTAT.BITS.CMDABT         /**< Command Abort Flag */
#define bFMU_PVIOL       sFMU.FSTAT.BITS.PVIOL          /**< Protection Violation Flag */
#define bFMU_ACCERR      sFMU.FSTAT.BITS.ACCERR         /**< Access Error Flag */
#define bFMU_CWSABT      sFMU.FSTAT.BITS.CWSABT         /**< Command Write Sequence Abort Flag */
#define bFMU_CCIF        sFMU.FSTAT.BITS.CCIF           /**< Command Complete Interrupt Flag */
#define bFMU_CMDPRT      sFMU.FSTAT.BITS.CMDPRT         /**< Command Protection Level */
#define bFMU_CMDP        sFMU.FSTAT.BITS.CMDP           /**< Command Protection Status Flag */
#define bFMU_CMDDID      sFMU.FSTAT.BITS.CMDDID         /**< Command Domain ID */
#define bFMU_DFDIF       sFMU.FSTAT.BITS.DFDIF          /**< Double Fault Detect Interrupt Flag */
#define bFMU_SALV_USED   sFMU.FSTAT.BITS.SALV_USED      /**< Salvage Used Flag */
#define bFMU_PERDY       sFMU.FSTAT.BITS.PERDY          /**< Program-Erase Ready Flag */
#define bFMU_CCIE        sFMU.FCNFG.BITS.CCIE           /**< Command Complete Interrupt Enable */
#define bFMU_ERSREQ      sFMU.FCNFG.BITS.ERSREQ         /**< Mass Erase Request (read-only) */
#define bFMU_DFDIE       sFMU.FCNFG.BITS.DFDIE          /**< Double Bit Fault Detect Interrupt Enable */
#define bFMU_ERSIEN0     sFMU.FCNFG.BITS.ERSIEN0        /**< Erase IFR Sector Enable Block 0 */
#define bFMU_ERSIEN1     sFMU.FCNFG.BITS.ERSIEN1        /**< Erase IFR Sector Enable Block 1 */
#define bFMU_RWSC        sFMU.FCTRL.BITS.RWSC           /**< Read Wait-State Control */
#define bFMU_LSACTIVE    sFMU.FCTRL.BITS.LSACTIVE       /**< Low Speed Active Mode Enable */
#define bFMU_FDFD        sFMU.FCTRL.BITS.FDFD           /**< Force Double Bit Fault Detect */
#define bFMU_ABTREQ      sFMU.FCTRL.BITS.ABTREQ         /**< Abort Request */

#endif /* FMU_MCX153_LIB_H */

