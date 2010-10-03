import sys
import time
import serial
import ConfigParser
import tank

class DaemonApp(object):
    """This is the curfuel daemon main class, it is instantiated from a
    script in the root directory.
    """
    def __init__(self):
        self.port = None
        self.port_name = None
        self.port_retries = 10
        self.port_speed = 9600
        self.tank_orientation = None
        self.fuel_token = "raw_fuel_level"
        self.min_value = 0.0
        self.max_value = 1023.0
        self.tank = None
    
    def load_config(self, filename):
        cp = ConfigParser.ConfigParser()
        cp.read(filename)
        self.port_name = cp.get("serial", "device")
        self.min_value = float(cp.get("calibration", "minimum"))
        self.max_value = float(cp.get("calibration", "maximum"))

        # Setup tank
        orientation = cp.get("tank", "orientation")
        depth = float(cp.get("tank", "depth"))
        length = float(cp.get("tank", "length"))
        width = float(cp.get("tank", "width"))
        self.tank = tank.Tank(orientation, depth, length, width)

    def open_port(self):
        """Extremely fault tolerant open... you never know."""
        retries = self.port_retries
        while True:
            try:
                self.port = serial.Serial(self.port_name, self.port_speed,
                        timeout=5)
                break
            except:
                if retries > 0:
                    retries -= 1
                    pass
                else:
                    raise

    def parse_line(self, line):
        tokens = line.split(":")

        # invalid line
        if len(tokens) != 3:
            print("Invalid Line!")
            return None

        uc_ticks, command, value = tokens

        # wrong command
        if command != self.fuel_token:
            print("Unknown command")
            return None

        return uc_ticks, float(value)

    def handle_value(self, value):
        # too low!
        if value < self.min_value:
            print("Value below threshold!")
            return

        # too high
        if value > self.max_value:
            print("Value above threshold!")
            return

        scale = self.max_value - self.min_value
        rel_value = value - self.min_value
        ratio = rel_value / scale
        liquid_level = ratio * self.tank.depth
        cubic_meters = self.tank.get_liquid_volume(liquid_level)
        gallons = self.tank.litres_to_gallons(cubic_meters)

        print("raw:%f   %.1f cm   %.1f litres   %.1f gallons" % (value,
                liquid_level, cubic_meters * 1000.0, gallons))

    def get_lines(self, delay):
        """Generator of serial lines."""
        while True:
            self.port.write("f")
            data = ""
            while self.port.inWaiting():
                byte = self.port.read()
                if byte == '\n':
                    tokens = self.parse_line(data)
                    if tokens:
                        yield tokens
                    break
                else:
                    data += byte
            time.sleep(delay)

    def calibrate(self):
        print("Manually move the sending unit from the lowest to the highest position")
        print("multiple times. Use the min and max in your configuration file.")
        print("")
        self.open_port()

        min = 1024.0
        max = 0.0

        for ticks, value in self.get_lines(0.5):
            if value > max:
                max = value
            if value < min:
                min = value
            status = "\r    Min: %.2f  Max: %.2f   (Hit ^C to stop)" % (min, max)
            sys.stdout.write(status)
            sys.stdout.flush()

    def run(self):
        self.open_port()

        for ticks, value in self.get_lines(1.0):
            self.handle_value(value)

    def shutdown(self):
        self.port.close()
