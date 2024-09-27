/*
//Measure Angle with a MPU-6050(GY-521)
 ATtiny85 as an I2C Master  Ex1          BroHogan                      1/21/11
 * I2C master reading 
 * SETUP:
 * ATtiny Pin 1 = (RESET) N/U                      ATtiny Pin 2 = (A3)  D3  potenciometer
 * ATtiny Pin 3 = A2 D4 sirena                     ATtiny Pin 4 = GND
 * ATtiny Pin 5 = SDA  0                           ATtiny Pin 6 = led  1
 * ATtiny Pin 7 = SCK  A1 D2                       ATtiny Pin 8 = VCC (2.7-5.5V)
 * NOTE! - It's very important to use pullups on the SDA & SCL lines!
 * TinyWireM USAGE & CREDITS: - see TinyWireM.h
 * NOTES:
 meranie stavu baterie
 https://howtomechatronics.com/tutorials/arduino/arduino-and-mpu6050-accelerometer-and-gyroscope-tutorial/
*/
#include <TinyWireM.h>                  // I2C Master lib for ATTinys which use USI

// procesor ATTINY85  8Mhz    E2  DF  FF 

#include <Adafruit_NeoPixel.h>
#define PIN   1
#define NUMPIXELS     3
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

//#define MPU_addr 0x68
#define MPU_addr 0x53  //ADXL345
#define LED 1
#define pot A3
#define piezo A2
int16_t AcX, AcY, AcZ,GcX,GcY,GcZ;
int16_t temp;
int16_t pamx, pamy, pamz, pamGx, pamGy, pamGz;  
uint32_t timer;
uint16_t cas;
uint16_t melody;
uint16_t casled;
uint16_t cas_alarm;
int16_t citlivost;
uint8_t caspixel;
uint8_t pT;
uint8_t postup;
uint8_t r,g,b;
bool alarm;



//-------------------------------------------------------------------
int getVCC() {
  //reads internal 1V1 reference against VCC
   ADMUX = _BV(MUX3) | _BV(MUX2); // For ATtiny85/45
   delay(1); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  uint8_t low = ADCL;
  unsigned int val = (ADCH << 8) | low;
  //discard previous result
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  low = ADCL;
  val = (ADCH << 8) | low;
   return ((long)1024 * 1100) / val;  
}




//-podprogram alarm signal--------------------------------------------------------------
void ton1 () {
 cas++;melody++;
if(pT==0) {digitalWrite(piezo,!digitalRead(piezo));cas=0;melody=0;pT=10;}                  
if(pT==10 && /*&& melody<10 &&*/ cas>=2)  {digitalWrite(piezo,!digitalRead(piezo));cas=0;}  
}


//-----------

