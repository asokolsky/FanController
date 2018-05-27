/******************************************************************************* 
SerialCommand - An Arduino library to tokenize and parse commands received over
a serial port. 
Copyright (C) 2011-2013 Steven Cogswell  <steven.cogswell@gmail.com>
http://awtfy.com

See SerialCommand.h for version history. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************************/
#include <Arduino.h>
#include <string.h>
#include "Trace.h"
#include "SerialCommand.h"

const static char *SerialCommand::delim = " ";     // null-terminated list of delimeter chars for tokenizing (default " ")


#ifdef SERIALCOMMAND_HARDWAREONLY
SerialCommand::SerialCommand() 
{
  clearBuffer(); 
}
#else
SerialCommand::SerialCommand(SoftwareSerial *softSer) : 
  softSerial(softSer)
{		
  clearBuffer(); 
}
#endif

/**
 * Retrieve the next token ("word" or "argument") from the Command buffer.  
 * returns a NULL if no more tokens exist.   
 */
char *SerialCommand::next() 
{
  return strtok_r(NULL, delim, &last); 
}

bool SerialCommand::available()
{
#ifdef SERIALCOMMAND_HARDWAREONLY
  return (Serial.available() > 0);
#else
  return (((softSerial != 0) ? softSerial->available() : Serial.available()) > 0) 
#endif
}

/** 
 * Check the Serial stream for characters, and assemble them into a buffer.  
 * When the terminator character (default '\r') is seen, 
 * pars the buffer for a prefix command, and calls handlers setup in addCommand()
 */
void SerialCommand::readAndDispatch() 
{
  // If we're using the Hardware port, check it.   Otherwise check the user-created SoftwareSerial Port
  while(available()) 
  {
    char inChar = 
#ifdef SERIALCOMMAND_HARDWAREONLY
      Serial.read();
#else
      (softSerial != 0) ? softSerial->read() : Serial.read();
#endif
    DEBUG_PRNT(inChar);   // Echo back to serial stream
		if(isprint(inChar))   // Only printable characters into the buffer
		{
      buffer[bufPos++] = toupper(inChar);   // Put character into buffer
      buffer[bufPos]='\0';  // Null terminate
      if(bufPos >= (sizeof(buffer)-1))
        bufPos = 0; // wrap buffer around if full  
		}
		else if (inChar == term) // Check for the terminator (default '\r') meaning end of command
    {     
      //process the command                  
			DEBUG_PRINT("Received: "); DEBUG_PRNTLN(buffer);
			bufPos = 0;           // Reset to start of buffer
			char *token = strtok_r(buffer, delim, &last);   // Search for command at start of buffer
			if (token == NULL) return; 
      boolean matched = false;			
			for(int i = 0; i < numCommand; i++) 
      {
				DEBUG_PRINT("Comparing ["); DEBUG_PRNT(token); DEBUG_PRINT("] to ["); DEBUG_PRNT(commandList[i].command); DEBUG_PRINTLN("]");
				// Compare the found command against the list of known commands for a match
				if(strncmp(token, commandList[i].command, sizeof(buffer) - 1) == 0) 
				{
					DEBUG_PRINT("Matched Command: "); DEBUG_PRNTLN(token);
					// Execute the stored handler function for the command
					(*commandList[i].function)(); 
					matched = true; 
					break; 
				}
			}
			if(!matched) 
      {
				(*defaultHandler)(); 
			}
			clearBuffer(); 
		}
	}
}

/**
 * Adds a "command" and a handler function to the list of available commands.  
 * This is used for matching a found token in the buffer, and gives the pointer
 * to the handler function to deal with it. 
 */
bool SerialCommand::addCommand(const char *command, void (*function)())
{
  if(numCommand >= MAXSERIALCOMMANDS) 
  {
    DEBUG_PRINTLN("Too many handlers - recompile changing MAXSERIALCOMMANDS"); 
    return false;
  }
  DEBUG_PRNT(numCommand); DEBUG_PRINT("-Adding command for "); DEBUG_PRNTLN(command);	
  commandList[numCommand].command = command;
  commandList[numCommand].function = function; 
  numCommand++; 
  return true;
}
