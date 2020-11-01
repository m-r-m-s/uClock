/* USB MIDI Sync Slave Box Monitor
 *  
 * This example demonstrates how to create a
 * MIDI hid compilant slave clock box with 
 * monitor support using oled displays
 * 
 * You need the following libraries to make it work
 * - Midi Library
 * - USB-MIDI and MIDIUSB
 * - u8g2
 * - uClock
 *
 * This example make use of drift values (10, 14) 
 * respectively for internal and external drift reference.
 * This example was tested on a macbook 
 * runing ableton live 9 as master clock
 *
 * This example code is in the public domain.
 */

#include <USB-MIDI.h>
#include <U8x8lib.h>

//
// BPM Clock support
//
#include <uClock.h>

USBMIDI_CREATE_DEFAULT_INSTANCE();
U8X8 * u8x8;

// MIDI clock, start, stop, note on and note off byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

char bpm_str[4];
float bpm = 126.0;
uint8_t bpm_blink_timer = 1;
uint8_t clock_state = 1;

void handle_bpm_led(uint32_t * tick)
{
  // BPM led indicator
  if ( !(*tick % (96)) || (*tick == 1) ) {  // first compass step will flash longer
    bpm_blink_timer = 8;
    TXLED1;
  } else if ( !(*tick % (24)) ) {   // each quarter led on
    TXLED1;
  } else if ( !(*tick % bpm_blink_timer) ) { // get led off
    TXLED0;
    bpm_blink_timer = 1;
  }
}

// Internal clock handlers
void ClockOut96PPQN(uint32_t * tick) {
  // Send MIDI_CLOCK to external gears
  MIDI.sendRealTime(MIDI_CLOCK);
  handle_bpm_led(tick);
}

void onClockStart() {
  MIDI.sendRealTime(MIDI_START);
}

void onClockStop() {
  MIDI.sendRealTime(MIDI_STOP);
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
  //
  // MIDI setup
  //
  MIDI.begin();
  MIDI.setHandleClock(onExternalClock);
  MIDI.setHandleStart(onExternalStart);
  MIDI.setHandleStop(onExternalStop);

  //
  // OLED setup
  // Please check you oled model to correctly init him
  //
  //u8x8 = new U8X8_SH1106_128X64_NONAME_HW_I2C(U8X8_PIN_NONE);
  u8x8 = new U8X8_SSD1306_128X64_NONAME_HW_I2C(U8X8_PIN_NONE);
  u8x8->begin();
  u8x8->setFont(u8x8_font_pressstart2p_r); 
  u8x8->clear();
  u8x8->drawUTF8(0, 0, "uClock"); 

  //
  // uClock Setup
  //
  // fine tunning adjstments for you clock slaves/host setDrift(internal, external)
  uClock.setDrift(10, 14);
  uClock.init();
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  // For MIDI Sync Start and Stop
  uClock.setOnClockStartOutput(onClockStart);
  uClock.setOnClockStopOutput(onClockStop);
  uClock.setMode(uClock.EXTERNAL_CLOCK);
}

void printBpm(float _bpm, uint8_t col, uint8_t line) {
    int b = (int)_bpm;
    //int c = (int)((_bpm-b)*100);
    int c = (int)((_bpm-b)*10);
    itoa(b, bpm_str, 10);
    if (b > 99) {
      u8x8->drawUTF8(col, line, bpm_str);
    } else {
      bpm_str[2] = "\0";
      u8x8->drawUTF8(col, line, " ");
      u8x8->drawUTF8(col+1, line, bpm_str);
    }
    u8x8->drawUTF8(col+3, line, ".");
    itoa(c, bpm_str, 10);
    u8x8->drawUTF8(col+4, line, bpm_str);
    u8x8->drawUTF8(col+5, line, "bpm"); 
}

void loop() {
  MIDI.read();
  // DO NOT ADD MORE PROCESS HERE AT THE COST OF LOSING CLOCK SYNC
  // Since arduino make use of Serial RX interruption we need to 
  // read Serial as fast as we can on the loop
  if (bpm != uClock.getTempo()) {
    bpm = uClock.getTempo();
    printBpm(bpm, 8, 7);
  }
  if (clock_state != uClock.state) { 
    clock_state = uClock.state;
    if (clock_state >= 1) {
      u8x8->drawUTF8(0, 7, "playing"); 
    } else { 
      u8x8->drawUTF8(0, 7, "stoped "); 
    }
  }
}
