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

#include "project.h"

cy_smif_event_cb_t RxCmpltCallback;
cy_en_smif_slave_select_t TARGET_SS = CY_SMIF_SLAVE_SELECT_2;
const uint32_t TARGET_SLOT = 2;

/* SPIM Access Modes */
//#define SPI_MODE                  (0u)      /* Transmits in SPI SDR mode */
//#define DPI_MODE                  (1u)      /* Transmits in DPI SDR mode */ 
//#define QPI_MODE                  (2u)      /* Transmits in QPI SDR mode */ 

#define DID_REG_SIZE      	      (8u)	     /* Device ID status register size */

//#define ADDRESS_SIZE      	      (3u)	    /* F-RAM memory address size */

#define MEM_CMD_WREN              (0x06) 	/* Write Enable */
#define MEM_CMD_RDID        (0x9F) 	/* Read device ID */
#define MEM_CMD_WRAR              (0x71) 	/* Write to any one register (SR1, CR1, CR2, CR4, CR5), one byte at a time */

#define CMD_WITHOUT_PARAM   (0u)    
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
* Function Name: WriteCmdWREN
****************************************************************************//**
*
* This function enables the Write bit in the status register. 
* The function sends the WREN 0x06 command to the external memory.
*
* \param baseaddr
* Holds the base address of the SMIF block registers.
*
* \param smifContext
* The internal SMIF context data.
*
* \param spimode
* Determines SMIF width single,dual,or quad.
*
*******************************************************************************/
//void WriteCmdWREN(SMIF_Type *baseaddr,
//                            cy_stc_smif_context_t *smifContext,
//                            uint8_t spimode)
//{    
//    cy_en_smif_txfr_width_t CY_SMIF_WIDTH = CY_SMIF_WIDTH_SINGLE;
//    
//    if (spimode==SPI_MODE)
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_SINGLE;
//     else if
//       (spimode==DPI_MODE)
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_DUAL;    
//     else if
//       (spimode==QPI_MODE)
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_QUAD;   
//    else
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_SINGLE;     
//    
//    /* Transmit command */
//    Cy_SMIF_TransmitCommand( baseaddr,
//                            MEM_CMD_WREN,				  
//                            CY_SMIF_WIDTH,
//                            CMD_WITHOUT_PARAM, 
//                            CMD_WITHOUT_PARAM, 
//                            CY_SMIF_WIDTH, 
//                            TARGET_SS, 
//                            TX_LAST_BYTE, 
//                            smifContext);     
//        
//    /* Check if the SMIF IP is busy */
//    while(Cy_SMIF_BusyCheck(baseaddr))
//    {
//        /* Wait until the SMIF IP operation is completed. */
//    }
//}

