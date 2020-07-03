/*
 * step divider using a teensy 3.0
 * 
 * //--------connections-------------//
 * clockOutputs = 5,6,7,8,10,11,12
 * clock input = pin3
 * reset input = pin4
 * Rotate input = pin21 (shift divs along)
 * led strip = pin15
 * button for trig or gate pulse - pin20
 */

//-------------------------------------------------------
 /*this sketch uses some code from 
 * https://github.com/joeSeggiola/arduino-eurorack-projects/tree/master/clock-divider
* voltage protection thread - https://forum.arduino.cc/index.php?topic=586153.0
* 
*/

#include <FastLED.h>
#define NUM_LEDS 7
#define DATA_PIN 15
CRGB leds[NUM_LEDS];

#include <Bounce2.h>

// --------set up the ins and outs----------//
int clockOutputs[7] = {6,7,8,5,10,11,12}; //divider outputs 

const int ROT = 21; // use loop checking for rotating as alternative to interrupts
unsigned long newTime;
unsigned long oldTime;
unsigned long triggerTime;//1000 for gate mode?;// trigger time needs to be diff for Gate or trigger
unsigned long triggerGate = 1000;
unsigned long triggerTrig =50;
volatile bool RotFlag = false;
int check =0;
//---------------//
//add switch for trigger being Gate sent or trigger sent = different reset times
//if gate mode, div indicator leds = green
//if trigger mode, div indicators = blue
int gateModePin =20;//pin for gate mode
boolean currentState = LOW;
boolean lastState = LOW;
boolean switchState = LOW;
Bounce debouncer5 = Bounce(); // Instantiate a Bounce object

//---------------//

const int CLOCK_INPUT = 3;// Input signal pin, using interrupts
const int RESET_INPUT = 4;// Reset signal pin, using interrupts

volatile bool clock1 = false; // Clock signal digital reading, set in clock ISR
volatile bool clockFlag = false; // Clock signal change flag, set in  clock ISR
volatile bool resetFlag = false; // Reset flag, set in the reset ISR

unsigned long n = 7; // Number of divisions
unsigned long count = -1; // Input clock counter, -1 in order to go to 0 no the first pulse
int DIVISIONS[8] { 1, 2, 4, 8, 16, 32,64,0}; //divisions of the input clock 
int divHolderEnd =0; //slot for rotarion value
int divHolderFront=0; //slot for rotation value

//led on time
unsigned long newOnTime[7] = {0,0,0,0,0,0,0};
unsigned long oldOnTime[7] = {0,0,0,0,0,0,0};
unsigned long onTime = 50;//time for div led indicator to be on
int a; //for colour shift
int b; //for colour shift
int c; //colour change blue/off for gate or trigger indication
//------------------------------------------//

void setup() {
//Serial.begin(38400);
//set output pins and put them low
  for (int i = 0; i < n; i++) {
    pinMode(clockOutputs[i], OUTPUT);
        digitalWrite(clockOutputs[i], LOW);
  }

 // add interrupts and set as inputs
  pinMode(CLOCK_INPUT, INPUT);
  pinMode(RESET_INPUT, INPUT);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INPUT), isrClock, RISING);//CHANGE?
  attachInterrupt(digitalPinToInterrupt(RESET_INPUT), isrReset, RISING);
  
  pinMode(ROT, INPUT);

  pinMode(gateModePin, INPUT_PULLUP);//for the rotation button
  debouncer5.attach(gateModePin);
  debouncer5.interval(25); // interval in ms

//set out the leds
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS); 
  FastLED.setBrightness(20);

//test on startup
  colorWipe(255,   0,   0, 50); // Red
  colorWipe(  0,   0, 255, 50); // Blue
  colorWipe(  0, 255,   0, 50); // Green
  colorWipe(  0, 0, 0, 50); // black

 }//-------------end of startup----------//

void loop() {

//------check if gate or trigger-------//
 {//---- change state for gateMode on/off button------//
  currentState = digitalRead(gateModePin);
  if (currentState == LOW && lastState == HIGH){//if button has just been pressed
 //   Serial.println("pressed");//debug
    //toggle the state of the LED
    if (switchState == LOW){
     triggerTime =triggerTrig;
     c=255; //change the led colour by changing RGB value
      switchState = HIGH;
    } else {
    triggerTime = triggerGate;
    c=0; //change the led colour
      switchState = LOW;
    }
  }
  lastState = currentState;
}//---------end of gateMode selection------//


//-----------check if rotation input required---------//
if(RotFlag == true){
 if(millis() > (newTime + triggerTime)){
  RotFlag = false;}
  }
  
if(RotFlag == false){
  check = analogRead(ROT);//check to see if input on A6
  if (check >250){
    RotFlag = true;
    divHolderFront = DIVISIONS[6];
        for(int rotate=6; rotate>0;rotate--){ 
          DIVISIONS[rotate] = DIVISIONS[rotate-1];
  //        Serial.println("yes");
          }
    DIVISIONS[0] = divHolderFront; 
    newTime =millis();}
 }
  
//------check if clock signal received------------------//
  // Clock signal changed
  if (clockFlag) {
    clockFlag = false;
    if (clock1) {
      // Clock rising, update counter
      if (resetFlag) {
        
        resetFlag = false;
        count = 0;
      } else {
        count++;
      }
      //-------------------------
   //   if(count >= 64){//(16*4)){
   //     count=0;
   //   }
    }
    processTriggerMode();//update outputs
  }

//---------------------switch of leds if on for long enough---//
//switch the div indicator led off if on for long enough
for(int i = 0;i<n;i++){   
        if(newOnTime[i] > oldOnTime[i]+onTime){
         digitalWrite(clockOutputs[i],LOW);
         leds[i].setRGB(0,0,0);
          }
         FastLED.show();
      }
} //----------end of loop------//


void isrClock() {
  clock1 = digitalRead(CLOCK_INPUT) == HIGH;
  clockFlag = true;
}
void isrReset() {
  resetFlag = true;
}
///--------------------------///
void processTriggerMode() {

  if (clock1) {

    if (count == 0){a = 0;b = 255; }
      else  {a = 255;b = 0;}

   for (int x = 0; x < n; x++) {
    if (count % DIVISIONS[x] == 0){
      leds[x].setRGB(b,a,c);
      oldOnTime[x] =newOnTime[x];
      newOnTime[x] = millis();
      digitalWrite(clockOutputs[x],HIGH);
        }//end of if

   else {
        for (int y = 0; y < n; y++) {
             digitalWrite(clockOutputs[y],LOW);}
          } FastLED.show();
          //end of else  
      }
      //end of for
    }//end of if clock

}

///

void colorWipe(int r, int g, int b, int wait) {
  for(int i=0; i<NUM_LEDS; i++) { // For each pixel in strip...

    leds[i].setRGB(r,g,b);
    FastLED.show();                          //  Update strip to match
    delay(wait);                         //  Pause for a moment
  }
}
