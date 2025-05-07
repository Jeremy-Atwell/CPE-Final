void running()
{
    fanMotor(TRUE);

    //LED blue to add later here(just change port for blue LED to HIGH)
    
    if(temp < threshold)
    {
        state = 2;
    }
    else if(water < waterThreshold)
    {
        state = 3;
    }
}