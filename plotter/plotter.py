#!/usr/bin/python

from __future__ import with_statement
import time
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.ticker as ticker

LITRE_GALLON_FACTOR = 0.264172052
input_filename = "../curfuel.txt"
sample_size = 10000
now = int(time.time())

def extract_record_from_line(line):
    tokens = line.split(":")
    timestamp = int(float(tokens[1]))
    gallons = float(tokens[-1])
    return timestamp, gallons

def get_record_lines():
    with open(input_filename, "r") as fp:
        for line in fp:
            if line[0] == "r":
                yield line

def get_sample_count():
    """From the assumptions that we are going to generate all the missing
    seconds from the first valid record, we just need to find the number of
    seconds between the first record and now to know how many records we have.
    """
    for line in get_record_lines():
        timestamp, gallons = extract_record_from_line(line)
        break
    return int(now) - timestamp

def read_entries():
    """Read the file and generate the missing seconds."""
    last_timestamp = 0
    for line in get_record_lines():
        timestamp, gallons = extract_record_from_line(line)

        # Keep track of the last timestamp so we can fill the gaps
        if last_timestamp == 0:
            last_timestamp = timestamp
        for i in range(last_timestamp, timestamp):
            yield i, gallons
        last_timestamp = timestamp

    # Now generate all the seconds between the last entry and now
    for i in range(last_timestamp, now):
        yield i, gallons


class Sampler:
    def __init__(self, tag, date_format, base_timestamp, total, hours):
        self.tag = tag
        self.base_timestamp = base_timestamp
        self.total_seconds = hours * 3600
        self.min_timestamp = now - self.total_seconds
        self.records = []
        self.date_format = date_format
        # Sampling is basically how many record to skip between each keepers
        self.sampling = int(self.total_seconds / sample_size)
        self.next_sampled = None
        if self.sampling < 1:
            self.sampling = 1

    def append(self, timestamp, gallons):
        """Decide wether we are keeping a record or not."""
        if timestamp > self.min_timestamp:
            if not self.next_sampled:
                self.next_sampled = timestamp

            if timestamp == self.next_sampled:
                self.records.append((timestamp, gallons))
                self.next_sampled += self.sampling

    def __len__(self):
        return len(self.records)

    def save_chart(self):
        min_time = self.records[0][0] - 1
        max_time = now + 1

        # Stupid boundaries for fill to not look gayzor
        self.records.insert(0, (min_time, 0.0))
        self.records.append((max_time, 0.0))

        levels = [r[1] for r in self.records]
        stamps = [r[0] for r in self.records]
        N = len(levels)

        date_format = self.date_format
        def ftime(t, pos=None):
            return time.strftime(date_format, time.localtime(float(t)))

        fig = plt.figure()
        ax = fig.add_subplot(111)
        ax.fill(stamps, levels, color="#D0B76D", linewidth=2,
                edgecolor="#AD985A")
        ax.set_ylim(0, 275)
        ax.set_xlim(self.min_timestamp, max_time)
        ax.set_ylabel("Gallons", fontsize=11)
        ax.yaxis.set_ticks(range(50, 275, 50))
        ax.tick_params(length=2)


        max_litres = int(275 / LITRE_GALLON_FACTOR)
        ax2 = ax.twinx()
        ax2.set_ylim(0, max_litres)
        ax.set_xlim(self.min_timestamp, max_time)
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

total = get_sample_count()
samplers = (
    Sampler("yearly", "%b %Y", now, total, 365 * 24),
    Sampler("monthly", "%a %d", now, total, 30 * 24),
    Sampler("weekly", "%a %d",  now, total, 7 * 24),
    Sampler("daily", "%H:%M", now, total, 1 * 24),
    Sampler("hourly", "%H:%M", now, total, 1),
)

for timestamp, gallons in read_entries():
    for sampler in samplers:
        sampler.append(timestamp, gallons)

for sampler in samplers:
    sampler.save_chart()
