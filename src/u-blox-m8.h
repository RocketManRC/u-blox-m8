/*
  A library to manage u-blox M8 GNSS receiver modules. It is particularly suited
  for precision timing applications.

  Added to Github on February 11, 2019
*/

#ifndef ubloxm8_h
#define ubloxm8_h

#include "Arduino.h"  // Only using Arduino Strings and HardwareSerial...

#define MAXBUFFERSIZE 1024  // using this to define the maximum buffer size

extern void sendByte(byte b);
extern void sendPacket(byte *packet, byte len);

const double mm2m = 1.0e-3;
const double en7 = 1.0e-7;
const double en5 = 1.0e-5;

struct _header
{
  uint8_t   cl;
  uint8_t   id;
  uint16_t  length;
};

// The NAV-PVT message is my one concession to the M7 receiver...

// u-blox 7 nav-pvt packet

struct _navpvt7hdr
{
  uint8_t   cl = 0x01;
  uint8_t   id = 0x07;
  uint16_t  length = 84;
};

typedef struct
{
  _navpvt7hdr header;
  uint32_t  iTOW;
  uint16_t  year;
  uint8_t   month;
  uint8_t   day;
  uint8_t   hour;
  uint8_t   min;
  uint8_t   sec;
  uint8_t   valid;
  uint32_t  tAcc;
  int32_t   nano;
  uint8_t   fixType;
  uint8_t   flags;
  uint8_t   flags2;
  uint8_t   numSV;
  int32_t   lon;
  int32_t   lat;
  int32_t   height;
  int32_t   hMSL;
  uint32_t  hAcc;
  uint32_t  vAcc;
  int32_t   velN;
  int32_t   velE;
  int32_t   velD;
  int32_t   gSpeed;
  int32_t   headMot;
  uint32_t  sAcc;
  uint32_t  headAcc;
  uint16_t  pDOP;
  uint8_t   reserved[6];
  int32_t		headVeh;
} _navpvt7;

// u-blox 8 nav-pvt packet

struct _navpvt8hdr
{
  uint8_t   cl = 0x01;
  uint8_t   id = 0x07;
  uint16_t  length = 92;
};

typedef struct   // u-blox 8
{
  _navpvt8hdr header;
  uint32_t  iTOW;
  uint16_t  year;
  uint8_t   month;
  uint8_t   day;
  uint8_t   hour;
  uint8_t   min;
  uint8_t   sec;
  uint8_t   valid;
  uint32_t  tAcc;
  int32_t   nano;
  uint8_t   fixType;
  uint8_t   flags;
  uint8_t   flags2;
  uint8_t   numSV;
  int32_t   lon;
  int32_t   lat;
  int32_t   height;
  int32_t   hMSL;
  uint32_t  hAcc;
  uint32_t  vAcc;
  int32_t   velN;
  int32_t   velE;
  int32_t   velD;
  int32_t   gSpeed;
  int32_t   headMot;
  uint32_t  sAcc;
  uint32_t  headAcc;
  uint16_t  pDOP;
  uint8_t   reserved[6];
  int32_t		headVeh;
  int16_t   magDec; // I think that magDec and magAcc are the only differences from u-blox 7
  uint16_t  magAcc; // ... for this nav packet
} _navpvt8;

// u-blox 8 cfg-tp5 packet

struct _cfgtp5hdr
{
  uint8_t   cl = 0x06;
  uint8_t   id = 0x31;
  uint16_t  length = 32; // make this 0 for command to poll (not supporting TIMEPULSE2)
};

typedef struct   // u-blox 8 configure time pulse
{
  _cfgtp5hdr header;
  uint8_t   tpIdx;    // 0 = TIMEPULSE, 1 = TIMEPULSE2
  uint8_t   version;  // 0 for this version
  uint16_t  reserved;
  int16_t   antCableDelay;  // antenna cable delay in ns
  int16_t   rfGroupDelay;   // RF group delay in ns
  uint32_t  freqPeriod;     // frequency in Hz or period in us
  uint32_t  freqPeriodLock; // frequency or period when locked
  uint32_t  pulseLenRatio;  // pulse length or duty cycle
  uint32_t  pulseLenRatioLock;  // pulse length or duty cycle when locked
  int32_t   userConfigDelay;    // user configurable time pulse delay in ns
  uint32_t  flags;          // configuration flags
} _cfgtp5;

