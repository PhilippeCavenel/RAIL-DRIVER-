/*********************************************************************
*
* DRIVER RAIL V 1.3
*
* MAIN.H
*
*********************************************************************
* Processor: PIC18F4680
* Frequency: 32 Mhz
* Compiler: C18
* TYPE                SIZE     RANGE
* char(1,2)            8 bits  -128 127
* signed char          8 bits  -128 127
* unsigned char        8 bits  0 255
* int                 16 bits  -32,768 32,767
* unsigned int        16 bits  0 65,535
* short               16 bits  -32,768 32,767
* unsigned short      16 bits  0 65,535
* short long          24 bits  -8,388,608 8,388,607
* unsigned short long 24 bits  0 16,777,215
* long                32 bits  -2,147,483,648 2,147,483,647
* unsigned long       32 bits  0 4,294,967,295
*********************************************************************/


#include <p18cxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "ECANPoll.h"

// TRUE FALSE definition as bool not recognized
#define TRUE							1
#define FALSE							0
                         
rom char RAIL_DRIVER_HEADER_STRING[] 	= "RAIL DRIVER V 1.39g\0";
rom char VERSION_STRING[] 				= "VERSION 01-10-2025\0";
rom char BOARD_NUMBER_STRING[] 			= "BOARD \0";
rom char MODE_STRING[] 					= "MODE \0";
rom char MEMORY_STRING[] 				= "MEMORY \0";
rom char AUTOMATION_STRING[] 			= "AUTOMATION MODE \0";
rom char AUTOMATION_NUMBER_STRING[] 	= "NUMBER OF AUTOMATION \0";

rom char START_STRING[] 				= "START\0";
rom char RESET_STRING[] 				= "RESET\0";
rom char STOP_STRING[] 					= " STOP \0";
rom char RUN_STRING[] 					= " RUN \0";
rom char SYNC_STRING[] 					= " SYNC \0";
rom char DONE_STRING[] 					= " DONE \0";
rom char ONTRACK_STRING[]				= "ONTRACK\0";
rom char OFFTRACK_STRING[]				= "OFFTRACK\0";
rom char BOARD_PROMPT_STRING[]			= "\n\rBoard \0";
rom char SPEED_STRING[]					= "SP \0";
rom char INERTIA_STRING[]				= "IN \0";
rom char INIT_STRING[]					= "INIT \0";
rom char VOLT_STRING[]					= "ERROR\0";

// PROTOCOL
#define MAXSIZETOKEN					10
#define MAXSIZEIDENT					3
#define MAXERRORINFO					60
#define MAXTRAMESIZE					80 // Should be multiple of 8
#define MAXTRAMECAN						MAXTRAMESIZE-16 
#define MAXMESSAGESIZE					MAXTRAMECAN-2 // We need to add lastchar and end of string

#define TRAMESYNCTRACE					0xAA
#define TRAMEPRINTHEADER				0xCC
#define TRAMEPRINTFOOTER				0xDD

#define TRAMEREQUESTHEADER				0xEE
#define TRAMEREQUESTFOOTER				0xFF

#define ENDOFPRINTFTRAME				0x00

// AUTOMATION
#define MAXTIMER						15
#define MAXTIMERDELAY					255
#define MAXSPEEDVALUE					15
#define MAXAUTOMATION					70

// ACCELARATION RATE
#define MAX_STEP						50

// INTERNAL SPEED SETTING
#define MAXINTERNALSPEED				2000 // speed x 2000 is in short range ( -15 < speed < 15)

#define INITTIMERVALUE					0xFFF

//EEPROM
#define SMALL_BUFFER_SIZE				80
#define SUCCESS							1
#define ERROR							0
#define IN_PROGRESS	  					-1
#define MATCH							1
#define DISMATCH	    				2

// Don't change this order EEPROM ADDRESS SETTING !
#define MAGICNUMBER_ADDRESS				0
#define MAGICNUMBERSIZE					8
#define MODE_ADDRESS					MAGICNUMBERSIZE
#define KNOB_ADDRESS					(MODE_ADDRESS+1)
#define KNOB_SIZE						8	
#define NEXTTAUTOMATION_ADDRESS			(KNOB_ADDRESS+KNOB_SIZE)		
#define GPIO0DIR_ADDRESS				(NEXTTAUTOMATION_ADDRESS+1) 
#define GPIO1DIR_ADDRESS				(GPIO0DIR_ADDRESS+1) 	
#define GPIO2DIR_ADDRESS				(GPIO1DIR_ADDRESS+1) 		
#define GPIO3DIR_ADDRESS				(GPIO2DIR_ADDRESS+1) 

