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
#ifndef _LCD_H_
#define _LCD_H_

#include <inttypes.h>

/* Generates control pin mask */
#define _2PINS(RS,E) \
   ((RS&0xf) << 4) | (E&0xf)

/* Generates data pin mask for 4 pin mode */
#define _4PINS(D4,D5,D6,D7) \
   ((D7&0xf) << 12) | ((D6&0xf) << 8) | \
   ((D5&0xf) << 4)  |  (D4&0xf)

/* Generates data pin mask for 8 pin mode */
#define _8PINS(D0,D1,D2,D3,D4,D5,D6,D7) \
   ((((uint32_t)D7)&0xf) << 28) | ((((uint32_t)D6)&0xf) << 24) | \
   ((((uint32_t)D5)&0xf) << 20) | ((((uint32_t)D4)&0xf) << 16) | \
   ((((uint32_t)D3)&0xf) << 12) | ((((uint32_t)D2)&0xf) << 8) | \
   ((((uint32_t)D1)&0xf) << 4)  | (D0&0xf)

// Function constants passed to constructor, can be OR'd together
#define FUNCTION_4BIT  0x10 // enable 4 pin mode
#define FUNCTION_8BIT  0x00 // enable 8 pin mode (default)
#define FUNCTION_2LINE 0x08 // LCD has two lines, line two starts at addr 0x40
#define FUNCTION_1LINE 0x00 // LCD has one continues line
#define FUNCTION_5x11  0x04 // use 5x11 custom characters
#define FUNCTION_5x8   0x00 // use 5x8 custom characters

// CMD_DISPLAY parameters, can be OR'd together
#define DISPLAY_ON  0x04      // display on
#define DISPLAY_OFF 0x00      // display off
#define DISPLAY_CURSOR   0x02 // cursor on
#define DISPLAY_NOCURSOR 0x00 // cursor off
#define DISPLAY_BLINK   0x01  // cursor blink on
#define DISPLAY_NOBLINK 0x00  // curson blink off

// CMD_ENTRY_MODE parameters, can be OR'd together
#define ENTRY_CURSOR_INC 0x02 // increment cursor position
#define ENTRY_CURSOR_DEC 0x00 // decrement curson position
#define ENTRY_SHIFT      0x01 // shift entire display
#define ENTRY_NOSHIFT    0x00 // don't shift display

// CMD_SHIFT parameters, can be OR'd together
#define SHIFT_SCREEN 0x08 // shift display 
#define SHIRT_CURSOR 0x00 // shift cursor
#define SHIRT_RIGHT  0x04 // to the right
#define SHIRT_LEFT   0x00 // to the left

/**
 * Class for controlling HD44780 based LCD modules from the Arduino.
 *
 * The Arduino has 14 pins.  Therefore we can easily encode a single pin
 * number inside 4bits (0-15).  This class uses a 8 bit mask to encode
 * the control pins (RW, E) and a 32 bit mask to encode the data
 * pins (D0~D8).
 *
 * Control Pins
 * Byte    | B1 | B0 |
 * --------+----+----+
 * LCD Pin | RS |  E |
 *
 * Default pin arrangement is:
 * RS=2, Enable=3
 *
 * Data Pins
 * Byte    | B7 | B6 | ... | B0 |
 * --------+----+----+-----+----+
 * LCD Pin | D0 | D1 | ... | D7 |
 *
 * Default pin arrangement is (4pin mode uses D4~D7):
 * D0=4, D1=5... , D7=11
 */
class Lcd {
protected:
   uint8_t  _cols; // number of columns wide
   uint8_t  _function;
   
   uint8_t  _ctrl_pins;
   uint32_t _data_pins;
   
   /*
    * Sends 8bits to the lcd AND waits for the command to finish.
    *
    * The most common delay execution time is 42 micro seconds so
    * the delay is optional and only needed for commands that have
    * a different execution time.
    */
   void send(uint8_t data, uint16_t delay_micro = 42);
   
   /* sends the lower 4bits to the lcd on pins D7~D4 */
   void send_4bits(uint8_t data);
   
   /* select instruction register (0) or data register (1) */
   void select_reg(uint8_t reg);

   /* pulses the enable pin to signal the LCD to read data pins */
   void enable();
   
   inline bool is4bit() const
   { return _function & FUNCTION_4BIT; }
   
public:
   /**
    * Creates the Lcd class.
    * \param width number of columns in the LCD module
    * \param func function parameters (mask of FUNCTION_* defines)
    */
   Lcd(uint8_t width, uint8_t func = 0);
   
   /**
    * Setup Arduino pins and initialize LCD module.
    */
   void setup();
   
   /**
    * Sets the 8 bit mask for the control pins (RW,E).
    */
   inline void set_ctrl_pins(uint8_t mask)
   { _ctrl_pins = mask; }

   /**
    * Sets the 32 bit mask for the data pins.  When using 4 wire mode, only use
    * the upper 4 pins (D4~D7) in the lower 4 bytes (B3~B0).
    */
   inline void set_data_pins(uint32_t mask)
   { _data_pins = mask; }

   /**
    * Sets entry mode parameters.  Combination of ENTRY_* defines.
    */
   void entry_mode(uint8_t entry);
   
   /**
    * Sets display mode parameters.  Combination of DISPAY_* defines.
    */
   void display(uint8_t display);

   /**
    * Sets cursor or display shift dehaviour.  Combination of SHIFT_* defines.
    */
   void shift(uint8_t shift);

   /**
    * Clear the display.
    */
   void clear();

   /**
    * Move cursor to home position (top/left).
    */
   void home();
   
   /**
    * Move the cursor to a given column and row.
    * \param col
    * \param row
    */
   void move_to(int col, int row);

   /**
    * Print a single ASCII character to the LCD screen at the current
    * cursor position.
    */
   void print(char c);

   /**
    * Print a string of ASCII characters to the LCD screen at the
    * current cursor position.
    */
   void print(char* str);
   
   /**
    * Define character in CGRAM.  This defines a custom character.  When the
    * LCD is setup to use 5x8 font the user can specify up to eight characters.
    * For 5x11 font four characters can be defined.
    *
    * Custom characters are bitmaps!  For 5x8 font the bitmap must be 8 bits
    * wide and 8 bits hight.  For 5x11 font the bitmap must be 8 bits wide and
    * 10 bits high.
    * font the array must contain 11 bytes.
    *
    * Each byte in the character data array represents a row of bits in a
    * character.  The lower 5 bits of each byte is used to specify the dots in
    * the character that should be black (1) and white (0).
    *
    * To print the user defined character, pass the index of of the character
    * to the print method.
    *
    * Example: define bullet character
    *
    * X X X 0 0 0 0 0
    * X X X 0 0 0 0 0
    * X X X 0 1 1 1 0
    * X X X 1 1 1 1 1
    * X X X 1 1 1 1 1
    * X X X 0 1 1 1 0
    * X X X 0 0 0 0 0
    * X X X 0 0 0 0 0
    * 
    * {0x00, 0x00, 0x0E, 0x1F, 0x1F, 0x0E, 0x00, 0x00}
    *
    * \param index zero based index of custom character
    * \param data character data array
    */
   void define_char(uint8_t index, uint8_t data[]);
};

#endif