struct _ackhdr
{
  uint8_t   cl = 0x05;
  uint8_t   id = 0x01;
  uint16_t  length = 2;
};

typedef struct
{
  _ackhdr header;
  uint8_t   clsId;  // Class ID of the Acknowledged Message
  uint8_t   msgId;  // Message ID of the Acknowledged Message
} _ack;

struct _nakhdr
{
  uint8_t   cl = 0x05;
  uint8_t   id = 0x00;
  uint16_t  length = 2;
};

typedef struct
{
  _nakhdr header;
  uint8_t clsId;  // Class ID of the Not-Acknowledged Message
  uint8_t msgId;  // Message ID of the Not-Acknowledged Message
} _nak;

struct _navsathdr
{
  uint8_t   cl = 0x01;
  uint8_t   id = 0x35;
  uint16_t  length = 0;  // this is a variable length message
};

struct _navsatintro
{
  uint32_t  iTOW;
  uint8_t   version;
  uint8_t   numSvs;
  uint16_t  reserved1;
};

struct _navsatblock
{
  uint8_t   gnssId;
  uint8_t   svId;
  uint8_t   cno;
  int8_t    elev;
  int16_t   azim;
  int16_t   prRes;
  uint32_t  flags;
};

typedef struct
{
  _navsathdr header;
  _navsatintro intro;
  _navsatblock block[0];  // this will be variable length, there is lots of room in the 1K buffer
} _navsat;  // this is the received message

struct _cfggnsshdr
{
  uint8_t   cl = 0x06;
  uint8_t   id = 0x3E;
  uint16_t  length = 0;  // this is a variable length message
};

struct _cfggnssintro
{
  uint8_t  msgVer;
  uint8_t  numTrkChHw;
  uint8_t  numTrkChUse;
  uint8_t  numConfigBlocks;
};

struct _cfggnssblock
{
  uint8_t   gnssId;
  uint8_t   resTrkCh;
  uint8_t   maxTrkCh;
  int8_t    reserved;
  uint32_t  flags;
};

typedef struct
{
  _cfggnsshdr header;
  _cfggnssintro intro;
  _cfggnssblock block[0];  // this will be variable length, there is lots of room in the 1K buffer
} _cfggnss;  // this is the received message

//Declare a buffer for every packet we know about. We are only going
//to load those packets.
typedef union
{
    _navpvt7 navpvt7;
    _navpvt8 navpvt8;
    _cfgtp5  cfgtp5;
    _nak nak;
    _ack ack;
    _navsat navsat;
    _cfggnss cfggnss;
    byte data[MAXBUFFERSIZE]; // I added this because you can't predict the size of variable length messages...
} _buf;

// This is a declaration of packets we know about. We are only going to load those
struct _navpvt7hdr navpvt7hdr;
struct _navpvt8hdr navpvt8hdr;
struct _cfgtp5hdr  cfgtp5hdr;
struct _ackhdr     ackhdr;
struct _nakhdr     nakhdr;
struct _navsathdr  navsathdr;
struct _cfggnsshdr cfggnsshdr;

// Array of packet headers and array of packet names
struct _header *packetheaders[] = { (struct _header *)&navpvt7hdr, (struct _header *)&navpvt8hdr, \
  (struct _header *)&cfgtp5hdr, (struct _header *)&ackhdr, (struct _header *)&nakhdr, \
  (struct _header *)&navsathdr, (struct _header *)&cfggnsshdr };

const char *packetnames[] = { "navpvt7", "navpvt8", "cfgtp5", "ack", "nak", "navsat", "cfggnss" };

enum class State { sync1, sync2, header, payload, check };

/*
  This is the class for parsing incoming packets. It uses a state-machine
  approach and the method parse() expects a single byte for input.
  Parse() returns the name of the packet on completion or a null drawString
  otherwise.

  Parse uses Arduino type Strings.
*/

class ublox
{
  public:
    ublox()
    {
        state = State::sync1;
        count = 0;
    };

