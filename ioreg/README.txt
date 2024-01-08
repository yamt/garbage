Usage examples:

./find_serial.sh AppleUSBFTDI 1
./find_serial.sh com_silabs_driver_CP210xVCPDriver 1

# ESP32 devkit-c
./find_serial.sh com_silabs_driver_CP210xVCPDriver 0

# sipeed m1s

## D0
```
spacetanuki% ./find_serial.sh AppleUSBFTDI 0
/dev/tty.usbserial-SI88480
spacetanuki% pyserial-miniterm /dev/tty.usbserial-SI88480 2000000
```

## M0
```
spacetanuki% ./find_serial.sh AppleUSBFTDI 1
/dev/tty.usbserial-SI88481
spacetanuki% pyserial-miniterm /dev/tty.usbserial-SI88481 2000000
```
