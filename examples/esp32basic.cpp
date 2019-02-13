/*
  This a simple example of using the library on an ESP32. For a more
  complex example see esp32oled.cpp (the OLED part is minimal)
*/

#include <Arduino.h>
#include "u-blox-m8.h"

HardwareSerial gpsSerial( 1 );  // use the ESP32 second serial port
ublox gps;                      // this is initializing the parser. The buffer is in this object
navpvt8 nav( gps );             // this defines the UBX-NAV-PVT message
cfggnss gc( gps );              // this defines the UBX-CFG-GNSS message and command

// Send a byte to the receiver, defined here to keep the library hardware independent
void sendByte(byte b)
{
  gpsSerial.write( b );
}

// Send the packet specified to the receiver, here for the same reason as above
void sendPacket(byte *packet, byte len)
{
  for (byte i = 0; i < len; i++)
  {
      gpsSerial.write( packet[i] );
  }
}

void setup()
{
  Serial.begin( 115200 );                        // for the console on USB (port 1)

  // Initialize the ESP32 second port on pins 15 & 16
  gpsSerial.begin( 9600, SERIAL_8N1, 15, 16 );   // remember that the receiver defaults to 9600 baud

  delay( 100 );

  changeBaudrate();  // tell the receiver to use 115200 baud (hard coded to 115200 right now)
  delay( 100 );

  gpsSerial.begin( 115200, SERIAL_8N1, 15, 16 );  // change our hardware serial port to 115200 baud
  delay( 100 );

  gpsSerial.flush();    // this eliminates receiving a lot of junk, probably not necessary
  delay( 100 );

  disableNmea();      // NMEA is the receiver’s default and clutters things especially when debugging.
  delay( 100 );

  enableNavPvt();    // Finally our command to periodically get the UBX-NAV-PVT message
  delay( 100 );
}

void loop()
{
  if( gpsSerial.available() )
  {
    uint8_t c = gpsSerial.read();

    String r = (char *)gps.parse( c );

    if( r.length() > 0 )  // the parser will return the name of the message when one is received
    {
      if( r == "navpvt8" )
      {    // There are all kinds of goodies in the UBX-NAV-PVT message
        Serial.println( nav.getnumSV() );
        Serial.print( nav.getlat(), 5 );
        Serial.print( " ");
        Serial.print( nav.getlon(), 5 );
        Serial.print( " ");
        Serial.println( nav.getheight(), 2 );
        Serial.print( nav.getpDOP(), 2 );
        Serial.print( " ");
        Serial.print( nav.gethAcc() );
        Serial.print( " ");
        Serial.println( nav.getvAcc() );
        Serial.print( nav.getnano() );
        Serial.print( " ");
        Serial.println( nav.gettacc() );  // this is the estimated timing accuracy in nanoseconds
        Serial.println( nav.getflags(), 16 );

        // Here is how to caculate UTC of the timepulse to the nanosecond!
        // It really should be extended to include days, months and year...
        double sec = 3600.0 * nav.gethour() + 60.0 * nav.getminute() + 1.0 * nav.getsecond() + nav.getnano() * 1e-9;
        int hr = sec / 3600;
        int mn = (sec - hr * 3600) / 60;
        double sc = sec - hr * 3600 - mn * 60;

        Serial.print( hr );
        Serial.print( ":" );
        Serial.print( mn );
        Serial.print( ":" );
        Serial.println( sc, 9 );
      }
    }
  }
}
