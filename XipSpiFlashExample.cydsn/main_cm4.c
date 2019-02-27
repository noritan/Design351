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
#include "project.h"
#include <stdio.h>
#include <string.h>

/* Array of cy_stc_smif_block_config_t structures */
typedef struct {
    const cy_stc_smif_block_config_t    *smifConfig;
    const uint32_t                      null;
} stc_smif_ipblocks_arr_t;

const stc_smif_ipblocks_arr_t smifIpBlocksArray = {
    .smifConfig = &smifBlockConfig,
    .null = 0x00000000
};

typedef struct {
    uint32_t        objectSize;
    uint32_t        magicNumber;                // Magic Number
    uint32_t        keyStorage;                 // Key storage placeholder
    const stc_smif_ipblocks_arr_t   *smifIpBlocksArray;         // Pointer to the array of IP blocks
    uint32_t        appStart;
    uint32_t        appType;
    const uint8_t   dummy[0x1E4];
    uint32_t        crc16;
} stc_user_toc2_t;

CY_SECTION(".cy_toc_part2") __USED
const stc_user_toc2_t test_toc = {
    .objectSize     = sizeof(stc_user_toc2_t) - 4,
    .magicNumber    = 0x01211220,
    .appStart       = 0x10000000,
    .appType        = 0,
    .smifIpBlocksArray  = &smifIpBlocksArray
};

CY_SECTION(".cy_xip")
void dump(uint32_t baseAddress) {
//    uint32_t    baseAddress = smifMemConfigs[0]->baseAddress;
    uint32_t    size = 0x0100;
    uint8_t     *array = (uint8_t *)baseAddress;
    uint32_t    i;
    uint32_t    j;
    char        buf[132];
    for (i = 0; i < size; i+=0x10) {
        buf[0] = '\0';
        sprintf(buf+strlen(buf), "%08lX:", baseAddress + i);
        for (j = 0; j < 16; j++) {
            sprintf(buf+strlen(buf), " %02X", array[i + j]);
        }
        sprintf(buf+strlen(buf), " \r\n");
        UART_PutString(buf);
    }
 }

int main(void)
{
    cy_en_smif_status_t smif_status;
    
    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    UART_Start();
    UART_PutString("Hello World\r\n");
    
    Cy_SysLib_Delay(1u);

    smif_status = SMIF_Start((cy_stc_smif_block_config_t *)&smifBlockConfig, 1000u);
    if (smif_status != CY_SMIF_SUCCESS) {
        UART_PutString("\r\n\r\nSMIF_Start failed\r\n");
    }

    smif_status = SMIF_Mem_QuadEnable(0);
    if (smif_status!=CY_SMIF_SUCCESS) {
        UART_PutString("\r\n\r\nSMIF Cy_SMIF_Memslot_QuadEnable failed\r\n");
    }
    
    while (SMIF_Mem_IsBusy(0)) {
        /* Wait until the Erase operation is completed */
    }
    
    SMIF_SetMode(CY_SMIF_MEMORY);
    
    dump(smifMemConfigs[0]->baseAddress);
    
    dump(0x16007c00);

    dump(0x16007d00);


    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
