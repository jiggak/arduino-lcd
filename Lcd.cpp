/*
 * Copyright 2008 Josh Kropf
 * 
 * This file is part of the Arduino LCD Library.
 * 
 * Arduino LCD Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Arduino LCD Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Lemon Launcher; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "Lcd.h"

extern "C" {
   #include <inttypes.h>
   #include "WConstants.h"
   //#include "WProgram.h"
}

/* extract 4 bit value from the mask at the given offset */
#define PIN(mask,off) (mask>>off)&0xf

/* List of available commands */
#define CMD_CLEAR      0x01
#define CMD_HOME       0x02
#define CMD_ENTRY_MODE 0x04
#define CMD_DISPLAY    0x08
#define CMD_SHIFT      0x10
#define CMD_FUNCTION   0x20
#define CMD_CGRAM_ADR  0x40
#define CMD_DGRAM_ADR  0x80

#define REG_INST 0x00
#define REG_DATA 0x01

void
Lcd::select_reg(uint8_t reg)
{
   digitalWrite(PIN(_ctrl_pins, 4), reg);
}

void
Lcd::send(uint8_t data, uint16_t delay_micros)
{
   if (is4bit()) {
      // in 4 pin mode, send upper 4 bits then lower 4 bits
      send_4bits(data >> 4);
      send_4bits(data & 0xf);
   } else {
      uint32_t pins = _data_pins;
      
      for (int i=0; i<8; i++) {
         digitalWrite(pins & 0xf, data & 1);
         pins >>= 4;
         data >>= 1;
      }
      
      enable();
   }
   
   // wait for command to execute... the spec says to poll the busy
   // flag and wait for it to go low... but a simple delay seems to
   // be effective enough in practice
   delayMicroseconds(delay_micros);
   
   // for some crazy reason weird things happen when using a
   // micro second delay but a single millisecond delay works
   // UPDATE: now the microsecond delay works again... weird
   //delay(1);
}

void
Lcd::send_4bits(uint8_t data)
{
   // 4 pin mode we use only the upper 16 bits (4bits x 4 pins)
   uint32_t pins = _data_pins;
   
   for (int i=0; i<4; i++) {
      digitalWrite(pins & 0xf, data & 1);
      pins >>= 4;
      data >>= 1;
   }
   
   enable();
}

void
Lcd::enable()
{
   digitalWrite(PIN(_ctrl_pins,0), LOW);
   delayMicroseconds(1);
   digitalWrite(PIN(_ctrl_pins,0), HIGH);
   delayMicroseconds(1);
   digitalWrite(PIN(_ctrl_pins,0), LOW);
}

Lcd::Lcd(uint8_t width, uint8_t func) : _cols(width), _function(func)
{
   // set default pin masks
   _ctrl_pins = _2PINS(2,3);
   _data_pins = _8PINS(4,5,6,7,8,9,10,11);
}

void
Lcd::setup()
{
   uint8_t cpins = _ctrl_pins;
   uint32_t dpins = _data_pins;
   
   // setup control pins
   for (int i=0; i<2; i++) {
      pinMode(cpins&0xf, OUTPUT);
      cpins >>=4;
   }
   
   // setup data pins
   for (int i=0; i<(is4bit()? 4 : 8); i++) {
      pinMode(dpins&0xf, OUTPUT);
      dpins >>=4;
   }
   
   // spec says wait for the internal reset circuit to finish
   // before trying to do initialization by instruction
   delay(40);
   
   select_reg(REG_INST);  // all instruction registers
   
   if (is4bit()) {
      send_4bits(0x03);  // function set 8bit wait 5 millis
      delay(5);

      send_4bits(0x03);  // function set 8bit wait 100 micros
      delayMicroseconds(100);

      send_4bits(0x03);  // function set 8 bit wait 5 millis
      delay(5);

      // setting 4bit function must happen twice so send 4 bit
      // parameter and don't delay at all
      send_4bits(0x02);
   } else {
      send(CMD_FUNCTION | FUNCTION_8BIT, 4100);
      send(CMD_FUNCTION | FUNCTION_8BIT, 100);
      send(CMD_FUNCTION | FUNCTION_8BIT, 1000);
   }
   
   // set final funtion paramters, can't be set again after this
   send(CMD_FUNCTION | _function);
   
   // remaining three instructions of the initialization routine
   display(DISPLAY_OFF);
   clear();
   entry_mode(ENTRY_CURSOR_INC | ENTRY_NOSHIFT);

   // the following was not part of the initialization routine in the spec but I found
   // it is required otherwise some crazy text is printed to the screen... specificly
   // home and clear must be called in this exact order
   home();
   clear();
   
   // finally, leave the setup routine with the display turned on
   display(DISPLAY_ON);
}

void
Lcd::entry_mode(uint8_t entry)
{
   select_reg(REG_INST);
   send(CMD_ENTRY_MODE | entry);
}

void
Lcd::display(uint8_t display)
{
   select_reg(REG_INST);
   send(CMD_DISPLAY | display);
}

void
Lcd::shift(uint8_t shift)
{
   select_reg(REG_INST);
   send(CMD_SHIFT | shift);
}

void
Lcd::clear()
{
   select_reg(REG_INST);
   send(CMD_CLEAR, 1640);
}

void
Lcd::home()
{
   select_reg(REG_INST);
   send(CMD_HOME, 1640);
}

void
Lcd::move_to(int col, int row)
{
   select_reg(REG_INST);
   
   if (_function & FUNCTION_2LINE) {
      switch(row) {
         case 1:
            send(CMD_DGRAM_ADR | col-1); break;
         case 3:
            send(CMD_DGRAM_ADR | _cols+col-1); break;
         case 2:
            send(CMD_DGRAM_ADR | 0x40+col-1); break;
         case 4:
            send(CMD_DGRAM_ADR | 0x40+_cols+col-1 ); break;
      }
   } else {
      send(CMD_DGRAM_ADR | ((row-1)*_cols)+col-1);
   }
}

void
Lcd::print(char c)
{
   select_reg(REG_DATA);
   send(c, 46);
}

void
Lcd::print(char* s)
{
   while(*s != 0x00) {
      print(*s);
      s++;
   }
}

void
Lcd::define_char(uint8_t index, uint8_t data[])
{
   int h = _function & FUNCTION_5x11? 11 : 8;
   for (int i=0; i<h; i++) {
      select_reg(REG_INST);
      send(CMD_CGRAM_ADR | (index*h)+i);
      
      select_reg(REG_DATA);
      send(data[i], 46);
   }
}
