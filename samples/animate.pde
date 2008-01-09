/*
 * Define custom characters for creating an animation
 */
#include <Lcd.h>

#define LCD_WIDTH 16                     // number of columns in LCD module
Lcd lcd = Lcd(LCD_WIDTH, FUNCTION_4BIT); // use 4 pin mode

// x x x 1 1 1 1 1
// x x x 1 0 0 0 0
// x x x 1 0 0 0 0
// x x x 1 0 0 0 0
// x x x 1 0 0 0 0
// x x x 1 0 0 0 0
// x x x 1 1 1 1 1
// x x x 0 0 0 0 0
uint8_t left[] = { 0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0x00 };
  
// x x x 1 1 1 1 1
// x x x 0 0 0 0 1
// x x x 0 0 0 0 1
// x x x 0 0 0 0 1
// x x x 0 0 0 0 1
// x x x 0 0 0 0 1
// x x x 1 1 1 1 1
// x x x 0 0 0 0 0
uint8_t right[] = { 0x1F, 0x01, 0x01, 0x01, 0x01, 0x01, 0x1F, 0x00 };
  
// x x x 1 1 1 1 1
// x x x 0 0 0 0 0
// x x x 0 0 0 0 0
// x x x 0 0 0 0 0
// x x x 0 0 0 0 0
// x x x 0 0 0 0 0
// x x x 1 1 1 1 1
// x x x 0 0 0 0 0
uint8_t middle_empty[] = { 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00 };

// x x x 1 1 1 1 1
// x x x 0 0 0 0 0
// x x x 0 1 1 1 0
// x x x 1 1 1 1 1
// x x x 0 1 1 1 0
// x x x 0 0 0 0 0
// x x x 1 1 1 1 1
// x x x 0 0 0 0 0
uint8_t middle_full[] = { 0x1F, 0x00, 0x0E, 0x1F, 0x0E, 0x00, 0x1F, 0x00 };

int pos = 2; // current position of animated blip
int dir = 1; // 1 -> blip is moving to the right, -1 -> to the left

void draw_bar()
{
  lcd.print((char)0);   // draw left part of bar
  
  for(int i=0; i<(LCD_WIDTH-2); i++)
    lcd.print((char)2); // draw middle section
  
  lcd.print((char)1);   // draw right part of bar
  
  lcd.move_to(2,1);     // return cursor to starting pos
}

void animate()
{
  lcd.print((char)
  pos = pos + dir;

  if (pos == LCD_WIDTH-1 && dir == 1)
    dir = -1;
  else if (pos == 2 && dir == -1
    dir = 1;
}

void setup()
{
  // set pin assignment
  lcd.set_ctrl_pins(_2PINS(2,3));     // RS->2, E->3
  lcd.set_data_pins(_4PINS(4,5,6,7)); // D4->4, D5->5, D6->6, D7->7
  
  lcd.setup(); // setup arduino and initialize LCD
  
  // define the custom characters
  lcd.define_char(0, left);
  lcd.define_char(1, right);
  lcd.define_char(2, middle_empty);
  lcd.define_char(3, middle_full);
  
  draw_bar();
}

void loop()
{
  animate();
  delay(10);
}

