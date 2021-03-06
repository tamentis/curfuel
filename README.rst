.. -*- restructuredtext -*-

=========
 curfuel
=========

curfuel is:
 - an avr driver/program to collect data from a resistive fuel sending unit
 - a python daemon reading the data from the serial/usb port

Requirements
============
 - Python 2.5 or above (3.x untested)
 - PySerial
 - avr-gcc + avrdude + toolchain (to compile/upload the driver)
 - A resistive sending unit plugged on an analogic input of your AVR.

Driver
======
The driver is written in C and will cross-compile with GCC. It will make
the Atmel microcontroller wait for instructions from the serial/usb port
and sample the fuel level ten times over 100ms to create an average and
return it on the serial/usb port.

Daemon
======
This small piece of software will poll the micro-controller every seconds,
reading the serial/usb port and look for the ``raw_fuel_level`` output token
then store the interpreted gallon level in a flat text file. The format is::

	code:utc_unix_timestamp:uc_ticks:raw_reading:gallons

``uc_ticks`` and ``raw_readings`` are kept as reference in case of anomaly,
they are respectively the internal millesecond tick of the microcontroller and
the raw reading on a 10bit scale averaged.

If parsing the file, expect ``code`` to be ``r`` in normal conditions. Errors
and warnings will be reported with ``e`` and ``w``.

The daemon will try as hard as possible to be error tolerant, it won't die
if the device is unplugged and will wait patiently for it to be available
again, you might get a few errors/warnings in your log file, but they are
properly prefixed.

Installation
============
Assuming you have the whole avr toolchain, you can go in the driver directory,
adjust the device and serial port in the Makefile and type::

    make clean
    make
    make upload

Now you can install the daemon with::

    python setup.py install

You can copy the configuration file somewhere convenient (/etc comes to mind),
update the device in the [serial] section and calibrate it::

    curfueld.py -c /etc/curfueld.ini

After moving your sending unit to both extremities a few times, take note of
the minimum and maximum and update your configuration file.

You can now start curfueld.py. There is currently no automatic way to
background the process, a few alternatives are just running it with &, using
screen/tmux.

License
=======
All the new code is under ISC license (BSD/MIT compatible). Parts of the
library were not rewritten and are licensed under LGPL (wiring and arduino).

TODO
====
 - Too much noise at the moment, we need to get the sending unit out and 
   try a smaller resistor to allow more current to flow through, but don't
   do that while the sensor is in the tank to avoid overheating. Test it out
   for 24 hours first in full and empty positions.

 - Investigate using imlib2 as a drawing library to create the same kind of
   charts currently created with matplot.

 - Refactor plotter.py completely into a library with two compiles sub-modules
   (record loader and chart generator) and a main script using this module
   every 10 minutes to update the charts.
