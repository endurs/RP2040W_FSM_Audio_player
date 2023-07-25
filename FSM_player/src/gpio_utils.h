#ifndef GPIO_UTILS_H
#define GPIO_UTILS_H

#define resetPin 22 

//led pins
#define led_status_pin 10
#define led_mode_pin 9
#define led_state_pin 8


// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s)) // idk what this, some sd card error thing 🤷‍♂️

void flasher (unsigned int timer);

#endif // GPIO_UTILS_H