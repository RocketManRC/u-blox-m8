/*
  This sample program uses one of those low cost ESP32 boards with OLED
  displays available from Banggood and AliExpress. These are great for
  evaluating performance of the receiver.
*/

#include <Arduino.h>
#include "u-blox-m8.h"

// Need library ESP8266_SSD1306 by Daniel Eichhorn (or may say Fabrice Weinberg)
#include "SSD1306.h"

#define SERIALDEBUG 0

HardwareSerial gpsSerial( 1 );
ublox gps;
navpvt8 nav( gps );
cfgtp5 tp( gps );
navsat ns( gps );
cfggnss gc( gps );

// these are things we are going to display so keep them global
int satTypes[7];
double snr;
double sec;
int hr;
int mn;
double sc;
int numSV;
double pDOP;
uint32_t flags;
int tacc;

// Initialize the OLED display
SSD1306  display( 0x3c, 5, 4 ); // Wemos board
//SSD1306 display (0x3c, 4, 15); // TTGO with LoRa


// Send a byte to the receiver
void sendByte(byte b)
{
  gpsSerial.write( b );
}

// Send the packet specified to the receiver
void sendPacket(byte *packet, byte len)
{
  for (byte i = 0; i < len; i++)
  {
      gpsSerial.write( packet[i] );
  }
}

void displayStatusMessage( int line, char *msg )
{
  display.drawString( 0, line * 16, msg );
}

void setup()
{
  display.init();
  display.setFont( ArialMT_Plain_16 );
  display.setTextAlignment( TEXT_ALIGN_LEFT );
  display.flipScreenVertically();
  display.clear();
  displayStatusMessage( 1, (char *)"u-blox Tester" );
  display.display();

  Serial.begin( 115200 );

  gpsSerial.begin( 9600, SERIAL_8N1, 15, 16 );

  delay( 100 );
  changeBaudrate();  // change to 115200

  delay( 100 );
  gpsSerial.begin( 115200, SERIAL_8N1, 15, 16 );

  delay( 100 );
  gpsSerial.flush();

  delay( 100 );
  disableNmea();

  delay( 100 );
  enableNavPvt();

  delay( 100 );
  enableNavSat();

  delay( 100 );
}

void loop()
{
  static uint32_t ckerrors = 0;

  int g = gps.getchecksumerrors();

  if( g != ckerrors )
  {
    Serial.print( "checksum errors: " );
    Serial.println( g );
    ckerrors = g;
  }
  // The following flags are for polling because it seems that a few
  // requests might be needed to get a response. So poll until we get one!
  static bool waitForCfgtp5 = true; // for config of the time pulse
  static bool waitForCfgGnss = true;// and the GNSS configuration (to disable SBAS)

  while( gpsSerial.available() )
  {
    uint8_t c = gpsSerial.read();

    String r = (char *)gps.parse( c );

    if( r.length() > 0 )
    {
      if( r == "navpvt8" )
      {
        if( waitForCfgtp5 )
        {
          pollTimePulseParameters();  // ask for the current time pulse parameters then we will set them
        }
        else if( waitForCfgGnss )
        {
          gc.pollCfggnss();           // same for the gnss configuration
        }
#if SERIALDEBUG
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
        Serial.println( nav.gettacc() );
        Serial.println( nav.getflags(), 16 );
#endif
        sec = 3600.0 * nav.gethour() + 60.0 * nav.getminute() + 1.0 * nav.getsecond() + nav.getnano() * 1e-9;
        hr = sec / 3600;
        mn = (sec - hr * 3600) / 60;
        sc = sec - hr * 3600 - mn * 60;

        numSV = nav.getnumSV();
        pDOP = nav.getpDOP();
        flags = nav.getflags();
        tacc =  nav.gettacc();
      }
      else if( r == "cfgtp5" )
      {
#if SERIALDEBUG
        Serial.print( tp.getAntCableDelay() );
        Serial.print( " ");
        Serial.println( tp.getRfGroupDelay() );

        Serial.print( tp.getFreqPeriod() );
        Serial.print( " ");
        Serial.println( tp.getFreqPeriodLock() );

        Serial.print( tp.getPulseLenRatio() );
        Serial.print( " ");
        Serial.println( tp.getPulseLenRatioLock() );

        Serial.print( tp.getUserConfigDelay() );
        Serial.print( " ");
        Serial.println( tp.getFlags(), 16 );

        Serial.println( "Configure time pulse parameters" );
#endif
        //Serial.println( "Configure time pulse parameters" );
        // Here we set our time pulse parameters
        tp.setPulseLenRatio( 500000 );
        tp.configureTimePulse();

        waitForCfgtp5 = false; // only need to do it once
      }
      else if( r == "navsat" )
      {
        int numsvs = ns.getnumSvs();
#if SERIALDEBUG
        Serial.print( "Num SVs: ");
        Serial.println( numsvs );
#endif
        for( int i = 0; i < 7; i++ )
          satTypes[i] = 0;

        snr = 0.0; // average snr
        int c = 0;

        for( int i = 0; i < numsvs; i++ )
        {
          int flags = (int)ns.getflags( i );

          if( flags & 8 )
          {
            c++;
            snr += 1.0 * ns.getcno( i );

            int gnssId = (int)ns.getgnssId( i );
            if( gnssId < 7 && gnssId >= 0 )
              satTypes[gnssId]++;

#if SERIALDEBUG
            Serial.print( gnssId );
            Serial.print( " " );
            Serial.print( (int)ns.getsvId( i ) );
            Serial.print( " " );
            Serial.print( (int)ns.getcno( i ) );
            Serial.print( " " );
            Serial.print( flags, 16 );
            Serial.print( "   " );
#endif
          }
        }

        if( c > 0 )
          snr = snr / c;

#if SERIALDEBUG
        Serial.println( snr );
#endif
        // *** NOTE we update the OLED display here right after the NAV-SAT
        // message is received. The OLED takes around 20ms to update and
        // we can get serial buffer overflows (and therefore lost packets)
        // if we try to do it somewhere else. We could move it back to the
        // main loop and have flags to see that packets have been received
        // and that the serial buffer is empty before updating...
        display.clear();

        char temp[40];

        sprintf( temp, "SV:%2.2d  PD:%2.2f", numSV, pDOP );
        displayStatusMessage( 0, temp );

        sprintf( temp, "0:%2.2d 6:%2.2d S:%3.1f", satTypes[0], satTypes[6], snr );
        displayStatusMessage( 1, temp );

        sprintf( temp, "%2.2d:%2.2d:%f", hr, mn, sc );
        displayStatusMessage( 2, temp );

        sprintf( temp, "f:%X ns: %d ck: %d", flags, tacc, ckerrors );
        //sprintf( temp, "%2.2d:%2.2d:%2.2d", nav.gethour(), nav.getminute(), nav.getsecond() );
        //static int count = 0;
        //sprintf( temp, "%d", count++ );
        displayStatusMessage( 3, temp );

        display.display();
      }
      else if( r == "cfggnss" )
      {
        //Serial.print( "Num Blocks: ");
        int numblocks = gc.getnumConfigBlocks();
        //Serial.println( numblocks );

        for( int i = 0; i < numblocks; i++ )
        {
          int gnssId = (int)gc.getgnssId(i);

          //Serial.print( gnssId );
          //Serial.print( " " );
          //Serial.print( (int)gc.getFlags(i), 16 );
          //Serial.print( " " );

          if( gnssId == 1 )
            gc.setCfggnss( 1, false );  // Disable SBAS
        }

        waitForCfgGnss = false;
      }
    }
  }
}
