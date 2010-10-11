#!/usr/bin/python

from __future__ import with_statement
import sys
import time
import ConfigParser
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.ticker as ticker
import recload

LITRE_GALLON_FACTOR = 0.264172052
now = int(time.time())

class Sampler:
    def __init__(self, filename, tag, date_format, base_timestamp, hours):
        self.filename = filename
        self.tag = tag
        self.base_timestamp = base_timestamp
        self.total_seconds = hours * 3600
        self.min_timestamp = now - self.total_seconds
        self.records = ()
        self.date_format = date_format

        # Sampling is basically how many record to skip between each keepers
        sample_size = 10000
        self.sampling = int(self.total_seconds / sample_size)
        self.next_sampled = None
        if self.sampling < 1:
            self.sampling = 1

    def save_chart(self):
        record_set = recload.read(self.filename, self.min_timestamp)

        stamps = record_set[0]
        levels = record_set[1]

        date_format = self.date_format
        def ftime(t, pos=None):
            return time.strftime(date_format, time.localtime(float(t)))

        fig = plt.figure()
        ax = fig.add_subplot(111)
        ax.fill_between(stamps, levels, color="#D0B76D", linewidth=2,
                edgecolor="#AD985A")
        ax.set_ylim(0, 275)
        ax.set_xlim(self.min_timestamp, now)
        ax.set_ylabel("Gallons", fontsize=11)
        ax.yaxis.set_ticks(range(50, 275, 50))
        ax.tick_params(length=2)


        max_litres = int(275 / LITRE_GALLON_FACTOR)
        ax2 = ax.twinx()
        ax2.set_ylim(0, max_litres)
        ax.set_xlim(self.min_timestamp, now)
        ax2.set_ylabel("Litres", fontsize=11)
        ax2.xaxis.set_major_formatter(ticker.FuncFormatter(ftime))
        ax2.yaxis.set_ticks(range(200, max_litres, 200))
        ax2.tick_params(length=0)

        ax.patch.set_alpha(0.5)
        ax2.patch.set_alpha(0.5)


        all_labels = ax.get_xticklabels() + ax.get_yticklabels() + \
                ax2.get_yticklabels()
        for tl in all_labels:
            tl.set_fontsize(9)

        ax.set_axisbelow(True)
        ax.grid(True, color="#aaaaaa", linestyle=":")

        fig.set_dpi(100)
        fig.set_size_inches(8.00, 2.00)

        fig.subplots_adjust(bottom = 0.15, top=0.96, right=0.92, left=0.08)

        fig.savefig("chart_%s.png" % self.tag, format="png")


class Plotter:
    def __init__(self, config_filename, data_filename):
        self.samplers = []
        self.filename = data_filename
        self.load_config(config_filename)

    def load_config(self, filename):
        cp = ConfigParser.ConfigParser()
        cp.read(filename)

        configs = {}
        for name, value in cp.items("samplers"):
            tag, var = name.split(".")
            if tag not in configs:
                configs[tag] = {}
            configs[tag][var] = value

        for tag in configs:
            hours = int(configs[tag]["hours"])
            sampler = Sampler(self.filename, tag, configs[tag]["format"], now, hours)
            self.samplers.append(sampler)

    def run(self):
        for sampler in self.samplers:
            sampler.save_chart()

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("usage: %s config.ini datafile" % sys.argv[0])
        sys.exit(-1)

    plotter = Plotter(sys.argv[1], sys.argv[2])
    plotter.run()