    const char *parse( uint8_t c )
    {
      uint8_t *p = (uint8_t *)&buffer;

      switch( state )
      {
        case State::sync1:
        {
          count = 0;
          result = (char *)"";

          if( c == 0xB5 )
          {
            count++;
            state = State::sync2;
          }
        }
        break;

        case State::sync2:
        {
          if( c == 0x62 )
          {
            count++;
            state = State::header;
          }
          else
          {
            state = State::sync1;
          }
        }
        break;

        case State::header:
          if( count < 6 )
          {
            p[ count++ - 2 ] = c; // note: we are not putting the sync bytes in the buffer
          }
          else
          {
            p[ count++ - 2 ] = c; // don't lose the incoming byte!
            struct _header *packetheader = (_header *)p;

            for( int i = 0; i < sizeof( packetheaders) / sizeof( void * ); i++ )
            {
              struct _header *h = (struct _header *)packetheaders[i];

              // can't always check packetlength because some packets have unknown length (set as 0)
              if( h->cl == packetheader->cl && h->id == packetheader->id && (h->length == packetheader->length || h->length == 0) )
              {
                result = (char *)packetnames[i]; // this will be the packet if there are no errors
                length = packetheader->length;
                payload_p = &(p[4]);
                state = State::payload; // this will only change if we have a packet we know about
                break; // don't need to look any farther once we find one
              }
            }

            if( state == State::payload ) // were we successfull?
            {
              // Nothing to do here for now...
            }
            else
            {
              state = State::sync1;
            }
          }
          break;

        case State::payload:
        {
          if( count < length + 6 )
          {
            p[ count++ - 2 ] = c;
          }
          else
          {
            // The checksum is over the header plus the payload
            calculatechecksum( checksum, payload_p - 4, length + 4 );

            if( checksum[0] == c ) // check the first checksum byte
              state = State::check;
            else
            {
              //Serial.println( " checksum error1" );
              state = State::sync1;
            }
          }
        }
        break;

        case State::check:
        {
          if( c == checksum[1] )
          {
            state = State::sync1; // set to look at next packet
            return result; // return our result!
          }
          else
          {
            //Serial.println( " checksum error2" );
            state = State::sync1;
          }
        }
        break;
      }

      return "";
    };

    uint8_t *getbuffer()
    {
      return buffer;
    };

  	void calculatechecksum( uint8_t *ck, uint8_t *payload, uint16_t length )
    {
      ck[0] = 0;
      ck[1] = 0;

      for( uint16_t i = 0; i < length; i++ )
      {
            ck[0] += payload[i];
            ck[1] += ck[0];
      }
    };

    uint8_t checksum[2];
    State state;
    uint16_t count;
    uint16_t length;
    uint8_t *payload_p;
    char *result = (char *)"";
    uint8_t buffer[sizeof(_buf)];
};

class navpvt7
{
  public:
    navpvt7( ublox &gps )
    {
      buffer = gps.getbuffer();
    };

    int32_t getnano() { return ((_navpvt7 *)buffer)->nano; }
    uint8_t getnumSV() { return ((_navpvt7 *)buffer)->numSV; }
    double getlon() { return ((_navpvt7 *)buffer)->lon * en7; }
    double getlat() { return ((_navpvt7 *)buffer)->lat * en7; }
    double getheight() { return ((_navpvt7 *)buffer)->height * mm2m; }
    double gethAcc() { return ((_navpvt7 *)buffer)->hAcc * mm2m; }
    double getvAcc() { return ((_navpvt7 *)buffer)->vAcc * mm2m; }
    double getpDOP() { return ((_navpvt7 *)buffer)->pDOP * 0.01L; }

  private:
    uint8_t *buffer;
};

class navpvt8
{
  public:
    navpvt8( ublox &gps )
    {
      buffer = gps.getbuffer();
    };

