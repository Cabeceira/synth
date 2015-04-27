/*
OUTPUT
PIN = 9 mozzi DEFAULT Sound OUT
PIN = 12 LED "metronome"
PIN = 13 LED ERROR NEVER HAPPENS
INPUT
PIN = 0 Rate of Arpegiator 
PIN = 1 Pitch
PIN = 2 LFO rate
PIN = 3 Filter Ressonance
*/
#include <MozziGuts.h> //Principal
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // Tables for waveforms
#include <tables/saw2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <EventDelay.h>
#include <AutoMap.h>//Map pot to required range of values
#include <LowPassFilter.h>
// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 64 // powers of 2 please

//Notes to frequency
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
//Note array, makes it easy to use intervals in the arpegiator
const float notas[] = {NOTE_B0,NOTE_C1,NOTE_CS1,NOTE_D1,NOTE_DS1,NOTE_E1,NOTE_F1,NOTE_FS1,NOTE_G1,NOTE_GS1,
	NOTE_A1,NOTE_AS1,NOTE_B1,NOTE_C2,NOTE_CS2,NOTE_D2,NOTE_DS2,NOTE_E2,NOTE_F2,NOTE_FS2,NOTE_G2,NOTE_GS2,
	NOTE_A2,NOTE_AS2,NOTE_B2,NOTE_C3,NOTE_CS3,NOTE_D3,NOTE_DS3,NOTE_E3,NOTE_F3,NOTE_FS3,NOTE_G3,NOTE_GS3,
	NOTE_A3,NOTE_AS3,NOTE_B3,NOTE_C4,NOTE_CS4,NOTE_D4,NOTE_DS4,NOTE_E4,NOTE_F4,NOTE_FS4,NOTE_G4,NOTE_GS4,
	NOTE_A4,NOTE_AS4,NOTE_B4,NOTE_C5,NOTE_CS5,NOTE_D5,NOTE_DS5,NOTE_E5,NOTE_F5,NOTE_FS5,NOTE_G5,NOTE_GS5,
	NOTE_A5,NOTE_AS5,NOTE_B5,NOTE_C6,NOTE_CS6,NOTE_D6,NOTE_DS6,NOTE_E6,NOTE_F6,NOTE_FS6,NOTE_G6,NOTE_GS6,
	NOTE_A6,NOTE_AS6,NOTE_B6,NOTE_C7,NOTE_CS7,NOTE_D7,NOTE_DS7,NOTE_E7,NOTE_F7,NOTE_FS7,NOTE_G7,NOTE_GS7,
	NOTE_A7,NOTE_AS7,NOTE_B7,NOTE_C8,NOTE_CS8,NOTE_D8,NOTE_DS8};


//Instantiate Oscillators
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> sine(SIN2048_DATA); //sine
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> saw(SAW2048_DATA); //saw
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> quadrada(SQUARE_NO_ALIAS_2048_DATA); //square
Oscil <2048, CONTROL_RATE> lfo(SIN2048_DATA); //LFO sine 

//criar um tempo
EventDelay beat;

//Low pass 
LowPassFilter filter;

//Automap para os potenciómetros
AutoMap lfoRateMap(0,1023,0.f,20.f); 
AutoMap filterQMap(0,1023,0,255);


//*****************************************************************************************************//
//Variables
//contador do arpegiator
byte steps = 1;
//selector de notas Potenciometro pin 1
int noteSelect = 0;
//defined at 500 to start but will be updated by Pot pin 0 
int rate =500;
int lfoRate=0;
byte filterQ;



//*****************************************************************************************************//
void setup()
{
	startMozzi(CONTROL_RATE);
	Serial.begin(9600);
	beat.set(rate);
	pinMode(13, OUTPUT);
	pinMode(12, OUTPUT);
}
void oscFreq(byte index){
	sine.setFreq(notas[index]);
	saw.setFreq(notas[index]);
	quadrada.setFreq(notas[index-12]);
}
//*****************************************************************************************************//
void updateControl(){
	digitalWrite(13, LOW);
	digitalWrite(12, LOW);
	//*****************************************************************************************************//
	//LOW PASS FILTER + LFO
	byte cutoff_freq = 100 + lfo.next()/2; // map the LFO into the filter range (0-255)
	filter.setCutoffFreq(cutoff_freq); //0-255 to represent 0-8192 Hz (AUDIO_RATE/2)
	
	filterQ = mozziAnalogRead(3)/4;
	filter.setResonance(filterQ); //0-255
	//LFO 
	
	lfoRate = lfoRateMap(mozziAnalogRead(2));
	lfo.setFreq(lfoRate);
	Serial.println(mozziAnalogRead(2));

	//*****************************************************************************************************//
	byte n; //Index of notas[] array
	//select first note with potentiometer
	noteSelect = mozziAnalogRead(1);
	n= noteSelect/17; //Better feel than analogMap
	
	//arpegiator
	if(beat.ready()){
		switch (steps) {
			case 1:
		    	oscFreq(n);
		    	digitalWrite(12, HIGH);
		    	break;
			case 2:
		    	oscFreq(n+3);  
		    	break;
			case 3:
		   		oscFreq(n+5);
		    	break;
			case 4:
		    	oscFreq(n+3);		      
		    	break;
			case 5:
		      	oscFreq(n+10);
		      	digitalWrite(12, HIGH);
		      	break;
			case 6:
		      	oscFreq(n+8);		      
		      	break;
			case 7:
		      	oscFreq(n+10);		      
		      	break;
			case 8:
		      	oscFreq(n+12);		      
		      	break;
		    
		    default:
		      	digitalWrite(13, HIGH);
		}
	 	
	//Back to the beginning
	if(steps==8){
		steps=1;
	} else {
		steps++;
	}
	//Beat rate 	
	rate = mozziAnalogRead(0);
	beat.set(rate);
	beat.start();//need to be at the end to work (Why?)
	} //Beat Ends  
}
//*****************************************************************************************************//
int updateAudio(){		 
return filter.next((sine.next() + quadrada.next()) >>3);	 	  
}

void loop(){
  audioHook(); // required here
}