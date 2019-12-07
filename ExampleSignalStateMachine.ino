// Simulate a progression of signal aspects and a grade crossing
// Examples for
//    using functions to abstract behaviors
//    using a state machine to sequence behaviors
//
//  John Plocher, 2018

#include <elapsedMillis.h>

// Sensor and LED Pins

#define SENSOR_PIN 5 // SENSOR
#define FLASHER_LEFT_PIN 11    // LEFT
#define FLASHER_RIGHT_PIN 10    // RIGHT
#define SIGNAL_G_PIN 9     // G
#define SIGNAL_Y_PIN 8     // Y
#define SIGNAL_R_PIN 7     // R

enum { STRIGGERED, SOCCUPIED, SSTOP, SAPPROACH, SCLEAR, SDARK };
int state = SCLEAR;

int           enable_flashers = 0;
elapsedMillis flashTime;	// used to control the time a flasher lamp stays on
elapsedMillis delayTime;	// used to simulate signal progression after occupancy goes away
long          flash_interval = 900; // time in milliseconds between alternating flashes

void setSTOP() {
	digitalWrite(SIGNAL_G_PIN, LOW);
	digitalWrite(SIGNAL_Y_PIN, LOW);
	digitalWrite(SIGNAL_R_PIN, HIGH);
}
void setAPPROACH() {
	digitalWrite(SIGNAL_G_PIN, LOW);
	digitalWrite(SIGNAL_Y_PIN, HIGH);
	digitalWrite(SIGNAL_R_PIN, LOW);
}
void setCLEAR() {
	digitalWrite(SIGNAL_G_PIN, HIGH);
	digitalWrite(SIGNAL_Y_PIN, LOW);
	digitalWrite(SIGNAL_R_PIN, LOW);
}
void setDARK() {
	digitalWrite(SIGNAL_G_PIN, LOW);
	digitalWrite(SIGNAL_Y_PIN, LOW);
	digitalWrite(SIGNAL_R_PIN, LOW);
}

void flashersOFF() {
	digitalWrite(FLASHER_LEFT_PIN, LOW);
	digitalWrite(FLASHER_RIGHT_PIN, LOW);
}

void runFlashers() {
	static int flash_state = 0;
	flash_state = ~flash_state;
	digitalWrite(FLASHER_LEFT_PIN, flash_state); // Alternate flashers
	digitalWrite(FLASHER_RIGHT_PIN, ~flash_state);
}

void setup() {
	Serial.begin(9600);

	pinMode(FLASHER_LEFT_PIN,   OUTPUT);
	pinMode(FLASHER_RIGHT_PIN,  OUTPUT);
	pinMode(SIGNAL_G_PIN,       OUTPUT);
	pinMode(SIGNAL_Y_PIN,       OUTPUT);
	pinMode(SIGNAL_R_PIN,       OUTPUT);
	
  flashersOFF();
	setSTOP();
	
	pinMode(SENSOR_PIN,        INPUT_PULLUP);
	
	flashTime = 0;
	delayTime = 0;
	enable_flashers = 0;
}


void loop() {
	int triggered = (digitalRead(SENSOR_PIN) == LOW);
	switch (state) {
		case STRIGGERED: // JUST OCCUPIED, reset delay/timer values
				if (!triggered) {
					state = SSTOP;
				} else {
					setSTOP();					
					enable_flashers = 1;
					runFlashers();
					flashTime = 0;
					state = SOCCUPIED;
				}
				break;
				
		case SOCCUPIED: // STILL OCCUPIED - RED + flashing
				if (!triggered) {
					state = SSTOP;
					delayTime = 0;
				} else {
					setSTOP();
					if (enable_flashers && (flashTime > flash_interval)) {
						runFlashers();
						flashTime = 0;
					}
					// stay in this state until the section becomes unoccupied...
				}
				break;
				
		case SSTOP: // UNOCCUPIED - RED + flashing for a few more seconds
				if (triggered) {
					state = SOCCUPIED;	// go back to OCCUPIED to keep flashers flashing uninterrupted
				} else {
					setSTOP();
					if (delayTime > 6000)  { // after a bit, change to Y
						state = SAPPROACH;
					}
					if (enable_flashers && (flashTime > flash_interval)) {
						runFlashers();
						flashTime = 0;
					}
				}
				break;
				
		case SAPPROACH: // UNOCCUPIED - YEL + no flashing
				if (triggered) {
					state = STRIGGERED;	// reset back to newly occupied
				} else {
					setAPPROACH();
					flashersOFF();
					enable_flashers = 0;
					if (delayTime > 10000)  { // change to G
						state = SCLEAR;
					}
				}
				break;
				
		case SCLEAR: // UNOCCUPIED - GRN + no flashing
				if (triggered) {
					state = STRIGGERED;	// reset back to newly occupied
				} else {
					setCLEAR();
					flashersOFF();
					if (delayTime > 15000)  { // change to DARK
						state = SDARK;
					}
				}
				break;
				
		case SDARK: // UNOCCUPIED - All Dark
				if (triggered) {
					state = STRIGGERED;	// reset back to newly occupied
				} else {
					flashersOFF();
					enable_flashers = 0;
					setDARK();
				}
				break;
	}
}

