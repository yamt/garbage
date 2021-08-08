# DuMang DK6 keyboard sync program

## What's this

* A program to sync the layer among [DuMang DK6](http://www.beyondq.com/%E8%B6%85%E9%85%B7%E7%A7%91%E6%8A%80-%E4%BA%A7%E5%93%81-%E6%AF%92%E8%9F%92%E9%94%AE%E7%9B%98-%E6%A8%A1%E5%9D%97%E5%8C%96%E9%94%AE%E7%9B%98-dk6-dumang.html) keyboards.

* Inspired from
  [dumang-sync tool](https://github.com/mayanez/dumang-keyboard-ctrl#sync-tool).

  The differences from dumang-sync tool include:

  * Written in C.

  * Has a workaround for macOS suspend/resume.
    (This was a main motivation to write this tool. It seemed
    more straightforward to implement in C than Python.)

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

Maybe it makes more sense to run it with launchd.
