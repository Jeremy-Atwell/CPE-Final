/*
*/

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;


void setup()
{
    U0init(9600);
}


void loop()
{

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