#define TRACK_CAN_NOTIFICATION_ADDRESS  (GPIO3DIR_ADDRESS+1)
#define CAN_NOTIFICATION_ADDRESS		TRACK_CAN_NOTIFICATION_ADDRESS
#define TRACK_SIZE						4

#define GPIO_CAN_NOTIFICATION_ADDRESS  (TRACK_CAN_NOTIFICATION_ADDRESS+TRACK_SIZE)
#define GPIO_SIZE						4

#define TIMER_CAN_NOTIFICATION_ADDRESS  (GPIO_CAN_NOTIFICATION_ADDRESS+GPIO_SIZE)
#define TIMER_SIZE						MAXTIMER
			
#define AUTOMATION_ADDRESS				(TIMER_CAN_NOTIFICATION_ADDRESS+TIMER_SIZE)		// Should be the last of the list
						
rom char  UNKNOWN_TOKEN_STRING[] 		= "Unknown token\0";
rom char  NUMBER_MISSING_STRING[] 		= "Number missing\0";
rom char  INCOMPLETE_REQUEST_STRING[] 	= "Incomplete request\0";
rom char  BAD_NUMBER_STRING[] 			= "Bad number\0";
rom char  MODE_MISSING_STRING[] 		= "Mode missing\0";
rom char  BAD_GPIO_NUMBER_STRING[] 		= "Bad GPIO number\0";
rom char  BAD_TIMER_NUMBER_STRING[] 	= "Bad TIMER number\0";
rom char  BAD_TIMER_VALUE_STRING[] 		= "Bad TIMER value\0";
rom char  BAD_LPO_NUMBER_STRING[] 		= "Bad LPO number\0";
rom char  BAD_AUT_STATUS_STRING[] 		= "Bad Automation status\0";
rom char  BAD_AUT_IDENT_STRING[] 		= "Bad Automation name\0";
rom char  MISSING_IDENT_STRING[] 		= "Automation name missing\0";
rom char  BAD_GPIO_DIR_STRING[] 		= "Bad GPIO direction\0";
rom char  BAD_GPIO_LEVEL_STRING[] 		= "Bad GPIO level\0";
rom char  BAD_LPO_LEVEL_STRING[] 		= "Bad LPO level\0";
rom char  BAD_TRACK_SPEED_STRING[] 		= "Bad track speed\0";
rom char  BAD_TRACK_DIR_STRING[] 		= "Bad track direction\0";
rom char  BAD_TRACK_NUMBER_STRING[] 	= "Bad track number\0";
rom char  BAD_BOARD_MODE_STRING[] 		= "Bad board mode\0";
rom char  WRONG_BOARD_NUMBER_STRING[]	= "Wrong board number\0";
rom char  AUTOMATIONSIZELIMIT_STRING[] 	= "Automation limit reached\0";
rom char  MISSING_SPACE_STRING[] 		= "Space missing\0";
rom char  IDENT_TOO_LONG_STRING[] 		= "Identifier too long\0";
rom char  AUTOMATIONLREADYEXISTS_STRING[] = "Automation already defined\0";
rom char  BADAUTOMATIONNUMBER_STRING[] 	= "Wrong automation number\0";
rom char  BAD_TRACK_INERTIA_STRING[] 	= "Bad inertia value\0";
rom char  BAD_USER_MODE_STRING[] 		= "Bad user mode\0";
rom char  UNKNOWN_ERROR_STRING[] 		= "Unknown Error\0";
rom char  BAD_CANPUSH_NUMBER_STRING[]	= "Bad CAN event value\0";

// ERROR CODE
#define UNKNOWN_TOKEN					0x1
#define NUMBER_MISSING					0x2
#define INCOMPLETE_REQUEST				0x3
#define BAD_NUMBER						0x4
#define	MODE_MISSING					0x5
#define BAD_GPIO_NUMBER					0x6
#define BAD_GPIO_DIR					0x7
#define BAD_USER_MODE					0x8
#define BAD_GPIO_LEVEL					0x9
#define	BAD_LPO_LEVEL					0xA
#define	BAD_LPO_NUMBER					0xB
#define BAD_TRACK_SPEED					0xC
#define BAD_TRACK_DIR					0xD
#define BAD_TRACK_NUMBER				0xE
#define BAD_BOARD_MODE					0xF
#define WRONG_BOARD_NUMBER				0x10
#define AUTOMATIONSIZELIMIT				0x11
#define MISSING_SPACE					0x12
#define IDENT_TOO_LONG					0x13
#define AUTOMATIONLREADYEXISTS			0x14
#define BADAUTOMATIONNUMBER				0x15
#define BAD_TIMER_NUMBER				0x16
#define BAD_TIMER_VALUE					0x17
#define BAD_TRACK_INERTIA				0x18
#define BAD_AUT_STATUS					0x19
#define BAD_AUT_IDENT					0x1A
#define MISSING_IDENT					0x1B
#define BAD_CANPUSH_NUMBER				0x1C

