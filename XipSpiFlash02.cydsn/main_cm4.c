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
#include <string.h>
#include "project.h"

#define     SMIF_PRIORITY       (1u)

#define     SLOT_NUMBER         (2u)  // F-RAM

#define MEM_CMD_WRAR              (0x71) 	/* Write to any one register (SR1, CR1, CR2, CR4, CR5), one byte at a time */
#define MEM_CMD_RDID              (0x9F) 	/* Read ID field */

uint8_t     MLC = 0x00;             /*Sets max latency for memory read at 100 MHz*/                   
uint8_t     RLC = 0x00;             /*Sets max latency for register read at 100 MHz*/

const cy_stc_smif_mem_cmd_t cmdReadIdContent = {
    .command = MEM_CMD_RDID,
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    .mode = 0xFFFFFFFFu,
    .modeWidth = CY_SMIF_WIDTH_SINGLE,
    .dummyCycles = 0,
    .dataWidth = CY_SMIF_WIDTH_SINGLE
};

cy_en_smif_status_t Cy_SMIF_Memslot_CmdReadId(
    SMIF_Type *base,
    cy_stc_smif_mem_config_t const *memDevice,
    uint8_t *buffer,
    cy_stc_smif_context_t *context,
    cy_en_smif_txfr_width_t spiMode
){
    cy_en_smif_status_t result = CY_SMIF_BAD_PARAM;
    cy_en_smif_slave_select_t slaveSelected;

//    cy_stc_smif_mem_device_cfg_t *device = memDevice->deviceCfg;
    cy_stc_smif_mem_cmd_t const *cmdReadId = &cmdReadIdContent;
    
    slaveSelected = (0U == memDevice->dualQuadSlots)?  memDevice->slaveSelect :
                                                    (cy_en_smif_slave_select_t)memDevice->dualQuadSlots;
    /* The write any register command */
    result = Cy_SMIF_TransmitCommand(base,
        (uint8_t)cmdReadId->command, spiMode,
        CY_SMIF_CMD_WITHOUT_PARAM, CY_SMIF_CMD_WITHOUT_PARAM,
        (cy_en_smif_txfr_width_t) CY_SMIF_CMD_WITHOUT_PARAM,
        slaveSelected,
        CY_SMIF_TX_NOT_LAST_BYTE,
        context
    );
    if (result != CY_SMIF_SUCCESS) return result;
    
    result = Cy_SMIF_ReceiveDataBlocking( base, buffer,
        0x07, spiMode, context);
   
    return(result);
}

cy_en_smif_status_t SMIF_Mem_CmdReadId(
    uint32_t slotNumber, 
    uint8_t *buffer,
    cy_en_smif_txfr_width_t spiMode
) {
    return Cy_SMIF_Memslot_CmdReadId(SMIF_HW,
        SMIF_memSlotConfigs[slotNumber],
        buffer,
        &SMIF_context,
        spiMode
    );
}



const cy_stc_smif_mem_cmd_t cmdWriteAnyRegContent = {
    .command = MEM_CMD_WRAR,
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    .mode = 0xFFFFFFFFu,
    .modeWidth = CY_SMIF_WIDTH_SINGLE,
    .dummyCycles = 0,
    .dataWidth = CY_SMIF_WIDTH_SINGLE
};
//const cy_stc_smif_mem_cmd_t *cmdWriteAnyReg = &cmdWriteAnyRegContent;

