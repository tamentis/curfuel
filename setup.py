#!/usr/bin/python

from distutils.core import setup

from curfuel import __version__

setup(
    name="curfuel",
    version=__version__,
    description="curfuel monitoring daemon",
    author="Bertrand Janin",
    author_email="tamentis@neopulsar.org",
    url="http://tamentis.com/projects/curfuel/",
    scripts=["curfueld.py"],
    classifiers=[
        "Development Status :: 4 - Beta",
        "Environment :: Console",
        "Intended Audience :: End Users/Desktop",
        "License :: OSI Approved :: ISC License (ISCL)",
        "License :: OSI Approved :: GNU Library or Lesser General Public License (LGPL)",
        "Operating System :: MacOS :: MacOS X",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX",
        "Programming Language :: Python :: 2.5",
        "Programming Language :: Python :: 2.6",
        "Topic :: Home Automation",
        "Topic :: System :: Monitoring",
    ]
)
