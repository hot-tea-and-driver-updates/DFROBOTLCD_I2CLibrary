
#include "DFR0554.h"
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cy_scb_uart.h"
#include "cy_device_headers.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//helper function definitions
bool LCD_Command(uint8_t datatoSend);
bool LCD_Send(uint8_t *data, uint8_t len);
bool setReg(uint8_t addr, uint8_t data);

extern cy_stc_scb_i2c_context_t I2C_context;

//#define I2C_HW SCB6
//bit shifting
#define LCD_ADDRESS     0x3e
#define RGB_ADDRESS     0x60

/*!
 *  @brief color define
 */
#define WHITE           0 //standard
#define RED             1
#define GREEN           2
#define BLUE            3

#define REG_RED         0x04        // pwm2
#define REG_GREEN       0x03        // pwm1
#define REG_BLUE        0x02        // pwm0

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT      0x08

/*!
 *  @brief commands
 */
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

/*!
 *  @brief flags for display entry mode
 */
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/*!
 *  @brief flags for display on/off control
 */
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

/*!
 *  @brief flags for display/cursor shift
 */
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04 //wont use, use enum instead
#define LCD_MOVELEFT 0x00 //wont use, use enum instead

/*!
 *  @brief flags for function set
 */
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00 //standard
//end of macros from LCD github

//global variables
uint8_t _showfunction;
uint8_t _showcontrol;
uint8_t _showmode;
uint8_t _initialized;
uint8_t _numlines,_currline;
uint8_t _lcdAddr;
uint8_t _RGBAddr;
uint8_t _cols;
uint8_t _rows;
uint8_t _backlightval;


//will use this in combination with enumerations
const uint8_t color_define[4][3] =
{
    {255, 255, 255},            // white
    {255, 0, 0},                // red
    {0, 255, 0},                // green
    {0, 0, 255},                // blue
};

typedef struct DFRobot_RGBLCD{ 
	uint8_t _lcdAddr;
	uint8_t _RGBAddr;
	uint8_t _lcd_cols;
	uint8_t _lcd_rows;
}DFRLCD;

//LCD_start will be both LCD_init() and LCD_begin() from the DFRgithub
bool LCD_Start(CySCB_Type *master, cy_stc_scb_i2c_context_t *context){
	bool m_return = false;
	_showfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
	CyDelay(50);

	    ///< Send function set command sequence
	LCD_Command(LCD_FUNCTIONSET | _showfunction);
	CyDelay(5);  // wait more than 4.1ms

		///< second try
	LCD_Command(LCD_FUNCTIONSET | _showfunction);
	CyDelay(5);

	LCD_Command(LCD_FUNCTIONSET | _showfunction);

	_showcontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;

	LCD_Display(On);
    ///< clear it off
    LCD_Clear();

    LCD_Blink(Off);

    LCD_Cursor(Off);

    _showmode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

    LCD_Command(LCD_ENTRYMODESET | _showmode);

	setReg(REG_MODE1, 0);
    setReg(REG_OUTPUT, 0xFF);
    setReg(REG_MODE2, 0x20);

    m_return = LCD_SetRGB(255, 255, 255);
	if(m_return == false){
		printf("Unable to correctly initialize LCD...\r\n\n");
	}
	return m_return;
}

bool LCD_Blink(enum mode variable){
	bool value = false;
	if(variable == On){
		_showcontrol |= LCD_BLINKON;
		value = LCD_Command(LCD_DISPLAYCONTROL | _showcontrol);
	}else{
	    _showcontrol &= ~LCD_BLINKON;
	    value = LCD_Command(LCD_DISPLAYCONTROL | _showcontrol);
	}
	if(value == false){
		printf("Unable to configure Blink...\r\n\n");
	}
	return value;
}

// Positions the cursor at COL and ROW where 0<=COL<=15 and 0<=ROW<=1
// Returns FALSE on error, TRUE otherwise.
bool LCD_SetCursor(uint8_t col, uint8_t row){
	bool variable = false;
	if((col >= 0)&&(col <= 15)){
		if((row >= 0)&&(row <= 1)){
			col = (row == 0 ? col|0x80 : col|0xc0);
			uint8_t data[3] = {0x80, col};
			variable = LCD_Send(data, 2);
		}
	}
	if(variable == false){
		printf("Unsuccessful cursor placement..\r\n\n");
	}
	return false;
}
// Scrolls display on char left if DIRECTION is Left, one char right otherwise.
// Returns FALSE on error, TRUE otherwise.

