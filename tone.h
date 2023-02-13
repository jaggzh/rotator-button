#ifndef _TONE_H
#define _TONE_H

/* #include <Tone.h> */
#include "defines.h"
#include "pitches.h"

//#define MEL_SHAVE MEL_CLOSE  // do this if you've removed it to save memory

/* void setup_tone(void); */

enum {
	MEL_SHAVE=0,
	MEL_CLOSE,
	MEL_BETTYBOOP,
	MEL_BETTYBOOP2,
	MEL_END
};
#define MEL_DEFAULT MEL_CLOSE
struct Note { int freq; char dur; };

#ifndef _IN_TONE_CPP
extern struct Note * melodies[MEL_END];
#endif

#define PLAY_CLOSE() play_melody_given(melodies[MEL_CLOSE])
#define PLAY_SHAVE() play_melody_given(melodies[MEL_SHAVE])
#define PLAY_BETTYBOOP() play_melody_given(melodies[MEL_BETTYBOOP])
#define PLAY_BETTYBOOP2() play_melody_given(melodies[MEL_BETTYBOOP2])
#define PLAY_ERROR() tone(PINPIEZO, 300, 1000)

#ifdef USE_TONE
	#define play_melody() play_melody_given(melodies[MEL_DEFAULT])
	void play_melody_given(Note *melody);
	void tone_quick_freq(unsigned int freq);
	void tone_quick(void);
	void tone_freq_durms(unsigned int freq, unsigned int dur);
#else
	#warning "USE_TONE is not defined. No sound."
	#define play_melody()
	#define play_melody_given(m)
	#define tone_quick()
	#define tone(a,b,c)
#endif // USE_TONE

#endif // _TONE_H
