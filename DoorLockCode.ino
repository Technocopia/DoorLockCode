
#if defined ARDUINO_ARCH_ESP32
//green
#define W0 27
//white
#define W1 14
#define DoorP 25
#define DoorE 26
#define button 32
#define loadcard 0
#else
//green
#define W0 2
//white
#define W1 3
#define DoorP 7
#define DoorE 4
#define button 8
#define loadcard 6
#endif
#include <Arduino.h>
#include <SimplePacketComs.h>
#include <TinyUSBSimplePacketComs.h>
#include "CardReadCommand.h"
//#include "codes.h" // also where #define sitecode is
//#define NUM_STATIC_CARDS (sizeof(cards) / sizeof(unsigned long int))


// Configuration that we'll store on disk

#define sitecode 17

char bits[80];
int bitcnt = 0;
unsigned long long bitw = 0;
unsigned int timeout = 1000;
boolean valid = true;
int parity(unsigned long int x);
const char *filename = "/db.json";  // <- SD library uses 8.3 filenames
TinyUSBSimplePacketComs coms;

CardReadCommand * command;
//portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
// Wiegand 0 bit ISR. Triggered by wiegand 0 wire.
bool loadCardMode = false;
long startLoadCardMode = 0;
#if defined ARDUINO_ARCH_ESP32
void IRAM_ATTR W0ISR() {
#else
	void W0ISR() {
#endif

	if (digitalRead(W0))
		return;
	//portEXIT_CRITICAL(&mux);
	bitw = (bitw << 1) | 0x0; // shift in a 0 bit.
	bitcnt++;               // Increment bit count
	timeout = millis();         // Reset timeout
	//portEXIT_CRITICAL(&mux);

}

// Wiegand 1 bit ISR. Triggered by wiegand 1 wire.
#if defined ARDUINO_ARCH_ESP32
void IRAM_ATTR W1ISR() {
#else
	void W1ISR() {
#endif
	if (digitalRead(W1))
		return;
	//portEXIT_CRITICAL(&mux);
	bitw = (bitw << 1) | 0x1; // shift in a 1 bit
	bitcnt++;               // Increment bit count
	timeout = millis();         // Reset Timeout
	//portEXIT_CRITICAL(&mux);

}


void setup() {
	coms.initializeUSB();
	Serial.begin(115200);
	USBDevice.setID(0x16c0, 0x486);
	while( !USBDevice.mounted() ) delay(1);
	command=new CardReadCommand();
	coms.attach(command);


	pinMode(W0, INPUT_PULLUP);
	pinMode(W1, INPUT_PULLUP);
	pinMode(button, INPUT_PULLUP);
	pinMode(loadcard, INPUT_PULLUP);
	pinMode(DoorP, OUTPUT);
	pinMode(DoorE, OUTPUT);
	digitalWrite(DoorP, LOW);
	digitalWrite(DoorE, HIGH);

	// Set up edge interrupts on wiegand pins.

	// Wiegand is used by many rfid card readers and mag stripe readers.

	// It has two wires. a '1' wire and a '0' wire. It will output a short pulse on
	//    one of these wires to indicate to you that it has read a 1 or a 0 bit.
	//    you need to count the pulses. It's a lot like rotary phone wheels
	//    only in binary and with 2 wires.

	// make sure W0 ans W1 trigger interrupts on a 1->0 transition.
	attachInterrupt(digitalPinToInterrupt(W0), W0ISR, FALLING);
	attachInterrupt(digitalPinToInterrupt(W1), W1ISR, FALLING);

	for (unsigned int i = 0; i < sizeof(bits); i++)
		bits[i] = 0;
	digitalWrite(DoorP, LOW);
	digitalWrite(DoorE, HIGH);
	//Keyboard.begin();
}

void open() {
	Serial.println("Opening door");
	digitalWrite(DoorP, HIGH); // Open door.
	delay(1100);
	digitalWrite(DoorP, LOW); // close
	Serial.println("Locking door");
}

bool haveCard() {
	return (((millis() - timeout) > 100) && bitcnt > 30);
}

long int getIDOfCurrentCard() {
	unsigned long long bitwtmp = bitw;
	int bitcnttmp = bitcnt;
	bitcnt = 0;
	bitw = 0;
	//portEXIT_CRITICAL(&mux);
	for (int i = bitcnttmp; i != 0; i--)
		Serial.print((unsigned int) (bitwtmp >> (i - 1) & 0x00000001)); // Print the card number in binary
	Serial.print(" (");
	Serial.print(bitcnttmp);
	Serial.println(")");
	boolean ep, op;
	unsigned int site;
	unsigned long int card;

	// Pick apart card.
	site = (bitwtmp >> 25) & 0x00007f;
	card = (bitwtmp >> 1) & 0xffffff;
	op = (bitwtmp >> 0) & 0x000001;
	ep = (bitwtmp >> 32) & 0x000001;

	// Check parity
	if (parity(site) != ep)
		valid = false;
	if (parity(card) == op)
		valid = false;

	// Print card info
	Serial.print("Got " + String());
	Serial.print("Site: ");
	Serial.println(site);
	Serial.print("Card: ");
	Serial.println(card);
	Serial.print("ep: ");
	Serial.print(parity(site));
	Serial.println(ep);
	Serial.print("op: ");
	Serial.print(parity(card));
	Serial.println(op);

	valid = true;
	return card;
}
#if defined ARDUINO_ARCH_ESP32
void IRAM_ATTR loop() {
#else
	void loop() {
#endif
	coms.server();
	if (haveCard()) { // The reader hasn't sent a bit in 2000 units of time. Process card.
		int card = getIDOfCurrentCard();
		Serial.println("Got card: "+String(card));
		command->recentCard=card;
	}
	delay(30);


}

int parity(unsigned long int x) {
	unsigned long int y;
	y = x ^ (x >> 1);
	y = y ^ (y >> 2);
	y = y ^ (y >> 4);
	y = y ^ (y >> 8);
	y = y ^ (y >> 16);
	return y & 1;
}
