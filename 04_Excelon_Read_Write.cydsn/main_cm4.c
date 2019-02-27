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

// Op-codes used to control the Excelon device.
#define MEM_CMD_WREN    (0x06) 	/* Write Enable */
#define MEM_CMD_WRAR    (0x71) 	/* Write to any one register, one byte at a time */

// Declare Memory and Register Latency Cycles.
// Refer to QSPI F-RAM (CY15x104QSN)datasheet for details
#define MLC (8u)        /*Sets max latency for memory read at 100 MHz*/                   
#define RLC (3u)        /*Sets max latency for register read at 100 MHz*/


/*******************************************************************************
* Function Name: SMIF_Mem_Cmd{Spi,Dpi,Qpi}WriteEnable
****************************************************************************//**
*
* This function sends the Write Enable command to the memory device.
*
* \note This function uses the low-level Cy_SMIF_TransmitCommand() API.
* The Cy_SMIF_TransmitCommand() API works in a blocking mode. In the dual quad mode,
* this API is called for each memory.
*
* \param slotNumber
* The slot number which is assigned to a target memory device.
*
* \return A status of the command transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_CmdWriteEnableCommon(
    SMIF_Type *base,
    cy_stc_smif_mem_config_t const *memDevice,
    cy_stc_smif_context_t const *context,
    cy_stc_smif_mem_cmd_t const *cmd
){
    cy_en_smif_status_t result = CY_SMIF_BAD_PARAM;
    cy_en_smif_slave_select_t slaveSelected;

    CY_ASSERT_L1(NULL != cmd);  

    slaveSelected =
        (0U == memDevice->dualQuadSlots)?
            memDevice->slaveSelect :
            (cy_en_smif_slave_select_t)memDevice->dualQuadSlots;

    result = Cy_SMIF_TransmitCommand(
        base,
        (uint8_t) cmd->command, cmd->cmdWidth,
        NULL, 0u, (cy_en_smif_txfr_width_t)0u,
        slaveSelected, CY_SMIF_TX_LAST_BYTE,
        context
    );
    return result;
}

//======================================================
//  Implementation for SPI access
//======================================================
const cy_stc_smif_mem_cmd_t cmdSpiWriteEnableContent = {
    .command = MEM_CMD_WREN,
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    .dummyCycles = 0,
    .dataWidth = CY_SMIF_WIDTH_SINGLE,
};

cy_en_smif_status_t SMIF_Mem_CmdSpiWriteEnable(
    uint32_t slotNumber
){
    return Cy_SMIF_Memslot_CmdWriteEnableCommon(
        SMIF_HW,                                    
        SMIF_memSlotConfigs[slotNumber],
        &SMIF_context,
        &cmdSpiWriteEnableContent
    );
}

//======================================================
//  Implementation for DPI access
//======================================================
const cy_stc_smif_mem_cmd_t cmdDpiWriteEnableContent = {
    .command = MEM_CMD_WREN,
    .cmdWidth = CY_SMIF_WIDTH_DUAL,
    .addrWidth = CY_SMIF_WIDTH_DUAL,
    .dummyCycles = 0,
    .dataWidth = CY_SMIF_WIDTH_DUAL,
};

cy_en_smif_status_t SMIF_Mem_CmdDpiWriteEnable(
    uint32_t slotNumber
){
    return Cy_SMIF_Memslot_CmdWriteEnableCommon(
        SMIF_HW,                                    
        SMIF_memSlotConfigs[slotNumber],
        &SMIF_context,
        &cmdDpiWriteEnableContent
    );
}

//======================================================
//  Implementation for QPI access
//======================================================
const cy_stc_smif_mem_cmd_t cmdQpiWriteEnableContent = {
    .command = MEM_CMD_WREN,
    .cmdWidth = CY_SMIF_WIDTH_QUAD,
    .addrWidth = CY_SMIF_WIDTH_QUAD,
    .dummyCycles = 0,
    .dataWidth = CY_SMIF_WIDTH_QUAD,
};

cy_en_smif_status_t SMIF_Mem_CmdQpiWriteEnable(
    uint32_t slotNumber
){
    return Cy_SMIF_Memslot_CmdWriteEnableCommon(
        SMIF_HW,                                    
        SMIF_memSlotConfigs[slotNumber],
        &SMIF_context,
        &cmdQpiWriteEnableContent
    );
}

/*******************************************************************************
* Function Name: SMIF_Mem_Cmd{Spi,Dpi,Qpi}WriteAnyReg
****************************************************************************//**
*
* This function writes one byte (no burst write) to the status or configuration register. 
* The function sends WRAR, 0x71 command to the external memory.
*
* \param slotNumber
* The slot number which is assigned to a target memory device.
*
* \param addr
* The address of the register to write data to.
*
* \param writeBuff
* The buffer to be written to the register.
*
* \param size
* The size of data to be written.  This should be '1'
*
* \param CmdCmpltCb
* Callback function to be invoked when the data written.
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_CmdWriteAnyReg(
    SMIF_Type *base,
    cy_stc_smif_mem_config_t const *memDevice,
    uint8_t const *addr,
    uint8_t* writeBuff,
    uint32_t size,
    cy_smif_event_cb_t cmdCmpltCb,
    cy_stc_smif_context_t *context,
    cy_stc_smif_mem_cmd_t const *cmd,
    cy_stc_smif_mem_cmd_t const *cmdWriteEnable
){
    cy_en_smif_status_t result = CY_SMIF_BAD_PARAM;
    cy_en_smif_slave_select_t slaveSelected;

    cy_stc_smif_mem_device_cfg_t *device = memDevice->deviceCfg;
    
    CY_ASSERT_L1(NULL != cmd);

    if (NULL != addr)
    {
        slaveSelected
            = (0U == memDevice->dualQuadSlots)?
                memDevice->slaveSelect :
                (cy_en_smif_slave_select_t)memDevice->dualQuadSlots;
                
        result = Cy_SMIF_TransmitCommand(
            base,
            (uint8_t) cmdWriteEnable->command, cmdWriteEnable->cmdWidth,
            NULL, 0u, (cy_en_smif_txfr_width_t)0u,
            slaveSelected, CY_SMIF_TX_LAST_BYTE,
            context
        );
        if (CY_SMIF_SUCCESS == result) {
            result = Cy_SMIF_TransmitCommand(
                base,
                (uint8_t)cmd->command, cmd->cmdWidth,
                addr, device->numOfAddrBytes, cmd->addrWidth,
                slaveSelected, CY_SMIF_TX_NOT_LAST_BYTE,
                context
            );
        }
        if((CY_SMIF_SUCCESS == result) && (cmd->dummyCycles > 0U))
        {
            result = Cy_SMIF_SendDummyCycles(base, cmd->dummyCycles);
        }

        if(CY_SMIF_SUCCESS == result)
        {
            result = Cy_SMIF_TransmitData(
                base,
                writeBuff, size, cmd->dataWidth,
                cmdCmpltCb, context
            );
        }
    }

    return(result);
}

//======================================================
//  Implementation for SPI access
//======================================================
const cy_stc_smif_mem_cmd_t cmdSpiWriteAnyRegContent = {
    .command = MEM_CMD_WRAR,
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    .dummyCycles = 0,
    .dataWidth = CY_SMIF_WIDTH_SINGLE,
};

cy_en_smif_status_t SMIF_Mem_CmdSpiWriteAnyReg(
    uint32_t slotNumber, 
    uint8_t const *addr, 
    uint8_t *writeBuff, 
    uint32_t size, 
    cy_smif_event_cb_t CmdCmpltCb
){
    return Cy_SMIF_Memslot_CmdWriteAnyReg(
        SMIF_HW,                                    
        SMIF_memSlotConfigs[slotNumber], 
        addr, 
        writeBuff, 
        size,
        CmdCmpltCb,
        &SMIF_context,
        &cmdSpiWriteAnyRegContent,
        &cmdSpiWriteEnableContent
    );
}

//======================================================
//  Implementation for DPI access
//======================================================
const cy_stc_smif_mem_cmd_t cmdDpiWriteAnyRegContent = {
    .command = MEM_CMD_WRAR,
    .cmdWidth = CY_SMIF_WIDTH_DUAL,
    .addrWidth = CY_SMIF_WIDTH_DUAL,
    .dummyCycles = 0,
    .dataWidth = CY_SMIF_WIDTH_DUAL,
};

cy_en_smif_status_t SMIF_Mem_CmdDpiWriteAnyReg(
    uint32_t slotNumber, 
    uint8_t const *addr, 
    uint8_t *writeBuff, 
    uint32_t size, 
    cy_smif_event_cb_t CmdCmpltCb
){
    return Cy_SMIF_Memslot_CmdWriteAnyReg(
        SMIF_HW,                                    
        SMIF_memSlotConfigs[slotNumber], 
        addr, 
        writeBuff, 
        size,
        CmdCmpltCb,
        &SMIF_context,
        &cmdDpiWriteAnyRegContent,
        &cmdDpiWriteEnableContent
    );
}

//======================================================
//  Implementation for QPI access
//======================================================
const cy_stc_smif_mem_cmd_t cmdQpiWriteAnyRegContent = {
    .command = MEM_CMD_WRAR,
    .cmdWidth = CY_SMIF_WIDTH_QUAD,
    .addrWidth = CY_SMIF_WIDTH_QUAD,
    .dummyCycles = 0,
    .dataWidth = CY_SMIF_WIDTH_QUAD,
};

cy_en_smif_status_t SMIF_Mem_CmdQpiWriteAnyReg(
    uint32_t slotNumber, 
    uint8_t const *addr, 
    uint8_t *writeBuff, 
    uint32_t size, 
    cy_smif_event_cb_t CmdCmpltCb
){
    return Cy_SMIF_Memslot_CmdWriteAnyReg(
        SMIF_HW,                                    
        SMIF_memSlotConfigs[slotNumber], 
        addr, 
        writeBuff, 
        size,
        CmdCmpltCb,
        &SMIF_context,
        &cmdQpiWriteAnyRegContent,
        &cmdQpiWriteEnableContent
    );
}

/*******************************************************************************
* Function Name: InitializeQioMode
****************************************************************************//**
*
* This functions sets the F-RAM to QPI mode.  
* Configures all user registers (status and configuration registers).
*
*******************************************************************************/
cy_en_smif_status_t InitializeQioMode(
    uint32_t slotNumber
){                                
    // Write CR2 for interface mode
    {
        uint8_t CR2_Address[3] = {0x00, 0x00, 0x03};  // address 0x000003
        uint8_t CR2_Parameter[1] = {0x00};  // Enter to SPI mode
        
        // Try QPI mode
        SMIF_Mem_CmdQpiWriteAnyReg(slotNumber, CR2_Address, CR2_Parameter, sizeof CR2_Parameter, 0);

        // Try DPI mode
        SMIF_Mem_CmdDpiWriteAnyReg(slotNumber, CR2_Address, CR2_Parameter, sizeof CR2_Parameter, 0);
    }

    // Write CR1 for Memory Latency Code
    {
        uint8_t CR1_Address[3] = {0x00, 0x00, 0x02};  // address 0x000002
        uint8_t CR1_Parameter[1] = {(MLC<<4)|0x02};  // Set MLC parameter with QUAD

        
        cy_en_smif_status_t status = SMIF_Mem_CmdSpiWriteAnyReg(
            slotNumber, CR1_Address, CR1_Parameter, sizeof CR1_Parameter, 0
        );
        if (status != CY_SMIF_SUCCESS) {
            return status;
        }
    }

    // Write CR5 for Register Latency Code
    {
        uint8_t CR5_Address[3] = {0x00, 0x00, 0x06};  // address 0x000006
        uint8_t CR5_Parameter[1] = {(RLC<<6)};  // Set RLC parameter

        cy_en_smif_status_t status = SMIF_Mem_CmdSpiWriteAnyReg(
            slotNumber, CR5_Address, CR5_Parameter, sizeof CR5_Parameter, 0
        );
        if (status != CY_SMIF_SUCCESS) {
            return status;
        }
    }
    return CY_SMIF_SUCCESS;
}

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

