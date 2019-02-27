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
// Read a page of Serial Memory into buffer
//   Supporting 3-byte address only
//**********************************************
cy_en_smif_status_t ReadSerialMemory(
    uint32_t slotNumber,
    uint32_t address,
    uint8_t *buffer,
    uint32_t size
){
    cy_en_smif_status_t status;
    uint8_t arrayAddress[3];

    /* Convert 32-bit address to 3-byte array */
    arrayAddress[0] = CY_LO8(address >> 16);
    arrayAddress[1] = CY_LO8(address >> 8);
    arrayAddress[2] = CY_LO8(address);
    
    while (SMIF_Mem_IsBusy(slotNumber)) {
        /* Wait till the memory controller command is completed */
        // Do nothing
    }
	
	/* The Quad I/O Read */
    status = SMIF_Mem_CmdRead(slotNumber, arrayAddress, buffer, size, NULL);
    if (status != CY_SMIF_SUCCESS) {
        return status;
    }
    
    while (SMIF_BusyCheck()) {
        /* Wait until the SMIF IP operation is completed. */
        // Do nothing
    }
    return status;
}

//**********************************************
// Write a buffer into Serial Memory
//   Supporting 3-byte address only
//**********************************************
cy_en_smif_status_t WriteSerialMemory(
    uint32_t slotNumber,
    uint32_t address,
    uint8_t *buffer,
    uint32_t size
){
    cy_en_smif_status_t status;
    uint8_t arrayAddress[3];

    /* Convert 32-bit address to 3-byte array */
    arrayAddress[0] = CY_LO8(address >> 16);
    arrayAddress[1] = CY_LO8(address >> 8);
    arrayAddress[2] = CY_LO8(address);
    
    while (SMIF_Mem_IsBusy(slotNumber)) {
        /* Wait till the memory controller command is completed */
        // Do nothing
    }
	
    /* Send Write Enable to external memory */	
    status = SMIF_Mem_CmdWriteEnable(slotNumber);
    if (status != CY_SMIF_SUCCESS) {
        return status;
    }
	
	/* Quad I/O Program */       
    status = SMIF_Mem_CmdProgram(slotNumber, arrayAddress, buffer, size, NULL);
    if (status != CY_SMIF_SUCCESS) {
        return status;
    }	
        
    while (SMIF_BusyCheck()) {
        /* Wait till the memory controller command is completed */
        // Do nothing
    }
    return status;
}

//**********************************************
// Erase a sector
//   Supporting 3-byte address only
//**********************************************
cy_en_smif_status_t EraseSerialMemory(
    uint32_t slotNumber,
    uint32_t address
){
    cy_en_smif_status_t status;
    uint8_t arrayAddress[3];

    /* Convert 32-bit address to 3-byte array */
    arrayAddress[0] = CY_LO8(address >> 16);
    arrayAddress[1] = CY_LO8(address >> 8);
    arrayAddress[2] = CY_LO8(address);
    
    while (SMIF_Mem_IsBusy(slotNumber)) {
        /* Wait till the memory controller command is completed */
        // Do nothing
    }
	
    /* Send Write Enable to external memory */	
    status = SMIF_Mem_CmdWriteEnable(slotNumber);
    if (status != CY_SMIF_SUCCESS) {
        return status;
    }
	
	/* Sector Erase command */       
    status = SMIF_Mem_CmdSectorErase(slotNumber, arrayAddress);
    if (status != CY_SMIF_SUCCESS) {
        return status;
    }	
        
    while (SMIF_BusyCheck()) {
        /* Wait till the memory controller command is completed */
        // Do nothing
    }
    return status;
}

//**********************************************
// Dump a buffer content
//**********************************************
void DumpBuffer(
    uint8_t const *buffer,
    uint32_t size
){
    uint32_t    i;
    uint32_t    j;
    for (i = 0; i < size; i+=0x10) {
        printf("%08lX:", i);
        for (j = 0; j < 0x10; j++) {
            printf(" %02X", buffer[i + j]);
        }
        printf(" \r\n");
    }
}