#define UNKNOWN_ERROR					0xF0

// MSSING ERROR CODE
#define EINVAL							22

// Bits de Configuration 
#pragma config OSC 		= 				IRCIO67 // Oscillateur Interne
#pragma config FCMEN 	= 				OFF
#pragma config IESO 	= 				OFF
#pragma config PWRT 	= 				ON
#pragma config BOREN 	= 				BOHW
#pragma config BORV 	= 				3
#pragma config WDT 		= 				OFF  
#pragma config STVREN   =				OFF
#pragma config PBADEN 	= 				OFF
#pragma config MCLRE 	= 				OFF
#pragma config LVP 		= 				OFF

// Voltage issue management
// HLVDL3:HLVDL0	Trip point (˜ VDD)
// 0000	2.12 V – 2.22 V
// 0001	2.18 V – 2.28 V
// 0010	2.31 V – 2.42 V
// 0011	2.38 V – 2.49 V
// 0100	2.54 V – 2.66 V
// 0101	2.72 V – 2.85 V
// 0110	2.82 V – 2.95 V  
// 0111	3.05 V – 3.19 V
// 1000	3.31 V – 3.47 V 
// 1001	3.46 V – 3.63 V
// 1010	3.63 V – 3.80 V  
// 1011	3.81 V – 3.99 V
// 1100	4.01 V – 4.20 V <== LOWTH
// 1101	4.23 V – 4.43 V 
// 1110	4.48 V – 4.69 V 
// 1111	External voltage input

#define LOWTH 	0b1100


// CAN
#define CAN_FREE						1
#define CAN_GET_DATA					2
#define CAN_REQUEST						3
#define CAN_PRINT						4
#define CAN_GET_FOOTER					5

#define READY							1
#define WAITING_FOR_DATA				0

#define WAITDELAYTRAMECAN				250
#define SYNCHROSENDDELAY				100
#define MAXINPUTCANBUFFER				4

// SYNCHRO BOARD VIA CAN
#define SYNC_ID 0x0FF


// RS232
#define USARTBUFFERSIZE					127

// VALUES TRACK DIRECTION
#define TRACK_STOP						0	
#define TRACK_FORWARD					1
#define TRACK_BACKWARD					2

// DELAY FOR DCC SIGNAL
#define DCC_0							48
#define DCC_1							18

// DCC PREAMBLE SIZE
#define PREAMBLE_SIZE					21
#define FRAME_SIZE						(PREAMBLE_SIZE+(3*9)+2)
#define MAX_INERTIA_COUNTER				MAXSPEEDVALUE
#define INITWAITDCCCOUNTER				4

// CURRENT DETECTION 
#define HYSTERERISHIGH					3
#define HYSTERERISLOW					1
#define SAMPLEFORAVERAGE				30
#define SAMPLEFORCALIBRATION			5
#define TRACKCALIBRATIONDELAY			2

// GPIO DETECTION
#define GPIOTHRESHOLD					100

// CIRCUIT 0 ////////////////////////////
#define S1T0							PORTAbits.RA4
#define S2T0    						PORTAbits.RA7
#define CURT0       					0x0 + 0x1 // ADCON0 ON AN0 + ADON

// CIRCUIT 1 ////////////////////////////
#define S1T1							PORTAbits.RA6
#define S2T1    						PORTCbits.RC0
#define CURT1       					0x4 + 0x1 // ADCON0 ON AN1 + ADON

// CIRCUIT 2 ////////////////////////////
#define S1T2							PORTCbits.RC1
#define S2T2    						PORTCbits.RC2
#define CURT2       					0x8 + 0x1 // ADCON0 ON AN2 + ADON

// CIRCUIT 3 ////////////////////////////
#define S1T3							PORTCbits.RC3
#define S2T3    						PORTDbits.RD0
#define CURT3       					0x8 + 0x4 + 0x1 // ADCON0 ON AN3 + ADON

// TRACK STATUS /////////////////////////
#define INKNOB0							0x10 + 0x1 // ADCON0 ON AN4 + ADON	
#define INKNOB1							0x10 + 0x4 + 0x1 // ADCON0 ON AN5 + ADON			
						
// OUTPUT ////////////////////////////
#define OUT0        					PORTDbits.RD6
#define OUT1        					PORTDbits.RD7
#define OUT2        					PORTBbits.RB0
#define OUT3        					PORTBbits.RB1
#define OUT4        					PORTBbits.RB4
#define OUT5        					PORTBbits.RB5

