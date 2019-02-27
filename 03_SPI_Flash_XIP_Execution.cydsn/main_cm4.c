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
#include <stdio.h>
#include "project.h"

//**********************************************
// Dump a memory content
//**********************************************
CY_SECTION(".cy_xip")
void DumpSerialMemory(
    uint8_t const *address,
    uint32_t size
){
    uint32_t    i;
    uint32_t    j;
    for (i = 0; i < size; i+=0x10) {
        printf("%08lX:", i);
        for (j = 0; j < 0x10; j++) {
            printf(" %02X", address[i + j]);
        }
        printf(" \r\n");
    }
}

int main(void) {
    uint32_t targetSlot = 0;  // S25FL512S

    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    UART_Start();
    printf("\r\n03: SPI Flash XIP Execution\r\n");
    
    // Initialize SMIF as QUAD I/O
    SMIF_Start((cy_stc_smif_block_config_t *)&smifBlockConfig, 100u);
    {
        cy_en_smif_status_t status;

        status = SMIF_Mem_QuadEnable(targetSlot);
        if (status != CY_SMIF_SUCCESS) {
            printf("\r\nQuad Enable command failed.\r\n");
            for (;;) ;
        }
    }

    {
        while (SMIF_Mem_IsBusy(targetSlot)) {
        //while (SMIF_BusyCheck()) {
            /* Wait till the memory controller command is completed */
            // Do nothing
        }
    }

    // Test sequence
    {
        uint32_t baseAddress = S25FL512S_SlaveSlot_0.baseAddress;

        printf("\r\nEnter to XIP mode\r\n");

        SMIF_SetMode(CY_SMIF_MEMORY);
        
        printf("\r\nDump a page\r\n");

        DumpSerialMemory((uint8_t *)(baseAddress + 0x0000), 256);

        DumpSerialMemory((uint8_t *)(baseAddress + 0x1000), 256);

        DumpSerialMemory((uint8_t *)(baseAddress + 0x2000), 256);
    }    

    for(;;) {
        /* Place your application code here. */
    }
}

/*******************************************************************************
* Function Name: _write
*******************************************************************************/
#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library to allow
    the usage of the floating point conversion specifier. */
    asm (".global _printf_float");
    /* For GCC compiler revise _write() function for printf functionality */
    int _write(int file, char *ptr, int len)
    {
        int nChars = 0;

        /* Suppress the compiler warning about an unused variable. */
        if (0 != file)
        {
        }
        
        for (/* Empty */; len != 0; --len)
        {
            /* Block until there is space in the TX FIFO or buffer. */
            while (0UL == UART_Put(*ptr))
            {
            }
            
            ++nChars;
            ++ptr;
        }
        
        return (nChars);
    }
#endif

//**********************************************
// User defined TOC2 element
//**********************************************
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

/* [] END OF FILE */
