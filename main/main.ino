
/*
CPE Final
Jeremy Atwell, Aidan Chauvin, Jackie Ching, Jaeiz Ocampo
*/

#include <LiquidCrystal.h>
#include <RTClib.h>
#include <Stepper.h>
#include "DHT.h"

#define DHTpin 10
#define DHTTYPE DHT11
#define RDA 0x80 
#define TBE 0x20
#define STEPS 2038

volatile unsigned char* port_b = (unsigned char*) 0x25;
volatile unsigned char* ddr_b = (unsigned char*) 0x24;
volatile unsigned char* pin_b = (unsigned char*) 0x23;

volatile unsigned char* port_c = (unsigned char*) 0x28;
volatile unsigned char* ddr_c = (unsigned char*) 0x27;
volatile unsigned char* pin_c = (unsigned char*) 0x26;

volatile unsigned char* port_d = (unsigned char*) 0x2B;
volatile unsigned char* ddr_d = (unsigned char*) 0x2A;
volatile unsigned char* pin_d = (unsigned char*) 0x29;

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

const int RS = 11, E = 12, D4 = 2, D5 = 3, D6 = 4, D7 = 5;
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
RTC_DS3231 rtc;
DHT dht(DHTpin, DHTTYPE);
Stepper stepper(STEPS, 30, 32, 31, 33);
const long updateTime = 60000;
unsigned long prevMillis = 0;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
"Friday", "Saturday"};
int prevState = 0;          //Checks if state of cooler changed
int stepperState = 0;       //Current stepperState(0 = off, 1 = on)
int prevStepperState = 0;   //Checks if stepperState changed
bool change = false;        //Checks if change has happened in the system

void setup() {
    U0init(9600);
    lcd.begin(16,2);
    adc_init();
    rtc.begin();
    pin_begin();
   dht.begin();
    attachInterrupt(digitalPinToInterrupt(19), isr_funct, RISING);
}
int state =0;
int stepCount = 0;

