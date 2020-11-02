/*
  * PCD8544 - Interface with Philips PCD8544 (or compatible) LCDs.
     Used Libarary for PCD8544 Copyright (c) 2010 Carlos Rodrigues.
  * In V4: adding reading of Battery through analog pin to detect state whether 
           charging or discharging.
  * In V5: adding button.         
           assume it's pull up connection.
  * In V6: adding Buzzer.         
           Action depend on Temp value.
  * In v7: Testing and improving some function.         
*/
/*
 * Connection of Nokia-5110's pins:
 * Connect the pin RST to the pin 6 of Arduino through the 10K resistor.
 * Connect the pin SCE to the pin 5 of Arduino through the 1K resistor.
 * Connect the pin D/C to the pin 4 of Arduino through the 10K resistor.
 * Connect the pin DIN to the pin 3 of Arduino through the 10K resistor.
 * Connect the pin CLK to the pin 2 of Arduino through the 10K resistor.
 * Connect the pin VCC to the 3.3V pin of Arduino.
 * Connect the pin LED to the middle pin of 1k potentiometer through 330 ohm resistor 
   and connect the other two pins to the VCC and the ground.
 * Connect the pin GND to the GND of Arduino.
 */

/*MLX90614 needed libraries:*/
#include <Wire.h>
#include <Adafruit_MLX90614.h>
/*Nokia 5110 Libarary:*/
#include <PCD8544.h>
//MLX:
#define FACTOR 1.14
#define THERESHOLD 40
//Nokia 5110:
#define RST 7
#define CE  6
#define DC  5
#define DIN 4
#define CLK 3

// Bounds of the display
#define LCD_X_WIDTH 84
#define LCD_Y_HEIGHT 48
#define CONTRAST 50 // for 5V: 24 , for 3V: 50 .
#define BATTERY_WIDHT 23
#define BATTERY_HEIGHT 10
#define CHARGESPARK_WIDHT 5
#define CHARGESPARK_HEIGHT 10
#define MEDICALBAG_WIDHT 26
#define MEDICALBAG_HEIGHT 26
#define BELL_WIDHT 8
#define BELL_HEIGHT 9
#define MARK_WIDHT 6
#define MARK_HEIGHT 10

#define NUMOFCELLS 3

// Battery pin:
#define BATTERY_PIN A0
#define REMOVE 0
#define ADD 1
// Button pin:
#define BUTTON_PIN 2
// Buzzer pin:
#define BUZZER_PIN 8

int btnValue = 0;
char toggle = 0, flag = 0;
char currentEmptyCell=0; // assume at first the battery is fully charged.
unsigned long prev = 0, prev1 = 0;
double objTemp = 0;

const byte chargSpark_Bitmap[] {
  0x30, 0x3c, 0xfe, 0xf0, 0x30,
  0x00, 0x00, 0x01, 0x00, 0x00
};
const byte bell_Bitmap[]={
  0x00, 0xfc, 0xfe, 0xff, 0xff, 0xfe, 0xfc, 0x00, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};
const byte mark_Bitmap[] = { 
   0x00, 0x7f, 0x7f, 0x7f, 0x7f, 0x00,  
   0x00, 0x03, 0x03, 0x03, 0x03, 0x00, 
};
byte battery_Bitmap[] = {
  0xfc, 0xfc, 0xff, 0x01, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0x01, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0x01, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0x01, 0xff,
  0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03
};
const byte medical_bag_Bitmap[] = { 
  0x00, 0x80, 0xb8, 0xbc, 0xbe, 0x9e, 0x8f, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x8f, 0x9e, 0xbe, 0xbc, 0xb8, 0x80, 0x00, 
  0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x1f, 0x8f, 0x8f, 0xe7, 0xe7, 0x8f, 0x8f, 0x1f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xf8, 0xf1, 0xf1, 0xe7, 0xe7, 0xf1, 0xf1, 0xf8, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03
};

static PCD8544 lcd = PCD8544(CLK, DIN, DC, RST, CE);
/*Create object called mlx:*/
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void cellChange(byte *arrPtr,char state);
void sparkToggle(void);
void chargingMode(void);
void batteryCheck(void);
void drawing_clear(char width, char height,char coloums, char pixels);

void setup() {
  //Serial.begin(9600);
  pinMode(BATTERY_PIN,INPUT);
  pinMode(BUTTON_PIN,INPUT);
  pinMode(BUZZER_PIN,OUTPUT);
  mlx.begin(); //Initiate MLX.
  // PCD8544-compatible displays may have a different resolution...
  lcd.begin(LCD_X_WIDTH, LCD_Y_HEIGHT);
  lcd.setContrast(CONTRAST);
  delay(500);
}

void loop() {
  objTemp = 0;
  batteryCheck();
  btnValue = digitalRead(BUTTON_PIN);
  if(btnValue == LOW){
    delay(70);
    btnValue = digitalRead(BUTTON_PIN);
    if(btnValue == LOW) objTemp = mlx.readObjectTempC();
    }  
  lcd.setCursor(0, 2);
  lcd.print("Amb: ");
  lcd.print(mlx.readAmbientTempC(),1);  
  lcd.setCursor(0, 4);
  lcd.print("Obj: ");
  lcd.print(objTemp*FACTOR,1);
  //Serial.println(objTemp*FACTOR);
  if(objTemp >= THERESHOLD) {
    digitalWrite(BUZZER_PIN,HIGH);
    lcd.setCursor(20,0);
    lcd.drawBitmap(bell_Bitmap,BELL_WIDHT,ceil(BELL_HEIGHT / 8.0) );
    lcd.setCursor(58,2);
    lcd.drawBitmap(medical_bag_Bitmap,MEDICALBAG_WIDHT,ceil(MEDICALBAG_HEIGHT / 8.0) );
  }
  else {
    digitalWrite(BUZZER_PIN,LOW);
    drawing_clear(20,0,2,3); //clear bell
    drawing_clear(58,2,4,21);//clear medical bag
  }
}

/*
  * The typical VDD dependence of the ambient and object temperature is 0.6°C/V.
   double vddCurrnet = 0;
   double ambientTempCompensated = mlx.readAmbientTempC()-(vddCurrnet-VDD)*0.6;
*    double objectTempCompensated = mlx.readObjectTempC()-(vddCurrnet-VDD)*0.6;
*/
