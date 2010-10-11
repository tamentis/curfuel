from distutils.core import setup, Extension

module1 = Extension("recload",
        sources = ["recload.c", "fgetln.c", "memrchr.c"])

setup (name = "plotter",
       version = "0.1.1",
       description = "Chart module for curfuel.",
       ext_modules = [module1])

