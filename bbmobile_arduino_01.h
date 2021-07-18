
//---------------------------------------------------------------------------
// bbmobile_arduino_01
//---------------------------------------------------------------------------
// This is the very first verion of library for playing with BBMobile modules
// you can freely change and redistribute it - dont forget to add link to our website:
// www.bbmagic.net
// To jest pierwsza wersja biblioteki do obs�ugi modułów BBMobile
// możesz ją dowolnie modyfikować i redystrybuować - nie zapomnij tylko o zawarciu linku:
// www.bbmagic.net
//---------------------------------------------------------------------------
#ifndef BBMOBILE_ARDUINO_H
#define BBMOBILE_ARDUINO_H

	extern String bbm_buf ;

  byte BBMobileIsConnected(void) ;
  byte BBMobileGetMessage(Stream *bbm) ;
  byte BBMobileSend(Stream *bbm, String s) ;
  byte BBMobileSendJson(Stream *bbm, char *j) ;

  int BBMobileGetFieldInt(String *f, int *ret) ;
  int BBMobileGetFieldFloat(String *f, float *ret) ;

#endif