/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_CmdWriteEnable
****************************************************************************//**
*
* This function sends the Write Enable command to the memory device.
*
* \note This function uses the low-level Cy_SMIF_TransmitCommand() API.
* The Cy_SMIF_TransmitCommand() API works in a blocking mode. In the dual quad mode,
* this API is called for each memory.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param context
* The internal SMIF context data. \ref cy_stc_smif_context_t
*
* \param memDevice
* The device to which the command is sent.
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

    //cy_stc_smif_mem_device_cfg_t *device = memDevice->deviceCfg;

    /* The memory Write Enable */
    //cy_stc_smif_mem_cmd_t* writeEn = memDevice->deviceCfg->writeEnCmd;
    
    CY_ASSERT_L1(NULL != cmd);  

    slaveSelected = (0U == memDevice->dualQuadSlots)?  memDevice->slaveSelect :
                                            (cy_en_smif_slave_select_t)memDevice->dualQuadSlots;

    result = Cy_SMIF_TransmitCommand(
        base,
        (uint8_t) cmd->command, cmd->cmdWidth,
        CY_SMIF_CMD_WITHOUT_PARAM, CY_SMIF_CMD_WITHOUT_PARAM, (cy_en_smif_txfr_width_t)CY_SMIF_CMD_WITHOUT_PARAM,
        slaveSelected, CY_SMIF_TX_LAST_BYTE,
        context
    );
    return result;
}

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
* Function Name: WriteCmdSPIWriteAnyReg
****************************************************************************//**
*
* This function writes one byte (no burst write) to the status or configuration register. 
* The function sends WRAR, 0x71 command to the external memory.
*
* \param baseaddr
* Holds the base address of the SMIF block registers.
*
* \param smifContext
* The internal SMIF context data.
*
* \param tst_txBuffer 
* Data to write in the external memory.
* 
* \param txSize 
* The size of data.
* 
* \param address 
* The address to write data to.  
*
* \param spimode
* Determines SMIF width single,dual,or quad.
*
*******************************************************************************/
//void WriteCmdSPIWriteAnyReg(SMIF_Type *baseaddr, 
//                    cy_stc_smif_context_t *smifContext, 
//                    uint8_t tst_txBuffer[], 
//                    uint32_t txSize, 
//                    uint8_t *address,
//                    uint8_t spimode)
//
//{         
//    cy_en_smif_txfr_width_t CY_SMIF_WIDTH = CY_SMIF_WIDTH_SINGLE;
//    
//    if (spimode==SPI_MODE)
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_SINGLE;
//     else if
//       (spimode==DPI_MODE)
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_DUAL;    
//     else if
//      (spimode==QPI_MODE)
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_QUAD;  
//    else
//     CY_SMIF_WIDTH = CY_SMIF_WIDTH_SINGLE;
//    
//    /* Set the write enable (WEL) bit in SR1 */
//    WriteCmdWREN(baseaddr, smifContext, spimode);
//    //SMIF_Mem_CmdWriteEnable(TARGET_SLOT);
//    
//    /* Transmit command */
//	Cy_SMIF_TransmitCommand( baseaddr,
//                            MEM_CMD_WRAR,		
//                            CY_SMIF_WIDTH,
//                            address, 
//                            ADDRESS_SIZE, 
//                            CY_SMIF_WIDTH, 
//                            TARGET_SS, 
//                            TX_NOT_LAST_BYTE, 
//                            smifContext);
//    /* Transmit data */
//    Cy_SMIF_TransmitData(baseaddr, 
//                            tst_txBuffer,
//                            txSize, 
//                            CY_SMIF_WIDTH, 
//                            RxCmpltCallback, 
//                            smifContext);
//    
//    /* Check if the SMIF IP is busy */
//   while(Cy_SMIF_BusyCheck(baseaddr))
//    {
//        /* Wait until the SMIF IP operation is completed */
//    }
//}

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

    if ((NULL != addr) /*&& (size <= device->programSize)*/)
    {
        slaveSelected = (0U == memDevice->dualQuadSlots)?  memDevice->slaveSelect :
                                                        (cy_en_smif_slave_select_t)memDevice->dualQuadSlots;
        result = Cy_SMIF_TransmitCommand(
            base,
            (uint8_t) cmdWriteEnable->command, cmdWriteEnable->cmdWidth,
            CY_SMIF_CMD_WITHOUT_PARAM, CY_SMIF_CMD_WITHOUT_PARAM, (cy_en_smif_txfr_width_t)CY_SMIF_CMD_WITHOUT_PARAM,
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
* Function Name: WriteCmdRDID
****************************************************************************//**
*
* This function reads 8-byte device ID from the external memory
* The function sends the RDID, 0x9F command to the external memory.
*
* \param baseaddr
* Holds the base address of the SMIF block registers.
*
* \param smifContext
* The internal SMIF context data.
*
* \param tst_rxBuffer 
* The buffer for read data.
* 
* \param spimode
* Determines SMIF width single,dual,or quad.
*
* \param latency
* Register latency cycle during register read.
*
*******************************************************************************/
//void WriteCmdRDID(SMIF_Type *baseaddr,
//                    cy_stc_smif_context_t *smifContext,
//                     uint8_t tst_rxBuffer[],
//                    uint8_t spimode,
//                    uint8_t latency)
//{
//    cy_en_smif_txfr_width_t CY_SMIF_WIDTH = CY_SMIF_WIDTH_SINGLE;
//    
//    if (spimode==SPI_MODE)
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_SINGLE;
//     else if
//       (spimode==DPI_MODE)
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_DUAL;    
//     else if
//       (spimode==QPI_MODE)
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_QUAD;   
//    else
//       CY_SMIF_WIDTH = CY_SMIF_WIDTH_SINGLE; 
//    
//    /* Transmit command */
//    Cy_SMIF_TransmitCommand( baseaddr,
//                            MEM_CMD_RDID,				
//                            CY_SMIF_WIDTH,
//                            CMD_WITHOUT_PARAM, 
//                            CMD_WITHOUT_PARAM, 
//                            CY_SMIF_WIDTH, 
//                            TARGET_SS, 
//                            TX_NOT_LAST_BYTE, 
//                            smifContext);
//    
//    /* Sends extra dummy clocks to adde clock cycle latency */
//    Cy_SMIF_SendDummyCycles(baseaddr, (uint32_t)latency);
//    
//    /* Receive data */
//    Cy_SMIF_ReceiveData( baseaddr,
//                        tst_rxBuffer, 					
//                        DID_REG_SIZE, 
//                        CY_SMIF_WIDTH, 
//                        RxCmpltCallback,
//                        smifContext);       
//  
//    /* Check if the SMIF IP is busy */
//    while(Cy_SMIF_BusyCheck(baseaddr))
//    {
//        /* Wait until the SMIF IP operation is completed */
//    }
//}

cy_en_smif_status_t Cy_SMIF_Memslot_CmdReadId(
    SMIF_Type *base,
    cy_stc_smif_mem_config_t const *memDevice,
    uint8_t* readBuff,
    uint32_t size,
    cy_smif_event_cb_t cmdCmpltCb,
    cy_stc_smif_context_t *context,
    cy_stc_smif_mem_cmd_t const *cmd
){
    cy_en_smif_status_t result = CY_SMIF_BAD_PARAM;
    cy_en_smif_slave_select_t slaveSelected;
    //cy_stc_smif_mem_device_cfg_t *device = memDevice->deviceCfg;
//    cy_stc_smif_mem_cmd_t *cmdRead = device->readCmd;

    {
        slaveSelected = (0U == memDevice->dualQuadSlots)?  memDevice->slaveSelect :
                               (cy_en_smif_slave_select_t)memDevice->dualQuadSlots;

        result = Cy_SMIF_TransmitCommand(
            base,
            (uint8_t)cmd->command, cmd->cmdWidth,
            (void *)0, CY_SMIF_CMD_WITHOUT_PARAM, cmd->addrWidth,
            slaveSelected, CY_SMIF_TX_NOT_LAST_BYTE,
            context
        );

        if((CY_SMIF_SUCCESS == result) && (0U < cmd->dummyCycles))
        {
            result = Cy_SMIF_SendDummyCycles(base, cmd->dummyCycles);
        }

        if((CY_SMIF_SUCCESS == result) && (CY_SMIF_NO_COMMAND_OR_MODE != cmd->mode))
        {
            result = Cy_SMIF_TransmitCommand(
                base,
                (uint8_t)cmd->mode, cmd->dataWidth,
                CY_SMIF_CMD_WITHOUT_PARAM, CY_SMIF_CMD_WITHOUT_PARAM, (cy_en_smif_txfr_width_t) CY_SMIF_CMD_WITHOUT_PARAM,
                slaveSelected, CY_SMIF_TX_NOT_LAST_BYTE,
                context
            );
        }

        if(CY_SMIF_SUCCESS == result)
        {
            result = Cy_SMIF_ReceiveData(
                base,
                readBuff, size, cmd->dataWidth,
                cmdCmpltCb, context
            );
        }
    }

    return(result);
}

const cy_stc_smif_mem_cmd_t cmdSpiReadIdContent = {
    .command = MEM_CMD_RDID,
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    .dummyCycles = RLC,
    .mode = CY_SMIF_NO_COMMAND_OR_MODE,
    .dataWidth = CY_SMIF_WIDTH_SINGLE,
};

cy_en_smif_status_t SMIF_Mem_CmdSpiReadId(
    uint32_t slotNumber, 
    uint8_t *readBuff, 
    uint32_t size, 
    cy_smif_event_cb_t CmdCmpltCb    
){
    return Cy_SMIF_Memslot_CmdReadId(
        SMIF_HW,                                    
        SMIF_memSlotConfigs[slotNumber], 
        readBuff, 
        size,
        CmdCmpltCb,
        &SMIF_context,
        &cmdSpiReadIdContent
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

//void PowerUpMemoryDefaultSPI (void)
//{                                
//     uint8_t SR_CR_RegDefault[5] = {0x00, ((MLC<<4)|0x02), 0x00, 0x08, (RLC<<6)}; /*{ SR1, CR1, CR2, CR4, CR5 }*/
//      
//    /* Burst write configuration registers in SPI mode using WRR */   
//    WriteCmdWRSR (SMIF0, &smifContext,&SR_CR_RegDefault[0], 0x05, SPI_MODE);
//        
//    /* Burst write configuration registers in DPI mode using WRR */
//    WriteCmdWRSR (SMIF0, &smifContext,&SR_CR_RegDefault[0], 0x05, DPI_MODE);
//    
//    /* Burst write configuration registers in QPI mode using WRR */
//    WriteCmdWRSR (SMIF0, &smifContext,&SR_CR_RegDefault[0], 0x05, QPI_MODE); 
//}    

void PowerUpMemoryDefaultSPI (void)
{                                
    // Write CR2 for interface mode
    {
        uint8_t CR2_Address[3] = {0x00, 0x00, 0x03};  // address 0x000003
        uint8_t CR2_Parameter[1] = {0x00};  // Enter to SPI mode
        
        // Try QPI mode
//        WriteCmdSPIWriteAnyReg(SMIF_HW, &SMIF_context, CR2_Parameter, sizeof CR2_Parameter, CR2_Address, QPI_MODE);
        SMIF_Mem_CmdQpiWriteAnyReg(TARGET_SLOT, CR2_Address, CR2_Parameter, sizeof CR2_Parameter, 0);

        // Try DPI mode
//        WriteCmdSPIWriteAnyReg(SMIF_HW, &SMIF_context, CR2_Parameter, sizeof CR2_Parameter, CR2_Address, DPI_MODE);
        SMIF_Mem_CmdDpiWriteAnyReg(TARGET_SLOT, CR2_Address, CR2_Parameter, sizeof CR2_Parameter, 0);
    }

    // Write CR1 for Memory Latency Code
    {
        uint8_t CR1_Address[3] = {0x00, 0x00, 0x02};  // address 0x000003
        uint8_t CR1_Parameter[1] = {(MLC<<4)|0x02};  // Set MLC parameter with QUAD
        
//        WriteCmdSPIWriteAnyReg(SMIF_HW, &SMIF_context, CR1_Parameter, sizeof CR1_Parameter, CR1_Address, SPI_MODE);
        SMIF_Mem_CmdSpiWriteAnyReg(TARGET_SLOT, CR1_Address, CR1_Parameter, sizeof CR1_Parameter, 0);
    }

    // Write CR5 for Register Latency Code
    {
        uint8_t CR5_Address[3] = {0x00, 0x00, 0x06};  // address 0x000003
        uint8_t CR5_Parameter[1] = {(RLC<<6)};  // Set RLC parameter

//        WriteCmdSPIWriteAnyReg(SMIF_HW, &SMIF_context, CR5_Parameter, sizeof CR5_Parameter, CR5_Address, SPI_MODE);
        SMIF_Mem_CmdSpiWriteAnyReg(TARGET_SLOT, CR5_Address, CR5_Parameter, sizeof CR5_Parameter, 0);
    }
}

/*******************************************************************************
* Function Name: PrintData
****************************************************************************//**
*
* This function prints the content of the TX buffer to the UART console.
* 
* \param  tst_txBuffer - the buffer to output to the console
* 
* \param  size -  the size of the buffer to output to the console
*
*******************************************************************************/
void PrintData(uint8_t * tst_txBuffer, uint32_t size)
{
    uint32_t index;
   
    for(index=0; index<size; index++)
    {
        printf("0x%02X ", (unsigned int) tst_txBuffer[index]);
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

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    UART_Start();
    printf("Hello World\r\n");
    
    SMIF_Start((cy_stc_smif_block_config_t *)&smifBlockConfig, 1000u);
    
    PowerUpMemoryDefaultSPI();
    
    {
        uint8_t rxBuffer[16];
        SMIF_Mem_CmdSpiReadId(TARGET_SLOT, &rxBuffer[0], DID_REG_SIZE, 0);
        printf("\r\nRead Device ID (RDID 0x9F)\r\n ");
        PrintData(&rxBuffer[0], DID_REG_SIZE);
        printf("\r\n");
        fflush(stdout);
    }
    

    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