void setup(){
    
    pixels.begin(); // This initializes the NeoPixel library
    pinMode(piezo,OUTPUT);
    digitalWrite(piezo,false); 
    cas_alarm=1000;pT=0;postup=0;
   delay(10);
   //  kontrola baterie ---------------------------------------
    if (getVCC() >=2900) {r=0;b=0;g=4;}
    if (getVCC() <2900 && getVCC()>=2700) {r=5;b=0;g=5;}
    if (getVCC() <2700 ) {r=5;b=0;g=0;}
   pixels.setPixelColor(0, pixels.Color(g,r,b));  pixels.setPixelColor(1, pixels.Color(g,r,b));  pixels.setPixelColor(2, pixels.Color(g,r,b)); 
   pixels.show();  
  
   delay(2000);
   
 // pinMode(LED,OUTPUT);
  TinyWireM.begin();                    // initialize I2C lib
  TinyWireM.beginTransmission(MPU_addr);
  TinyWireM.write(0x6B);
  TinyWireM.write(0);
  TinyWireM.endTransmission(true);
 
}
void loop(){
 //---------------------------------------
  if (millis() - timer >= 100) {
   timer = millis();      casled++;caspixel++;cas_alarm++;
   }
  if (casled>10000) {casled=10000;}
   if (cas_alarm>10000){cas_alarm=10000;}
 //--------------------------------------- 
  TinyWireM.beginTransmission(MPU_addr);
  TinyWireM.write(0x3B);                          //adresa od zrychlenie
 // TinyWireM.write(0x43);                        //adresa od gyroskopu
  TinyWireM.endTransmission(false);
  TinyWireM.requestFrom(MPU_addr,14);    //14 bytov prijem
  AcX=TinyWireM.read()<<8|TinyWireM.read();   //zrychlenie x
  AcY=TinyWireM.read()<<8|TinyWireM.read();
  AcZ=TinyWireM.read()<<8|TinyWireM.read();
  temp=TinyWireM.read()<<8|TinyWireM.read();  //teplota
  GcX=TinyWireM.read()<<8|TinyWireM.read();   //gyroskop X
  GcY=TinyWireM.read()<<8|TinyWireM.read();
  GcZ=TinyWireM.read()<<8|TinyWireM.read();
  
   

    citlivost=analogRead(pot)*6+500;
    if (pamx>AcX+citlivost  ||  pamx<AcX-citlivost  || pamy>AcY+citlivost  ||  pamy<AcY-citlivost  || pamz>AcZ+citlivost  ||  pamz<AcZ-citlivost/* ||
      pamGx>GcX+citlivost ||  pamGx<GcX-citlivost || pamGy>GcY+citlivost ||  pamGy<GcY-citlivost || pamGz>AcZ+citlivost ||  pamGz<GcZ-citlivost */){casled=0;pamx=AcX;pamy=AcY;pamz=AcZ;pamGx=GcX;pamGy=GcY;pamGz=GcZ;
       if (alarm==false) {alarm=true;}
       }
  
 
 //--------------------------------------------------------------------------------------------------------------------------------------------------
 

//------  neopixel led -----------------------------------------------------------------------------------------------------------------------
  
  if (casled<15) {
     if (casled<5){r=0;b=255;g=0;}
     if (casled>=5 && casled<10){r=255;b=0;g=0;}
     if (casled>=10 ){r=0;b=0;g=255;}
     
   if (caspixel>=1 && postup==0) {caspixel=0;postup=10;
     pixels.setPixelColor(0, pixels.Color(0,0,0));  pixels.setPixelColor(1, pixels.Color(0,0,0));  pixels.setPixelColor(2, pixels.Color(g,r,b)); 
     pixels.show(); 
     }

   if (caspixel>=1 && postup==10) {caspixel=0;postup=20;
     pixels.setPixelColor(0, pixels.Color(0,0,0));  pixels.setPixelColor(1, pixels.Color(g,r,b));  pixels.setPixelColor(2, pixels.Color(0,0,0)); 
     pixels.show(); 
    }
  if (caspixel>=1 && postup==20) {caspixel=0;postup=0;
     pixels.setPixelColor(0, pixels.Color(g,r,b));  pixels.setPixelColor(1, pixels.Color(0,0,0));  pixels.setPixelColor(2, pixels.Color(0,0,0)); 
     pixels.show(); 
    }
  }
//---------------------------------------------------------------------------------------------------------------------------------------------

 if(casled>=15 && casled<100)   {caspixel=0;postup=0;
     r=40;b=0;g=0;
     pixels.setPixelColor(0, pixels.Color(g,r,b));  pixels.setPixelColor(1, pixels.Color(0,0,0));  pixels.setPixelColor(2, pixels.Color(0,0,0)); 
     pixels.show(); 
    }
if(casled>=100 && casled<200)   {
     r=00;b=0;g=40;
     pixels.setPixelColor(0, pixels.Color(0,0,0));  pixels.setPixelColor(1, pixels.Color(g,r,b));  pixels.setPixelColor(2, pixels.Color(0,0,0)); 
     pixels.show(); 
    }
if(casled>=200 && casled<300)   {
     r=0;b=40;g=0;
     pixels.setPixelColor(0, pixels.Color(0,0,0));  pixels.setPixelColor(1, pixels.Color(0,0,0));  pixels.setPixelColor(2, pixels.Color(g,r,b)); 
     pixels.show(); 
    }
  if(casled>=300)   {
      if (getVCC() >=2900) {r=0;b=0;g=4;}
      if (getVCC() <2900 && getVCC()>=2500) {r=5;b=0;g=5;}
      if (getVCC() <2500 ) {r=5;b=0;g=0;}
     pixels.setPixelColor(0, pixels.Color(0,0,0));  pixels.setPixelColor(1, pixels.Color(0,0,0));  pixels.setPixelColor(2, pixels.Color(g,r,b)); 
     pixels.show(); 
    }

 //----------------------------------------------------------   A L A R M  --------------------------------------------------------------------------

// if (cas_alarm<15) {ton1();} else {digitalWrite(piezo,false);}

if (alarm==true && cas_alarm<=2 ) {digitalWrite(piezo,!digitalRead(piezo));} else  {digitalWrite(piezo,false);}
if (alarm==false) {cas_alarm=0;}
if (alarm==true && cas_alarm>5) {alarm=false;} 

if (cas_alarm>1000) {cas_alarm=1000;}

}
