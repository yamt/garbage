# DuMang DK6 keyboard sync program

## What's this

* Inspired from
  [dumang-keyboard-ctrl](https://github.com/mayanez/dumang-keyboard-ctrl).

* Sync the layer among boards.

* Right now, two boards are assumed.
  It should be trivial to extend it for more boards though.

* Written for macOS.
  But it should be trivial to port to a platform where
  [libusb](https://github.com/libusb/libusb) and
  [hidapi](https://github.com/libusb/hidapi) work.
  (Except power.c, which is very macOS specific.)

## How to build and run

```
brew install libusb hidapi
make
./dksync
```