    uint32_t gettacc() { return ((_navpvt8 *)buffer)->tAcc; }
    uint8_t getnumSV() { return ((_navpvt8 *)buffer)->numSV; }
    double getlon() { return ((_navpvt8 *)buffer)->lon * en7; }
    double getlat() { return ((_navpvt8 *)buffer)->lat * en7; }
    double getheight() { return ((_navpvt8 *)buffer)->height * mm2m; }
    double gethAcc() { return ((_navpvt8 *)buffer)->hAcc * mm2m; }
    int32_t getvAcc() { return ((_navpvt8 *)buffer)->vAcc; }
    double getpDOP() { return ((_navpvt8 *)buffer)->pDOP * 0.01L; }
    uint8_t getflags() { return ((_navpvt8 *)buffer)->flags; }
    uint16_t getyear() { return ((_navpvt8 *)buffer)->year; }
    uint8_t getmonth() { return ((_navpvt8 *)buffer)->month; }
    uint8_t getday() { return ((_navpvt8 *)buffer)->day; }
    uint8_t gethour() { return ((_navpvt8 *)buffer)->hour; }
    uint8_t getminute() { return ((_navpvt8 *)buffer)->min; }
    uint8_t getsecond() { return ((_navpvt8 *)buffer)->sec; }
    int32_t getnano() { return ((_navpvt8 *)buffer)->nano; }

  private:
    uint8_t *buffer;
};

class cfgtp5
{
  public:
    cfgtp5( ublox &gps )
    {
      buffer = gps.getbuffer();
      _gps = gps;
    };

    uint16_t  getAntCableDelay() { return ((_cfgtp5 *)buffer)->antCableDelay; }
    uint16_t  getRfGroupDelay() { return ((_cfgtp5 *)buffer)->rfGroupDelay; }
    uint32_t  getFreqPeriod() { return ((_cfgtp5 *)buffer)->freqPeriod; }
    uint32_t  getFreqPeriodLock() { return ((_cfgtp5 *)buffer)->freqPeriodLock; }
    uint32_t  getPulseLenRatio() { return ((_cfgtp5 *)buffer)->pulseLenRatio; }
    uint32_t  getPulseLenRatioLock() { return ((_cfgtp5 *)buffer)->pulseLenRatioLock; }
    int32_t   getUserConfigDelay() { return ((_cfgtp5 *)buffer)->userConfigDelay; }
    uint32_t  getFlags()  { return ((_cfgtp5 *)buffer)->flags; }

    void setAntCableDelay( uint16_t  antCableDelay ) { ((_cfgtp5 *)buffer)->antCableDelay = antCableDelay; }
    void setRfGroupDelay( uint16_t  rfGroupDelay ) { ((_cfgtp5 *)buffer)->rfGroupDelay = rfGroupDelay; }
    void setFreqPeriod( uint32_t  freqPeriod ) { ((_cfgtp5 *)buffer)->freqPeriod = freqPeriod; }
    void setFreqPeriodLock( uint32_t  freqPeriodLock ) { ((_cfgtp5 *)buffer)->freqPeriodLock = freqPeriodLock; }
    void setPulseLenRatio( uint32_t  pulseLenRatio ) { ((_cfgtp5 *)buffer)->pulseLenRatio = pulseLenRatio; }
    void setPulseLenRatioLock( uint32_t  pulseLenRatioLock ) { ((_cfgtp5 *)buffer)->pulseLenRatioLock = pulseLenRatioLock; }
    void setUserConfigDelay( int32_t  userConfigDelay ) { ((_cfgtp5 *)buffer)->userConfigDelay = userConfigDelay; }
    void setFlags( uint32_t  flags ) { ((_cfgtp5 *)buffer)->flags = flags; }

    void configureTimePulse()
    {
      sendByte( 0xB5 );
      sendByte( 0x62 );

      sendPacket( buffer, cfgtp5hdr.length + 4 );

      uint8_t ck[2];
      _gps.calculatechecksum( ck, buffer, cfgtp5hdr.length + 4 );

      sendPacket( ck, 2 );
    }

  private:
    uint8_t *buffer;
    ublox _gps;
};

class navsat
{
  public:
    navsat( ublox &gps )
    {
      buffer = gps.getbuffer();
      _gps = gps;
    };

