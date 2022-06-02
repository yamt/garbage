# Expected usage:
#   % ioreg -a -c xxx -r -l | python3 find_serial.py
import sys
import plistlib

name = sys.argv[1]
device_b_interface_number = int(sys.argv[2])


def visit(o):
    for c in o.get("IORegistryEntryChildren", []):
        if (
            o.get("IORegistryEntryName") == name
            and o.get("bInterfaceNumber") == device_b_interface_number
        ):
            if c.get("IOObjectClass") == "IOSerialBSDClient":
                print(c["IODialinDevice"])
            elif c.get("IOObjectClass") == "IORS232SerialStreamSync":
                print(c["IORegistryEntryChildren"][0]["IODialinDevice"])
            else:
                visit(c)
        else:
            visit(c)


list = plistlib.load(sys.stdin.buffer, fmt=plistlib.FMT_XML)
for c in list:
    visit(c)
