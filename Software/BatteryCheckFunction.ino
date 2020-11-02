 /*
  * analog in Arduino : 5V == 1023
  * Lithium Battery charging module:
    -Fully charged Battery's Voltage is 3.9 Volt. [ADC Value Min Limit = 3.9V*1023/5V = 797.94]
    -Normal Voltage of Battery is 3.7 Volt. [ADC Value Min Limit = 3.7V*1023/5V = 757.02]
    -Empty Voltage of Battery is 3.3 Volt. [ADC Value Min Limit = 3.3V*1023/5V = 675.15]
    -Charging's Cutoff Voltage at 4.2V. [ADC Value Min Limit = 4.2V*1023/5V = 859.32]
    -so if voltage of battery is near to 4.1 and above, means charging. [ADC Value Min Limit = 4.1V*1023/5V = 838.86]
    -TP4056 output Voltage min is 4.137 volt and max is 4.263 volt.
 */
 
void batteryCheck(){
  char charg_warning=0; // 0: warning, 1: charge
  int required_empty_cells = 5;
  int adcValue = analogRead(BATTERY_PIN);
  if(adcValue > 830) //Charging Mode:
    {charg_warning = 1; chargingMode();}
  else if(adcValue > 777) required_empty_cells = 0; //Full Battery, Margain: 3.8 to 3.9 :: 777.48 to 797.94
  else if(adcValue > 716) required_empty_cells = 1; //Half Full Battery, Margain: 3.5 to >3.8 :: 716.1 :: 777  
  else if(adcValue > 675) required_empty_cells = 2; //Low Battery, Margain: 3.3 to >3.5 :: 675.15 :: 716.1 
  else                    required_empty_cells = 3; //Empty Battery, Warning Must charge. 
    
  if(required_empty_cells!=5){
     if(currentEmptyCell > required_empty_cells)  //must add cells.
        do{ cellChange(battery_Bitmap,ADD);}while(currentEmptyCell > required_empty_cells);
     else if (currentEmptyCell < required_empty_cells) // must remove cells.  
        do{ cellChange(battery_Bitmap,REMOVE);}while(currentEmptyCell < required_empty_cells);
  }
  if(currentEmptyCell >= 2) {
    if(charg_warning == 0) // Warning
    {
      lcd.setCursor(47, 0);
      lcd.drawBitmap(mark_Bitmap, MARK_WIDHT, ceil(MARK_HEIGHT / 8.0));    
   }
   else if(charg_warning == 1) //Charging
   {
    sparkToggle();
   }
  }
  else {
    lcd.setCursor(47, 0);
     drawing_clear(47,0,2,1);// delete spark drawing.
  }
}
