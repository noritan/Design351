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
#include "stdio_user.h"
#include <stdio.h>

void GPIO_Toggle(void);

int main(void)
{
    /* SMIF interrupt initialization status */
    cy_en_sysint_status_t intr_init_status;
    
    /* SMIF status */
    cy_en_smif_status_t smif_status;    
    
    UART_Start();
    UART_PutString("UART Started\r\n");
    
    Cy_GPIO_SetDrivemode(GPIO_PRT0, 3, CY_GPIO_DM_STRONG_IN_OFF);
    
    __enable_irq(); /* Enable global interrupts. */
    
    SMIF_1_Start((cy_stc_smif_block_config_t *)&smifBlockConfig, 100);
    
    smif_status = Cy_SMIF_Memslot_QuadEnable(SMIF_1_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_1_context);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF Cy_SMIF_Memslot_QuadEnable failed\r\n");
    }
    
    while(Cy_SMIF_Memslot_IsBusy(SMIF_1_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_1_context))
    {
        /* Wait until the Erase operation is completed */
    }
    
    Cy_SMIF_SetMode(SMIF_1_HW, CY_SMIF_MEMORY);

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        GPIO_Toggle();
    }
}

CY_SECTION (".cy_xip") __USED void GPIO_Toggle(void)
{
    Cy_GPIO_Inv(GPIO_PRT0, 3);
    Cy_SysLib_Delay(500);
}

/* [] END OF FILE */
