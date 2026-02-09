#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

extern int keyIdx;


void keyboardDriver();
void sendLightStatusByte();
void setCapsLockLightStatus(bool val);
void setScrollLockLightStatus(bool val);
void setNumLockLightStatus(bool val);

#endif