cy_en_smif_status_t Cy_SMIF_Memslot_CmdWriteAnyRegAnyMode(
    SMIF_Type *base,
    cy_stc_smif_mem_config_t const *memDevice,
    uint8_t const *addr,
    uint8_t value,
    cy_stc_smif_context_t *context,
    cy_en_smif_txfr_width_t spiMode
){
    cy_en_smif_status_t result = CY_SMIF_BAD_PARAM;
    cy_en_smif_slave_select_t slaveSelected;

    cy_stc_smif_mem_device_cfg_t *device = memDevice->deviceCfg;
    cy_stc_smif_mem_cmd_t const *cmdWriteAnyReg = &cmdWriteAnyRegContent;
    
//    CY_ASSERT_L1(NULL != cmdProg);

    if (addr == NULL) return CY_SMIF_BAD_PARAM;
    
    slaveSelected = (0U == memDevice->dualQuadSlots)?  memDevice->slaveSelect :
                                                    (cy_en_smif_slave_select_t)memDevice->dualQuadSlots;
    /* The write any register command */
    result = Cy_SMIF_TransmitCommand(base,
        (uint8_t)cmdWriteAnyReg->command, spiMode,
        addr, device->numOfAddrBytes, spiMode,
        slaveSelected,
        CY_SMIF_TX_NOT_LAST_BYTE,
        context
    );
    if (result != CY_SMIF_SUCCESS) return result;

    if (cmdWriteAnyReg->dummyCycles > 0U) {
        result = Cy_SMIF_SendDummyCycles(base, cmdWriteAnyReg->dummyCycles);
        if (result != CY_SMIF_SUCCESS) return result;
    }

    result = Cy_SMIF_TransmitData(base,
        &value, 1u, spiMode,
        0,
        context
    );
    return result;
}

cy_en_smif_status_t SMIF_Mem_CmdWriteAnyRegAnyMode(
    uint32_t slotNumber, 
    uint8_t const *addr, 
    uint8_t value,
    cy_en_smif_txfr_width_t spiMode
) {
    return Cy_SMIF_Memslot_CmdWriteAnyRegAnyMode(SMIF_HW,
        SMIF_memSlotConfigs[slotNumber],
        addr, value,
        &SMIF_context,
        spiMode
    );
}



/*******************************************************************************
* Function Name: PowerUpMemoryDefaultSPI
****************************************************************************//**
*
* This functions sets the F-RAM to SPI mode.  
* Configures all user registers (status and configuration registers) to factory default.
*
*******************************************************************************/
void SMIF_QIOModeInit (void)
{                                
    // Write CR2 for interface mode
    if (1) {
        uint8_t CR2_Address[3] = {0x00, 0x00, 0x03};  // address 0x000003
        uint8_t CR2_Value;
        
        // Enter to SPI mode with QPI mode
        CR2_Value = 0x00;  // Enter to SPI mode
        SMIF_Mem_CmdWriteAnyRegAnyMode(SLOT_NUMBER, CR2_Address, CR2_Value, CY_SMIF_WIDTH_QUAD);

        // Enter to SPI mode with DPI mode
        CR2_Value = 0x00;  // Enter to SPI mode
        SMIF_Mem_CmdWriteAnyRegAnyMode(SLOT_NUMBER, CR2_Address, CR2_Value, CY_SMIF_WIDTH_DUAL);

//        // Enter to QPI mode with SPI mode
//        CR2_Value = 0x40;  // Enter to QPI mode
//        SMIF_Mem_CmdWriteAnyRegAnyMode(SLOT_NUMBER, CR2_Address, CR2_Value, CY_SMIF_WIDTH_SINGLE);
    }

    // Write CR1 for Memory Latency Code
    if (1) {
        uint8_t CR1_Address[3] = {0x00, 0x00, 0x02};    // address 0x000002
        uint8_t CR1_Value = (MLC<<4)|0x00;              // Set MLC parameter with QUAD
        
        SMIF_Mem_CmdWriteAnyRegAnyMode(SLOT_NUMBER, CR1_Address, CR1_Value, CY_SMIF_WIDTH_SINGLE);
    }

    return;
    
    // Write CR5 for Register Latency Code
    if (1) {
        uint8_t CR5_Address[3] = {0x00, 0x00, 0x06};    // address 0x000006
        uint8_t CR5_Value = (RLC<<6);                   // Set RLC parameter

        SMIF_Mem_CmdWriteAnyRegAnyMode(SLOT_NUMBER, CR5_Address, CR5_Value, CY_SMIF_WIDTH_SINGLE);
    }
}

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

