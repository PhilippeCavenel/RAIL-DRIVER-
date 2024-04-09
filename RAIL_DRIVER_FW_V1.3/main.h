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

#include "ECANPoll.h"

// TRUE FALSE definition as bool not recognized
#define TRUE							1
#define FALSE							0
                         
#define RAIL_DRIVER_HEADER				"RAIL DRIVER..V 1.3"
#define VERSION							"VERSION......07-04-2024"
#define BOARD_NUMBER					"BOARD........"
#define MEMORY							"MEMORY......."
#define AUTOMATION						"AUTOMATION..."
#define MODE							"MODE........."

// PROTOCOL
#define MAXINPUTSTRING					200
#define MAXOUTPUTSTRING					200
#define MAXSIZETOKEN					10
#define MAXSIZEIDENT					3
#define MAXTRAMESIZE					80 // Should be multiple of 8
#define MAXTRAMECAN						MAXTRAMESIZE-24
#define MAXMESSAGESIZE					MAXTRAMECAN-2 // We need to add last char and end of string

#define TRAMEPRINTHEADER				0xCC
#define TRAMEPRINTFOOTER				0xDD

#define TRAMEREQUESTHEADER				0xEE
#define TRAMEREQUESTFOOTER				0xFF

#define ENDOFPRINTFTRAME				0x00

// AUTOMATION
#define MAXTIMER						15
#define MAXTIMERDELAY					255
#define MAXSPEEDVALUE					15
#define MAXAUTOMATION					100

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
#define AUTOMATION_ADDRESS				(GPIO3DIR_ADDRESS+1)		// Should be the last of the list
						

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

#define UNKNOWN_ERROR					0xF0

// MSSING ERROR CODE
#define EINVAL							22

// Bits de Configuration 
#pragma config OSC 		= 				IRCIO67 // Oscillateur Interne
#pragma config FCMEN 	= 				OFF
#pragma config IESO 	= 				OFF
#pragma config PWRT 	= 				OFF
#pragma config BOREN 	= 				OFF // BOHW
#pragma config BORV 	= 				3
#pragma config WDT 		= 				OFF
#pragma config PBADEN 	= 				OFF
#pragma config MCLRE 	= 				OFF
#pragma config LVP 		= 				OFF

// CAN
#define CAN_REQUEST						1
#define CAN_PRINT						2
#define CAN_UNKNOWN						3
#define WAITDELAYTRAMECAN				500
#define SYNCHROSENDDELAY				100

// RS232
#define _XTAL_FREQ    					32000000
#define _BAUD         					115200
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
#define HYSTERERISHIGH					2
#define HYSTERERISLOW					1
#define SAMPLEFORAVERAGE				30
#define SAMPLEFORCALIBRATION			5
#define TIMECALIBRATION 				0xFFF

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
#define ERR								"ERR"
#define AUTFULL							"AUTFULL"
#define NOAUT							"NOAUT"
#define TIMEOUT							"TIMEOUT"
#define NODCC							"NODCC"
#define NOANA							"NOANA"
#define GPIOIN							"GPIOIN"
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
#define AUTOMATIC						"AUTOMATIC"
#define CALIB							"CALIB"

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

#define REQ_PROGRAM_REQUEST_GPIO_EVENT 				(41+MAXSIZEIDENT)

#define START_HERE_TO_COPY_FOR_AUTOMATION			REQ_PROGRAM_REQUEST_GPIO_EVENT

#define REQ_PROGRAM_REQUEST_EVENT_BOARD_GPIO_NUMBER (42+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_GPIO_NUMBER 		(43+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_GPIO_LEVEL 		(44+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_TIMER_EVENT 			(45+MAXSIZEIDENT)	
#define REQ_PROGRAM_REQUEST_EVENT_BOARD_TIMER_NUMBER (46+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_TIMER_NUMBER 		(47+MAXSIZEIDENT)			

#define REQ_PROGRAM_REQUEST_TRACK_EVENT 			(48+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_BOARD_TRACK_NUMBER (49+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_TRACK_NUMBER 		(50+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_EVENT_VEHICLE_STATUS 	(51+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_GPIO_SETTING 			(52+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_ACTION_GPIO_SET_NUMBER 	(53+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_ACTION_GPIO_SET_LEVEL 	(54+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_TIMER_SETTING 			(55+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_ACTION_TIMER_SET_NUMBER (56+MAXSIZEIDENT)	
#define REQ_PROGRAM_REQUEST_ACTION_TIMER_SET_DELAY	(57+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_LPO_SETTING 			(58+MAXSIZEIDENT)
#define REQ_PROGAM_REQUEST_ACTION_LPO_SET_NUMBER	(59+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_ACTION_LPO_SET_LEVEL 	(60+MAXSIZEIDENT)