// SWITCH IN ////////////////////////////
#define IN0								PORTCbits.RC5
#define IN1								PORTCbits.RC6
#define IN2								PORTCbits.RC7
#define IN3								PORTDbits.RD4
#define IN4								PORTDbits.RD5

// GPIO IN ///////////////////////////
#define GPIO0							PORTDbits.RD1
#define GPIO1							PORTDbits.RD2
#define GPIO2							PORTDbits.RD3
#define GPIO3							PORTCbits.RC4

///////////////////////////////////////////////
// Token
///////////////////////////////////////////////

#define PROG							"PROG"
#define COM								"COM"
#define DCC								"DCC"
#define STOP							"STOP"
#define RUNALL 							"RUNALL"
#define SYNCHRO 						"SYNC"
#define ANA								"ANA"
#define LPO								"LPO"
#define GPIO							"GPIO"
#define TRACK							"TRACK"
#define IN								"IN"
#define ACT								"ACT"
#define STA								"STA"
#define OUT								"OUT"
#define ONTRACK							"ONTRACK"
#define OFFTRACK						"OFFTRACK"
#define DEL								"DEL"
#define FORW							"FORW"
#define BACK							"BACK"
#define GSTAT							"GSTAT"
#define LSTAT							"LSTAT"
#define TSTAT							"TSTAT"
#define BSTAT							"BSTAT"
#define AUTLIST							"AUTLIST"
#define AUT								"AUT"
#define BOARD							"BOARD"
#define RUN								"RUN"
#define RESET							"RESET"
#define TIMER							"TIMER"
#define INERTIA							"INERTIA"
#define SPEED							"SPEED"
#define AUTON							"AUTON"
#define AUTOFF							"AUTOFF"
#define DUMP							"DUMP"
#define KNOB0							"KNOB0"
#define KNOB1							"KNOB1"
#define MANUAL							"MANUAL" 	// All track managed by knobs
#define MANUAL0							"MANUAL0"	// Only track 0 managed by knobs
#define MANUAL1							"MANUAL1"	// Only track 1 managed by knobs
#define MANUAL2							"MANUAL2"	// Only track 2 managed by knobs
#define MANUAL3							"MANUAL3"	// Only track 3 managed by knobs
#define AUTOMATIC						"AUTOMATIC"
#define CALIB							"CALIB"
#define PUSHCAN							"PUSHCAN"
#define ONCAN							"ONCAN"
#define	OFFCAN							"OFFCAN"

///////////////////////////////////////////////
//Token Value
///////////////////////////////////////////////

#define PROGValue 						0x01
#define COMValue 						0x02
#define DCCValue 						0x03
#define STOPValue 						0x04
#define RUNALLValue 					0x05
#define ANAValue 						0x06
#define LPOValue 						0x07
#define GPIOValue 						0x08
#define TRACKValue 						0x09
#define VALValue 						0x0A
#define INValue 						0x0B
#define ACTValue 						0x0C
#define STAValue 						0x0D
#define OUTValue 						0x0E
#define ONTRACKValue 					0x0F
#define OFFTRACKValue 					0x10
#define DELValue 						0x11
#define FORWValue 						0x13
#define BACKValue 						0x14
#define GSTATValue 						0x15
#define TSTATValue						0x16
#define BSTATValue 						0x17
#define LSTATValue 						0x18
#define AUTLISTValue 					0x19
#define AUTValue 						0x1A
#define BOARDValue 						0x1B
#define ERRValue 						0x1C
#define AUTFULLValue 					0x1D
#define NOAUTValue 						0x1E
#define TIMEOUTValue 					0x1F
#define NODCCValue 						0x20
#define NOANAValue						0x21
#define GPIOINValue 					0x22
#define RUNValue 						0x23
#define RESETValue 						0x24
#define TIMERValue						0x25
#define INERTIAValue					0x26
#define SPEEDValue						0x27
#define AUTONValue						0x28
#define AUTOFFValue						0x29
#define DUMPValue						0x2A
#define MANUALValue						0x2B
#define AUTOMATICValue					0x2C
#define CALIBValue						0x2D
#define MANUAL0Value					0x2E
#define MANUAL1Value					0x2F
#define MANUAL2Value					0x30
#define MANUAL3Value					0x31
#define SYNCHROValue 					0x40
#define CANPUSHValue					0x41


// SPECIFIC VALUES

#define KNOB0Value						0xFF	// Value over max value 99
#define KNOB1Value						0xEE	// Value over max value 99


///////////////////////
// STRUCTURE REQUEST
// ////////////////////

// DONT CHANGE THE FOLLOWING VALUES OR ORDERS
/////////////////////////////////////////////

#define REQ_TYPE_ENTRY								0
#define REQ_BOARD_NUMBER 							1
#define REQ_GLOBAL_COMMAND 							2