#define BUFFER_SIZE (256)


/*******************************************************************************
* Function Name: main
********************************************************************************/
int main(void) {
    uint32_t targetSlot = 2;  // CY15B104QSN

    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    UART_Start();
    printf("\r\n04: Excelon Read/Write\r\n");
    
    // Initialize SMIF as QUAD I/O
    SMIF_Start((cy_stc_smif_block_config_t *)&smifBlockConfig, 1000u);
    // Workaround for unimplemented MODE byte description
    SMIF_memSlotConfigs[targetSlot]->deviceCfg->programCmd->dummyCycles = 2;
    // Initialize the operation mode
    {
        cy_en_smif_status_t status;

        status = InitializeQioMode(targetSlot);
        if (status != CY_SMIF_SUCCESS) {
            printf("\r\nQuad I/O Initialization failed.\r\n");
            for (;;) ;
        }
    }

    // Test sequence
    {
        uint8_t readBuffer0[BUFFER_SIZE];
        uint8_t writeBuffer0[BUFFER_SIZE];
        uint8_t readBuffer2[BUFFER_SIZE];
        uint32_t page = 0x034567;

        printf("\r\nRead and Dump a page\r\n");
        
        ReadSerialMemory(targetSlot, page, readBuffer0, sizeof readBuffer0);
        
        DumpBuffer(readBuffer0, sizeof readBuffer0);
        
        printf("\r\nModify buffer and Write to the page\r\n");

        {
            uint32_t i;
            for (i = 0; i < BUFFER_SIZE; i++) {
                writeBuffer0[i] = readBuffer0[i] + i + 53;
            }
        }
        
        DumpBuffer(writeBuffer0, sizeof writeBuffer0);

        WriteSerialMemory(targetSlot, page, writeBuffer0, sizeof writeBuffer0);
       

        printf("\r\nRead, Dump, and Validate the page\r\n");
        
        ReadSerialMemory(targetSlot, page, readBuffer2, sizeof readBuffer2);
        
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
