#include <Arduino.h>
/*
  uCLOCK USB MIDI CLOCK RECEIVER
  https://github.com/midilab/uClock
  https://github.com/adafruit/Adafruit_TinyUSB_Arduino/blob/master/examples/MIDI/midi_test/midi_test.ino
*/

#include <Adafruit_TinyUSB.h> // NOTE: for Seed Xiao use version 0.10.5: https://github.com/adafruit/Adafruit_TinyUSB_Arduino/releases/tag/0.10.5
#include <MIDI.h>

// USB MIDI object
Adafruit_USBD_MIDI usb_midi;

// Create a new instance of the Arduino MIDI Library,
// and attach usb_midi as the transport.
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI_USB);

#include <uClock.h>

uint8_t bpm_blink_timer = 1;

void handle_bpm_led(uint32_t tick) {
  // BPM led indicator --> XIAO VERSION!!!
  if ( !(tick % (96)) || (tick == 1) ) {  // first compass step will flash longer
    bpm_blink_timer = 8;
    digitalWrite(LED_BUILTIN, LOW); // for the XIAO, write a 0 (LOW) to the pin to turn the LED_BUILTIN on.
  } else if ( !(tick % (24)) ) {   // each quarter led on
    bpm_blink_timer = 1;
    digitalWrite(LED_BUILTIN, LOW);
  } else if ( !(tick % bpm_blink_timer) ) { // get led off
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

// Internal clock handlers
void ClockOut96PPQN(uint32_t tick) {
  // Send MIDI_CLOCK to external gears
  MIDI_USB.sendClock();
  handle_bpm_led(tick);
}

void onClockStart() {
  MIDI_USB.sendStart();
}

void onClockStop() {
  MIDI_USB.sendStop();
}

// External clock handlers
void onExternalClock() {
  uClock.clockMe();
}

void onExternalStart() {
  uClock.start();
}

void onExternalStop() {
  uClock.stop();
}

void setup() {

  // A led to count bpms
  pinMode(LED_BUILTIN, OUTPUT);

  // MIDI setup
  MIDI_USB.begin(MIDI_CHANNEL_OMNI); // Initialize MIDI, and listen to all MIDI channels
  MIDI_USB.turnThruOff(); // software Thru is enabled by default on Serial so we need to disable.
  MIDI_USB.setHandleStart(onExternalStart);
  MIDI_USB.setHandleStop(onExternalStop);
  MIDI_USB.setHandleClock(onExternalClock);
  
  // uClock Setup
  // Initialize the clock
  uClock.init();
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  // For MIDI Sync Start and Stop
  uClock.setOnClockStartOutput(onClockStart);  
  uClock.setOnClockStopOutput(onClockStop);
  uClock.setMode(uClock.EXTERNAL_CLOCK);

  // wait until device mounted
  while( !USBDevice.mounted() ) delay(1);

}

void loop() {

  // MIDI Controllers should discard incoming MIDI messages 
  while (MIDI_USB.read()) { // read() checks the input buffer for any received MIDI commands and passes them to the correct function.
    // ignore incoming messages
  }
  // MIDI_USB.read();

}
