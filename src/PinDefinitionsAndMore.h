/*
 *  PinDefinitionsAndMore.h
 *
 *  Contains pin definitions for IRremote examples for various platforms
 *  as well as definitions for feedback LED and tone() and includes
 *
 *  Copyright (C) 2021  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of IRMP https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  Arduino-IRremote is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

/*
 * Pin mapping table for different platforms
 *
 * Platform     IR input    IR output   Tone
 * -----------------------------------------
 * DEFAULT/AVR  2           3           4
 * ATtinyX5     0           4           3
 * ATtiny167    9           8           5 // Digispark pro number schema
 * ATtiny167    3           2           7
 * ATtiny3217   10          11          3 // TinyCore schema
 * ATtiny1604   2           PA5/3       %
 * SAMD21       3           4           5
 * ESP8266      14 // D5    12 // D6    %
 * ESP32        15          4           %
 * BluePill     PA6         PA7         PA3
 * APOLLO3      11          12          5
 */
//#define IRMP_MEASURE_TIMING // For debugging purposes.
//


#define IR_RECEIVE_PIN          16  // D15
#define IR_SEND_PIN              17  // D4
#define tone(a,b,c) void()      // no tone() available on ESP32
#define noTone(a) void()
#define TONE_PIN                42 // Dummy for examples using it
#define APPLICATION_PIN         16 // RX2 pin


#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)