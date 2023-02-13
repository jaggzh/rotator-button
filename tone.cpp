#define _IN_TONE_CPP
#include <Arduino.h>
#include "defines.h"
/* #include <Arduino.h> */
#ifdef USE_TONE
#include "tone.h"
/* #include <Tone.h> */

/* Tone tone1; */

/* void setup_tone(void) { */
/* 	tone1.begin(); */
/* } */

struct Note *melodies[MEL_END] = {
	(struct Note[]) { // shave and a haircut
		{ NOTE_C4, 4 },
		{ NOTE_G3, 8 },
		{ NOTE_G3, 8 },
		{ NOTE_A3, 4 },
		{ NOTE_G3, 4 },
		{ 0, 4 },
		{ NOTE_B3, 4 },
		{ NOTE_C4, 4 },
		{ 0, 0 }
	},
	(struct Note[]) { // close encounters
		{ NOTE_AS5, 4 }, //   B-
		{ NOTE_C6, 4 },  // > C
		{ NOTE_GS5, 4 }, // < A-
		{ NOTE_GS4, 4 }, // < A-
		{ NOTE_DS5, 1 }, // > E-
		{ 0, 0 }
	},
	(struct Note[]) { // Betty Boop (boop boop bee doop)
		{ NOTE_C5, 4 },
		{ NOTE_C5, 4 },
		{ 0, 1 },        // pause
		{ NOTE_D5, 4 }, // < A-
		{ NOTE_C5, 4 },
		{ 0, 0 }
	},
	(struct Note[]) { // Betty Boop (..boop boop bee doop)
		{ NOTE_C5, 1 },
		{ NOTE_C5, 1 },
		{ 0, 0 }
	},
};

void play_melody_given(Note *melody) {
	// iterate over the notes of the melody:
	for (int thisNote = 0; melody[thisNote].dur; thisNote++) {
		// to calculate the note duration, take one second divided by the note type.
		//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
		int noteDuration = 1000 / melody[thisNote].dur;
		/* tone(PIN_PIEZO, melody[thisNote].freq, noteDuration); */
		if (melody[thisNote].freq != 0)
			tone(PIN_PIEZO, melody[thisNote].freq);

		delay(noteDuration);

		// to distinguish the notes, set a minimum time between them.
		// the note's duration + 30% seems to work well:
		// stop the tone playing:
		if (melody[thisNote].freq != 0)
			noTone(PIN_PIEZO);
		delay(30);
	}
}

void tone_freq_durms(unsigned int freq, unsigned int dur) {
	tone(PIN_PIEZO, freq, dur);
	delay((dur*16)/14);
	noTone(PIN_PIEZO);
}

void tone_quick_freq(unsigned int freq) {
	tone(PIN_PIEZO, freq, 40);
	delay(60);
	noTone(PIN_PIEZO);
}

void tone_quick(void) {
	tone_quick_freq(440);
}

#endif // USE_TONE
