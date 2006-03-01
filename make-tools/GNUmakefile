CXX = g++ -O3 -mno-cygwin -Wl,-s
CC = gcc -O3 -mno-cygwin -Wl,-s

mju-sed.exe: mju-sed.cpp
	$(CXX) -o $@ mju-sed.cpp 

lemon.exe: lemon.c
	$(CC) -o $@ lemon.c


clean:
	-$(RM) mju-sed.exe
	-$(RM) lemon.exe
