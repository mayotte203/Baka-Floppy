#include <MIDI.h>
#include <Arduino.h>

/**********
 * MoppyInstruments handle the sound-creation logic for your setup.  The
 * instrument class provides a systemMessage handler function and a deviceMessage
 * handler function for handling messages received by the network.
 *
 * Uncomment the appropriate instrument class for your setup
 */

// Floppy drives directly connected to the Arduino's digital pins
#include "src/MoppyInstruments/FloppyDrives.h"
FloppyDrives instrument = FloppyDrives();


//Uncomment the 2 next lines and comment the 2 lines above this comment to switch to L298N mode
//#include "src/MoppyInstruments/L298N.h"
//L298N instrument = L298N(); // please see src/MoppyInstruments/L298N.h for pinout and additionnal info


// A single device (e.g. xylophone, drums, etc.) connected to shift registers
//#include "src/MoppyInstruments/ShiftRegister.h"
//ShiftRegister instrument = ShiftRegister();

/**********
 * MoppyNetwork classes receive messages sent by the Controller application,
 * parse them, and use the data to call the appropriate handler as implemented
 * in the instrument class defined above.
 *
 * Uncomment the appropriate networking class for your setup
 */

//// UDP Implementation using some sort of network stack?  (Not implemented yet)
// #include "src/MoppyNetworks/MoppyUDP.h"
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1,    midi1);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3,    midi2);
//MIDI_CREATE_DEFAULT_INSTANCE();
//The setup function is called once at startup of the sketch
void setup()
{
	// Call setup() on the instrument to allow to to prepare for action
    instrument.setup();
    midi1.setHandleNoteOn(instrument.noteOnHandler);
    midi1.setHandleNoteOff(instrument.noteOffHandler);
   // midi1.setHandlePitchBend(instrument.pitchBendHandler);
    midi1.setHandleStart(instrument.startHandler);
    midi1.setHandleStop(instrument.stopHandler);
    midi1.setHandleControlChange(instrument.controlChangeHandler);
    midi1.begin(MIDI_CHANNEL_OMNI);
    midi1.turnThruOff();
    Serial1.begin(200000);
    midi2.setHandleNoteOn(instrument.noteOnHandler);
    midi2.setHandleNoteOff(instrument.noteOffHandler);
   // midi1.setHandlePitchBend(instrument.pitchBendHandler);
    midi2.setHandleStart(instrument.startHandler);
    midi2.setHandleStop(instrument.stopHandler);
    midi2.setHandleControlChange(instrument.controlChangeHandler);
    midi2.begin(MIDI_CHANNEL_OMNI);
    midi2.turnThruOff();
    Serial3.begin(31250);
    pinMode(17, OUTPUT);
    digitalWrite(17, LOW);
    pinMode(0, OUTPUT);
    digitalWrite(14, LOW);
}

// The loop function is called in an endless loop
void loop()
{
	// Endlessly read messages on the network.  The network implementation
	// will call the system or device handlers on the intrument whenever a message is received.
    midi1.read();
    midi2.read();
}
