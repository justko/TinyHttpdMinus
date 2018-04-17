.SUFFIXES:.out .cpp
.PHONY:all clean

MicroClient.out:MicroClient_main.cpp MicroClient.cpp MicroClient.h
	g++ MicroClient_main.cpp MicroClient.cpp -o MicroClient.out

MicroHttpd.out:MicroHttpd_main.cpp MicroHttpd.cpp MicroHttpd.h
	g++ MicroHttpd_main.cpp MicroHttpd.cpp -I. -o MicroHttpd.out
all:MicroHttpd.out MicroClient.out

clean:
	rm -f *.out