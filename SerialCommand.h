/******************************************************************************* 
SerialCommand - An Arduino library to tokenize and parse commands received over
a serial port. 
Copyright (C) 2011-2013 Steven Cogswell  <steven.cogswell@gmail.com>
http://awtfy.com

Version 20131021A.   

Version History:
May 11 2011 - Initial version
May 13 2011 - Prevent overwriting bounds of SerialCommandCallback[] array in addCommand()
			defaultHandler() for non-matching commands
Mar 2012 - Some const char * changes to make compiler happier about deprecated warnings.  
           Arduino 1.0 compatibility (Arduino.h header) 
Oct 2013 - SerialCommand object can be created using a SoftwareSerial object, for SoftwareSerial
           support.  Requires #include <SoftwareSerial.h> in your sketch even if you don't use 
           a SoftwareSerial port in the project.  sigh.   See Example Sketch for usage. 
Oct 2013 - Conditional compilation for the SoftwareSerial support, in case you really, really
           hate it and want it removed.  
May 2015 - Alex Sokolsky - improvements for readability and (my) style
           Make commands processing case-insensitive

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
#ifndef SerialCommand_h
#define SerialCommand_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include "WProgram.h"
#endif

// If you want to use SerialCommand with the hardware serial port only, and want to disable
// SoftwareSerial support, and thus don't have to use "#include <SoftwareSerial.h>" in your
// sketches, then uncomment this define for SERIALCOMMAND_HARDWAREONLY, and comment out the 
// corresponding #undef line.  
//
// You don't have to use SoftwareSerial features if this is not defined, you can still only use 
// the Hardware serial port, just that this way lets you get out of having to include 
// the SoftwareSerial.h header. 
#define SERIALCOMMAND_HARDWAREONLY 1
//#undef SERIALCOMMAND_HARDWAREONLY

#ifdef SERIALCOMMAND_HARDWAREONLY
#warning "Warning: Building SerialCommand without SoftwareSerial Support"
#endif

#ifndef SERIALCOMMAND_HARDWAREONLY 
#include <SoftwareSerial.h>  
#endif

#define MAXSERIALCOMMANDS	10

class SerialCommand
{
public:
#ifdef SERIALCOMMAND_HARDWAREONLY
  SerialCommand();
#else
  SerialCommand(SoftwareSerial *softSer=0);
#endif


  /** is there any input available? */
  bool available();
  /** Main entry point */
  void readAndDispatch();
  /** get the next token found in command buffer (for getting arguments to commands) */
  char *next();                                  
  /**  Add commands to processing dictionary */
  bool addCommand(const char *, void(*)());
  /** A handler to call when no valid command received. */
  void addDefaultHandler(void (*function)()) 
  {
    defaultHandler = function;
  }

	
private:
  char buffer[32];   // Buffer of stored characters while waiting for terminator character // should be <=256, defaults to 16
  byte bufPos;                        // Current position in the buffer
  const static char *delim;           // = " "; null-terminated list of delimeter chars for tokenizing (default " ")
  char term = '\r';                   // Character that signals end of command (default '\r')
  char *last;                         // State variable used between calls to next()
  typedef struct _callback {
    const char *command;
    void (*function)();
  } SerialCommandCallback;            // Data structure to hold Command/Handler function key-value pairs
  byte numCommand = 0;                // counter of meaningful elements in commandList
  SerialCommandCallback commandList[MAXSERIALCOMMANDS];   // Actual definition for command/handler array
  void (*defaultHandler)();           // Pointer to the default handler function 
#ifndef SERIALCOMMAND_HARDWAREONLY 
  SoftwareSerial *softSerial;       // Pointer to a user-created SoftwareSerial object
#endif
  /**
  * Initialize the command buffer
  */
  void clearBuffer()
  {
    buffer[0] = '\0';
    bufPos=0; 
  }
};


#endif //SerialCommand_h