    uint8_t  getnumSvs() { return ((_navsat *)buffer)->intro.numSvs; }
    uint8_t  getgnssId( int satnum ) { return ((_navsat *)buffer)->block[satnum].gnssId; }
    uint8_t  getsvId( int satnum ) { return ((_navsat *)buffer)->block[satnum].svId; }
    uint8_t  getcno( int satnum ) { return ((_navsat *)buffer)->block[satnum].cno; }
    uint8_t  getelev( int satnum ) { return ((_navsat *)buffer)->block[satnum].elev; }
    uint8_t  getazim( int satnum ) { return ((_navsat *)buffer)->block[satnum].azim; }
    uint8_t  getprRes( int satnum ) { return ((_navsat *)buffer)->block[satnum].prRes; }
    uint8_t  getflags( int satnum ) { return ((_navsat *)buffer)->block[satnum].flags; }

    void pollNavsat()
    {
      sendByte( 0xB5 );
      sendByte( 0x62 );

      byte *pNavsathdr = (byte *)&navsathdr;

      for( int i = 0; i < sizeof(navsathdr); i++ )
        buffer[i] = pNavsathdr[i];

      sendPacket( buffer, navsathdr.length + 4 );

      uint8_t ck[2];
      _gps.calculatechecksum( ck, buffer, navsathdr.length + 4 );

      sendPacket( ck, 2 );
    }

  private:
    uint8_t *buffer;
    ublox _gps;
};

class cfggnss
{
  public:
    cfggnss( ublox &gps )
    {
      buffer = gps.getbuffer();
      _gps = gps;
    };

    uint8_t  getnumConfigBlocks() { return ((_cfggnss *)buffer)->intro.numConfigBlocks; }
    uint8_t  getgnssId( int blocknum ) { return ((_cfggnss *)buffer)->block[blocknum].gnssId; }
    uint32_t  getFlags( int blocknum ) { return ((_cfggnss *)buffer)->block[blocknum].flags; }

    void pollCfggnss()
    {
      sendByte( 0xB5 );
      sendByte( 0x62 );

      byte *pCfggnsshdr = (byte *)&cfggnsshdr;

      for( int i = 0; i < sizeof(cfggnsshdr); i++ )
        buffer[i] = pCfggnsshdr[i];

      sendPacket( buffer, cfggnsshdr.length + 4 );

      uint8_t ck[2];
      _gps.calculatechecksum( ck, buffer, cfggnsshdr.length + 4 );

      sendPacket( ck, 2 );
    }

    void setCfggnss( int gnssId, bool enable )
    {
      if( enable )
        ((_cfggnss *)buffer)->block[gnssId].flags |= 1;
      else
        ((_cfggnss *)buffer)->block[gnssId].flags &= 0xFFFFFFFE;

      sendByte( 0xB5 );
      sendByte( 0x62 );

      _cfggnsshdr *pCfggnsshdr = (_cfggnsshdr *)&buffer[0];

      sendPacket( buffer, pCfggnsshdr->length + 4 );

      uint8_t ck[2];
      _gps.calculatechecksum( ck, buffer, pCfggnsshdr->length + 4 );

      sendPacket( ck, 2 );
    }

  private:
    uint8_t *buffer;
    ublox _gps;
};

// *** ublox configuration stuff
// This depends on sendPacket() and sendByte() being defined in the main program

// Print the packet specified to the PC  in a hexadecimal form, for debugging
void printPacket(byte *packet, byte len)
{
    char temp[3];

    for (byte i = 0; i < len; i++)
    {
        sprintf(temp, "%.2X", packet[i]);
        Serial.print(temp);

        if (i != len - 1)
        {
            Serial.print(' ');
        }
    }

    Serial.println();
}

// Send a packet to the receiver to restore default configuration
void restoreDefaults()
{
    // CFG-CFG packet
    byte packet[] = {
        0xB5, // sync char 1
        0x62, // sync char 2
        0x06, // class
        0x09, // id
        0x0D, // length
        0x00, // length
        0xFF, // payload
        0xFF, // payload
        0x00, // payload
        0x00, // payload
        0x00, // payload
        0x00, // payload
        0x00, // payload
        0x00, // payload
        0xFF, // payload
        0xFF, // payload
        0x00, // payload
        0x00, // payload
        0x17, // payload
        0x2F, // CK_A
        0xAE, // CK_B
    };

    sendPacket(packet, sizeof(packet));
}

