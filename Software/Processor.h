/** @file Processor.h
 * Handle the processor core low power mode.
 * @author Adrien RICCIARDI
 * @version 1.0 : 20/06/2014
 */
#ifndef H_PROCESSOR_H
#define H_PROCESSOR_H

//--------------------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------------------
/** Put the whole system in RC_IDLE mode to consume ~1mA. A timer will automatically awake the system each 64 seconds.
 * @param Is_Low_Power_Required Set to 1 to enable low power mode or to 0 to wake up the whole system.
 */
void ProcessorSetLowPowerMode(unsigned char Is_Low_Power_Enabled);

#endif