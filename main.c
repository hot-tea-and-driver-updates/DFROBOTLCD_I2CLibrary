/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Empty PSoC6 Application
*              for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#include "DFR0554.h"
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cy_scb_uart.h"
#include "cy_device_headers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
/*Global*/
cy_stc_scb_i2c_context_t I2C_context;
int blink = 0;
int cursor = 0;
int display = 1;

/* Assign pins for UART on SCB5: P5[0], P5[1] */
#define UART_PORT       P5_0_PORT
#define UART_RX_NUM     P5_0_NUM
#define UART_TX_NUM     P5_1_NUM

/* Assign divider type and number for UART */
#define UART_CLK_DIV_TYPE     (CY_SYSCLK_DIV_16_BIT)
#define UART_CLK_DIV_NUMBER   (11U)

//function prototypes
void I2C_Scan();
void print_Usage();
void Display();
void Blink();
void Cursor();
void Move_Cursor(uint32_t column,uint32_t row);
void CustomColor();


int main(void)
{
    cy_rslt_t result;
    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    __enable_irq();
    UARTsetup();
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                     CY_RETARGET_IO_BAUDRATE);

    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("Initializing UART communication... \r\n\n");
    printf("CSE121 Lab 3.2 DFR0554 Library\r\n\n");

    setvbuf( stdin, NULL, _IONBF, 0 );

    result = Cy_SCB_I2C_Init(I2C_HW, &I2C_config,&I2C_context);
    if (result != CY_RSLT_SUCCESS)
    {
    	printf("I2C unsuccessful, unable to start LCD...\r\n");
    	return false;
        CY_ASSERT(0);
    }else{

    	printf("I2C successfully started..\r\n");
    }
    Cy_SCB_I2C_Enable(I2C_HW);

    I2C_Scan();
    printf("\r\n\n");
    I2C_Scan();
    printf("\r\n\n");
    I2C_Scan();
	printf("Scan complete.\r\n\n");


    LCD_Start(I2C_HW, &I2C_context);
    printf("Completed LCD initialization..\r\n\n");
    char c;
    printf("LCD can be controlled via key entries: \r\n\n");
    print_Usage();
    uint32_t column;
    uint32_t row;
    LCD_Print("CSE121 Lab 3.2");
    LCD_SetCursor(0,1);
    LCD_Print("47");
    for(;;){
    	c = getchar();
    	if(c == 'd'){
    	    printf("\x1b[2J\x1b[;H");
    	    print_Usage();
    	}
    	if(c == 'h'){
    		print_Usage();
    	}
    	else if(c == 'o'){
    		Display();
    	}
    	else if(c == 'b'){
    		Blink();
    	}
    	else if(c == 'c'){
    		Cursor();
    	}
    	else if(c == 'm'){
    		printf("Set Column and Row (0 <= Column <= 16 & 0 <= Row <= 1\r\n");
    		printf("Enter column: ");
    		scanf("%lu",&column);
    		printf("\r\n");
    		printf("Enter row: ");
    		scanf("%lu",&row);
    		printf("\r\n");
    		Move_Cursor(column, row);
    	}
    	else if(c == 'l'){
    		printf("Scrolling left..\r\n");
    		LCD_Scroll(Left);
    	}
    	else if(c == 'r'){
    		printf("Scrolling right..\r\n");
    		LCD_Scroll(Right);
    	}
    	else if(c == 'W'){
    		printf("Setting RGB to White..\r\n");
    		LCD_SetColor(White);
    	}
    	else if(c == 'B'){
    		printf("Setting RGB to Blue..\r\n");
    		LCD_SetColor(Blue);
    	}
    	else if(c == 'R'){
    		printf("Setting RGB to Red..\r\n");
    		LCD_SetColor(Red);
    	}
    	else if(c == 'G'){
    		printf("Setting RGB to Green..\r\n");
    		LCD_SetColor(Green);
    	}
    	else if(c == 'P'){
    		printf("Setting RGB to custom color..\r\n");
    		CustomColor();
    	}
    }
}

void CustomColor(){
	uint32_t red;
	uint32_t green;
	uint32_t blue;
	printf("Please enter a red value: ");
	scanf(" %lu",&red);
	printf("\r\n");
	printf("Please enter a green value: ");
	scanf(" %lu",&green);
	printf("\r\n");
	printf("Please enter a blue value: ");
	scanf(" %lu",&blue);
	printf("\r\n");
	printf("Setting RGB color..\r\n");
	LCD_SetRGB((uint8_t)red,(uint8_t)green,(uint8_t)blue);
}

void Move_Cursor(uint32_t column,uint32_t row){
	LCD_SetCursor((uint8_t)column,(uint8_t)row);
}

void Cursor(){
	//printf("Current value of Cursor: %d\r\n",cursor);
	if(cursor == 0){
		printf("Turning on Cursor\r\n");
		LCD_Cursor(On);
		cursor = 1;
	}else{
		printf("Turning off Cursor\r\n");
		LCD_Cursor(Off);
		cursor = 0;
	}
}
void Blink(){
	//printf("Current value of Blink: %d\r\n",blink);
	if(blink == 0){
		printf("Turning on Blink\r\n");
		LCD_Blink(On);
		blink = 1;
	}else{
		printf("Turning off Blink\r\n");
		LCD_Blink(Off);
		blink = 0;
	}
}


