#ifndef _LOCAL_DEFINES_H
#define _LOCAL_DEFINES_H

#define PIN_BTN_TX    A3
#define PIN_MPU_INT   7
#define PIN_BTN_OURS  A2
#define PIN_PIEZO     4

#define USE_TONE

/* Using internal pullups on Control side,
 * so we TRIGGER ON LOW (we pull down) */
#define SEND_TRIGGERED() digitalWrite(PIN_BTN_TX, LOW)
#define SEND_RELEASED() digitalWrite(PIN_BTN_TX, HIGH)

#endif // _LOCAL_DEFINES_H