bool LCD_Scroll(enum direction variable){
	bool value = false;
	if(variable == Left){
		value = LCD_Command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
	}
	if(variable == Right){
		value = LCD_Command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
	}
	if(value == false){
		printf("Unable to correctly scroll...\r\n\n");
	}
	return value;
}
// Prints all characters from zero terminated string STR.
// If strlen(str) > 16, call LCD_Autoscroll(On) first.
// Returns FALSE on error, TRUE otherwise.
bool LCD_Print(char *str){
	printf("Printing to LCD Display...\r\n");
	bool variable = false;
	uint8_t data[strlen(str)+1];
	data[0] = 0x40;
	LCD_Cursor(On);
	if(strlen(str) > 16){
		printf("Unable to print to display...\r\n\n");
		return false;
	}else{
		for(int i = 0; i < strlen(str); i++){
			data[i+1] = str[i];
		}
		variable = LCD_Send(data, (strlen(str)+1));
		LCD_Cursor(Off);
	}
	if (variable == false){
		printf("Unable to print to display...\r\n\n");
	}
	return variable;
}

// Sets display color if COLOR is White, Red, Green, or Blue, no-op otherwise.
// Returns FALSE on error, TRUE otherwise.
bool LCD_SetColor(enum color variable){
	bool value = false;
	if((variable == White)||(variable == Red)||(variable == Green)||(variable == Blue)){
	    value = LCD_SetRGB(color_define[variable][0], color_define[variable][1], color_define[variable][2]);
	}
	if(value == false){
	printf("Unable to set LCD color..\r\n\n");
	}
	return value;
}

bool LCD_Cursor(enum mode variable){
	bool value = false;
    if(variable == On){
    	_showcontrol |= LCD_CURSORON;
    	value = LCD_Command(LCD_DISPLAYCONTROL | _showcontrol);
    }else{
        _showcontrol &= ~LCD_CURSORON;
        value = LCD_Command(LCD_DISPLAYCONTROL | _showcontrol);
    }
    if(value == false){
    	printf("Unable to display LCD cursor...\r\n\n");
    }
    return value;
}
bool LCD_SetRGB(uint8_t red, uint8_t green, uint8_t blue){
	bool variable = false;
    variable = setReg(REG_RED, red);
    variable = setReg(REG_GREEN, green);
    variable = setReg(REG_BLUE, blue);
	if (variable == false){
		printf("Unable to set RGB...\r\n\n");
	}
	return variable;
}
bool LCD_Display(enum mode variable){
	bool value = false;
	if(variable == On){
		 _showcontrol |= LCD_DISPLAYON;
		 value = LCD_Command(LCD_DISPLAYCONTROL | _showcontrol);
	}
	if(variable == Off){
	    _showcontrol &= ~LCD_DISPLAYON;
	    value = LCD_Command(LCD_DISPLAYCONTROL | _showcontrol);
	}
	if(value == false){
		printf("Unable to set Display..\r\n\n");
	}
	return value;
}

bool LCD_Clear(){
	bool value = false;
	value = LCD_Command(LCD_CLEARDISPLAY);
	CyDelay(2000);
	if(value == false){
		printf("Unable to clear LCD display...\r\n\n");
	}
	return value;
}

bool LCD_Command(uint8_t datatoSend){
	bool value = false;
	uint8_t data[3] = {0x80, datatoSend};
	value = LCD_Send(data, 2);
	return value;
}
bool LCD_Send(uint8_t *data, uint8_t len){
	bool value = false;
	uint32_t rval;
	rval = Cy_SCB_I2C_MasterSendStart(I2C_HW,LCD_ADDRESS,CY_SCB_I2C_WRITE_XFER,10,&I2C_context);
	if(rval == CY_SCB_I2C_SUCCESS){
		for(int i = 0; i < len; i++){
			Cy_SCB_I2C_MasterWriteByte(I2C_HW, data[i], 5,&I2C_context);
			CyDelay(5);
		}
		value = true;
	}
	Cy_SCB_I2C_MasterSendStop(I2C_HW, 10,&I2C_context);
	return value;
}


bool setReg(uint8_t addr, uint8_t data){
	bool value = false;
    uint32_t wval; //write_value
    uint32_t rval; //read_value


    wval = Cy_SCB_I2C_MasterSendStart(I2C_HW,RGB_ADDRESS,CY_SCB_I2C_WRITE_XFER,10,&I2C_context);
    //Cy_SCB_I2C_MasterWriteByte(master,addr,0,context);
    Cy_SCB_I2C_MasterWriteByte(I2C_HW,addr,0,&I2C_context);
	CyDelay(5);
	Cy_SCB_I2C_MasterWriteByte(I2C_HW,data,0,&I2C_context);
	rval = Cy_SCB_I2C_MasterSendStop(I2C_HW,0,&I2C_context);
    if((rval == CY_SCB_I2C_SUCCESS)&(wval == CY_SCB_I2C_SUCCESS)){//if the I2C operation is a success, then print out the address
    	value = true;
	}
    return value;
}







