#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H

#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"

// DFplayer components
extern DFRobotDFPlayerMini myDFPlayer;

void initialize_DFplayer();
void printDetail(uint8_t type, int value);
void playFirstSong();

#endif