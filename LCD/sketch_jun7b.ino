/*
  JHD162A 16x2 LCD - Hello World Tutorial
  
  HARDWARE CONNECTIONS:
  ====================
  LCD Pin 1 (VSS)      -> Arduino GND
  LCD Pin 2 (VCC)      -> Arduino +5V
  LCD Pin 3 (VEE/V0)   -> Potentiometer Middle Pin (For contrast)
  
  LCD Pin 4 (RS)       -> Arduino Digital Pin 12
  LCD Pin 5 (R/W)      -> Arduino GND (Forces LCD into Write-only mode)
  LCD Pin 6 (E)        -> Arduino Digital Pin 11
  
  LCD Pin 7-10(DB0-DB3)-> NOT CONNECTED (Left open in 4-bit mode)
  
  LCD Pin 11 (DB4)     -> Arduino Digital Pin 5
  LCD Pin 12 (DB5)     -> Arduino Digital Pin 4
  LCD Pin 13 (DB6)     -> Arduino Digital Pin 3
  LCD Pin 14 (DB7)     -> Arduino Digital Pin 2
  
  LCD Pin 15 (LED+)    -> Arduino +5V (Connect via a 220-ohm resistor!)
  LCD Pin 16 (LED-)    -> Arduino GND
  
  POTENTIOMETER CONNECTIONS (10k Ohm):
  ====================================
  Outer Pin 1          -> Arduino +5V
  Middle Pin           -> LCD Pin 3 (VEE)
  Outer Pin 2          -> Arduino GND
*/

#include <LiquidCrystal.h>

// Initialize the library by associating the interface pins
// Syntax: LiquidCrystal lcd(RS, Enable, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  // Set up the LCD's number of columns and rows (16 columns, 2 rows)
  lcd.begin(16, 2);
  
  // Clear any existing data on the screen
  lcd.clear();
  // 1. Move to the top-left corner and print the first line
  lcd.setCursor(0, 0); // Column 0, Row 0
  //lcd.print("Line One Text");
  lcd.print("1234567890123456");
  
  // 2. Move to the bottom-left corner and print the second line
  lcd.setCursor(0, 1); // Column 0, Row 1 (This moves it to the new line!)
  //lcd.print("Line Two Text");
  // Print our message to the LCD
  lcd.print("1234567890123456");
  
}

void loop() {
  // Nothing to loop because we want a static message.
}
