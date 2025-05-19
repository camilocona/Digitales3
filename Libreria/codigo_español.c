#ifndef FMU_MCX153_LIB_H             // Inicio del include guard para evitar inclusiones múltiples
#define FMU_MCX153_LIB_H

#include <stdint.h>                 // Incluye definiciones de tipos enteros de tamaño fijo (ej. uint32_t)

/********************************************************************
 * 1. Declaración de registros FMU como uniones con campos de bits
 ********************************************************************/

/**
 * \typedef __FMU_FSTAT_t
 * Registro de estado del módulo Flash (Flash Status Register)
 */
typedef union {
    uint32_t WORD;                  // Acceso completo de 32 bits al registro
    struct {
        uint32_t FAIL:1;            // Bit 0: Flag que indica fallo en comando (1=fallo)
        uint32_t RESERVED0:1;       // Bit 1: Reservado, sin uso
        uint32_t CMDABT:1;          // Bit 2: Flag que indica aborto de comando
        uint32_t RESERVED1:1;       // Bit 3: Reservado, sin uso
        uint32_t PVIOL:1;           // Bit 4: Flag de violación de protección
        uint32_t ACCERR:1;          // Bit 5: Flag de error de acceso o parámetros inválidos
        uint32_t CWSABT:1;          // Bit 6: Flag de aborto en secuencia de escritura
        uint32_t CCIF:1;            // Bit 7: Flag de comando completado
        uint32_t CMDPRT:2;          // Bits 8-9: Nivel de protección del comando
        uint32_t CMDP:1;            // Bit 10: Estado de protección del comando
        uint32_t RESERVED2:1;       // Bit 11: Reservado, sin uso
        uint32_t CMDDID:4;          // Bits 12-15: ID del dominio del comando
        uint32_t DFDIF:1;           // Bit 16: Flag de interrupción por doble fallo ECC
        uint32_t SALV_USED:1;       // Bit 17: Flag que indica uso de "salvage" en borrado
        uint32_t RESERVED3:13;      // Bits 18-30: Reservados, sin uso
        uint32_t PERDY:1;           // Bit 31: Flag de listo para programa o borrado
    } BITS;                         // Estructura para acceso a bits individuales
} __FMU_FSTAT_t;

/* Máscara para bit FAIL (bit 0) del registro FSTAT */
#define mFMU_FSTAT_FAIL        0x00000001UL
/* Máscara para bit CMDABT (bit 2) del registro FSTAT */
#define mFMU_FSTAT_CMDABT      0x00000004UL
/* Máscara para bit PVIOL (bit 4) del registro FSTAT */
#define mFMU_FSTAT_PVIOL       0x00000010UL
/* Máscara para bit ACCERR (bit 5) del registro FSTAT */
#define mFMU_FSTAT_ACCERR      0x00000020UL
/* Máscara para bit CWSABT (bit 6) del registro FSTAT */
#define mFMU_FSTAT_CWSABT      0x00000040UL
/* Máscara para bit CCIF (bit 7) del registro FSTAT */
#define mFMU_FSTAT_CCIF        0x00000080UL
/* Máscara para bits CMDPRT (bits 8 y 9) del registro FSTAT */
#define mFMU_FSTAT_CMDPRT      0x00000300UL
/* Máscara para bit CMDP (bit 10) del registro FSTAT */
#define mFMU_FSTAT_CMDP        0x00000400UL
/* Máscara para bits CMDDID (bits 12-15) del registro FSTAT */
#define mFMU_FSTAT_CMDDID      0x0000F000UL
/* Máscara para bit DFDIF (bit 16) del registro FSTAT */
#define mFMU_FSTAT_DFDIF       0x00010000UL
/* Máscara para bit SALV_USED (bit 17) del registro FSTAT */
#define mFMU_FSTAT_SALV_USED   0x00020000UL
/* Máscara para bit PERDY (bit 31) del registro FSTAT */
#define mFMU_FSTAT_PERDY       0x80000000UL

/* Constante para valor 0 del campo CMDPRT: protección segura normal */
#define kFMU_FSTAT_CMDPRT_SecureNormal         0x0
/* Constante para valor 1 del campo CMDPRT: protección segura privilegiada */
#define kFMU_FSTAT_CMDPRT_SecurePrivileged     0x1
/* Constante para valor 2 del campo CMDPRT: protección no segura normal */
#define kFMU_FSTAT_CMDPRT_NonsecureNormal      0x2
/* Constante para valor 3 del campo CMDPRT: protección no segura privilegiada */
#define kFMU_FSTAT_CMDPRT_NonsecurePrivileged  0x3

/**
 * \typedef __FMU_FCNFG_t
 * Registro de configuración del módulo Flash (Flash Configuration Register)
 */