void loop() {
  unsigned long currentMillis = millis();
  updateFunc(currentMillis);
  swapTimer();

  stepperControl();
  
  switch (state){
       case 0: //disabled state
          disabled();
           break;
       case 1: //running state
            running();
            break;
        case 2: //idle state
            idle();
            break;
        case 3: //error
            error();
            break;
        default:
            lcd.setCursor(0,0);
            lcd.print(":(");
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
 *myUBRR0  = tbaud; //
}

// ISR FUNCTION 

void isr_funct (){
  if(state == 0){
    state = 2; //change to idle ON
    lcd.clear();
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    lcd.setCursor(0,0);
    lcd.print(hum);
    lcd.setCursor(0,1);
    lcd.print(temp);
  }else if (state != 0){
    state = 0; //disable state OFF
  }
}

///////////////////
// ADC FUNCTIONS //
///////////////////
void adc_init() {
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
unsigned int adc_read(unsigned char adc_channel_num)
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
void disabled()
{
  fanMotor(0);
  *port_h &= 0xBF; //blue off
  *port_h &= 0xF7; // red off
  *port_h &= 0xDF; // green off
  *port_h |= 0x10; // yellow on
}
void error()
{
  fanMotor(0);
  lcdError();
  *port_h &= 0xBF; //blue off
  *port_h &= 0xDF; // green off
  *port_h &= 0xEF; //yellow off
  *port_h |= 0x08; //red on
  if((*pin_b & 0x80) ){
    state = 2;
    lcd.clear();
  }
}
void idle()
{
  fanMotor(0);
  *port_h &= 0xBF; //blue off
  *port_h &= 0xF7; // red off
  *port_h &= 0xEF; //yellow off
  *port_h |= 0x20; //green on
  unsigned int water = adc_read(0);
  float temp = dht.readTemperature();
  if(water <= 250){
    state = 3;
  }
  if(temp > 21){
    state = 1;
  }
}
void running()
{
  fanMotor(1);
  *port_h &= 0xF7; // red off
  *port_h &= 0xDF; // green off
  *port_h &= 0xEF; //yellow off
  *port_h |= 0x40; //blue on
  unsigned int water = adc_read(0);
  float temp = dht.readTemperature();
  if(water <= 250){
    state = 3;
  }
  if(temp <= 21){
    state = 2;
  }
}
void pin_begin(){ //Begins most input and output pins used
    *ddr_h |= 0x08;
    *ddr_h |= 0x10;
    *ddr_h |= 0x20;
    *ddr_h |= 0x40;    
    *ddr_b &= 0x7F;
    *port_b |= 0x80;
    *ddr_c &= 0xF7;
    *ddr_c &= 0xFB;
    *port_c |= 0x08;
    *port_c |= 0x04;

    *ddr_c |= 0x02;
    *ddr_c |= 0x01;
    *ddr_d |= 0x80;
}
void lcdError (){   //Displays error on LCD
  lcd.setCursor(0,0);
  lcd.print("   ***ERROR***");
  lcd.setCursor(0,1);
  lcd.print("  WATER TOO LOW");
}
void updateFunc(unsigned long x){
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    if(x - prevMillis >= updateTime && state != 0) {
    prevMillis = x;
    lcd.setCursor(0,0);
    lcd.print(hum);
    lcd.setCursor(0,1);
    lcd.print(temp);
}
}

void fanMotor(bool state)   //Turns fan on/off based on parameter
{
    if(state == true)       //If state true, fan turns on
    {
        *port_c |= 0x02;  //pin 36
        *port_c |= 0x01;  //pin 37
        *port_d &= 0x7F;  //pin 38
    }
    else if(state == false) //If state false, fan turns off
    {
        *port_c &= 0xFD;
        *port_c &= 0xFE;
        *port_d &= 0xBF;
    }
}

void stepperControl() //Controls whether stepper moves or not based on buttons
{
if(*pin_c & 0x08)      //If pin34 high, execute
{
  stepper.setSpeed(5);
    stepper.step(10);  //Opens vent
    stepperState = 1;
}
else if(*pin_c & 0x04)      //If pin35 high, execute
{
    stepper.setSpeed(5);
    stepper.step(-10); //Closes vent
    stepperState = 1;
}
else
{
  stepperState = 0;
}
}

void swapTimer()                //Timer that prints to serial monitor whenever state/stepper changes
{
  change = false;               //Defaults to nothing changed
    DateTime now = rtc.now();   //Creates RealTimeClock
    char time[80];             //Large buffer to ensure everything is printed
    char stringStatement[70] = "Default status";   //Temporary generic string statement to use as buffer

    if(stepperState == 1 && prevStepperState == 0)       //Checks for state of stepper motor
    {
      change = true;
      strcpy(stringStatement, "Vent changing position, stepper motor is on.");
    }
    if(stepperState == 0 && prevStepperState == 1)
    {
      change = true;
      strcpy(stringStatement, "Vent stopped changing position, stepper motor is off.");
    }
    if(state != prevState)
    {
      change = true;
        switch (state){
       case 0: //disabled state
          strcpy(stringStatement, "State changed to disabled.");
           break;
       case 1: //running state
            strcpy(stringStatement, "State changed to running.");
            break;
        case 2: //idle state
            strcpy(stringStatement, "State changed to idle.");
            break;
        case 3: //error state
            strcpy(stringStatement, "State changed to error.");
            break;
    }
}
if(change == true)
{
  sprintf(time, "%04d/%02d/%02d %s %02d:%02d:%02d %s", 
        now.year(), now.month(), now.day(),
        daysOfTheWeek[now.dayOfTheWeek()],
        now.hour(), now.minute(), now.second(), stringStatement);   //Combines integers into a char string

    printChar(time);  //Prints current time and state change or vent position change.
}
  prevState = state;
  prevStepperState = stepperState;
}

unsigned char U0kbhit()
{
  return *myUCSR0A & RDA; //Checks if serial transmission is empty
}
unsigned char U0getchar()
{
  return *myUDR0; //Reads keyboard input to send out
}
void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);  //Checks if there is data arduino wants to send back
  *myUDR0 = U0pdata;            //Sends data back via UDR register
}

void printChar(char cs1[])  //Serial.println command
{
  int i = 0;                //Initializes count variable
  while(cs1[i] != '\0')     //Checks array element until equals '\0' or nothing
  {
    U0putchar(cs1[i]);      //Puts characters in terminal one-by-one
    i++;                    //Increases count iteration to goto next character
  }
  U0putchar('\n');          //Ends the line on the terminal

}