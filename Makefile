

RnR-Utility: hid.o RnR.o
	gcc -g -o RnR-Utility hid.o RnR.o -ludev

hid.o: linux/hid.c
	gcc -g -c linux/hid.c -o hid.o -I .
	
RnR.o: RnR.c
	gcc -g -c RnR.c -o RnR.o -I .