typedef union {
    uint32_t WORD;                  // Acceso completo de 32 bits
    struct {
        uint32_t RESERVED0:7;       // Bits 0-6: Reservados, sin uso
        uint32_t CCIE:1;            // Bit 7: Habilita interrupción al completar comando
        uint32_t ERSREQ:1;          // Bit 8: Solicitud de borrado masivo (solo lectura)
        uint32_t RESERVED1:7;       // Bits 9-15: Reservados, sin uso
        uint32_t DFDIE:1;           // Bit 16: Habilita interrupción por doble fallo ECC
        uint32_t RESERVED2:7;       // Bits 17-23: Reservados, sin uso
        uint32_t ERSIEN0:4;         // Bits 24-27: Habilita borrado de sectores IFR bloque 0
        uint32_t ERSIEN1:4;         // Bits 28-31: Habilita borrado de sectores IFR bloque 1
    } BITS;                         // Acceso por bits
} __FMU_FCNFG_t;

/* Máscara para bit CCIE (bit 7) del registro FCNFG */
#define mFMU_FCNFG_CCIE      0x00000080UL
/* Máscara para bit ERSREQ (bit 8) del registro FCNFG */
#define mFMU_FCNFG_ERSREQ    0x00000100UL
/* Máscara para bit DFDIE (bit 16) del registro FCNFG */
#define mFMU_FCNFG_DFDIE     0x00010000UL
/* Máscara para bits ERSIEN0 (bits 24-27) del registro FCNFG */
#define mFMU_FCNFG_ERSIEN0   0x0F000000UL
/* Máscara para bits ERSIEN1 (bits 28-31) del registro FCNFG */
#define mFMU_FCNFG_ERSIEN1   0xF0000000UL

/**
 * \typedef __FMU_FCTRL_t
 * Registro de control del módulo Flash (Flash Control Register)
 */
typedef union {
    uint32_t WORD;                  // Acceso completo de 32 bits
    struct {
        uint32_t RWSC:4;           // Bits 0-3: Control de estados de espera para lectura de flash
        uint32_t RESERVED0:4;      // Bits 4-7: Reservados, sin uso
        uint32_t LSACTIVE:1;       // Bit 8: Habilita modo activo a baja velocidad
        uint32_t RESERVED1:7;      // Bits 9-15: Reservados, sin uso
        uint32_t FDFD:1;           // Bit 16: Fuerza detección de doble fallo ECC
        uint32_t RESERVED2:7;      // Bits 17-23: Reservados, sin uso
        uint32_t ABTREQ:1;         // Bit 24: Solicitud de aborto de comando
        uint32_t RESERVED3:7;      // Bits 25-31: Reservados, sin uso
    } BITS;
} __FMU_FCTRL_t;

/* Máscara para bits RWSC (bits 0-3) del registro FCTRL */
#define mFMU_FCTRL_RWSC       0x0000000FUL
/* Máscara para bit LSACTIVE (bit 8) del registro FCTRL */
#define mFMU_FCTRL_LSACTIVE   0x00000100UL
/* Máscara para bit FDFD (bit 16) del registro FCTRL */
#define mFMU_FCTRL_FDFD       0x00010000UL
/* Máscara para bit ABTREQ (bit 24) del registro FCTRL */
#define mFMU_FCTRL_ABTREQ     0x01000000UL

/**
 * \typedef __FMU_FCCOB_t
 * Registros de objeto común de comando Flash (FCCOB0 - FCCOB7)
 */
typedef union {
    uint32_t WORD;                  // Acceso completo 32 bits
    struct {
        uint32_t CCOBn:32;         // Bits 0-31: Valor del comando o parámetro
    } BITS;
} __FMU_FCCOB_t;

/********************************************************************
 * 2. Declaración de la estructura del módulo FMU que agrupa todos los registros
 ********************************************************************/
typedef struct {
    volatile __FMU_FSTAT_t FSTAT;       // Offset 0x00: Registro estado Flash
    volatile __FMU_FCNFG_t FCNFG;       // Offset 0x04: Registro configuración Flash
    volatile __FMU_FCTRL_t FCTRL;       // Offset 0x08: Registro control Flash
    volatile __FMU_FCCOB_t FCCOB0;      // Offset 0x0C: Registro comando común 0
    volatile __FMU_FCCOB_t FCCOB1;      // Offset 0x10: Registro comando común 1
    volatile __FMU_FCCOB_t FCCOB2;      // Offset 0x14: Registro comando común 2
    volatile __FMU_FCCOB_t FCCOB3;      // Offset 0x18: Registro comando común 3
    volatile __FMU_FCCOB_t FCCOB4;      // Offset 0x1C: Registro comando común 4
    volatile __FMU_FCCOB_t FCCOB5;      // Offset 0x20: Registro comando común 5
    volatile __FMU_FCCOB_t FCCOB6;      // Offset 0x24: Registro comando común 6
    volatile __FMU_FCCOB_t FCCOB7;      // Offset 0x28: Registro comando común 7
} __FMU__t;

/********************************************************************
 * 3. Macros de acceso rápido a registros y bits del módulo FMU
 ********************************************************************/

/* Dirección base del módulo FMU, según manual */
#define kFMU_BASE_ADDR 0x40095000UL

