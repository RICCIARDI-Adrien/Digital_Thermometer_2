/** @file Screen.h
 * Display digits on the two 7-segment displays.
 * @author Adrien RICCIARDI
 * @version 1.0 : 12/04/2014
 */
#ifndef H_SCREEN_H
#define H_SCREEN_H

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------
/** Display a '-' sign. */
#define SCREEN_CHARACTER_CODE_MINUS 10

/** Display an empty character. */
#define SCREEN_CHARACTER_CODE_EMPTY 11

//--------------------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------------------
/** Initialize the screen and shut it off. */
void ScreenInitialize(void);

/** Refresh the displayed data.
 * @note This function must be called at a 120Hz rate.
 */
void ScreenRefresh(void);

/** Display data.
 * @param Left_Character_Code The leftmost character code.
 * @param Right_Character_Code The rightmost character code.
 * @note Use the SCREEN_DIGIT_CODE_xxx values or the numbers from 0 to 9 to represent the digits.
 */
void ScreenSetDisplayedCharacters(unsigned char Left_Character_Code, unsigned char Right_Character_Code);

/** Enable or disable screen module to save power.
 * @param Is_Low_Power_Enabled Set to 1 to enable low power mode or to 0 to wake up the module.
 */
void ScreenSetLowPowerMode(unsigned char Is_Low_Power_Enabled);

#endif