ANAGRAMS_C_FLAGS=-c -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes -Wswitch -Wwrite-strings

anagrams.exe: anagrams.o
	gcc -o anagrams.exe anagrams.o

anagrams.o: anagrams.c anagrams.make
	gcc ${ANAGRAMS_C_FLAGS} -o anagrams.o anagrams.c
