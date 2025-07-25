#ifndef PARSER_H
#define PARSER_H

#include <QString>

// Parser based on code for PIC18F4680

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
#define MAXSIZETOKEN					10
#define MAXSIZEIDENT					3


#define TRUE							1
#define FALSE							0


class parser
{
public:
    char    gl_parserErrorCode;
    char    gl_message[MAXOUTPUTSTRING];
    QString gl_errorContext;

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

};

#endif // PARSER_H