/*******************************************************************************
* Function Name: InitMemory
********************************************************************************
* Summary:
*   This function initializes the SMIF interface.
*
*******************************************************************************/
void InitMemory(void)
{
    cy_en_smif_status_t smif_status;
    
    #if defined(SMIF_SMIF_IRQ__INTC_ASSIGNED)
       (void)Cy_SysInt_Init(&SMIF_SMIF_IRQ_cfg, &SMIF_Interrupt);
    #endif /* defined(SMIF_SMIF_IRQ__INTC_ASSIGNED) */
    
    /* Init the SMIF block */
    smif_status = Cy_SMIF_Init(SMIF_HW, &SMIF_config, 1000u, &SMIF_context);
    if (smif_status != CY_SMIF_SUCCESS) {
        HandleErrorMemory();
    }   
    Cy_SMIF_SetDataSelect(SMIF_HW, CY_SMIF_SLAVE_SELECT_0, CY_SMIF_DATA_SEL0);
    Cy_SMIF_SetDataSelect(SMIF_HW, CY_SMIF_SLAVE_SELECT_2, CY_SMIF_DATA_SEL0);
    Cy_SMIF_Enable(SMIF_HW, &SMIF_context);
    
    /* Enable the SMIF interrupt */
    NVIC_EnableIRQ(smif_interrupt_IRQn);
    
    SMIF_QIOModeInit();
}

/*******************************************************************************
* Function Name: WriteMemory
********************************************************************************
* Summary:
*   This function writes data to the external memory in the quad mode. 
*   The function sends the Quad Page Program: 0x38 command to the external memory. 
*
* Parameters:
*   txBuffer: Data to write in the external memory.
*   txSize: The size of data.
*   address: The address to write data to.   
*
*******************************************************************************/
void WriteMemory(
    uint32_t slotNumber,
    uint8_t txBuffer[], 
    uint32_t txSize, 
    uint32_t address
) {
    cy_en_smif_status_t smif_status;
    uint8_t arrayAddress[3];

    /* Convert 32-bit address to 3-byte array */
    arrayAddress[0] = CY_LO8(address >> 16);
    arrayAddress[1] = CY_LO8(address >> 8);
    arrayAddress[2] = CY_LO8(address);
    
    /* Set QE */    
//    smif_status = SMIF_Mem_QuadEnable(slotNumber);
//    if(smif_status!=CY_SMIF_SUCCESS)
//    {
//        HandleErrorMemory();
//    }
    
    while (SMIF_Mem_IsBusy(slotNumber)) {
        /* Wait till the memory controller command is completed */
        //MEM_DELAY_FUNC;
    }
	
    /* Send Write Enable to external memory */	
    smif_status = SMIF_Mem_CmdWriteEnable(slotNumber);
    if (smif_status!=CY_SMIF_SUCCESS) {
        HandleErrorMemory();
    }
	
	/* Quad Page Program command */       
    smif_status = SMIF_Mem_CmdProgram(slotNumber, arrayAddress, txBuffer, txSize, 0);
    if (smif_status!=CY_SMIF_SUCCESS) {
        HandleErrorMemory();
    }	
        
    while (SMIF_Mem_IsBusy(slotNumber)) {
        /* Wait till the memory controller command is completed */
        //MEM_DELAY_FUNC;
    }
}