#define REQ_PROGRAM_REQUEST_AUT_SETTING 			(61+MAXSIZEIDENT)
#define REQ_PROGAM_REQUEST_ACTION_AUT_SET_IDENT		(62+MAXSIZEIDENT)
#define REQ_PROGRAM_REQUEST_ACTION_AUT_SET_STATUS 	(63+MAXSIZEIDENT*2)

#define REQ_PROGRAM_REQUEST_TRACK_SETTING 			(64+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_NUMBER (65+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_SPEED 	(66+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_DIR 	(67+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_INERTIA	(68+MAXSIZEIDENT*2)

#define REQ_PROGRAM_REQUEST_SET_USER_MODE 			(69+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_USER_MODE 		(70+MAXSIZEIDENT*2)

#define REQ_PROGRAM_REQUEST_DCC_SETTING 			(71+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_DCC_ADDRESS_SETTING 	(72+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_ACTION_DCC_COMMAND_SETTING 	(73+MAXSIZEIDENT*2)

#define REQ_PROGRAM_REQUEST_STATUS					(74+MAXSIZEIDENT*2)
#define REQ_PROGRAM_REQUEST_STATUS_MANUAL			(75+MAXSIZEIDENT*2)	
#define REQ_PROGRAM_REQUEST_IDENT					(76+MAXSIZEIDENT*2)

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

#define REQUESTSIZE									(REQ_EVENT_REQUEST_EVENT_TIMER_NUMBER+1)

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

//Common Anode             0    1    2      3     4     5     6    7     8     9     -   <space>
const char digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x40, 0x0};

unsigned char  				gl_S1T0char = 0;
unsigned char  				gl_S2T0char = 0;

unsigned char  				gl_S1T1char = 0;
unsigned char  				gl_S2T1char = 0;

unsigned char  				gl_S1T2char = 0;
unsigned char  				gl_S2T2char = 0;

unsigned char  				gl_S1T3char = 0;
unsigned char  				gl_S2T3char = 0;

unsigned char  				gl_OUTchar[6];


// BOARD NUMBER FROM SWITCH SETTING
unsigned char				gl_boardNumber;

// MASTER 
unsigned char				gl_master;

// SPEED MANAGEMENT IN ANALOG MODE
unsigned char				gl_speedCounter;
 
   // SPEED CIRCUIT 
short						gl_setPoint[4];	  			// SPEED AND DIRECTION REQUESTED							
short 						gl_setStep[4];	  			// INERTIA TO CHANGE SPEED AND DIRECTION REQUESTED
short 						gl_setStepCounter[4];	  	// INERTIA TO CHANGE SPEED AND DIRECTION REQUESTED
short 						gl_curSpeed[4];	  			// CUR SPEED AND DIRECTION

// low level value
unsigned char 				gl_speed[4];  
unsigned char 				gl_direction[4];  // DIRECTION CIRCUIT 

// SIGNAL MANAGEMENT IN DIGITAL MODE
unsigned char 				gl_signalState;
unsigned char 				gl_dcc[FRAME_SIZE]; 
char 						gl_dcc_ready;

// MODE
unsigned char 				gl_boardMode;	// ANA or DCC	

// MUTEX
unsigned char 				gl_mutex;

unsigned char				gl_trackNumber;

// USART
#pragma udata USART
static unsigned char 		gl_receivedUSARTData[USARTBUFFERSIZE];
#pragma udata

unsigned char 				gl_receivedUSARTPointer;
unsigned char 				gl_getDataUSARTPointer;
char						gl_inputCounter;

// PROTOCOL
#pragma udata PROTOCOL
unsigned char 				gl_request[REQUESTSIZE];
unsigned char 				gl_outputBuffer[MAXTRAMESIZE];
unsigned char 				gl_inputBuffer[MAXTRAMESIZE];
unsigned char 				gl_tmpBuffer[REQUESTSIZE];

unsigned char		 		gl_automation[MAXAUTOMATION][NEW_AUTOMATIONSIZE];
unsigned char 				gl_inputUartString[MAXINPUTSTRING];
char 						gl_message[MAXOUTPUTSTRING];
#pragma udata

unsigned char 				gl_nexAvailableAutomation;
unsigned char				gl_outputBufferCounter;
unsigned char				gl_InputBufferPointer;
unsigned char 				gl_getDataCANPointer;
unsigned char 				gl_canMode;
char						gl_requestTrameStart;
char						gl_printTrameStart;

// error info
unsigned char 				gl_parserErrorCode;

// CUR TRACK STATUS
unsigned int				gl_average[4];
unsigned int 				gl_noVehicule[4];
unsigned char  				gl_calibration;
unsigned char  				gl_OUTSTATchar[4];
unsigned char  				gl_trackNotification[4];

int							gl_knobValue0; // Between -15 and 15
int							gl_knobValue1; // Between 0 and 100
short 						gl_lastKnobValue0;
short 						gl_lastKnobValue1;

