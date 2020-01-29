/*
  This sample program uses one of those low cost ESP32 boards with OLED
  displays available from Banggood and AliExpress. These are great for
  evaluating performance of the receiver.

  The ESP32 has 3 hardware serial ports and the first is used for the USB to
  serial (for debugging). Port 2 is used for connection to the u-blox. Port 3
  can be used to display incoming data from another device such as a FPGA.
  The setup of the ports including input and output pins is in the .begin() method.
*/

#include <Arduino.h>
#include "u-blox-m8.h"

// Need library ESP8266_SSD1306 by Daniel Eichhorn (or may say Fabrice Weinberg)
#include "SSD1306.h"

#include "driver/timer.h"

// variables for PPS code
const byte        interruptPin = 13;              // Assign the interrupt pin for firebeetle32
//const byte        interruptPin = 4;              // Assign the interrupt pin for esp-wrover-kit
const byte        IRQpin = 25;
hw_timer_t * timer = NULL;                        // pointer to a variable of type hw_timer_t
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;  // synchs between main loop and interrupt?
volatile uint32_t ppsCount = 0;					          // increment this for every PPS pulse
volatile uint64_t timerVal;                       // the timer value on PPS rising edge interrupt
char deltaPPS[21];                                // store the PPS delta coming from the ESP32

// PPS - Digital Event Interrupt
// Enters on rising edge
//=======================================
void IRAM_ATTR handleInterrupt()
{
  //REG_WRITE( GPIO_OUT_W1TS_REG, BIT25 );  // NOTE if IRQpin changed have to edit this!
  REG_WRITE( GPIO_OUT_W1TC_REG, BIT25 );  // Changed to set low for Pulse Width Tool

  portENTER_CRITICAL_ISR( &mux );

  //timer_get_counter_value( (timer_group_t)0, (timer_idx_t)0, (uint64_t *)&timerVal );

  ppsCount++;

  portEXIT_CRITICAL_ISR(&mux);
}

void setupPPS()
{
  pinMode( IRQpin, OUTPUT ); // pin25 will be used as an output for testing PPS send
  digitalWrite( IRQpin, 1 ); // initialized high for Pulse Width Tool

  // PPS setup
  pinMode(interruptPin, INPUT_PULLUP);                                            // sets pin high
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING); // attaches pin to interrupt on Rising Edge

  timer = timerBegin(0, 8, true);
  timer_set_counter_value( (timer_group_t)0, (timer_idx_t)0, 0 );                                                                                    // 0 = first timer                                                                                  // 8 is prescaler so 80MHZ divided by 8 = 10MHZ signal ie 0.000001 of a second                                                                                  // true - counts up
  timerStart(timer);                                                              // starts the timer
}

#define SERIALDEBUG 0  // setting this will send debug messages to the console
#define SOCKETSERVER 0 // set this to 1 to use a socketserver to send data from serial port 2

#if SOCKETSERVER
#include <WiFi.h>

WiFiServer wifiServer(23);

const char* ssid = "";
const char* password =  "";
#endif

//HardwareSerial serial2( 2 ); // whatever come in here we will send to the socket client

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

  gpsSerial.begin( 9600, SERIAL_8N1, 15, 16 ); // input from u-blox on pin15, output to u-blox on 16

  //.begin(  115200, SERIAL_8N1, 14, 0 ); // whatever comes in here on pin 14 we will send to a socket client

#if SOCKETSERVER
  WiFi.begin(ssid, password);

  while( WiFi.status() != WL_CONNECTED )
  {
     delay( 1000 );
     Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());

  wifiServer.begin();
  wifiServer.setNoDelay(true);
#endif

  delay( 4000 );
  changeBaudrate( 115200 );  // change to 115200

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

#if SERIALDEBUG
  Serial.println( "u-blox initialized" );
#endif

#if !SOCKETSERVER // I think that the timer and interrupts interferes with the socket server...
  setupPPS();
#endif
}

void loop()
{
  static uint32_t ckerrors = 0;

  static uint32_t lastPpsCount = ppsCount;

  if( (ppsCount != lastPpsCount) && !digitalRead( interruptPin ) )
  {
    lastPpsCount = ppsCount;

    digitalWrite( IRQpin, 1 );
  }

#if SERIALDEBUG
  int g = gps.getchecksumerrors();

  if( g != ckerrors )
  {
    Serial.print( "checksum errors: " );
    Serial.println( g );
    ckerrors = g;
  }
#else
  ckerrors = gps.getchecksumerrors();
#endif

#if SOCKETSERVER
  static WiFiClient client;
  static uint64_t lastTimerVal = timerVal;

  if( wifiServer.hasClient() && !client.connected() )
    client = wifiServer.available();

  if( client.connected() )
  {
    while( /* serial2.available() */ false )
    {
      byte b;
      b = serial2.read();

      if( b == '\r' )
      {
        char temp[40];
        if( sc < 10.0 )
          sprintf( temp, " %2.2d:%2.2d:0%2.9f", hr, mn, sc );
        else
          sprintf( temp, " %2.2d:%2.2d:%2.9f", hr, mn, sc );

        client.write( temp );

        // add the PPS counter to the stream
        sprintf( temp, " %d", ppsCount );
        client.write( temp );
        sprintf( temp, " %lld", timerVal );
        client.write( temp );
        long int et = timerVal - lastTimerVal;
        lastTimerVal = timerVal;
        sprintf( temp, " %ld", et );
        client.write( temp );

        client.write( '\r' );
        client.write( '\n' );
      }
      else
        client.write( b );
    }
  }
#else // if no SOCKETSERVER then put the PPS delta time on the screen...
  while( /* serial2.available() */ false )
  {
    static int i = 0;

    byte b = /* serial2.read() */ 0;

    if( b == '\r' )
    {
      deltaPPS[i] = 0;
      i = 0;
    }

    if( i < 21 )
    {
      if( i > 7 )
        deltaPPS[i - 8] = b;

      i++;
    }
  }
#endif

  // The following flags are for polling because it seems that a few
  // requests might be needed to get a response. So poll until we get one!
  static bool waitForCfgtp5 = true; // for config of the time pulse
  static bool waitForCfgGnss = true;// and the GNSS configuration (to disable SBAS)

  while( gpsSerial.available() )
  {
    uint8_t c = gpsSerial.read();

    char *r = (char *)gps.parse( c );

    if( strlen( r ) > 0 )
    {
      if( strcmp( r, "navpvt8" ) == 0 )
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
      else if( strcmp( r, "cfgtp5" ) == 0 )
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
      else if( strcmp( r, "navsat" ) == 0 )
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
#if SOCKETSERVER
        sprintf( temp, "0:%2.2d 6:%2.2d S:%3.1f", satTypes[0], satTypes[6], snr );
        displayStatusMessage( 1, temp );
#else
        displayStatusMessage( 1, deltaPPS );
#endif
        sprintf( temp, "%2.2d:%2.2d:%f", hr, mn, sc );
        displayStatusMessage( 2, temp );

        sprintf( temp, "f:%X ns: %d ck: %d", flags, tacc, ckerrors );
        //sprintf( temp, "%2.2d:%2.2d:%2.2d", nav.gethour(), nav.getminute(), nav.getsecond() );
        //static int count = 0;
        //sprintf( temp, "%d", count++ );
        displayStatusMessage( 3, temp );

        display.display();
      }
      else if( strcmp( r, "cfggnss" ) == 0 )
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
