#!/usr/bin/python
"""This is the curfuel daemon, it is meant to run eternally on a machine
connected to an AVR microcontroller reading fuel level on its analog input.
"""
import sys
import curfuel

if __name__ == '__main__':
    calibration = False

    if len(sys.argv) < 2:
        print("usage: %s [-c] config.ini" % sys.argv[0])
        sys.exit(-1)

    if "-c" in sys.argv:
        calibration = True
        sys.argv.remove("-c")

    app = curfuel.DaemonApp()
    app.load_config(sys.argv[1])
    try:
        if calibration:
            app.calibrate()
        else:
            app.run()
    except KeyboardInterrupt:
        app.shutdown()