/* Macro para convertir la dirección base en un puntero a la estructura del módulo FMU */
#define sFMU (*((__FMU__t *) kFMU_BASE_ADDR))

/* Macros para acceso rápido a los registros completos */

/* Macro para registro de estado Flash (FSTAT) */
#define rFMU_FSTAT       sFMU.FSTAT.WORD
/* Macro para registro de configuración Flash (FCNFG) */
#define rFMU_FCNFG       sFMU.FCNFG.WORD
/* Macro para registro de control Flash (FCTRL) */
#define rFMU_FCTRL       sFMU.FCTRL.WORD
/* Macro para registro comando común FCCOB0 */
#define rFMU_FCCOB0      sFMU.FCCOB0.WORD
/* Macro para registro comando común FCCOB1 */
#define rFMU_FCCOB1      sFMU.FCCOB1.WORD
/* Macro para registro comando común FCCOB2 */
#define rFMU_FCCOB2      sFMU.FCCOB2.WORD
/* Macro para registro comando común FCCOB3 */
#define rFMU_FCCOB3      sFMU.FCCOB3.WORD
/* Macro para registro comando común FCCOB4 */
#define rFMU_FCCOB4      sFMU.FCCOB4.WORD
/* Macro para registro comando común FCCOB5 */
#define rFMU_FCCOB5      sFMU.FCCOB5.WORD
/* Macro para registro comando común FCCOB6 */
#define rFMU_FCCOB6      sFMU.FCCOB6.WORD
/* Macro para registro comando común FCCOB7 */
#define rFMU_FCCOB7      sFMU.FCCOB7.WORD

/* Macros para acceso a bits o campos individuales dentro de los registros */

/* Bits del registro FSTAT */

/* Macro para bit FAIL (flag fallo de comando) */
#define bFMU_FAIL        sFMU.FSTAT.BITS.FAIL
/* Macro para bit CMDABT (flag aborto comando) */
#define bFMU_CMDABT      sFMU.FSTAT.BITS.CMDABT
/* Macro para bit PVIOL (flag violación protección) */
#define bFMU_PVIOL       sFMU.FSTAT.BITS.PVIOL
/* Macro para bit ACCERR (flag error acceso) */
#define bFMU_ACCERR      sFMU.FSTAT.BITS.ACCERR
/* Macro para bit CWSABT (flag aborto secuencia escritura) */
#define bFMU_CWSABT      sFMU.FSTAT.BITS.CWSABT
/* Macro para bit CCIF (flag comando completado) */
#define bFMU_CCIF        sFMU.FSTAT.BITS.CCIF
/* Macro para bits CMDPRT (nivel protección comando) */
#define bFMU_CMDPRT      sFMU.FSTAT.BITS.CMDPRT
/* Macro para bit CMDP (estado protección comando) */
#define bFMU_CMDP        sFMU.FSTAT.BITS.CMDP
/* Macro para bits CMDDID (ID dominio comando) */
#define bFMU_CMDDID      sFMU.FSTAT.BITS.CMDDID
/* Macro para bit DFDIF (flag doble fallo ECC) */
#define bFMU_DFDIF       sFMU.FSTAT.BITS.DFDIF
/* Macro para bit SALV_USED (flag uso salvado en borrado) */
#define bFMU_SALV_USED   sFMU.FSTAT.BITS.SALV_USED
/* Macro para bit PERDY (flag listo para programa/borrado) */
#define bFMU_PERDY       sFMU.FSTAT.BITS.PERDY

/* Bits del registro FCNFG */

/* Macro para bit CCIE (habilita interrupción comando completado) */
#define bFMU_CCIE        sFMU.FCNFG.BITS.CCIE
/* Macro para bit ERSREQ (solicitud borrado masivo) */
#define bFMU_ERSREQ      sFMU.FCNFG.BITS.ERSREQ
/* Macro para bit DFDIE (habilita interrupción doble fallo ECC) */
#define bFMU_DFDIE       sFMU.FCNFG.BITS.DFDIE
/* Macro para bits ERSIEN0 (habilita borrado sectores IFR bloque 0) */
#define bFMU_ERSIEN0     sFMU.FCNFG.BITS.ERSIEN0
/* Macro para bits ERSIEN1 (habilita borrado sectores IFR bloque 1) */
#define bFMU_ERSIEN1     sFMU.FCNFG.BITS.ERSIEN1

/* Bits del registro FCTRL */

/* Macro para bits RWSC (control estados espera lectura flash) */
#define bFMU_RWSC        sFMU.FCTRL.BITS.RWSC
/* Macro para bit LSACTIVE (modo activo baja velocidad) */
#define bFMU_LSACTIVE    sFMU.FCTRL.BITS.LSACTIVE
/* Macro para bit FDFD (fuerza detección doble fallo ECC) */
#define bFMU_FDFD        sFMU.FCTRL.BITS.FDFD
/* Macro para bit ABTREQ (solicitud aborto comando) */
#define bFMU_ABTREQ      sFMU.FCTRL.BITS.ABTREQ

#endif /* FMU_MCX153_LIB_H */         // Fin del include guard