// Send a set of packets to the receiver to disable NMEA messages
void disableNmea()
{
    // Array of two bytes for CFG-MSG packets payload
    byte messages[][2] = {
        {0xF0, 0x0A},
        {0xF0, 0x09},
        {0xF0, 0x00},
        {0xF0, 0x01},
        {0xF0, 0x0D},
        {0xF0, 0x06},
        {0xF0, 0x02},
        {0xF0, 0x07},
        {0xF0, 0x03},
        {0xF0, 0x04},
        {0xF0, 0x0E},
        {0xF0, 0x0F},
        {0xF0, 0x05},
        {0xF0, 0x08},
        {0xF1, 0x00},
        {0xF1, 0x01},
        {0xF1, 0x03},
        {0xF1, 0x04},
        {0xF1, 0x05},
        {0xF1, 0x06},
    };

    // CFG-MSG packet buffer
    byte packet[] = {
        0xB5, // sync char 1
        0x62, // sync char 2
        0x06, // class
        0x01, // id
        0x03, // length
        0x00, // length
        0x00, // payload (first byte from messages array element)
        0x00, // payload (second byte from messages array element)
        0x00, // payload (not changed in the case)
        0x00, // CK_A
        0x00, // CK_B
    };
    byte packetSize = sizeof(packet);

    // Offset to the place where payload starts
    byte payloadOffset = 6;

    // Iterate over the messages array
    for (byte i = 0; i < sizeof(messages) / sizeof(*messages); i++)
    {
        // Copy two bytes of payload to the packet buffer
        for (byte j = 0; j < sizeof(*messages); j++)
        {
            packet[payloadOffset + j] = messages[i][j];
        }

        // Set checksum bytes to the null
        packet[packetSize - 2] = 0x00;
        packet[packetSize - 1] = 0x00;

        // Calculate checksum over the packet buffer excluding sync (first two) and checksum chars (last two)
        for (byte j = 0; j < packetSize - 4; j++)
        {
            packet[packetSize - 2] += packet[2 + j];
            packet[packetSize - 1] += packet[packetSize - 2];
        }
        sendPacket(packet, packetSize);
    }
}

// Send a packet to the receiver to change baudrate to 115200
void changeBaudrate()
{
    // CFG-PRT packet
    byte packet[] = {
        0xB5, // sync char 1
        0x62, // sync char 2
        0x06, // class
        0x00, // id
        0x14, // length
        0x00, // length
        0x01, // payload
        0x00, // payload
        0x00, // payload
        0x00, // payload
        0xD0, // payload
        0x08, // payload
        0x00, // payload
        0x00, // payload
        0x00, // payload
        0xC2, // payload
        0x01, // payload
        0x00, // payload
        0x07, // payload
        0x00, // payload
        0x03, // payload
        0x00, // payload
        0x00, // payload
        0x00, // payload
        0x00, // payload
        0x00, // payload
        0xC0, // CK_A
        0x7E, // CK_B
    };

    sendPacket(packet, sizeof(packet));
}

// Send a packet to the receiver to change frequency to 100 ms
void changeFrequency()
{
    // CFG-RATE packet
    byte packet[] = {
        0xB5, // sync char 1
        0x62, // sync char 2
        0x06, // class
        0x08, // id
        0x06, // length
        0x00, // length
        0x64, // payload
        0x00, // payload
        0x01, // payload
        0x00, // payload
        0x01, // payload
        0x00, // payload
        0x7A, // CK_A
        0x12, // CK_B
    };

    sendPacket(packet, sizeof(packet));
}

// Send a packet to the receiver to disable unnecessary channels
void disableUnnecessaryChannels()
{
    // CFG-GNSS packet
    byte packet[] = {
        0xB5, // sync char 1
        0x62, // sync char 2
        0x06, // class
        0x3E, // id
        0x24, // length
        0x00, // length

        0x00, 0x00, 0x16, 0x04, 0x00, 0x04, 0xFF, 0x00, // payload
        0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x03, 0x00, // payload
        0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x03, 0x00, // payload
        0x00, 0x00, 0x00, 0x01, 0x06, 0x08, 0xFF, 0x00, // payload
        0x00, 0x00, 0x00, 0x01,                         // payload

        0xA4, // CK_A
        0x25, // CK_B
    };

    sendPacket(packet, sizeof(packet));
}

