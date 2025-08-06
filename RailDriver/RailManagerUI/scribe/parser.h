#ifndef PARSER_H
#define PARSER_H

#include <QString>

// Parser based on code for PIC18F4680

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
#define MANUAL							"MANUAL"
#define MANUAL0							"MANUAL0"
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



#define MAXOUTPUTSTRING					200

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
#define BAD_TRACK_NUM                   0xE
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
#define MAXSIZETOKEN					10
#define MAXSIZEIDENT					3
#define MAXTRAMESIZE					80 // Should be multiple of 8
#define TRAMEREQUESTHEADER				0xEE
#define TRAMEREQUESTFOOTER				0xFF
#define TRAMEPRINTHEADER				0xCC
#define TRAMEPRINTFOOTER				0xDD
#define CAN_REQUEST                     1
#define CAN_PRINT                       2
#define CAN_UNKNOWN                     3


#define TRUE							1
#define FALSE							0


class parser
{
public:
    char    gl_parserErrorCode;
    char    gl_message[MAXOUTPUTSTRING];
    QString gl_errorContext;
    unsigned char  gl_getDataCANPointer=0;
    unsigned char  gl_InputBufferPointer=0;
    unsigned char  gl_inputBuffer[MAXTRAMESIZE];
    unsigned char  gl_canMode;
    char           gl_requestTrameStart;
    char           gl_printTrameStart;
    unsigned char  gl_master;
    unsigned char  gl_tmpBuffer[REQUESTSIZE];
    unsigned char  gl_request[REQUESTSIZE];

    unsigned char  gl_requestHeaderTrameDetected=0;
    unsigned char  gl_printHeaderTrameDetected=0;
    unsigned char  gl_requestFooterTrameDetected=0;
    unsigned char  gl_printFooterTrameDetected=0;



    struct parserObject {
        QString Name;
        QString BoardNumber;
        QString Event;
        QString Action;
        QString Mode;
    };


public:
    parser();
    void traceError();
    char getToken(char* inputString, char* inputToken, char* stringPointer);
    char getValue(char* inputString, char* Value, char* stringPointer);
    char getIdent(char* inputString, char* stringPointer,char *ident);
    char parseLine(char* inputString, parserObject* returnedObject);
    void initRequest(unsigned char* request);
    unsigned char uncompressData(unsigned char* data);
    unsigned char getInputRequestFromCAN(unsigned char* request,int *mode);

};

#endif // PARSER_H
