# MPLAB IDE generated this makefile for use with GNU make.
# Project: RAIL_DRIVER_FW_V1.2.mcp
# Date: Tue Jan 30 23:10:08 2024

AS = MPASMWIN.exe
CC = mcc18.exe
LD = mplink.exe
AR = mplib.exe
RM = rm

RAIL_DRIVER_FW_V1.2.cof : ECANPoll.o main.o
	$(LD) /p18F4585 /l"C:\Program Files (x86)\Microchip\mplabc18\v3.47\lib" "18f4585_g.lkr" "ECANPoll.o" "main.o" /u_CRUNTIME /z__MPLAB_BUILD=1 /m"RAIL_DRIVER_FW_V1.2.map" /w /o"RAIL_DRIVER_FW_V1.2.cof"

ECANPoll.o : ECANPoll.c ecanpoll.h ECANPoll.c ecanpoll.def ../../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/p18cxxx.h ../../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/p18f4585.h
	$(CC) -p=18F4585 "ECANPoll.c" -fo="ECANPoll.o" --verbose -w3 -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

main.o : main.c ../../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/stdio.h ../../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/stdlib.h ECANPoll.h main.h main.c ../../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/p18cxxx.h ../../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/p18f4585.h ../../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/stdarg.h ../../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/stddef.h ../../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/errno.h ecanpoll.def
	$(CC) -p=18F4585 "main.c" -fo="main.o" -w1

clean : 
	$(RM) "ECANPoll.o" "main.o" "RAIL_DRIVER_FW_V1.2.cof" "RAIL_DRIVER_FW_V1.2.hex" "RAIL_DRIVER_FW_V1.2.map"