// DONT CHANGE THE FOLLOWING VALUES OR ORDERS
/////////////////////////////////////////////
#define REQ_COMMAND_REQUEST_SET_GPIO 				3
#define REQ_COMMAND_REQUEST_GPIO_NUMBER 			4
#define REQ_COMMAND_REQUEST_GPIO_LEVEL 				5

#define REQ_COMMAND_REQUEST_SET_TIMER 				6
#define REQ_COMMAND_REQUEST_TIMER_NUMBER 			7
#define REQ_COMMAND_REQUEST_TIMER_DELAY				8

#define REQ_COMMAND_REQUEST_SET_LPO 				9
#define REQ_COMMAND_REQUEST_LPO_NUMBER 				10
#define REQ_COMMAND_REQUEST_LPO_LEVEL 				11

#define REQ_COMMAND_REQUEST_SET_AUT 				12
#define REQ_COMMAND_REQUEST_AUT_IDENT 				13
#define REQ_COMMAND_REQUEST_AUT_STATUS				(14+MAXSIZEIDENT)

#define REQ_COMMAND_REQUEST_SET_TRACK 				(15+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_TRACK_NUMBER 			(16+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_TRACK_SPEED 			(17+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_TRACK_DIR 				(18+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_TRACK_INERTIA			(19+MAXSIZEIDENT)

#define REQ_COMMAND_REQUEST_SET_USER_MODE			(20+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_USER_MODE				(21+MAXSIZEIDENT)

#define REQ_COMMAND_REQUEST_SET_DCC					(22+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_DCC_ADDRESS 			(23+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_DCC_COMMAND				(24+MAXSIZEIDENT)

#define REQ_COMMAND_REQUEST_GET_GPIO_STATUS			(25+MAXSIZEIDENT) 
#define REQ_COMMAND_REQUEST_GET_LPO_STATUS 			(26+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_GET_TRACK_STATUS 		(27+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_GET_BOARD_STATUS 		(28+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_GET_AUTOMATION_LIST 	(29+MAXSIZEIDENT)
#define REQ_COMMAND_REQUEST_GET_DUMP			 	(30+MAXSIZEIDENT)

#define REQ_COMMAND_REQUEST_GET_TIMER_STATUS		(31+MAXSIZEIDENT)	

#define REQ_PROGRAM_REQUEST_SET_BOARD_MODE 			(32+MAXSIZEIDENT)	
#define REQ_PROGRAM_REQUEST_BOARD_MODE 				(33+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_SET_GPIO 				(34+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER 		(35+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_SET_GPIO_DIR 			(36+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_SET_TIMER 				(37+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_SET_TIMER_NUMBER 		(38+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_SET_GPIO_VALUE 			(39+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_SET_AUTOMATION 			(40+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_SET_CAN_PUSH			(41+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER		(42+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_SET_CAN_PUSH_ACTIVE		(43+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_GPIO_EVENT 				(44+MAXSIZEIDENT)

#define START_HERE_TO_COPY_FOR_AUTOMATION			REQ_PROGRAM_REQUEST_GPIO_EVENT

#define REQ_PROGRAM_REQUEST_EVENT_BOARD_GPIO_NUMBER (45+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_GPIO_NUMBER 		(46+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_GPIO_LEVEL 		(47+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_TIMER_EVENT 			(48+MAXSIZEIDENT)	
#define REQ_PROGRAM_REQUEST_EVENT_BOARD_TIMER_NUMBER (49+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_TIMER_NUMBER 		(50+MAXSIZEIDENT)			

#define REQ_PROGRAM_REQUEST_TRACK_EVENT 			(51+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_BOARD_TRACK_NUMBER (52+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_TRACK_NUMBER 		(53+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_VEHICLE_STATUS 	(54+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_GPIO_SETTING 			(55+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_ACTION_GPIO_SET_NUMBER 	(56+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_ACTION_GPIO_SET_LEVEL 	(57+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_TIMER_SETTING 			(58+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_ACTION_TIMER_SET_NUMBER (59+MAXSIZEIDENT)	
#define REQ_PROGRAM_REQUEST_ACTION_TIMER_SET_DELAY	(60+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_LPO_SETTING 			(61+MAXSIZEIDENT)
#define REQ_PROGAM_REQUEST_ACTION_LPO_SET_NUMBER	(62+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_ACTION_LPO_SET_LEVEL 	(63+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_AUT_SETTING 			(64+MAXSIZEIDENT)
#define REQ_PROGAM_REQUEST_ACTION_AUT_SET_IDENT		(65+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_ACTION_AUT_SET_STATUS 	(66+MAXSIZEIDENT*2)

