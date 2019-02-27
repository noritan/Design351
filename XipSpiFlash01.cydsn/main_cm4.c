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
    
    /* Configure SMIF interrupt */
//    cy_stc_sysint_t smifIntConfig =
//    {
//        .intrSrc = smif_interrupt_IRQn,     /* SMIF interrupt */
//        .intrPriority = SMIF_PRIORITY       /* SMIF interrupt priority */
//    };    
//    Cy_SysInt_Init(&smifIntConfig, SMIF_Interrupt);

    #if defined(SMIF_SMIF_IRQ__INTC_ASSIGNED)
       (void)Cy_SysInt_Init(&SMIF_SMIF_IRQ_cfg, &SMIF_Interrupt);
    #endif /* defined(SMIF_SMIF_IRQ__INTC_ASSIGNED) */
    
    /* Init the SMIF block */
    smif_status = Cy_SMIF_Init(SMIF_HW, &SMIF_config, 1000u, &SMIF_context);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        HandleErrorMemory();
    }   
    Cy_SMIF_SetDataSelect(SMIF_HW, CY_SMIF_SLAVE_SELECT_0, CY_SMIF_DATA_SEL0);
    Cy_SMIF_Enable(SMIF_HW, &SMIF_context);  
    
    /* Enable the SMIF interrupt */
    NVIC_EnableIRQ(smif_interrupt_IRQn);
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
void WriteMemory(uint8_t txBuffer[], 
                    uint32_t txSize, 
                    uint32_t address)
{
    cy_en_smif_status_t smif_status;
    uint8_t arrayAddress[3];

    /* Convert 32-bit address to 3-byte array */
    arrayAddress[0] = CY_LO8(address >> 16);
    arrayAddress[1] = CY_LO8(address >> 8);
    arrayAddress[2] = CY_LO8(address);
    
    /* Set QE */    
//    smif_status = Cy_SMIF_Memslot_QuadEnable(SMIF_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_context);
    smif_status = SMIF_Mem_QuadEnable(0);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        HandleErrorMemory();
    }
    
//    while(Cy_SMIF_Memslot_IsBusy(SMIF_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_context))
    while (SMIF_Mem_IsBusy(0))
    {
        /* Wait till the memory controller command is completed */
        //MEM_DELAY_FUNC;
    }
	
    /* Send Write Enable to external memory */	
//    smif_status = Cy_SMIF_Memslot_CmdWriteEnable(SMIF_HW, smifMemConfigs[0], &SMIF_context);
    smif_status = SMIF_Mem_CmdWriteEnable(0);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        HandleErrorMemory();
    }
	
	/* Quad Page Program command */       
//    smif_status = Cy_SMIF_Memslot_CmdProgram(SMIF_HW, smifMemConfigs[0], arrayAddress, txBuffer, txSize, 0, &SMIF_context);
    smif_status = SMIF_Mem_CmdProgram(0, arrayAddress, txBuffer, txSize, 0);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        HandleErrorMemory();
    }	
        
//    while(Cy_SMIF_Memslot_IsBusy(SMIF_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_context))
    while (SMIF_Mem_IsBusy(0))
    {
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
void ReadMemory(uint8_t rxBuffer[], 
                            uint32_t rxSize, 
                            uint32_t address)
{   
    cy_en_smif_status_t smif_status;
    uint8_t arrayAddress[3];

    /* Convert 32-bit address to 3-byte array */
    arrayAddress[0] = CY_LO8(address >> 16);
    arrayAddress[1] = CY_LO8(address >> 8);
    arrayAddress[2] = CY_LO8(address);
    
    /* Set QE */		    
//    smif_status = Cy_SMIF_Memslot_QuadEnable(SMIF_1_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_1_context);
    smif_status = SMIF_Mem_QuadEnable(0);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        HandleErrorMemory();
    }
    
//    while(Cy_SMIF_Memslot_IsBusy(SMIF_1_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_1_context))
    while (SMIF_Mem_IsBusy(0))
    {
        /* Wait till the memory controller command is completed */
//        MEM_DELAY_FUNC;
    }
	
	/* The 4 Page program command */    
//    smif_status = Cy_SMIF_Memslot_CmdRead(SMIF_1_HW, smifMemConfigs[0], arrayAddress, rxBuffer, rxSize, &RxCmpltMemoryCallback, &SMIF_1_context);
    smif_status = SMIF_Mem_CmdRead(0, arrayAddress, rxBuffer, rxSize, 0);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        HandleErrorMemory();
    }
    
//    while(Cy_SMIF_BusyCheck(SMIF_1_HW))
    while (SMIF_BusyCheck())
    {
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

    SMIF_Start((cy_stc_smif_block_config_t *)&smifBlockConfig, 1000u);

    {
        uint8_t     array[0x100];
        uint32_t    i;
        for (i = 0; i < sizeof array; i++) {
            array[i] = i;
        }
        WriteMemory(array, sizeof array, 0x004000);
    }
        
    {
        uint8_t     array[0x100];
        ReadMemory(array, sizeof array, 0x004000);
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
        uint32_t    baseAddress = S25FL512S_SlaveSlot_0.baseAddress;
        uint32_t    size = 0x0100;//S25FL512S_SlaveSlot_0.memMappedSize;
        uint8_t     *array = (uint8_t *)baseAddress;
        uint32_t    i;
        uint32_t    j;
        for (i = 0; i < size; i+=0x10) {
            for (j = 0; j < 16; j++) {
                array[0x4000 + i + j] += j;
            }
        }
    }
    
    {
        uint32_t    baseAddress = S25FL512S_SlaveSlot_0.baseAddress;
        uint32_t    size = 0x0100;//S25FL512S_SlaveSlot_0.memMappedSize;
        uint8_t     *array = (uint8_t *)baseAddress;
        uint32_t    i;
        uint32_t    j;
        char        buf[132];
        for (i = 0; i < size; i+=0x10) {
            buf[0] = '\0';
            sprintf(buf+strlen(buf), "%08lX:", baseAddress + i);
            for (j = 0; j < 16; j++) {
                sprintf(buf+strlen(buf), " %02X", array[0x4000 + i + j]);
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
