#ifndef __KEY_H
#define __KEY_H

#include "main.h"

#define NO_KEY_PRES 0
#define KEY0_PRES   1
#define KEY1_PRES   2
#define KEY2_PRES   3
#define WKUP_PRES   4

uint8_t KEY_Scan(uint8_t mode);

#endif