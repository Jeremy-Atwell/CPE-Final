void error()
{
    fanMotor(FALSE);
    lcd.cursor(0,0);
    lcd.write("ERROR");

    //LED  to add later here(just change port for Red LED to HIGH)
    
    if(resetButton == 1 && water > waterThreshold)
    {
        state = 2; 
    }
}
