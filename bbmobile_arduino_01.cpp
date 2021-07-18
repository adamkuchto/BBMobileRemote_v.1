
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
#include "Arduino.h"
#include "bbmobile_arduino_01.h"

  String bbm_buf ;
  byte bbm_conn ;

//#define DEBUG_ACK  //-you must do Serial.begin in setup() before using this

//===========================================================================================
//-------gets
//-------returns
// 0 - no Bluetooth cnnection establised
// 1 - Bluetooth cnnection is opened
//===========================================================================================
byte BBMobileIsConnected(void)
{
  return(bbm_conn) ;
}

//===========================================================================================
//-------gets
// bbm - pointer to the BBMobile stream
//-------returns
// 0 - no message from BBMobile
// 1 - there is data in bbm_buf from BBMobile
//===========================================================================================
byte BBMobileGetMessage(Stream *bbm)
{
  if(bbm->available() > 0)
  {
    bbm_buf = bbm->readStringUntil('\n') ;   
    if(bbm_buf.substring(0, 5) == ">DCON")
    {
      bbm_conn =0 ;      
      return(0) ;     
    }else if(bbm_buf.substring(0, 4) == ">CON")
    {
      bbm_conn =1 ;
      return(0) ; 
    }
    return(1) ;
  }
  return(0) ;    
}

//===========================================================================================
//-------gets
// bbm - pointer to the BBMobile stream
// timeout_ms - timeout in miliseconds
//-------returns
// 0 - got ACK
// 1 - timeout occured
// 2 - an error occured
// 3 - Bluetooth connection has been closed
//===========================================================================================
byte BBMobileWaitAck(Stream *bbm, int timeout_ms)
{
  do
  {
    delay(1) ;
    if(bbm->available() > 0)
    {
      bbm_buf = bbm->readStringUntil('\n') ;
      #ifdef DEBUG_ACK  
        Serial.println("ACK: " + bbm_buf) ;
      #endif  //-def DEBUG_ACK          
      if(bbm_buf.substring(0, 3) == ">OK") return(0) ;
      else if(bbm_buf.substring(0, 3) == "$ok") return(0) ;
      else if(bbm_buf.substring(0, 3) == ">HI") return(0) ;
      else if(bbm_buf.substring(0, 5) == ">DCON")
      {
        bbm_conn =0 ;              
        return(3) ;        
      }else return(2) ;
    }
  }while(timeout_ms-- >0) ;
  return(1) ;
}

//===========================================================================================
//-------gets
// bbm - pointer to the BBMobile stream
// s - string to send
//-------returns
// 0 - got ACK
// 1 - timeout occured
// 2 - an error occured
// 3 - Bluetooth connection has been closed
//===========================================================================================
byte BBMobileSend(Stream *bbm, String s)
{
  bbm->println(s) ;
  return( BBMobileWaitAck(bbm, 1000) ) ;
}

//===========================================================================================
//-------gets
// bbm - pointer to the BBMobile stream
// j - pointer to JSON data in flash memoty
//-------returns
// 0 - got ACK
// 1 - timeout occured
// 2 - an error occured
// 3 - Bluetooth connection has been closed
//===========================================================================================
byte BBMobileSendJson(Stream *bbm, char *j)
{
  char bb ;
  while(1)
  {
    bb = pgm_read_byte(j) ;
    if(bb == 0) return ;
    bbm->write(bb) ;    
    j++ ;
  } ;
  return( BBMobileWaitAck(bbm, 3000) ) ;
}

//===========================================================================================
//-gets int value from specific field name f
//-------gets
// f - pointer to the string with BBMobile data
// ret - pointer to store int variable
//-returns
// 0 - everything is OK
//-2 - no such field
//-4 - field has no data
//===========================================================================================
int BBMobileGetFieldInt(String *f, int *ret)
{
  int i, j ;

  i = bbm_buf.lastIndexOf(*f) ;    
  if(i < 0) return(-2) ;     //-no such field
  i += f->length() ;
  i++ ;
  if(bbm_buf.charAt(i) == '"') return(-4) ;   //-field has no data
  *ret =bbm_buf.substring(i).toInt() ;
  return( 0 ) ;
}

//===========================================================================================
//-gets float value from specific field name f
//-------gets
// f - pointer to the string with BBMobile data
// ret - pointer to store float variable
//-returns
// 0 - everything is OK
//-2 - no such field
//-4 - field has no data
//===========================================================================================
int BBMobileGetFieldFloat(String *f, float *ret)
{
  int i, j ;

  i = bbm_buf.lastIndexOf(*f) ;    
  if(i < 0) return(-2) ;     //-no such field
  i += f->length() ;
  i++ ;
  if(bbm_buf.charAt(i) == '"') return(-4) ;   //-field has no data
  *ret =bbm_buf.substring(i).toFloat() ;
  return( 0 ) ;
}
