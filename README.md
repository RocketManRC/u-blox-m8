# u-blox-m8
A library for managing u-blox m8 GNSS receivers, particularly suited for precision timing applications.

This is a library for managing the u-blox M8 series of GNSS (“GPS”) receiver modules. It is particularly suited for precision timing applications and is key part of the OpenPPS project. It uses the u-blox UBX binary protocol exclusively.

The most common variants of the M8 family appear to be the M8Q and M8N which as far as I can tell just have different physical form factors. There are also LEA and NEO versions of the M8N and once again these appears to be just different sizes and pin arrangements.

From a precision timing perspective the M8 modules produce PPS (pulse-per-second) pulses with accuracies compared to UTC in the 10s of nanoseconds range. They are also capable of sustaining this indoors in a surprising number of situations and the modules themselves can be configured to continue to provide PPS even when the module temporarily loses lock.

The M8 modules can also be configured to provide pulses that are more or less than once a second and also produce precise frequencies on the PPS pin instead of timing pulses.

To achieve maximum timing accuracy u-blox recommends that PPS be configured for once a second and that SBAS satellite correction services be disabled. It appears the the M8 modules can take advantage of a combination of GNSS systems and in North America the combination of GPS and GLONASS is available and appears to work extremely well.

This library has the following features:

* Uses the Arduino framework (could easily be made independent of it).
* The library is contained in a single header file u-blox-m8.h which makes it easy to integrate into a project.
* Works best with hardware serial ports such as on the ESP32, Teensy and Adafruit M0 and M4 boards.
* Depends on the main program to read from and write to the M8 module so the library is hardware independent.
* The UBX parser is state machine based with single byte input which means it will not hold up the main loop when called there.
* Includes functions to configure for best timing performance and monitor estimated accuracy.
* Examples are provided for testing the library on an ESP32 board.

The library does not directly deal with handling PPS pulses. A lot more information on that topic is available in the OpenPPS project: [OpenPPS](https://www.rocketmanrc/OpenPPS.html)

The reference document for the u-blox M8 receiver is here: [M8 Protocol Description](https://www.u-blox.com/sites/default/files/products/documents/u-blox8-M8_ReceiverDescrProtSpec_(UBX-13003221)_Public.pdf)

# Using the Library

U-blox receivers can save their configuration on receipt of a UBX command however this is probably never a good idea, especially from a developer’s perspective. This library is going to be easiest to use if the receiver is in its default configuration to start with and during development it is important to remember that after you have changed something (like for example the baud rate) it will remain that way until the receiver is power cycled. It is easy to structure commands so that this doesn’t matter. For example if we are changing the baud rate from the default of 9600 to 115200 that command will be ignored if the baud rate is already 115200 which is fine but if we want to change our code so the baud rate is different from 115200 then the power needs to be cycled before we can test that. Just saying!

An important thing to note is that there is only one buffer for incoming messages and it is associated with the parser. This is only going to be a problem when we want to see the current configuration and change it.

Keep in mind that that messages from the receiver can either be periodic or polled. Once you have periodic messages enabled then you can’t be sure when you issue a poll command for another message that you will get the one you want next. That is probably only an issue when the message being polled is going to be used for configuration as mentioned in the previous paragraph. In that case you will want to poll the message to get the existing configuration data and then as soon as that message is received make the desired changes and then send the message back. 

