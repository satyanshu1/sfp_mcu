#include "HT_SSD1306Wire.h"

SSD1306Wire  screen(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst


void setup_display(){
  VextON();
  delay(100);
  screen.init();  // Initialising the UI will init the display too.
  screen.setFont(ArialMT_Plain_10);
}

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

void write_to_display(const String &data)
{
    screen.clear();  // clear the screen
    screen.setTextAlignment(TEXT_ALIGN_LEFT);
    screen.drawStringMaxWidth(0, 0, 128, data);
    screen.display();
}

void write_to_display_buffered(const String &data){
    // Initialize the log buffer
    // allocate memory to store 5 lines of text and 30 chars per line. scroll after that.
    screen.setLogBuffer(5, 30);

    for (uint16_t idx = 0; idx < data.length(); ) 
    {
        screen.clear();
        uint16_t end_idx = idx; 
        uint16_t rem_len = data.length() - idx;
        if (rem_len > 20)
            end_idx += 20;
        else
            end_idx += rem_len;

        screen.println(data.substring(idx, end_idx).c_str());  // Print to the screen
        screen.drawLogBuffer(0, 0);  // Draw it to the internal screen buffer
        screen.display();  // Display it on the screen
        delay(500);

        idx = end_idx;
    }
}