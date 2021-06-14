#ifndef TERMI8_H
	#define TERMI8_H

	#include <iostream>
	#include <malloc.h>
	#include <unistd.h>
	#include <cstring>
	#include <math.h>
	#include <ctime>

	#include "mytimer.h"

	struct __attribute__((__packed__)) Palette {
		unsigned char PAL0;
		unsigned char PAL1;
		unsigned char PAL2;
		unsigned char PAL3;
		unsigned char PAL4;
		unsigned char PAL5;
		unsigned char PAL6;
		unsigned char PAL7;
		unsigned char PAL8;
		unsigned char PAL9;
		unsigned char PAL10;
		unsigned char PAL11;
		unsigned char PAL12;
		unsigned char PAL13;
		unsigned char PAL14;
		unsigned char PAL15;
	};
	struct CPUFlags {
		bool Carry;
		bool Zero;
		bool InterruptDis;
		bool Decimal; //unused, no decimal mode implimented
		bool Break;
		bool Overflow;
		bool Negative;
	};
	struct CPURegisters {
		int PC;
		unsigned char SP;
		unsigned char A;
		unsigned char X;
		unsigned char Y;
		CPUFlags FLAGS;
	};
	enum BUTTONS {
		BUTTON_UP = 1 << 0,
		BUTTON_DOWN = 1 << 1,
		BUTTON_LEFT = 1 << 2,
		BUTTON_RIGHT = 1 << 3,
		BUTTON_A = 1 << 4,
		BUTTON_B = 1 << 5,
		BUTTON_C = 1 << 6,
		BUTTON_D = 1 << 7
	};
	enum INTERRUPTCHANNEL {
		INTERRUPT_HOR = 1 << 0, //horizontal interrupt gets triggered when the screen gets redrawn
		INTERRUPT_BTN = 1 << 1 //button interrupt gets triggered when the input buttons change
	};
	struct __attribute__((__packed__)) Sprite { //spites are 16x16 pixels, color 0 is transparent
		unsigned char PositionX; //position of bottom right pixel of sprite
		unsigned char PositionY; //""   ""
		unsigned char DataPointerLow; //pointer to data for sprite (data is 128 bytes long
		unsigned char DataPointerHigh;
	};
	enum AUDIOFLAGS { //the different outputs get ored together when multiple ones are slected
		AUDIO_SQR = 1 << 0, //Output square wave
		AUDIO_TRI = 1 << 1, //Output triangle
		AUDIO_RMP = 1 << 2, //Output ramp
		AUDIO_NOISE = 1 << 3, //Outut noise
		AUDIO_START = 1 << 4, //When bit is set then the ADSR gets startet
		AUDIO_MODE0 = 1 << 5, //Current mode of channel
		AUDIO_MODE1 = 1 << 6 //00: Atack, 01: Decay, 10: Sustain, 11: Release
	};
	struct __attribute__((__packed__)) AudioChannel {
		unsigned char FrequencyLow;
		unsigned char FrequencyHigh;
		unsigned char Flags;
		unsigned char DutyCycle;
		unsigned char Atack;
		unsigned char Decay;
		unsigned char Sustain;
		unsigned char Release;
		unsigned char Volume; //only the lower 4 bits are used
	};
	struct __attribute__((__packed__)) HardwareRegisters {
		unsigned char ScreenPointerLow;
		unsigned char ScreenPointerHigh;
		Palette ColorPalette;
		unsigned char interruptMask; //set bit to 1 to enable interrupts of this channel
		unsigned char interruptPending; //if set to 1 then there is an interrupt on that channel. Even if masked the bit gets set
		unsigned char currentInterrupt; //if interrupt get triggered the channel number bit of that interrupt get set. The next interrupt overwrites this so back it up if you need it
		unsigned char buttonState; //the state of the input buttons
		Sprite HardwareSprites[8];
		AudioChannel SoundChannels[4];
	};

	int prepare();
	int setMemoryPointer(void* memPointer);
	int execInstruction();
	int interruptCpu(int interruptChannel); //high level function that sets the hardware registers and then calls triggerInterrupt
	int triggerInterrupt(bool maskable); //low level function that calls cpu.cpp
	int loadRom(std::string filePath, int length, void* memory, int address);
	int drawScreen(HardwareRegisters* hwr);
#endif