#define REQ_PROGRAM_REQUEST_TRACK_SETTING 			(67+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_NUMBER (68+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_SPEED 	(69+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_DIR 	(70+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_INERTIA (71+MAXSIZEIDENT*2)

#define REQ_PROGRAM_REQUEST_SET_USER_MODE 			(72+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_USER_MODE 		(73+MAXSIZEIDENT*2)

#define REQ_PROGRAM_REQUEST_DCC_SETTING 			(74+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_DCC_ADDRESS_SETTING 	(75+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_DCC_COMMAND_SETTING 	(76+MAXSIZEIDENT*2)

#define REQ_PROGRAM_REQUEST_STATUS					(77+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_STATUS_MANUAL			(78+MAXSIZEIDENT*2)	
#define REQ_PROGRAM_REQUEST_IDENT					(79+MAXSIZEIDENT*2)

#define REQ_PROGRAM_REQUEST_DEL_AUTOMATION 			(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER 		(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+1)

#define REQ_EVENT_REQUEST_TRACK_EVENT 				(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+2)
#define REQ_EVENT_REQUEST_EVENT_BOARD_TRACK_NUMBER 	(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+3)
#define REQ_EVENT_REQUEST_EVENT_TRACK_NUMBER 		(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+4)
#define REQ_EVENT_REQUEST_EVENT_VEHICLE_STATUS 		(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+5)

#define REQ_EVENT_REQUEST_GPIO_EVENT 				(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+6)
#define REQ_EVENT_REQUEST_EVENT_BOARD_GPIO_NUMBER 	(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+7)
#define REQ_EVENT_REQUEST_EVENT_GPIO_NUMBER 		(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+8)
#define REQ_EVENT_REQUEST_EVENT_GPIO_LEVEL 			(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+9)	

#define REQ_EVENT_REQUEST_TIMER_EVENT 				(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+10)
#define REQ_EVENT_REQUEST_EVENT_BOARD_TIMER_NUMBER 	(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+11)
#define REQ_EVENT_REQUEST_EVENT_TIMER_NUMBER 		(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+12)

#define REQ_EVENT_REQUEST_EVENT_CAN_NOTIFICATION	(REQ_PROGRAM_REQUEST_IDENT+MAXSIZEIDENT+13)

#define REQUESTSIZE									(REQ_EVENT_REQUEST_EVENT_CAN_NOTIFICATION+1)

///////////////////////
// STRUCTURE AUTOMATION
// ////////////////////

#define GPIO_EVENT									0
#define TIMER_EVENT									1
#define TRACK_EVENT									2

#define SET_GPIO									0
#define	SET_TIMER									1
#define SET_LPO										2
#define SET_AUT										3
#define SET_TRACK									4
#define SET_USER_MODE								5
#define SET_DCC										6

#define NEW_AUTOMATION_EVENT_TYPE					0
#define NEW_AUTOMATION_EVENT_BOARD_NUMBER			1
#define NEW_AUTOMATION_EVENT_NUMBER					2
#define NEW_AUTOMATION_EVENT_VALUE					3
#define NEW_AUTOMATION_SET_COMMAND					4
#define NEW_AUTOMATION_SET_PARAM_1					5
#define NEW_AUTOMATION_SET_PARAM_2					6
#define NEW_AUTOMATION_SET_PARAM_3					7
#define NEW_AUTOMATION_SET_PARAM_4					8
#define NEW_AUTOMATION_STATUS						9
#define NEW_AUTOMATION_STATUS_MANUAL				10
#define NEW_AUTOMATION_IDENT						11
#define NEW_AUTOMATIONSIZE							(NEW_AUTOMATION_IDENT+MAXSIZEIDENT+1)


///////////////////////////////////////////////
// global variables
///////////////////////////////////////////////

#define SEG_A   0b00000001
#define SEG_B   0b00000010
#define SEG_C   0b00000100
#define SEG_D   0b00001000
#define SEG_E   0b00010000
#define SEG_F   0b00100000
#define SEG_G   0b01000000
#define SEG_DP  0b10000000


////////////////////////////////////////////////
//
//		AAAAAAAAA
//		F		B
//		F		B
//		F		B
//		GGGGGGGGG
//		E		C
//		E		C
//		E		C
//		DDDDDDDDD
//				DP
////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Table segments (Common Anode) : 0–9, -, espace, A–Z
//////////////////////////////////////////////////////////////////////////////
//Common Anode             0    1    2      3     4     5     6    7     8     9     -   <space>
const char digits[] = {
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F, // 0–9
    0x40, // -
    0x00, // espace
    0x77,0x7C,0x39,0x5E,0x79,0x71,0x3D,0x76,0x06,0x1E, // Letters
    0x75,0x38,0x37,0x54,0x3F,0x73,0x67,0x50,0x6D,0x78,
    0x3E,0x1C,0x2A,0x76,0x6E,0x5B
};


