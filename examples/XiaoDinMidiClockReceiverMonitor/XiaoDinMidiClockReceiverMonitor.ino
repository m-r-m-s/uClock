#include <Arduino.h>

#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// U8x8 object
U8X8 * u8x8;

#include <MIDI.h>
using namespace midi; // midi::Start, midi::Stop, midi::Continue, etc

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI_DIN); // initialize 1 instance of DIN (hardware) MIDI

#include <uClock.h>

float bpm = 126;
uint8_t bpm_blink_timer = 1;
uint8_t clock_state = 1;

void handle_bpm_led(uint32_t tick) {
  // BPM led indicator --> XIAO VERSION!!!
  if ( !(tick % (96)) || (tick == 1) ) {  // first compass step will flash longer
    bpm_blink_timer = 8;
    digitalWrite(LED_BUILTIN, LOW); // for XIAO, write a 0 (LOW) to the pin to turn the LED_BUILTIN on.
  } else if ( !(tick % (24)) ) {   // each quarter led on
    bpm_blink_timer = 1;
    digitalWrite(LED_BUILTIN, LOW);
  } else if ( !(tick % bpm_blink_timer) ) { // turn led off
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

// Internal clock handlers
void ClockOut96PPQN(uint32_t tick) {
  // Send MIDI_CLOCK to external gears
  MIDI_DIN.sendClock();
  // MIDI_DIN.sendRealTime(Clock);
  handle_bpm_led(tick);
}

void onClockStart() {
  MIDI_DIN.sendStart();
  // MIDI_DIN.sendRealTime(Start);
}

void onClockStop() {
  MIDI_DIN.sendStop();
  // MIDI_DIN.sendRealTime(Stop);
  digitalWrite(LED_BUILTIN, HIGH); 
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
  MIDI_DIN.begin(MIDI_CHANNEL_OMNI); // Initialize MIDI, and listen to all MIDI channels

  MIDI_DIN.turnThruOff(); // software Thru is enabled by default on Serial so we need to disable.

  MIDI_DIN.setHandleStart(onExternalStart); // MIDIlibrary clock handlers
  MIDI_DIN.setHandleStop(onExternalStop);
  MIDI_DIN.setHandleClock(onExternalClock);

  //
  // OLED setup
  // Please check you oled model to initialize correctly
  //
  u8x8 = new U8X8_SSD1306_128X32_UNIVISION_HW_I2C(/* reset=*/ U8X8_PIN_NONE);
  u8x8->begin();
  u8x8->setFont(u8x8_font_pressstart2p_r); 
  u8x8->clear();
  u8x8->drawUTF8(0, 0, "uClock"); 
  // delay(2000);
  // u8x8->clear();

  // uClock Setup
  // Initialize the clock
  uClock.init();
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  // For MIDI Sync Start and Stop
  uClock.setOnClockStartOutput(onClockStart);  
  uClock.setOnClockStopOutput(onClockStop);
  uClock.setMode(uClock.EXTERNAL_CLOCK);

}

void loop() {

  MIDI_DIN.read();

  if (bpm != uClock.getTempo()) {
    bpm = uClock.getTempo();
    u8x8->drawUTF8(8, 3, String(bpm, 1).c_str());
    u8x8->drawUTF8(8+5, 3, "bpm");
    // clear display ghost number for 2 digit
    // coming from 3 digit bpm changes
    if (bpm < 100) {
      u8x8->drawUTF8(8+4, 3, " ");
    }
  }
  if (clock_state != uClock.state) { 
    clock_state = uClock.state;
    if (clock_state >= 1) {
      u8x8->drawUTF8(0, 3, "playing"); 
    } else { 
      u8x8->drawUTF8(0, 3, "stopped"); 
    }
  }

}