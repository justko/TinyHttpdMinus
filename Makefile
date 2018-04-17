.SUFFIXES:.out .cpp
.PHONY:all clean

MicroHttpdClient.out:MicroHttpdClient.cpp
	g++ MicroHttpdClient.cpp -o MicroHttpdClient.out

MicroHttpdServer.out:MicroHttpdServer.cpp MicroHttpd.cpp MicroHttpd.h
	g++ MicroHttpdServer.cpp MicroHttpd.cpp -I. -o MicroHttpdServer.out
all:MicroHttpdClient.out MicroHttpdServer.out

clean:
	rm -f *.out