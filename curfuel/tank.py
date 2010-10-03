#!/usr/bin/python
import math

LITRE_GALLON_FACTOR = 0.264172052

class Tank(object):
    def __init__(self, orientation, depth, length, width):
        self.orientation = orientation
        self.depth = depth
        self.length = length
        self.width = width

    def get_volume(self):
        if self.orientation == "horizontal":
            return self._get_volume_horizontal()
        else:
            return self._get_volume_vertical()

    def litres_to_gallons(self, litres):
        return litres * 1000.0 * LITRE_GALLON_FACTOR

    def _get_volume_horizontal(self):
        """Depth is from top to bottom, length is the longest value and
        width is the measurement between the two rounded sides. Please
        provide the values in meters.
        """
        # This is the width of the middle flat section
        rect_width = self.width - self.depth

        # Volume of the middle section
        v_rect = rect_width * self.depth * self.length

        # Volume of the two half-cylinders combined
        r = (self.depth / 2.0)
        v_cyl = math.pi * (r ** 2) * self.length

        # Total volume cubic meters
        return v_rect + v_cyl

    def get_liquid_volume(self, liquid):
        """Calculate the volume in a horizontal tank given X meters of
        liquid at the bottom.
        """
        if self.orientation == "horizontal":
            return self._get_liquid_volume_horizontal(liquid)
        else:
            return self._get_liquid_volume_vertical(liquid)

    def _get_liquid_volume_horizontal(self, liquid):
        full = self.get_volume()

        # radius of the tank
        r = self.depth / 2.0

        # exactly half
        if liquid == r:
            return full / 2.0

        # overflow or close
        if liquid > self.depth:
            return full

        # Less or more than half (b == how far from center)
        if liquid < r:
            b = r - liquid
        else:
            b = liquid - r

        # angle and volume of the disk section
        angle = math.acos(b / r) * 2.0
        vol_section = ((r ** 2 * angle) / 2.0 * self.length)

        # half/length of the liquid line
        a = math.sqrt(r ** 2 - b ** 2)

        # volume of the triangle
        vol_triangle = b * a * self.length

        # This is the width of the middle flat section
        rect_width = self.width - self.depth

        # less than half
        if liquid < r:
            # volume of the middle section
            vol_middle = liquid * rect_width * self.length
            total_volume = vol_section - vol_triangle + vol_middle
        else:
            # volume of the middle section
            vol_middle = (self.depth - liquid) * rect_width * self.length
            empty_volume = vol_section - vol_triangle + vol_middle
            total_volume = full - empty_volume

        return total_volume

    def print_test_range(self):
        for d in range(1, 28):
            m = d * 0.0254
            v = calc_horizontal_liquid_volume(0.68, 1.63, 1.08, m)
            print(v * 1000.0 * LITRE_GALLON_FACTOR)