volatile char  				gl_S1T0char = 0;
volatile char  				gl_S2T0char = 0;

volatile char  				gl_S1T1char = 0;
volatile char  				gl_S2T1char = 0;
		
volatile char  				gl_S2T2char = 0;
volatile char  				gl_S1T2char = 0;

volatile char 				gl_S1T3char = 0;
volatile char  				gl_S2T3char = 0;

#pragma udata GLOBAL_DATA
volatile char  				gl_OUTchar[6];
#pragma udata

// BOARD NUMBER FROM SWITCH SETTING
volatile char				gl_boardNumber;

// MASTER 
volatile char				gl_master;

// SPEED MANAGEMENT IN ANALOG MODE
volatile char				gl_speedCounter;
 
// SPEED CIRCUIT 

#pragma udata GLOBAL_DATA
volatile short				gl_setPoint[4];	  			// SPEED AND DIRECTION REQUESTED							
volatile short 				gl_setStep[4];	  			// INERTIA TO CHANGE SPEED AND DIRECTION REQUESTED
volatile short 				gl_setStepCounter[4];	  	// INERTIA TO CHANGE SPEED AND DIRECTION REQUESTED
volatile short 				gl_curSpeed[4];	  			// CUR SPEED AND DIRECTION

// low level value
volatile char 				gl_speed[4];  
volatile char 				gl_direction[4];  // DIRECTION CIRCUIT 
#pragma udata

// SIGNAL MANAGEMENT IN DIGITAL MODE
volatile char 				gl_signalState;

#pragma udata GLOBAL_DATA
volatile char 				gl_dcc[FRAME_SIZE]; 
#pragma udata		

volatile char 				gl_dcc_ready;

// FLASHING LED
volatile int				gl_flashingCounter;
volatile char				gl_goFlashingCounter;

// MODE
volatile char 				gl_boardMode;	// ANA or DCC	

// MUTEX
volatile char 				gl_mutexLowIsr;
volatile char				gl_canReceivedDataReady[MAXINPUTCANBUFFER];

// USART

#pragma udata GLOBAL_DATA
volatile static char 		gl_receivedUSARTData[USARTBUFFERSIZE];
#pragma udata 
volatile char				gl_waitCanPrint;

// CAN
#pragma udata GLOBAL_DATA
volatile char 				gl_outputCANbuffer[MAXTRAMESIZE];
volatile char				gl_outputCANbufferCounter;

volatile char 				gl_inputCANbuffer[MAXINPUTCANBUFFER][MAXTRAMESIZE];
volatile char				gl_inputCANWriteBufferPointer[MAXINPUTCANBUFFER];
volatile char 				gl_inputCANReadBufferPointer[MAXINPUTCANBUFFER];

volatile char 				gl_inputCANmode[MAXINPUTCANBUFFER];
volatile char				gl_requestInputCANtrameStart[MAXINPUTCANBUFFER];
volatile char				gl_printInputCANtrameStart[MAXINPUTCANBUFFER];
volatile char				gl_currentCANid[MAXINPUTCANBUFFER];

volatile BYTE 				gl_data[MAXTRAMESIZE];
#pragma udata 

// SYNCHRO BOARD VIA CAN

volatile char 				gl_syncRequested; 

// UART
#pragma udata GLOBAL_DATA
volatile char 				gl_inputUartString[USARTBUFFERSIZE];
volatile char 				gl_message[USARTBUFFERSIZE];
volatile char 				gl_errorInfo[MAXERRORINFO];
#pragma udata

volatile char 				gl_receivedUSARTPointer;
volatile char 				gl_getDataUSARTPointer;
volatile char				gl_inputCounter;

// PROTOCOL AND AUTOMATION
#pragma udata GLOBAL_DATA
volatile char 				gl_request[REQUESTSIZE];
volatile char 				gl_tmpBuffer[REQUESTSIZE];
volatile char				gl_automation[MAXAUTOMATION][NEW_AUTOMATIONSIZE];
#pragma udata
volatile char 				gl_nexAvailableAutomation;

// error info
volatile char 				gl_parserErrorCode;

// CUR TRACK STATUS
#pragma udata GLOBAL_DATA
volatile  short				gl_average[4];
volatile  short 			gl_noVehicule[4];
volatile char  				gl_OUTSTATchar[4];
volatile char  				gl_trackNotification[4];

#pragma udata
volatile char  				gl_trackCalibration;
volatile char				gl_trackNumber;

// MANUAL or AUTOMATIC
volatile char				gl_userMode;	

