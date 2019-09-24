#include "Arduino.h"

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // Disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // Re-enable ADC

#include "RCSwitch.h"

RCSwitch rcSwitch = RCSwitch();
#define RFRX_PIN 	0 	//D2
//#define RFRX_PIN  1 	//D3
// Receiver on interrupt 0 => that is pin #2
// Receiver on interrupt 1 => that is pin #3
//#define RFTX_PIN 4
#define RFEN_PIN 	4

#define INOUT0_PIN	0
#define LED_PIN		1

void setup() {
	//Serial.begin(115200);
	//Serial.println("\nBIGBUTTON");

	pinMode(INOUT0_PIN, INPUT); //_PULLUP);

	for (byte i = 0; i < 6; i++) {
		pinMode(i, INPUT);      // Set all ports as INPUT to save energy
		digitalWrite (i, LOW);  //
	}
	adc_disable();          	// Disable Analog-to-Digital Converter

	goWaiting();

#ifdef RFTX_PIN
	rcSwitch.enableTransmit(RFTX_PIN);
#endif

	wdt_reset();            // Watchdog reset
	wdt_enable(WDTO_250MS);	// Watchdog enable Options: 15MS, 30MS, 60MS, 120MS, 250MS, 500MS, 1S, 2S, 4S, 8S
	//WDTCR |= _BV(WDIE);     // Interrupts watchdog enable
	//sei();                  // Enable interrupts
	//set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Sleep Mode: max
}

bool blink;
unsigned long blinkMillis;

#define WAITING 	0
#define WORKING		1
#define	SLEEPING	2
int state;
unsigned long waitingMillis;
unsigned long workingMillis;

void goWorking() {
	rcSwitch.disableReceive();
#ifdef RFEN_PIN
	//pinMode(RFEN_PIN, OUTPUT);
	//digitalWrite(RFEN_PIN, false);
#endif

	workingMillis = millis();
	pinMode(LED_PIN, OUTPUT);
	state = WORKING;
}

void goWaiting() {
	rcSwitch.enableReceive(RFRX_PIN);
#ifdef RFEN_PIN
	pinMode(RFEN_PIN, OUTPUT);
	digitalWrite(RFEN_PIN, false);
#endif

	waitingMillis = millis();
	pinMode(LED_PIN, OUTPUT);
	state = WAITING;
}

void goSleeping() {
	rcSwitch.disableReceive();
#ifdef RFEN_PIN
	//pinMode(RFEN_PIN, INPUT);

	pinMode(RFEN_PIN, OUTPUT);
	digitalWrite(RFEN_PIN, true);
#endif

	wdt_enable(WDTO_250MS);	// Watchdog enable Options: 15MS, 30MS, 60MS, 120MS, 250MS, 500MS, 1S, 2S, 4S, 8S
	//WDTCR |= _BV(WDIE);     // Interrupts watchdog enable
	WDTCR = _BV(WDIE);      // Interrupts watchdog enable
	sei();                  // Enable interrupts
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	pinMode(LED_PIN, INPUT);
	pinMode(INOUT0_PIN, INPUT);
	state = SLEEPING;
}

void loop() {
	wdt_reset();

	if(millis() - blinkMillis > (state + 1) * 64) {
		blinkMillis = millis();
		blink = !blink;
		digitalWrite(LED_PIN, blink);
	}

	if(state == SLEEPING) {
		pinMode(INOUT0_PIN, INPUT);
		if(digitalRead(INOUT0_PIN)) {
			goWorking();
		}
		else {
			sleep_enable();
			sleep_cpu();
		}
	}

	if(state == WORKING) {
		if(millis() - workingMillis > 6000) {
			goWaiting();
		}

		pinMode(INOUT0_PIN, INPUT);
		if(digitalRead(INOUT0_PIN)) {
			workingMillis = millis();
		}
	}

	if(state == WAITING) {
		pinMode(INOUT0_PIN, INPUT);
		if(digitalRead(INOUT0_PIN)) {
			goWorking();
		}

		if(millis() - waitingMillis > 24000) {
			goSleeping();
		}

		if (rcSwitch.available()) {
			unsigned long code = rcSwitch.getReceivedValue();
			//if(code == 2664495L) {
			if(code == 43055) {
				//1 on
				goWorking();

				pinMode(LED_PIN, OUTPUT);
				pinMode(INOUT0_PIN, OUTPUT);
				digitalWrite(LED_PIN, false);
				digitalWrite(INOUT0_PIN, true);

				for(unsigned int i = 0; i < 512; i++) {
					for(unsigned int j = 0; j < 512; j++) {
						wdt_reset();
					}
				}

				digitalWrite(LED_PIN, true);
				digitalWrite(INOUT0_PIN, false);
				//pinMode(LED_PIN, INPUT);
				pinMode(INOUT0_PIN, INPUT);
			}
			//if(code == 2664494L) {
			if(code == 43054) {
				//1 off
			}
			if(code == 2664493L) {
				//2 on
			}
			if(code == 2664492L) {
				//2 off
			}
			//Serial.print(rcSwitch.getReceivedValue());
			//Serial.print('\t');
			//Serial.print(rcSwitch.getReceivedBitlength());
			//Serial.print('\t');
			//Serial.print(rcSwitch.getReceivedDelay());
			//Serial.print('\t');
			//unsigned int* p = rcSwitch.getReceivedRawdata();
			//for(int i = 0; i < RCSWITCH_MAX_CHANGES; i++)
			//	Serial.print(*(p + i));
			//Serial.print('\t');
			//Serial.print(rcSwitch.getReceivedProtocol());
			//Serial.println();
			rcSwitch.resetAvailable();
		}
	}
}

ISR (WDT_vect) {
	//WDTCR |= _BV(WDIE);
	WDTCR = _BV(WDIE);
}
