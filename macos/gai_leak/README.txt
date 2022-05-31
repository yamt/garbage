leak sanitizer from homebrew-installed version of
llvm 12 (and 13) claims memory leaks in getaddrinfo
when querying about certain host names.
(see log.txt)

Mojave didn't have the issue.
Monterey has the issue.

"nw_hash_table_create" leak:
    to me, it seems host names which don't have any
    ipv6 addresses are affected. but i'm not sure about it.

https://feedbackassistant.apple.com/feedback/9947074

https://stackoverflow.com/questions/71747252/c-clang-addresssanitizer-unable-to-getting-work-on-mac
