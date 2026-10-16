#include "functions.h"

void setup(void) {
  initialize_DFplayer();

  myDFPlayer.volume(15); // Set volume value. From 0 to 30
  myDFPlayer.play(1);    // Play the first mp3
}

void loop() {
  
}