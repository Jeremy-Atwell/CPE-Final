/*
CPE Final
Jeremy Atwell, Aidan Chauvin, Jackie Ching, Jaeiz Ocampo
*/

#include <LiquidCrystal.h>
#include <RTClib.h>
#include <Stepper.h>

#define RDA 0x80 
#define TBE 0x20

volatile unsigned char* port_b = (unsigned char*) 0x25;
volatile unsigned char* ddr_b = (unsigned char*) 0x24;
volatile unsigned char* pin_b = (unsigned char*) 0x23;

volatile unsigned char* port_c = (unsigned char*) 0x28;
volatile unsigned char* ddr_c = (unsigned char*) 0x27;
volatile unsigned char* pin_c = (unsigned char*) 0x26;

volatile unsigned char* port_d = (unsigned char*) 0x2B;
volatile unsigned char* ddr_d = (unsigned char*) 0x2A;
volatile unsigned char* pin_d = (unsigned char*) 0x29;

volatile unsigned char* port_e = (unsigned char*) 0x2E;
volatile unsigned char* ddr_e = (unsigned char*) 0x2D;
volatile unsigned char* pin_e = (unsigned char*) 0x2C;

volatile unsigned char* port_g = (unsigned char*) 0x34;
volatile unsigned char* ddr_g = (unsigned char*) 0x33;
volatile unsigned char* pin_g = (unsigned char*) 0x32;

volatile unsigned char* port_h = (unsigned char*) 0x102;
volatile unsigned char* ddr_h = (unsigned char*) 0x101;
volatile unsigned char* pin_h = (unsigned char*) 0x100;
//UART Registers
volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
//ADC Registers
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

//Time control
const long updateTime = 60000;
unsigned long prevMillis = 0;

const int RS = 11, EN = 12, D4 = 2, D5 = 3, D6 = 4, D7 = 5;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
RTC_DS3231 rtc;

void setup()
{
    U0init(9600);   //Initializes serial monitor with baudrate 9600
    int state = 0;
    attachInterrupt(startButton,state = 1, RISING); //If start button pin goes from LOW to HIGH, set cooler state to idle.
}


void loop()
{
    switch (state){
        case 0: //disabled state
            //stuff
            break;
        case 1: //running state
            //stuff
            break;
        case 2: //idle state
            //stuff
            break;
        case 3: //error
            //stuff
            break;
        default:
            //stuff
    }

}

void U0init(int U0baud)
{
 unsigned long FCPU = 16000000;     //Frequency of Arduino is 16 MHz
 unsigned int tbaud;                //
 tbaud = (FCPU / 16 / U0baud - 1);  //
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;  //Sets parity to 0 and size to 8 bits
 *myUBRR0  = tbaud; //Sets baudrate of Arduino to 9600
}
///////////////////
// ADC FUNCTIONS //
///////////////////
void adc_init() 
{
  // setup the A register
  *my_ADCSRA |= 0x80; //enable ADC
  *my_ADCSRA &= 0xDF; //disable trigger
  *my_ADCSRA &= 0xF7; //disable interupt
  *my_ADCSRA &= 0xF8; //set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &= 0xF7;  
  *my_ADCSRB &= 0xF8; 
  // setup the MUX Register
  *my_ADMUX &= 0x7F;
  *my_ADMUX |= 0x40;
  *my_ADMUX &= 0xDF;
  *my_ADMUX &= 0b11100000;
}

unsigned int adc_read(unsigned char adc_channel_num) //work with channel 0
{
  
  *my_ADMUX &= 0b11100000;
  *my_ADCSRB &= 0b11110111;
  
  *my_ADMUX += adc_channel_num;
  
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  unsigned int val = (*my_ADC_DATA & 0x03FF);
  return val;
}

void fanMotor(bool power)  //Function that turns on/off
{
    motorPinAPort = 1;
    motorPinBPort = 1;
}

//Keeps track of time to update screen once per minute
void updateFunc()
{
    unsigned long currentMillis = millis();

    if(currentMillis - prevMillis == updateTime && state != 0)
{
    prevMillis = currentMillis;
    lcd.setCursor(0,0);
    lcd.write(humdity);
    lcd.setCursor(0,1);
    lcd.write(temp);
}

}