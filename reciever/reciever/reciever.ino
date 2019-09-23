#include "Arduino.h"
//The setup function is called once at startup of the sketch

#include "RCSwitch.h"
RCSwitch rcSwitch = RCSwitch();
#define RFRX_PIN 1 //D3
// Receiver on interrupt 0 => that is pin #2
// Receiver on interrupt 1 => that is pin #3
//#define RFTX_PIN 4

#define IN1_PIN 5
#define LED_PIN 13

#define OUT1_PIN 12
#define OUT2_PIN 11
#define OUT3_PIN 10
#define OUT4_PIN 9

unsigned long onMillis;
unsigned long onTimeout = 30 * 1000;
unsigned long on3Timeout = 500;
unsigned long on3Delay = 1000;
unsigned long on4Timeout = 500;

int outNo = 2;
bool changeOutNo;

void setup()
{
	Serial.begin(115200);
	Serial.println("\nBIGBUTTON");

	pinMode(IN1_PIN, INPUT_PULLUP);
	pinMode(LED_PIN, OUTPUT);

	pinMode(OUT1_PIN, OUTPUT);
	pinMode(OUT2_PIN, OUTPUT);
	pinMode(OUT3_PIN, OUTPUT);
	pinMode(OUT4_PIN, OUTPUT);

	digitalWrite(OUT1_PIN, true);
	digitalWrite(OUT2_PIN, true);
	digitalWrite(OUT3_PIN, true);
	digitalWrite(OUT4_PIN, true);

#ifdef RFRX_PIN
	rcSwitch.enableReceive(RFRX_PIN);
#endif

#ifdef RFTX_PIN
	rcSwitch.enableTransmit(RFTX_PIN);
#endif

	//onMillis = millis();
}

void loop()
{

	if(onMillis && ((millis() - onMillis) < onTimeout)) {
		digitalWrite(OUT1_PIN, false);
		//digitalWrite(OUT2_PIN, false);

		changeOutNo = true;
	}
	else {
		digitalWrite(OUT1_PIN, true);
		//digitalWrite(OUT2_PIN, true);

		//TODO:
		if(changeOutNo) {
			changeOutNo = false;
			outNo++;
			if(outNo > 4)
				outNo = 2;
			}
	}
	if(onMillis && (millis() - onMillis < on3Timeout + on3Delay) && (millis() - onMillis) > on3Delay) {
		if(outNo == 2)
			digitalWrite(OUT2_PIN, false);
		if(outNo == 3)
			digitalWrite(OUT3_PIN, false);
		if(outNo == 4)
			digitalWrite(OUT4_PIN, false);
	}
	else {
		digitalWrite(OUT2_PIN, true);
		digitalWrite(OUT3_PIN, true);
		digitalWrite(OUT4_PIN, true);
	}


	if(!digitalRead(IN1_PIN)) {
		digitalWrite(LED_PIN, true);
		onMillis = millis();
		digitalWrite(LED_PIN, false);
	}

#ifdef RFRX_PIN
	if (rcSwitch.available()) {
		digitalWrite(LED_PIN, true);

		//onMillis = millis();

		unsigned long code = rcSwitch.getReceivedValue();
		//if(code == 2664495L) {
		if(code == 43055) {
			//1 on
			onMillis = millis();
		}
		if(code == 2664493L) {
			//2 on
		}
		//if(code == 2664494L) {
		if(code == 43054) {
			//1 off
		}
		if(code == 2664492L) {
			//2 off
		}

		Serial.print(rcSwitch.getReceivedValue());
		Serial.print('\t');
		Serial.print(rcSwitch.getReceivedBitlength());
		Serial.print('\t');
		Serial.print(rcSwitch.getReceivedDelay());
		Serial.print('\t');
		unsigned int* p = rcSwitch.getReceivedRawdata();
		for(int i = 0; i < RCSWITCH_MAX_CHANGES; i++)
			Serial.print(*(p + i));
		Serial.print('\t');
		Serial.print(rcSwitch.getReceivedProtocol());
		rcSwitch.resetAvailable();
		Serial.println();

		digitalWrite(LED_PIN, false);
 	 }
#endif
}