// Send a packet to the receiver to enable NAV-PVT messages
void enableNavPvt()
{
    // CFG-MSG packet
    byte packet[] = {
        0xB5, // sync char 1
        0x62, // sync char 2
        0x06, // class
        0x01, // id
        0x03, // length
        0x00, // length
        0x01, // payload
        0x07, // payload
        0x01, // payload
        0x13, // CK_A
        0x51, // CK_B
    };

    sendPacket(packet, sizeof(packet));
}

// Send a packet to the receiver to enable NAV-SAT messages
void enableNavSat()
{
    // CFG-MSG packet
    byte packet[] = {
        0xB5, // sync char 1
        0x62, // sync char 2
        0x06, // class
        0x01, // id
        0x03, // length
        0x00, // length
        0x01, // payload
        0x35, // payload
        0x01, // payload
        0x00, // CK_A
        0x00, // CK_B
    };

    byte packetSize = sizeof(packet);

    for (byte j = 0; j < packetSize - 4; j++)
    {
        packet[packetSize - 2] += packet[2 + j];
        packet[packetSize - 1] += packet[packetSize - 2];
    }

    sendPacket(packet, sizeof(packet));
}

void pollTimePulseParameters()
{
  byte packet[] =
  {
      0xB5, // sync char 1
      0x62, // sync char 2
      0x06, // class
      0x31, // id
      0x00, // length
      0x00, // length
      0x00, // CK_A
      0x00, // CK_B
  };

  byte packetSize = sizeof(packet);

  for (byte j = 0; j < packetSize - 4; j++)
  {
      packet[packetSize - 2] += packet[2 + j];
      packet[packetSize - 1] += packet[packetSize - 2];
  }

  sendPacket(packet, sizeof(packet));
}

void sendTimePulseParameters( uint32_t flags )
{
  byte packet[sizeof(_cfgtp5) + 4];


  packet[0] =  0xB5; // sync char 1
  packet[1] =  0x62; // sync char 2
  packet[2] =  cfgtp5hdr.cl;
  packet[3] =  cfgtp5hdr.id;
  uint16_t *i16p = (uint16_t *)&(packet[4]);
  *i16p = cfgtp5hdr.length;

  _cfgtp5 *p = (_cfgtp5 *)&(packet[2]);

  p->tpIdx = 0;    // 0 = TIMEPULSE, 1 = TIMEPULSE2
  p->version = 0;  // 1 for this version
  p->reserved = 0;
  p->antCableDelay = 50;  // antenna cable delay in ns
  p->rfGroupDelay = 0;   // RF group delay in ns
  p->freqPeriod = 1000000;     // frequency in Hz or period in us
  p->freqPeriodLock = 1000000; // frequency or period when locked
  p->pulseLenRatio = 500000;  // pulse length or duty cycle
  p->pulseLenRatioLock = 100000;  // pulse length or duty cycle when locked
  p->userConfigDelay = 0;    // user configurable time pulse delay in ns
  p->flags = flags;

  byte packetSize = sizeof(packet);

  packet[packetSize - 1] = 0;
  packet[packetSize - 2] = 0;

  for (byte j = 0; j < packetSize - 4; j++)
  {
      packet[packetSize - 2] += packet[2 + j];
      packet[packetSize - 1] += packet[packetSize - 2];
  }

  sendPacket(packet, sizeof(packet));
}

void pollSatNavParameters()
{
  byte packet[] =
  {
      0xB5, // sync char 1
      0x62, // sync char 2
      0x01, // class
      0x35, // id
      0x00, // length
      0x00, // length
      0x00, // CK_A
      0x00, // CK_B
  };

  byte packetSize = sizeof(packet);

  for (byte j = 0; j < packetSize - 4; j++)
  {
      packet[packetSize - 2] += packet[2 + j];
      packet[packetSize - 1] += packet[packetSize - 2];
  }

  sendPacket(packet, sizeof(packet));
}

#endif
