/** @file Screen.c
 * @see Screen.h for description.
 * @author Adrien RICCIARDI
 */
#include <system.h>
#include "Screen.h"

//--------------------------------------------------------------------------------------------------
// Private macros and constants
//--------------------------------------------------------------------------------------------------
/** The port containing the pins used to select each display. */
#define SCREEN_PORT_SELECT_DISPLAY portb
/** The port outputing the data to display. */
#define SCREEN_PORT_DATA portc

/** The pin selecting the left display (active low). */
#define SCREEN_PIN_SELECT_LEFT_DISPLAY 5
/** The pin selecting the right display (active low). */
#define SCREEN_PIN_SELECT_RIGHT_DISPLAY 4

/** The logical value to enable a 7-segment display. */
#define SCREEN_ENABLE 0
/** The logical value to disable a 7-segment display. */
#define SCREEN_DISABLE 1

//--------------------------------------------------------------------------------------------------
// Private types
//--------------------------------------------------------------------------------------------------
/** Identify the displays. */
typedef enum
{
	Display_Left,
	Display_Right
} TDisplay;

//--------------------------------------------------------------------------------------------------
// Private variables
//--------------------------------------------------------------------------------------------------
/** The available fonts for the screen. */
static unsigned char Screen_Fonts[] =
{
	0x40, // '0'
	0xEB, // '1'
	0x12, // '2'
	0x0A, // '3'
	0x29, // '4'
	0x0C, // '5'
	0x04, // '6'
	0x68, // '7'
	0x00, // '8'
	0x08, // '9'
	0xBF, // '-'
	0xFF // Empty character
};

/** The currently displayed data (they have been converted to displayable fonts yet). */
static unsigned char Characters[2] = {0xFF, 0xFF}; // Display nothing when initialized

//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------
void ScreenInitialize(void)
{
	// Disable unwanted analog inputs 
	ansel &= 0x0F; // AN4, AN5, AN6, AN7
	anselh = 0; // AN8, AN9, AN10, AN11
	
	// Disable screen to avoid drawing artifacts
	SCREEN_PORT_SELECT_DISPLAY.SCREEN_PIN_SELECT_LEFT_DISPLAY = SCREEN_DISABLE;
	SCREEN_PORT_SELECT_DISPLAY.SCREEN_PIN_SELECT_RIGHT_DISPLAY = SCREEN_DISABLE;
	
	// Configure ports as output
	trisb.SCREEN_PIN_SELECT_LEFT_DISPLAY = 0;
	trisb.SCREEN_PIN_SELECT_RIGHT_DISPLAY = 0;
	trisc = 0;
	
	// Initialize the timer 0 device to generate an interrupt every 120Hz
	// The main clock is running at 1MHz, so timer clock will be 1000000/4 = 250KHz
	// 8-bit mode is selected, so timer will overflow and generate an interrupt when it reaches 255, thus with a rate of 250000/256 = 976,5625
	// By dividing the previous value by ten, we get a 122,1Hz interrupt frequency
	t0con = 0x42; // Do not enable timer, use 8-bit mode, clock from internal oscillator, use a prescaler of 8
	
	// Run the module
	ScreenSetLowPowerMode(0);
}

void ScreenRefresh(void)
{
	static TDisplay Last_Updated_Display = Display_Right; // Start by left display
	TDisplay Display_To_Refresh;
	
	// Disable currently enabled display
	if (Last_Updated_Display == Display_Left)
	{
		SCREEN_PORT_SELECT_DISPLAY.SCREEN_PIN_SELECT_LEFT_DISPLAY = SCREEN_DISABLE;
		Display_To_Refresh = Display_Right;
	}
	else
	{
		SCREEN_PORT_SELECT_DISPLAY.SCREEN_PIN_SELECT_RIGHT_DISPLAY = SCREEN_DISABLE;
		Display_To_Refresh = Display_Left;
	}
	
	// Output the data to display
	SCREEN_PORT_DATA = Characters[Display_To_Refresh];
	
	// Enable the display
	if (Display_To_Refresh == Display_Left) SCREEN_PORT_SELECT_DISPLAY.SCREEN_PIN_SELECT_LEFT_DISPLAY = SCREEN_ENABLE;
	else SCREEN_PORT_SELECT_DISPLAY.SCREEN_PIN_SELECT_RIGHT_DISPLAY = SCREEN_ENABLE;
	
	Last_Updated_Display = Display_To_Refresh;
}

void ScreenSetDisplayedCharacters(unsigned char Left_Character_Code, unsigned char Right_Character_Code)
{
	// Disable timer 0 interrupt while changing the data to display to avoid glitches
	intcon.TMR0IE = 0;
	
	Characters[0] = Screen_Fonts[Left_Character_Code];
	Characters[1] = Screen_Fonts[Right_Character_Code];
	
	// Reenable timer 0 interrupt
	intcon.TMR0IE = 1;
}

void ScreenSetLowPowerMode(unsigned char Is_Low_Power_Enabled)
{
	// Make module sleep
	if (Is_Low_Power_Enabled)
	{
		// Stop refreshing screen
		t0con.TMR0ON = 0;
		intcon.TMR0IE = 0; // Disable timer 0 interrupt
		
		// Disable screen
		SCREEN_PORT_DATA = 0xFF; // Blank screen
		SCREEN_PORT_SELECT_DISPLAY.SCREEN_PIN_SELECT_LEFT_DISPLAY = SCREEN_DISABLE;
		SCREEN_PORT_SELECT_DISPLAY.SCREEN_PIN_SELECT_RIGHT_DISPLAY = SCREEN_DISABLE;
		
		// Clear screen data to avoid displaying bad text when awoken
		Characters[0] = Screen_Fonts[SCREEN_CHARACTER_CODE_EMPTY];
		Characters[1] = Screen_Fonts[SCREEN_CHARACTER_CODE_EMPTY]; 
	}
	// Wake up module
	else
	{
		tmr0l = 0;
		intcon.TMR0IF = 0; // Reset interrupt flag
		intcon.TMR0IE = 0; // Enable timer 0 interrupt
		t0con.TMR0ON = 1; // Reenable timer 0
	}
}
	