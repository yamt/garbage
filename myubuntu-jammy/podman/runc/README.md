
## create a rootfs archive

```
3acffc61e039# ./create_rootfs_archive.sh centos centos.tar
+ REF=centos
+ TARFILE=centos.tar
+ podman image pull centos
Resolved "centos" as an alias (/etc/containers/registries.conf.d/000-shortnames.conf)
Trying to pull quay.io/centos/centos:latest...
Getting image source signatures
Copying blob 7a0437f04f83 [---------------------------------] 543.0KiB / 71.7MiB
Copying blob 7a0437f04f83 [>----------------------------------] 1.7MiB / 71.7MiB
:
: snipped
:
Copying blob 7a0437f04f83 [==================================] 71.6MiB / 71.7MiB
Copying blob 7a0437f04f83 [==================================] 71.7MiB / 71.7MiB
Copying blob 7a0437f04f83 done  
Copying blob 7a0437f04f83 done  
Copying config 300e315adb [====================================] 2.1KiB / 2.1KiB
Copying config 300e315adb done  
Copying config 300e315adb done  
Writing manifest to image destination
Storing signatures
+ IMG_ID=300e315adb2f96afe5f0b2780b87f28ae95231fe3bdd1e16b9ba606307728f55
+ podman image mount 300e315adb2f96afe5f0b2780b87f28ae95231fe3bdd1e16b9ba6063077
28f55
+ MNT=/var/lib/containers/storage/vfs/dir/2653d992f4ef2bfd27f94db643815aa567240c37732cae1405ad1c1309ee9859
+ tar -c -f centos.tar -C /var/lib/containers/storage/vfs/dir/2653d992f4ef2bfd27f94db643815aa567240c37732cae1405ad1c1309ee9859 .
+ podman image umount 300e315adb2f96afe5f0b2780b87f28ae95231fe3bdd1e16b9ba606307728f55
300e315adb2f96afe5f0b2780b87f28ae95231fe3bdd1e16b9ba606307728f55
3acffc61e039# ls -l
total 213744
-rw-r--r-- 1 root root 216555520 Jul 14 03:13 centos.tar
-rwxr-xr-x 1 root root       257 Jul 14 01:35 create_rootfs_archive.sh
-rwxr-xr-x 1 root root       470 Jul 14 01:50 runc_with_rootfs_archive.sh
3acffc61e039# 
```

## create a container with the rootfs archive

```
3acffc61e039# ./runc_with_rootfs_archive.sh c centos.tar
+ NAME=c
+ TARFILE=centos.tar
+ test -f centos.tar
+ RUNC=crun
+ mktemp -d /tmp/c.XXXXXXX
+ TMPDIR=/tmp/c.VNpX4VH
+ ROOTFS=/tmp/c.VNpX4VH/rootfs
+ mkdir -p /tmp/c.VNpX4VH/rootfs
+ tar -x -C /tmp/c.VNpX4VH/rootfs -f centos.tar
+ cd /tmp/c.VNpX4VH
+ crun spec
+ crun run c
sh-4.4# ps -ax
  PID TTY      STAT   TIME COMMAND
    1 pts/0    Ss     0:00 sh
    2 pts/0    R+     0:00 ps -ax
sh-4.4# cat - this is in a container
[1] 3
sh-4.4# ps -axww
  PID TTY      STAT   TIME COMMAND
    1 pts/0    Ss     0:00 sh
    3 pts/0    T      0:00 /usr/bin/coreutils --coreutils-prog-shebang=cat /usr/bin/cat - this is in a container
    6 pts/0    R+     0:00 ps -axww

[1]+  Stopped(SIGTTIN)        cat - this is in a container
sh-4.4# ps -O ppid,rss -ax
  PID  PPID   RSS S TTY          TIME COMMAND
    1     0  3224 S pts/0    00:00:00 sh
    3     1  1400 T pts/0    00:00:00 cat
    7     1  2052 R pts/0    00:00:00 ps
sh-4.4# 
```

### observe the container from a separate shell

```
3acffc61e039# ps -axww
  PID TTY      STAT   TIME COMMAND
    1 pts/0    Ss     0:00 /usr/bin/zsh
 4646 pts/0    S+     0:00 /bin/sh ./runc_with_rootfs_archive.sh c centos.tar
 4651 pts/0    S+     0:00 crun run c
 4652 pts/0    Ss+    0:00 sh
 4654 pts/0    T      0:00 /usr/bin/coreutils --coreutils-prog-shebang=cat /usr/bin/cat - this is in a container
 4659 pts/1    Ss     0:00 zsh
 4668 pts/1    R+     0:00 ps -axww
3acffc61e039# ps -O ppid,rss -ax
  PID  PPID   RSS S TTY          TIME COMMAND
    1     0  6048 S pts/0    00:00:00 zsh
 4646     1   536 S pts/0    00:00:00 runc_with_rootf
 4651  4646  3108 S pts/0    00:00:00 3
 4652  4651  3224 S pts/0    00:00:00 sh
 4654  4652  1400 T pts/0    00:00:00 cat
 4659     0  5172 S pts/1    00:00:00 zsh
 4669  4659  1148 R pts/1    00:00:00 ps
3acffc61e039# 
```
