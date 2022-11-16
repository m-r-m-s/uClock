/* USB MIDI Clock Sync Box Receiver
 * 
 * You must select MIDI from the "Tools > USB Type" menu
 * 
 * This example code is in the public domain.
 */

#include <uClock.h>

uint8_t bpm_blink_timer = 1;
void handle_bpm_led(uint32_t tick)
{
  // BPM led indicator
  if ( !(tick % (96)) || (tick == 1) ) {  // first compass step will flash longer
    bpm_blink_timer = 8;
    digitalWrite(LED_BUILTIN, HIGH);
  } else if ( !(tick % (24)) ) {   // each quarter led on
    digitalWrite(LED_BUILTIN, HIGH);
  } else if ( !(tick % bpm_blink_timer) ) { // get led off
    digitalWrite(LED_BUILTIN, LOW);
    bpm_blink_timer = 1;
  }
}

// Internal clock handlers
void ClockOut96PPQN(uint32_t tick) {
  // Send MIDI_CLOCK to external gears on other port?
  //usbMIDI.sendRealTime(usbMIDI.Clock);
  handle_bpm_led(tick);
}

void onClockStart() {
  //usbMIDI.sendRealTime(usbMIDI.Start);
}

void onClockStop() {
  //usbMIDI.sendRealTime(usbMIDI.Stop);
}

// External clock handlers
void onExternalClock()
{
  uClock.clockMe();
}

void onExternalStart()
{
  uClock.start();
}

void onExternalStop()
{
  uClock.stop();
}

void setup() {
  // A led to count bpms
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Setup realtime midi event handlers
  usbMIDI.setHandleClock(onExternalClock);
  usbMIDI.setHandleStart(onExternalStart);
  usbMIDI.setHandleStop(onExternalStop);

  // Setup our clock system
  // Inits the clock
  uClock.init();
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  // Set the callback function for MIDI Start and Stop messages.
  uClock.setOnClockStartOutput(onClockStart);  
  uClock.setOnClockStopOutput(onClockStop);
  // set to external sync mode
  uClock.setMode(1);
}

void loop() {
  // Grab all midi data as fast as we can!
  while (usbMIDI.read()) {}
}