void Display(){
	//printf("Current value of display: %d\r\n",display);
	if(display == 0){
		printf("Turning on Display\r\n");
		LCD_Display(On);
		display = 1;
	}else{
		printf("Turning off Display\r\n");
		LCD_Display(Off);
		display = 0;
	}
}
void print_Usage(){
	printf("\r\n\n");
	printf("'o' Switch Display On/Off\r\n");
	printf("'b' Switch Blinking On/Off\r\n");
	printf("'c' Switch Cursor On/Off\r\n");
	printf("'m' Move Cursor\r\n");
	printf("'l' Scroll Left\r\n");
	printf("'r' Scroll Right\r\n");
	printf("'W' Set Color White\r\n");
	printf("'R' Set Color Red\r\n");
	printf("'G' Set Color Green\r\n");
	printf("'B' Set Color Blue\r\n");
	printf("'P' Set Custom Color\r\n");
	printf("'h' Show Usage\r\n");
	printf("'d' Clear UART\r\n");
	printf("\r\n\n");
}

void I2C_Scan(){
    uint32_t wval; //write_value
    uint32_t rval; //read_value

	for(uint32_t I2Caddress = 0;I2Caddress < 0x80; I2Caddress++){
		//I2C master generates a start, sends an address with a READ/WRITE bit on it and receives acknowledgment
		//I2C master then sends a stop condition to complete the current transaction
		//if the I2C successfully sent a start and a stop to the current address, print out the address
        wval = Cy_SCB_I2C_MasterSendStart(I2C_HW,I2Caddress,CY_SCB_I2C_WRITE_XFER,10,&I2C_context);
        rval = Cy_SCB_I2C_MasterSendStop(I2C_HW,0,&I2C_context);
        if((rval == CY_SCB_I2C_SUCCESS)&(wval == CY_SCB_I2C_SUCCESS)){//if the I2C operation is a success, then print out the address
        	printf("%02X\r\n", (unsigned int)I2Caddress);
		}else{
			continue;
		}
	}
}


/*Function is called to setup UART at beginning of file*/
void UARTsetup(void){
	/* Allocate context for UART operation */
	cy_stc_scb_uart_context_t uartContext;

	/* Populate configuration structure */
	const cy_stc_scb_uart_config_t uartConfig =
	{
	    .uartMode                   = CY_SCB_UART_STANDARD,
	    .enableMutliProcessorMode   = false,
	    .smartCardRetryOnNack       = false,
	    .irdaInvertRx               = false,
	    .irdaEnableLowPowerReceiver = false,
	    .oversample                 = 8UL,
	    .enableMsbFirst             = false,
	    .dataWidth                  = 8UL,
	    .parity                     = CY_SCB_UART_PARITY_NONE,
	    .stopBits                   = CY_SCB_UART_STOP_BITS_1,
	    .enableInputFilter          = false,
	    .breakWidth                 = 11UL,
	    .dropOnFrameError           = false,
	    .dropOnParityError          = false,
	    .receiverAddress            = 0UL,
	    .receiverAddressMask        = 0UL,
	    .acceptAddrInFifo           = false,
	    .enableCts                  = false,
	    .ctsPolarity                = CY_SCB_UART_ACTIVE_LOW,
	    .rtsRxFifoLevel             = 0UL,
	    .rtsPolarity                = CY_SCB_UART_ACTIVE_LOW,
	    .rxFifoTriggerLevel = 63UL,
	    .rxFifoIntEnableMask = 0UL,
	    .txFifoTriggerLevel = 63UL,
	    .txFifoIntEnableMask = 0UL,
	};

	/* Connect SCB5 UART function to pins */
	Cy_GPIO_SetHSIOM(UART_PORT, UART_RX_NUM, P5_0_SCB5_UART_RX);
	Cy_GPIO_SetHSIOM(UART_PORT, UART_TX_NUM, P5_1_SCB5_UART_TX);

	/* Configure pins for UART operation */
	Cy_GPIO_SetDrivemode(UART_PORT, UART_RX_NUM, CY_GPIO_DM_HIGHZ);
	Cy_GPIO_SetDrivemode(UART_PORT, UART_TX_NUM, CY_GPIO_DM_STRONG_IN_OFF);


	/* Connect assigned divider to be a clock source for UART */
	Cy_SysClk_PeriphAssignDivider(PCLK_SCB5_CLOCK, UART_CLK_DIV_TYPE, UART_CLK_DIV_NUMBER);

	/* UART desired baud rate is 115200 bps (Standard mode).
	* The UART baud rate = (clk_scb / Oversample).
	* For clk_peri = 50 MHz, select divider value 36 and get SCB clock = (50 MHz / 36) = 1,389 MHz.
	* Select Oversample = 12. These setting results UART data rate = 1,389 MHz / 12 = 115750 bps.
	*/
	Cy_SysClk_PeriphSetDivider   (UART_CLK_DIV_TYPE, UART_CLK_DIV_NUMBER, 77U);
	Cy_SysClk_PeriphEnableDivider(UART_CLK_DIV_TYPE, UART_CLK_DIV_NUMBER);

	Cy_SCB_UART_Enable(SCB5);
}

/* [] END OF FILE */
