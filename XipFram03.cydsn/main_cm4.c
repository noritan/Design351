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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "project.h"

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
*   This function processes unrecoverable errors such as UART component 
*   initialization error or SMIF initialization error etc. In case of such error 
*   the system will Turn on RED_LED_ERROR and stay in a infinite loop of 
*   this function.
*
*******************************************************************************/
void HandleErrorMemory(void)
{
     /* Disable all interrupts */
    __disable_irq();
	
    /* Halt here */
    while(1u) {}
}


const uint32_t targetSlot = 2;

#define MEM_CMD_WREN              (0x06) 	/* Write Enable */
#define MEM_CMD_WRAR              (0x71) 	/* Write to any one register (SR1, CR1, CR2, CR4, CR5), one byte at a time */

#define TX_LAST_BYTE       	(1u) 	/* The last byte in command transmission 
									* (SS is set to high after transmission) 
									*/
#define TX_NOT_LAST_BYTE    (0u) 	/* Not the last byte in command transmission 
									* (SS remains low after transmission) 
									*/

#define MLC (8u)                       /*Sets max latency for memory read at 100 MHz*/                   
#define RLC (3u)                       /*Sets max latency for register read at 100 MHz*/
                                         /*Refer to QSPI F-RAM (CY15x104QSN)datasheet for details*/  

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
void InitializeQioMode(
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
        
        SMIF_Mem_CmdSpiWriteAnyReg(slotNumber, CR1_Address, CR1_Parameter, sizeof CR1_Parameter, 0);
    }

    // Write CR5 for Register Latency Code
    {
        uint8_t CR5_Address[3] = {0x00, 0x00, 0x06};  // address 0x000006
        uint8_t CR5_Parameter[1] = {(RLC<<6)};  // Set RLC parameter

        SMIF_Mem_CmdSpiWriteAnyReg(slotNumber, CR5_Address, CR5_Parameter, sizeof CR5_Parameter, 0);
    }
}

/*******************************************************************************
* Function Name: main
********************************************************************************/
int main(void)
{
    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    UART_Start();
    printf("\r\nSieve of Eratosthenes\r\n");
    
    SMIF_Start((cy_stc_smif_block_config_t *)&smifBlockConfig, 1000u);
    
    InitializeQioMode(targetSlot);
        
    // Set Write Enable for writable XIP operations.
//    SMIF_Mem_CmdSpiWriteEnable(TARGET_SLOT);
    SMIF_Mem_CmdWriteEnable(targetSlot);
    
    // Enter to XIP mode
    SMIF_SetMode(CY_SMIF_MEMORY);

    {
        uint32_t    baseAddress = CY15B104QSNQIO_SlaveSlot_2.baseAddress;
        uint32_t    size = CY15B104QSNQIO_SlaveSlot_2.memMappedSize;
        uint8_t     *array = (uint8_t *)baseAddress;
        uint32_t    primaryPrime = 2;
        uint32_t    i;
        uint32_t    j;
        uint32_t    n;
        uint64_t    sum;
        
        // Set all array bytes
        memset(array, 1, size);
        
        // Sieve
        for (i = primaryPrime; i < size; i++) {
            if (array[i]) {
                // If 'i' is a prime
                for (j = i + i; j < size; j+=i) {
                    // Mark as non-prime
                    array[j] = 0;
                }
            }
        }
        
        // Summarize all prime
        n = 0;
        sum = 0;
        for (i = primaryPrime; i < size; i++) {
            if (array[i]) {
//                printf("%lu\r\n", i);
                n++;
                sum += i;
            }
        }
        printf("\r\nTotal %lu primes found.\r\n", n);
        printf("Checksum %08lX%08lX\r\n", (uint32_t)(sum>>32), (uint32_t)sum);
        
    }

    // Flush STDOUT buffer
    fflush(stdout);
    
    for(;;)
    {
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
