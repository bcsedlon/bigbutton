#include "Arduino.h"

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // Disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // Re-enable ADC

#include "RCSwitch.h"

RCSwitch rcSwitch = RCSwitch();
#define RFRX_PIN 0 //D2
//#define RFRX_PIN 1 //D3
// Receiver on interrupt 0 => that is pin #2
// Receiver on interrupt 1 => that is pin #3
//#define RFTX_PIN 4
//#define RFEN_PIN 0

#define IN0_PIN		0
#define LED_PIN		1 //13
#define OUT0_PIN 	1 //12

unsigned long rfMillis;
unsigned long onMillis;
//unsigned long wakeupMillis;

unsigned long standbyMillis;
bool standby;

void setup()
{
	/*
	Serial.begin(115200);
	Serial.println("\nBIGBUTTON");
	*/

	pinMode(LED_PIN, OUTPUT);
	for(int i = 0; i < 4; i++) {
		digitalWrite(LED_PIN, HIGH);
		delay(250);
		digitalWrite(LED_PIN, LOW);
		delay(250);
	}

	pinMode(IN0_PIN, INPUT_PULLUP);

	for (byte i = 0; i < 6; i++) {
		pinMode(i, INPUT);      // Set all ports as INPUT to save energy
		digitalWrite (i, LOW);  //
	}
	adc_disable();          	// Disable Analog-to-Digital Converter


#ifdef RFRX_PIN
	rcSwitch.enableReceive(RFRX_PIN);
#endif

#ifdef RFEN_PIN
  pinMode(RFEN_PIN, OUTPUT);
  digitalWrite(RFEN_PIN, true);
#endif

#ifdef RFTX_PIN
	rcSwitch.enableTransmit(RFTX_PIN);
#endif

	wdt_reset();            // Watchdog reset
	wdt_enable(WDTO_250MS);	// Watchdog enable Options: 15MS, 30MS, 60MS, 120MS, 250MS, 500MS, 1S, 2S, 4S, 8S
	WDTCR |= _BV(WDIE);     // Interrupts watchdog enable
	sei();                  // Enable interrupts
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Sleep Mode: max

	standbyMillis = millis();
	standby = true;
}

void loop() {

	pinMode(IN0_PIN, INPUT_PULLUP);
	if(!digitalRead(IN0_PIN)) {
		standbyMillis = millis();
		standby = true;

		rcSwitch.enableReceive(RFRX_PIN);
	}
	if(millis() - standbyMillis > 10000) {
		standby = false;

		wdt_enable(WDTO_250MS);	// Watchdog enable Options: 15MS, 30MS, 60MS, 120MS, 250MS, 500MS, 1S, 2S, 4S, 8S
		//WDTCR |= _BV(WDIE);     // Interrupts watchdog enable
		WDTCR = _BV(WDIE);      // Interrupts watchdog enable
		sei();                  // Enable interrupts
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	}
	if(!standby) {
		sleep_enable();
		sleep_cpu();
	}
	//pinMode(LED_PIN, OUTPUT);
	//digitalWrite(LED_PIN, standby);

	if(standby) {

		if(millis() - onMillis > 15000 && onMillis != 0) {
			onMillis = 0;
			digitalWrite(LED_PIN, false);
			digitalWrite(OUT0_PIN, false);
			pinMode(LED_PIN, INPUT);
			pinMode(OUT0_PIN, INPUT);
		}

	#ifdef RFRX_PIN
		if (rcSwitch.available()) {

			standbyMillis = millis();

			unsigned long code = rcSwitch.getReceivedValue();
			//if(code == 2664495L) {
			if(code == 43055) {
				//1 on
				onMillis = millis();
				pinMode(LED_PIN, OUTPUT);
				pinMode(OUT0_PIN, OUTPUT);
				digitalWrite(LED_PIN, true);
				digitalWrite(OUT0_PIN, true);
			}
			if(code == 2664493L) {
				//2 on
			}
			//if(code == 2664494L) {
			if(code == 43054) {
				//1 off
				digitalWrite(LED_PIN, false);
				digitalWrite(OUT0_PIN, false);
				pinMode(LED_PIN, INPUT);
				pinMode(OUT0_PIN, INPUT);
			}
			if(code == 2664492L) {
				//2 off
			}

			/*
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
			*/
		 }
	#endif

		//if(millis() - wakeupMillis > 125) {
		//	sleep_enable();
		//	sleep_cpu();
		//}
	}
}

ISR (WDT_vect) {
	WDTCR |= _BV(WDIE);
	//wakeupMillis = millis();
}

