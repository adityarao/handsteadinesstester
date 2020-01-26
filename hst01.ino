#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <Button.h>
#include <ButtonEventCallback.h>
#include <PushButton.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// controls the 7 segment display
#include <TM1637Display.h>

#define PIN D7
#define NUM_LEDS 1
#define DATA_PIN D7
    
#define RESET_BUTTON_PIN D1
#define WIRE_PIN D2
#define SOUND_PIN D5

// 7-segment display 
#define CLK D3
#define DIO D4

#define NORMAL_FLASH_DURATION 1000

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

unsigned long t_NormalFlashDuration = 0;

unsigned int g_wireTouches = 0;

// Create an instance of BasicButton reading digital pin 5
PushButton resetButton = PushButton(RESET_BUTTON_PIN);
PushButton wireButton = PushButton(WIRE_PIN);

CRGB leds[NUM_LEDS];

// 4 segment display
TM1637Display display(CLK, DIO); //set up the 4-Digit Display.

const uint8_t SEG_READY[] = {
	SEG_E | SEG_G,                           // r
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,   // E
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,   // d
	SEG_B | SEG_C | SEG_D | SEG_F | SEG_G    // y
};


const uint8_t SEG_HELP[] = {
	SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // H
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
	SEG_D | SEG_E | SEG_F, 		           // L
	SEG_A | SEG_B | SEG_E | SEG_F | SEG_G  // P
};

const uint8_t SEG_NONE[] = {
	0,  
	0, 
	0, 		  
	0  
};

// tones

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

void setup()
{
	pinMode(SOUND_PIN, OUTPUT);

  	pixels.begin(); // This initializes the NeoPixel library.
 	pixels.setBrightness(255);	

 	display.setBrightness(0x0f);
	// reset display
 	display.setSegments(SEG_READY); 	

    // Open up the serial port so that we can write to it
    Serial.begin(115200);

    // When the button is first pressed, call the function onButtonPressed
    resetButton.onPress(onButtonPressed);
    // Once the button has been held for 1 second (1000ms) call onButtonHeld. Call it again every 0.5s (500ms) until it is let go
    resetButton.onHoldRepeat(1000, 100, onButtonHeld);
    // When the button is released, call onButtonReleased
    resetButton.onRelease(onButtonReleased);


    // When the button is first pressed, call the function onButtonPressed
    wireButton.onPress(onButtonPressed);
    // Once the button/wire has been touched for 100ms call onButtonHeld. Call it again every 0.1s (100ms) until it is let go
    wireButton.onHoldRepeat(100, 100, onButtonHeld);
    // When the button is released, call onButtonReleased
    wireButton.onRelease(onButtonReleased);
    t_NormalFlashDuration = millis();

    g_wireTouches = 0;

	pixels.setPixelColor(0, pixels.Color(0, 0, 255));
	pixels.show();    
}


void loop(){
    // Check the state of the button
    resetButton.update();
    wireButton.update();

    if(millis() - t_NormalFlashDuration > NORMAL_FLASH_DURATION) {
    	t_NormalFlashDuration = millis();
    }
}

// btn is a reference to the button that fired the event. That means you can use the same event handler for many buttons
void onButtonPressed(Button& btn){

    Serial.print("button pressed: ");

    if(btn.is(resetButton))
    	Serial.println("resetButton");

    if(btn.is(wireButton)) {
    	Serial.println("wireButton");

		tone(SOUND_PIN, NOTE_A7, 1000);
		delay(100);
		noTone(SOUND_PIN);
		tone(SOUND_PIN, NOTE_B7, 1000);
		delay(50);
		noTone(SOUND_PIN);    
		tone(SOUND_PIN, NOTE_C7, 1000);
		delay(100);
		noTone(SOUND_PIN);       				
    	
    }

    Serial.print("Total touches : ");
    Serial.println(g_wireTouches);

	pixels.setPixelColor(0, pixels.Color(255, 0, 0));
	pixels.show();
}

// duration reports back how long it has been since the button was originally pressed.
// repeatCount tells us how many times this function has been called by this button.
void onButtonHeld(Button& btn, uint16_t duration, uint16_t repeatCount){

	Serial.print("button has been held for ");
	Serial.print(duration);
	Serial.print(" ms; this event has been fired ");
	Serial.print(repeatCount);
	Serial.println(" times");

	if(btn.is(wireButton)) {
		if(repeatCount%2) {
			pixels.setPixelColor(0, pixels.Color(0, 0, 0));
			pixels.show();  
			display.setSegments(SEG_HELP);
			tone(SOUND_PIN, NOTE_F7, 1000);
			delay(10);
			noTone(SOUND_PIN);
			tone(SOUND_PIN, NOTE_D7, 1000);
			delay(5);
			noTone(SOUND_PIN);    
			tone(SOUND_PIN, NOTE_E7, 1000);
			delay(10);
			noTone(SOUND_PIN); 			
		} else {
			pixels.setPixelColor(0, pixels.Color(255, 0, 0));
			pixels.show();  
			display.setSegments(SEG_NONE);
			// if you press hold for long, its going to increase the counter
			g_wireTouches = g_wireTouches + (repeatCount/5);
			display.showNumberDec(g_wireTouches, false);
		}    	
	}
	
}

// duration reports back the total time that the button was held down
void onButtonReleased(Button& btn, uint16_t duration){
    
    if(btn.is(resetButton)) {
    	Serial.print("resetButton ");
    	g_wireTouches = 0;
		pixels.setPixelColor(0, pixels.Color(0, 0, 255));
		pixels.show();    

		// reset display
 		display.setSegments(SEG_READY); 

		tone(SOUND_PIN, NOTE_G7, 1000);
		delay(100);
		noTone(SOUND_PIN);
		tone(SOUND_PIN, NOTE_F7, 1000);
		delay(50);
		noTone(SOUND_PIN);    
		tone(SOUND_PIN, NOTE_E7, 1000);
		delay(100);
		noTone(SOUND_PIN);   
		tone(SOUND_PIN, NOTE_D7, 1000);
		delay(100);
		noTone(SOUND_PIN);
		tone(SOUND_PIN, NOTE_C7, 1000);
		delay(50);
		noTone(SOUND_PIN);    
		tone(SOUND_PIN, NOTE_B7, 1000);
		delay(100);
		noTone(SOUND_PIN);   
		tone(SOUND_PIN, NOTE_A7, 1000);
		delay(100);
		noTone(SOUND_PIN);  							
    }

    if(btn.is(wireButton)) {
    	Serial.print("wireButton ");	
    	g_wireTouches++ ;
    	// turn off the sound
    	digitalWrite(SOUND_PIN, LOW);   
		pixels.setPixelColor(0, pixels.Color(0, 255, 0));
		pixels.show();
		// reset display
 		display.showNumberDec(g_wireTouches, false); 		
    }

	Serial.print("button released after ");
	Serial.print(duration);
	Serial.println(" ms");

    Serial.print("Total touches : ");
    Serial.println(g_wireTouches);	
}
