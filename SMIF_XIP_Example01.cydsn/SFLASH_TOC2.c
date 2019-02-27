/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "cy_smif_memconfig.h"

/* Array of cy_stc_smif_block_config_t structures  */    
typedef struct
{
    const cy_stc_smif_block_config_t * smifCfg; /* Pointer to SMIF top-level configuration */
    const uint32_t null_t;                      /* NULL termination */
} stc_smif_ipblocks_arr_t;

const stc_smif_ipblocks_arr_t smifIpBlocksArr = {&smifBlockConfig, 0x00000000};  

typedef struct
{
    uint32_t objSize;
    uint32_t magicNumber;                               /* Magic Number */
    uint32_t keyStorage;                                /* Key storage placeholder */
    const stc_smif_ipblocks_arr_t* smifIpBlocksArrPtr;  /* Offset (0x0C): Pointer to the array of cy_stc_smif_block_config_t structures  */
    unsigned char const  dummy[0x34];                   /* Some data dummy*/
    uint32_t CRC16;
} stc_user_toc2_t;

CY_SECTION(".cy_toc_part2") __USED
const stc_user_toc2_t test_toc =
{
    .objSize = sizeof(stc_user_toc2_t),
    .magicNumber =   0x01211219,            /* First word is Magic Number (01211219) */
    .keyStorage  =   0xFFFFFFFF,            /* Key storage placeholder */
    .smifIpBlocksArrPtr = &smifIpBlocksArr  /* Offset (0x0C): Pointer to the array of cy_stc_smif_block_config_t structures  */
};


/* [] END OF FILE */
