"""Scons Script Execution."""

from subprocess import call
import sys
import os

Import("env")  # noqa
env = env  # noqa

try:
    script = os.path.join(os.getcwd(), "scripts/auto_header.py")
    print("Generating config.h")
    call("{} -o gen/config.h .env".format(script), shell=True)
except Exception as e:
    print(e)
    sys.exit(1)
