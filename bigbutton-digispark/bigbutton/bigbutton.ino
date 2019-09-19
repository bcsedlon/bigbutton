#include "Arduino.h"

#include "RCSwitch.h"
RCSwitch rcSwitch = RCSwitch();
//#define RFRX_PIN D3
#define RFTX_PIN 0

#define LED0_PIN 	1
#define BUTTON0_PIN	2
#define LED1_PIN	4
#define BUTTON1_PIN	3

unsigned int r1Off = 2664494;
unsigned int r1On = 2664495;
unsigned int r2Off = 2664492;
unsigned int r2On = 2664493;
unsigned int r3Off = 2664490;
unsigned int r3On = 2664491;
unsigned int r4Off = 2664486;
unsigned int r4On = 2664487;
unsigned int rAllOff = 2664481;
unsigned int rAllOn = 2664482;

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

bool lastButton0 = true;
bool lastButton1 = true;

void tx(int i) {
	int ledPin = LED0_PIN;
	unsigned int rOn = r1On;
	unsigned int rOff = r1Off;

	if(i == 1) {
		ledPin = LED1_PIN;
		rOn = r2On;
		rOff = r2On;
	}

	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, HIGH);

	pinMode(RFTX_PIN, OUTPUT);
	rcSwitch.enableTransmit(RFTX_PIN);
	rcSwitch.send(rOn, 24);
	rcSwitch.send(rOn, 24);
	rcSwitch.send(rOn, 24);

	//delay(1000);
	//rcSwitch.send(r1On, 24);

	//sleep_enable();
	//sleep_cpu();

	for(int i = 0; i < 8; i++) {
		digitalWrite(ledPin, LOW);
		delay(500);
		digitalWrite(ledPin, HIGH);
		delay(500);
	}

	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, HIGH);

	pinMode(RFTX_PIN, OUTPUT);
	rcSwitch.enableTransmit(RFTX_PIN);
	rcSwitch.send(rOff, 24);
	rcSwitch.send(rOff, 24);
	rcSwitch.send(rOff, 24);

	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, LOW);

	pinMode(ledPin, INPUT);
}

void setup()
{
	// Power Saving setup
	for (byte i = 0; i < 6; i++) {
		pinMode(i, INPUT);      // Set all ports as INPUT to save energy
		digitalWrite (i, LOW);  //
	}
	pinMode(BUTTON0_PIN, INPUT_PULLUP);
	pinMode(BUTTON1_PIN, INPUT_PULLUP);

	adc_disable();          // Disable Analog-to-Digital Converter

	wdt_reset();            // Watchdog reset
	//wdt_enable(WDTO_1S);    // Watchdog enable Options: 15MS, 30MS, 60MS, 120MS, 250MS, 500MS, 1S, 2S, 4S, 8S
	wdt_enable(WDTO_250MS);
	WDTCR |= _BV(WDIE);     // Interrupts watchdog enable
	sei();                  // enable interrupts
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Sleep Mode: max
}

void loop()
{
	if(!digitalRead(BUTTON0_PIN)) {
		if(lastButton0) {
			tx(0);
			lastButton0 = false;
		}
	}
	else {
		lastButton0 = true;
	}

	if(!digitalRead(BUTTON1_PIN)) {
		if(lastButton1) {
			tx(1);
			lastButton1 = false;
		}
	}
	else {
		lastButton1 = true;
	}

	sleep_enable();
	sleep_cpu();
}

ISR (WDT_vect) {
	WDTCR |= _BV(WDIE);
}

/*
void setup3() {
	for (byte i = 0; i < 6; i++) {
		pinMode(i, INPUT);      // Set all ports as INPUT to save energy
		digitalWrite (i, LOW);  //
	}
	adc_disable();

	pinMode(BUTTON_PIN, INPUT);        // sets the digital pin as input
	digitalWrite(BUTTON_PIN, HIGH);
}

void wakeUpNow(){        // here the interrupt is handled after wakeup

	//execute code here after wake-up before returning to the loop() function
	// timers and code using timers (serial.print and more...) will not work here.
	//digitalWrite(interruptPin, HIGH);
}

void sleepNow(){

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
    sleep_enable();                          // enables the sleep bit in the mcucr register so sleep is possible
    attachInterrupt(0, wakeUpNow, LOW);     // use interrupt 0 (pin 2) and run function wakeUpNow when pin 2 gets LOW
    //digitalWrite(ledPin, LOW);

    sleep_mode();                          // here the device is actually put to sleep!!

    sleep_disable();                       // first thing after waking from sleep: disable sleep...
    detachInterrupt(0);                    // disables interrupton pin 3 so the wakeUpNow code will not be executed during normal running time.
    //delay(250);                           // wait 2 sec. so humans can notice the interrupt LED to show the interrupt is handled
    //digitalWrite (interruptPin, LOW);       // turn off the interrupt LED

}

void loop3() {
	tx();
	sleepNow();
}
*/
