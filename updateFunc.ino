/*updateFunction*/

#include <LiquidCrystal.h>

//Timer stuff
const long updateTime = 60000;
unsigned long prevMillis = 0;
unsigned long currentMillis = millis();

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);


void updateFunc()
{

    if(currentMillis - prevMillis == updateTime && state != 0)
{
    prevMillis = currentMillis;
    lcd.setCursor(0,0);
    lcd.write(humdity);
    lcd.setCursor(0,1);
    lcd.write(temp);
}

}