// CUR GPIO STATUS
#pragma udata GLOBAL_DATA
volatile char				gl_GPIONotification[4];
volatile char				gl_GPIOchar[4];
volatile char				gl_GPIOcounter[4];
volatile  unsigned int		gl_GPIOstabilized[4];
#pragma udata

// CUR TIMER 
#pragma udata GLOBAL_DATA
volatile char				gl_TIMERValue[MAXTIMER];
volatile char				gl_TIMERNotification[MAXTIMER];
#pragma udata
volatile char				gl_timerNumber;
volatile  unsigned short	gl_timer;

// VOLTAGE WARNING
volatile char				gl_low;

// RUN OR STOP ALL
volatile char 				gl_stopAll;

// KNOBS
volatile char				gl_numberKnobData;
volatile int				gl_knobValue0; // Between -15 and 15
volatile int				gl_knobValue1; // Between 0 and 100
volatile int 				gl_lastKnobValue0;
volatile int 				gl_lastKnobValue1;

volatile  unsigned short 	gl_adcKnobValue0;
volatile  unsigned short 	gl_adcKnobValue1;
volatile  unsigned short 	gl_minAdcKnobValue0;
volatile  unsigned short 	gl_maxAdcKnobValue0;
volatile  unsigned short 	gl_minAdcKnobValue1;
volatile  unsigned short 	gl_maxAdcKnobValue1;

volatile char				gl_getKnobValue;
volatile char				gl_calibKnob;
volatile unsigned short		gl_deltaKnob0;
volatile unsigned short		gl_deltaKnob1;

///////////////////////////////////////////////
// function declaration
///////////////////////////////////////////////

// SPRINTF & PRINTF
int mySprintf(char *buf,const rom char *fmt, ...);

// EEPROM READ / WRITE FUNCTION
char 	ReadEEPROM( int adr, char *data);
void	CalibMinMaxKnob(void);
void 	ResetEEPROM(void);
void 	ReadEEPROMConfig(void);
char 	WriteCompletedEEPROM(void);
char 	WriteRdyEEPROM(void);
char 	WriteEEPROM( short adr, char data);
char	getAutomationFromEEPROM(void);

// PARSER AND REQUEST MANAGEMENT
char 	isAdigit(char car);
char 	isAhexaDigit(char car);
char 	toUpperCase(char car);
short 	strtol(const char* nptr);
void	clearError(void);
void 	traceError(void);
char 	getToken(char* inputString,char* inputToken,char* stringPointer);
char 	getValue(char* inputString,char* Value,char* stringPointer);
char    getIdent(char* inputString, char* stringPointer,char *ident);
char 	parser(char* inputString);
char 	memAvailable(void);
void	boardStatus(void);
char 	uncompressData(void);
char 	compressData(void);
void 	initRequest(void);
char 	removeAutomation(char automationNumber);
char 	saveAutomation(char automationNumber);
void 	assignAutomation(char automationCounter);
void 	setSpeed(char speed,char step,char trackNumber);
char 	manageRequest (char sendPrompt);

// UART
void 	initUSART(void);
void 	sendUSART(char data);
char 	getInputRequestFromUSART(char *inputString,char *inputCounter);
void 	prompt(char* message);

// CAN
void 	flushOut(void);
void	CANsendDelay(void);
int 	_user_putc(char c);
void 	sendRequestToCAN(void);
void 	CAN_SendSync(void);


char	getInputRequestFromCAN(void);
void	sendPrintToCAN(void);

// 7 SEGMENT TM1637
void 	set7segmentPort(char CLK, char DIO);
void 	twoWire_init(void);
void 	twoWire_start(void);
void 	twoWire_stop(void);
void 	twoWire_ack(void);
char 	twoWire_write(char data);
void 	TM1637_init(void);
void	TM1637_write(short number1,short number2);
void 	TM1637_display(short number1, short number2);
void 	TM1637_setBrightness(char level);
void 	TM1637_displayString(char *string);
void 	TM1637_writeStringWindow(const char *s, char start, char len);
char 	charToSegments(char c);
void 	delayMainLoop(int delay);


// INTERRUPT AND SIGNAL MANAGEMENT
void 	setDcc(char address, char command);
void 	setPort(void);

void 	interrupt_at_high_vector(void);
void 	high_isr(void);

void  	interrupt_at_low_vector(void);
void 	low_interrupt();
void 	low_isr(void);
void	low_isr_task(void);

// CODE INITIALISATION
void 	initEnvironment(void);
void 	PIC18FMainSettings(void);
void 	init(void);
void 	trackCalibration(void);

// EVENT MANAGEMENT
char 	getEventRequestFromTrack(void);
char 	getEventRequestFromGPIO(void);
char 	getEventRequestFromTIMER(void);
void 	getEventFromKNOB(void);







