#include <MIDI.h>

//const byte rowsPins[6] = {12, 11, 10, 13, 14, 15};
const byte rowsPins[6] = {15, 14, 13, 10, 11, 12};
const byte columnsPins[6] = {9, 8, 7, 6, 5, 4};
bool buttonsStates[36] = {};

MIDI_CREATE_DEFAULT_INSTANCE();

int noteON = 144;//144 = 10010000 in binary, note on command
int noteOFF = 128;//128 = 10000000 in binary, note off command

void setup() {
  Serial.begin(31250);
  for(int i = 0; i < 6; ++i)
  {
    pinMode(rowsPins[i], OUTPUT);
    pinMode(columnsPins[i], INPUT);
  }
}

void MIDImessage(int command, int MIDInote, int MIDIvelocity) {
  Serial.write(command);//send note on or note off command 
  Serial.write(MIDInote);//send pitch data
  Serial.write(MIDIvelocity);//send velocity data
}

bool keyboardStatus[36] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte ChannelCount = 16;
byte playing[16] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
void loop() {
  for(byte i = 0; i < 6; ++i)
  {
    digitalWrite(rowsPins[i], HIGH);
    for(byte j = 0; j < 6; ++j)
    {
      bool buttonState = digitalRead(columnsPins[j]);
      if(buttonsStates[6 * i + j] != buttonState)
      {
        buttonsStates[6 * i + j] = buttonState;
        if(buttonState)
        {
          for(byte z = 0; z < ChannelCount; ++z)
          {
            if(playing[z] == i + j * 6)
            {
              playing[z] = 255;
              MIDImessage(noteOFF + z, 6 * i + j + 24, 127);       
              z = ChannelCount;
            }
          }   
        }
        else
        {
          for(byte z = 0; z < ChannelCount; ++z)
          {
            if(playing[z] == 255)
            {
              playing[z] = i + j * 6;
              MIDImessage(noteON + z, 6 * i + j + 24, 127);
              z = ChannelCount;
            }
          }
        }
      }
    }
    digitalWrite(rowsPins[i], LOW);
  }
}
