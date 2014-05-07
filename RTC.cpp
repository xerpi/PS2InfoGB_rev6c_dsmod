// RTC for InfoGB
// Author: Krystian 'KarasQ' Karas
// Contact: k4rasq@gmail.com

#include "RTC.h"
#include "PS2Time.h"

// global PS2 timer
PS2Time timer;

// RTC data
RTC_Data RTC = {
  1, // RomBank
  0, // timer clock latch
  0, // timer clock register
  0, // timer seconds
  0, // timer minutes
  0, // timer hours
  0, // timer days
  0, // timer control
  0, // timer latched seconds
  0, // timer latched minutes
  0, // timer latched hours
  0, // timer latched days
  0, // timer latched control
  -1 // last time
};

void UpdateClockData(RTC_Data* RTC) {
  int now = timer.getTime(0);
  int diff = now - RTC->LastTime;
  
  if ( diff > 0 ) {
    // update the clock according to the last update time
    RTC->Seconds += diff % 60;
    if ( RTC->Seconds > 59 ) {
      RTC->Seconds -= 60;
      RTC->Minutes++;
    }

    diff /= 60;

    RTC->Minutes += diff % 60;
    if ( RTC->Minutes >= 60 ) {
      RTC->Minutes -= 60;
      RTC->Hours++;
    }

    diff /= 60;

    RTC->Hours += diff % 24;
    if ( RTC->Hours > 24 ) {
      RTC->Hours -= 24;
      RTC->Days++;
    }
    
    diff /= 24;

    RTC->Days += diff;
    if ( RTC->Days > 255 ) {
      if( RTC->Days > 511) {
        RTC->Days %= 512;
        RTC->Control |= 0x80;
      }
      RTC->Control = (RTC->Control & 0xfe) | (RTC->Days > 255 ? 1 : 0);
    }
  }
  
  RTC->LastTime = now;
}