/*******************************************************************************
* Function Name: ReadMemory
****************************************************************************//**
* Summary:
*   This function reads data from the external memory in the quad mode. 
*   The function sends the Quad I/O Read: 0xEB command to the external memory. 
*
* Parameters:
*   rxBuffer: The buffer for read data.
*   rxSize: The size of data to read.
*   address: The address to read data from. 
*
*******************************************************************************/
void ReadMemory(
    uint32_t slotNumber,
    uint8_t rxBuffer[], 
    uint32_t rxSize, 
    uint32_t address
) {   
    cy_en_smif_status_t smif_status;
    uint8_t arrayAddress[3];

    /* Convert 32-bit address to 3-byte array */
    arrayAddress[0] = CY_LO8(address >> 16);
    arrayAddress[1] = CY_LO8(address >> 8);
    arrayAddress[2] = CY_LO8(address);
    
//    /* Set QE */		    
//    smif_status = SMIF_Mem_QuadEnable(0);
//    if(smif_status!=CY_SMIF_SUCCESS)
//    {
//        HandleErrorMemory();
//    }
    
    while (SMIF_Mem_IsBusy(slotNumber)) {
        /* Wait till the memory controller command is completed */
//        MEM_DELAY_FUNC;
    }
	
	/* Quad Page read command */    
    smif_status = SMIF_Mem_CmdRead(slotNumber, arrayAddress, rxBuffer, rxSize, 0);
    if (smif_status != CY_SMIF_SUCCESS) {
        HandleErrorMemory();
    }

    while (SMIF_BusyCheck()) {
        /* Wait until the SMIF IP operation is completed. */
//        MEM_DELAY_FUNC;
    }
}

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    UART_Start();
    UART_PutString("Hello World\r\n");

    Cy_SysLib_Delay(1u);
    
    SMIF_Start((cy_stc_smif_block_config_t *)&smifBlockConfig, 1000u);

    SMIF_QIOModeInit();

    {
        char        buf[132];
        cy_en_smif_status_t smif_status;
        uint8_t     value;

        /* Send Write Enable to external memory */	
//        smif_status = SMIF_Mem_CmdWriteEnable(SLOT_NUMBER);
//        if (smif_status!=CY_SMIF_SUCCESS) {
//            HandleErrorMemory();
//        }
        smif_status = SMIF_Mem_CmdReadSts(SLOT_NUMBER, &value, 0x05);
        if (smif_status!=CY_SMIF_SUCCESS) {
            HandleErrorMemory();
        }
        sprintf(buf, "SR1=%02X\r\n", value);
        UART_PutString(buf);
    }
    
    {
        cy_en_smif_status_t smif_status;
        uint8_t     idBuffer[10] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
        smif_status = SMIF_Mem_CmdReadId(SLOT_NUMBER, idBuffer, CY_SMIF_WIDTH_SINGLE);
        if (smif_status!=CY_SMIF_SUCCESS) {
            HandleErrorMemory();
        }
        {
            uint32_t    i;
            char        buf[132];
            sprintf(buf, "ID:");
            for (i = 0; i < sizeof idBuffer; i++) {
                sprintf(buf+strlen(buf), " %02X", idBuffer[i]);
            }
            sprintf(buf+strlen(buf), " \r\n");
            UART_PutString(buf);
        }
    }

    {
        char        buf[132];
        cy_en_smif_status_t smif_status;
        uint8_t     value;

        /* Send Write Enable to external memory */	
        smif_status = SMIF_Mem_CmdWriteEnable(SLOT_NUMBER);
        if (smif_status!=CY_SMIF_SUCCESS) {
            HandleErrorMemory();
        }
        smif_status = SMIF_Mem_CmdReadSts(SLOT_NUMBER, &value, 0x05);
        if (smif_status!=CY_SMIF_SUCCESS) {
            HandleErrorMemory();
        }
        sprintf(buf, "SR1=%02X\r\n", value);
        UART_PutString(buf);
    }
    {
        uint8_t     array[0x100];
        uint32_t    i;
        for (i = 0; i < sizeof array; i++) {
            array[i] = i;
        }
        WriteMemory(SLOT_NUMBER, array, sizeof array, 0x000000);
    }
        
    {
        uint8_t     array[0x100];
        ReadMemory(SLOT_NUMBER, array, sizeof array, 0x000000);
        {
            uint32_t    i;
            uint32_t    j;
            char        buf[132];
            for (i = 0; i < sizeof array; i+=0x10) {
                buf[0] = '\0';
                sprintf(buf+strlen(buf), "%08lX:", i);
                for (j = 0; j < 16; j++) {
                    sprintf(buf+strlen(buf), " %02X", array[i + j]);
                }
                sprintf(buf+strlen(buf), " \r\n");
                UART_PutString(buf);
            }
        }
    }
        
    SMIF_SetMode(CY_SMIF_MEMORY);

    {
        uint32_t    baseAddress = CY15B104QSNQIO_SlaveSlot_2.baseAddress;
        uint32_t    size = 0x0100;
        uint8_t     *array = (uint8_t *)baseAddress;
        uint32_t    i;
        uint32_t    j;
        for (i = 0; i < size; i+=0x10) {
            for (j = 0; j < 16; j++) {
                array[i + j] += j;
            }
        }
    }
    
    {
        uint32_t    baseAddress = CY15B104QSNQIO_SlaveSlot_2.baseAddress;
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
    
    
        
    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