unsigned short 				gl_adcKnobValue0;
unsigned short 				gl_adcKnobValue1;
unsigned short 				gl_minAdcKnobValue0;
unsigned short 				gl_maxAdcKnobValue0;
unsigned short 				gl_minAdcKnobValue1;
unsigned short 				gl_maxAdcKnobValue1;

char						gl_getKnobValue;
char						gl_numberKnobData;
char						gl_calibKnob;
short						gl_deltaKnob0;
short						gl_deltaKnob1;


unsigned char				gl_userMode;	// MANUAL or AUTOMATIC


// CUR GPIO STATUS
unsigned char				gl_GPIONotification[4];
unsigned char				gl_GPIOchar[4];
unsigned char				gl_GPIOcounter[4];
unsigned int				gl_GPIOstabilized[4];

// CUR TIMER 

unsigned char				gl_TIMERValue[MAXTIMER];
unsigned char				gl_TIMERNotification[MAXTIMER];
unsigned char				gl_timerNumber;
unsigned short				gl_timer;

// RUN OR STOP ALL
unsigned char 				gl_stopAll;

// SYNCHRO SEND
unsigned short				gl_synchroSend;

///////////////////////////////////////////////
// function declaration
///////////////////////////////////////////////

// EEPROM READ / WRITE FUNCTION
unsigned char 	ReadEEPROM(unsigned int adr, unsigned char *data);
void		 	CalibMinMaxKnob();
void 			ResetEEPROM();
void 			ReadEEPROMConfig(void);
unsigned char 	WriteCompletedEEPROM(void);
unsigned char 	WriteRdyEEPROM(void);
unsigned char 	WriteEEPROM(unsigned short adr, unsigned char data);
unsigned char	getAutomationFromEEPROM();

// PARSER AND REQUEST MANAGEMENT
unsigned char 	isAdigit(unsigned char car);
unsigned char 	isAhexaDigit(unsigned char car);
unsigned char 	toUpperCase(unsigned char car);
unsigned short 	strtol(const char* nptr);
void 			traceError();
unsigned char 	getToken(unsigned char* inputString, unsigned char* inputToken, unsigned char* stringPointer);
unsigned char 	getValue(unsigned char* inputString, unsigned char* Value, unsigned char* stringPointer);
unsigned char   getIdent(unsigned char* inputString, unsigned char* stringPointer,unsigned char *ident);
unsigned char 	parser(unsigned char* inputString, unsigned char* request);
unsigned char 	memAvailable();
void			boardStatus();
unsigned char 	uncompressData(unsigned char* data);
unsigned char 	compressData(unsigned char* data);
void 			initRequest(unsigned char* request);
unsigned char 	removeAutomation(unsigned char automationNumber);
unsigned char 	saveAutomation(unsigned char automationNumber,unsigned char* data);
void 			assignAutomation(char *request,unsigned char automationCounter);
void 			setSpeed(char speed,char step,unsigned char trackNumber);
unsigned char 	manageRequest (unsigned char* request,unsigned char sendPrompt);
void 			setRequest(unsigned char* request, unsigned char* data,unsigned char init);
unsigned char 	getDataFromRequest(unsigned char* request, unsigned char* data);

// UART
void 			initUSART();
void 			sendUSART(unsigned char data);
unsigned char 	getInputRequestFromUSART(unsigned char *inputString,char *inputCounter);
void 			prompt(unsigned char* message);

// CAN
void 			flushOut();
void			 CANsendDelay();
int 			_user_putc(char c);
void 			sendRequestToCAN(unsigned char* request);
unsigned char	getInputRequestFromCAN(unsigned char* request);

// 7 SEGMENT TM1637
void 			set7segmentPort(unsigned char CLK, unsigned char DIO);
void 			twoWire_init();
void 			twoWire_start();
void 			twoWire_stop();
void 			twoWire_ack();
char 			twoWire_write(char data);
void 			TM1637_init(void);
void			TM1637_write(short number1,short number2);
void 			TM1637_display(short number1, short number2);
void 			TM1637_setBrightness(char level);


// INTERRUPT AND SIGNAL MANAGEMENT
void 			setDcc(unsigned char address, unsigned char command);
void 			setPort();
void 			interrupt_at_high_vector(void);
void  			interrupt_at_low_vector(void);
void 			high_isr(void);
void 			low_interrupt();
void 			low_isr(void);

// CODE INITIALISATION
void 			initSignal();
void 			PIC18FMainSettings();
void 			init();
void 			calibration();

// EVENT MANAGEMENT
unsigned char 	getEventRequestFromTrack(unsigned char* request);
unsigned char 	getEventRequestFromGPIO(unsigned char* request);
unsigned char 	getEventRequestFromTIMER(unsigned char* request);
void 			getEventFromKNOB();