int main(void) {
    uint32_t targetSlot = 0;  // S25FL512S

    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    UART_Start();
    printf("\r\n02: SPI Flash Read/Write\r\n");
    
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

    // Test sequence
    {
        uint8_t readBuffer0[256];
        uint8_t writeBuffer0[256];
//        uint8_t writeBuffer1[256];
        uint8_t readBuffer1[256];
        uint8_t readBuffer2[256];
//        uint8_t readBuffer3[256];
        uint32_t page = 0x000000;

        printf("\r\nRead and Dump a page\r\n");
        
        ReadSerialMemory(targetSlot, page, readBuffer0, sizeof readBuffer0);
        
        DumpBuffer(readBuffer0, sizeof readBuffer0);
        
        printf("\r\nErase, Read, and Dump the page\r\n");
        
        EraseSerialMemory(targetSlot, page);

        ReadSerialMemory(targetSlot, page, readBuffer1, sizeof readBuffer1);
        
        DumpBuffer(readBuffer1, sizeof readBuffer1);
        
        printf("\r\nModify buffer and Write to the page\r\n");

        {
            uint32_t i;
            for (i = 0; (i < sizeof readBuffer0) && (i < sizeof writeBuffer0); i++) {
                writeBuffer0[i] = readBuffer0[i] + i;
            }
        }
        
        WriteSerialMemory(targetSlot, page, writeBuffer0, sizeof writeBuffer0);
        
        DumpBuffer(writeBuffer0, sizeof writeBuffer0);

        printf("\r\nEnter to XIP mode\r\n");

        SMIF_SetMode(CY_SMIF_MEMORY);
        
        printf("\r\nRead, Dump, and Validate the page\r\n");
        
        {
            uint32_t    baseAddress = S25FL512S_SlaveSlot_0.baseAddress;
            uint8_t     *xipArray = (uint8_t *)baseAddress;
            uint32_t    i;
            for (i = 0; i < sizeof readBuffer2; i++) {
                    readBuffer2[i] = xipArray[page + i];
            }
        }
        
        DumpBuffer(readBuffer2, sizeof readBuffer2);
        
        {
            uint32_t i;
            uint32_t failCount = 0;
            for (i = 0; (i < sizeof writeBuffer0) && (i < sizeof readBuffer2); i++) {
                if (writeBuffer0[i] != readBuffer2[i]) {
                    failCount ++;
                }
            }
            if (failCount == 0) {
                printf("\r\nValidated!\r\n");
            } else {
                printf("\r\nFound %lu fails\r\n", failCount);
            }
        }
        
//        printf("\r\nModify buffer\r\n");
//
//        {
//            uint32_t i;
//            for (i = 0; (i < sizeof readBuffer2) && (i < sizeof writeBuffer1); i++) {
//                writeBuffer1[i] = readBuffer2[i] + i;
//            }
//        }
//        
//        printf("\r\nExit from XIP mode\r\n");
//
//        SMIF_SetMode(CY_SMIF_NORMAL);
//        
//        printf("\r\nSet Write Enable\r\n");
//
//        SMIF_Mem_CmdWriteEnable(targetSlot);
//        
//        printf("\r\nEnter to XIP mode\r\n");
//
//        SMIF_SetMode(CY_SMIF_MEMORY);
//        
//        printf("\r\nWrite the page in XIP\r\n");
//        
//        {
//            uint32_t    baseAddress = S25FL512S_SlaveSlot_0.baseAddress;
//            uint8_t     *xipArray = (uint8_t *)baseAddress;
//            uint32_t    i;
//            for (i = 0; i < sizeof writeBuffer1; i++) {
//                    xipArray[page + 0x1000 + i] = writeBuffer1[i];
//            }
//        }
//
//        printf("\r\nExit from XIP mode\r\n");
//
//        SMIF_SetMode(CY_SMIF_NORMAL);
//
//        printf("\r\nRead, Dump, and Validate the page\r\n");
//
//        ReadSerialMemory(targetSlot, page + 0x1000, readBuffer3, sizeof readBuffer3);
//
//        DumpBuffer(readBuffer3, sizeof readBuffer3);
//        
//        {
//            uint32_t i;
//            uint32_t failCount = 0;
//            for (i = 0; (i < sizeof writeBuffer1) && (i < sizeof readBuffer3); i++) {
//                if (writeBuffer1[i] != readBuffer3[i]) {
//                    failCount ++;
//                }
//            }
//            if (failCount == 0) {
//                printf("\r\nValidated!\r\n");
//            } else {
//                printf("\r\nFound %lu fails\r\n", failCount);
//            }
//        }
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

/* [] END OF FILE */
