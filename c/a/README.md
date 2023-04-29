
n1570.pdf 6.9.2 External object definitions
If the declaration of an identifier for an object is a tentative definition and has internal linkage, the declared type shall not be an incomplete type.

```
f7ae8b2751fc# gcc --version
gcc (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0
Copyright (C) 2019 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

f7ae8b2751fc# gcc -c -Wall -pedantic a.c
a.c:2:12: error: array size missing in 'a'
    2 | static int a[];
      |            ^
a.c:6:3: error: array index in initializer exceeds array bounds
    6 |  [1] = 2,
      |   ^
a.c:6:3: note: (near initialization for 'a')
a.c:6:8: warning: excess elements in array initializer
    6 |  [1] = 2,
      |        ^
a.c:6:8: note: (near initialization for 'a')
a.c:7:3: error: array index in initializer exceeds array bounds
    7 |  [3] = 2,
      |   ^
a.c:7:3: note: (near initialization for 'a')
a.c:7:8: warning: excess elements in array initializer
    7 |  [3] = 2,
      |        ^
a.c:7:8: note: (near initialization for 'a')
f7ae8b2751fc# 
```

```
f7ae8b2751fc# cc --version
Ubuntu clang version 11.0.0-2~ubuntu20.04.1
Target: x86_64-pc-linux-gnu
Thread model: posix
InstalledDir: /usr/bin
f7ae8b2751fc# cc -c -Wall -pedantic a.c 
f7ae8b2751fc# 
```
