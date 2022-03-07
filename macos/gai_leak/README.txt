leak sanitizer from homebrew-installed version of
llvm 12 (and 13) claims memory leaks in getaddrinfo
when querying about certain host names.
(see log.txt)

to me, it seems host names which don't have any
ipv6 addresses are affected. but i'm not sure about it.
