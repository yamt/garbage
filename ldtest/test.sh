cc -c -fno-common lib1.c lib2.c user.c
ar rcs test.a lib1.o lib2.o
ld -r -o test2.a user.o test.a
nm test2.a
