#include <parser.h>
#include <QString>
#include <QDebug>
#include <QWidget>
#include <QMessageBox>

parser::parser() {}

/////////////////////////////////////////////////////////////////////////////
// prompt
/////////////////////////////////////////////////////////////////////////////
void prompt(const QString& error) {
    QMessageBox::critical(nullptr, "Error", error);
}

/////////////////////////////////////////////////////////////////////////////
// traceError
/////////////////////////////////////////////////////////////////////////////
void parser::traceError(){

    QString error;

    switch (gl_parserErrorCode) {
    case UNKNOWN_TOKEN              : error = "Unknown token"; break;
    case NUMBER_MISSING            : error = "Missing Number"; break;
    case INCOMPLETE_REQUEST        : error = "Incomplete request"; break;
    case BAD_NUMBER                : error = "Bad number"; break;
    case MODE_MISSING              : error = "Mode missing"; break;
    case BAD_GPIO_NUMBER           : error = "Bad GPIO number"; break;
    case BAD_TIMER_NUMBER          : error = "Bad TIMER number"; break;
    case BAD_TIMER_VALUE           : error = "Bad TIMER value"; break;
    case BAD_LPO_NUMBER            : error = "Bad LPO number"; break;
    case BAD_AUT_STATUS            : error = "Bad Automation status"; break;
    case BAD_AUT_IDENT             : error = "Bad Automation name"; break;
    case MISSING_IDENT             : error = "Automation name missing"; break;
    case BAD_GPIO_DIR              : error = "Bad GPIO direction"; break;
    case BAD_GPIO_LEVEL            : error = "Bad GPIO level"; break;
    case BAD_LPO_LEVEL             : error = "Bad LPO level"; break;
    case BAD_TRACK_SPEED           : error = "Bad track speed"; break;
    case BAD_TRACK_DIR             : error = "Bad track direction"; break;
    case BAD_TRACK_NUM             : error = "Bad track number"; break;
    case BAD_BOARD_MODE            : error = "Bad board mode"; break;
    case WRONG_BOARD_NUMBER        : error = "Wrong board number"; break;
    case AUTOMATIONSIZELIMIT       : error = "Automation limit reached"; break;
    case MISSING_SPACE             : error = "Space missing"; break;
    case IDENT_TOO_LONG            : error = "Identifier too long"; break;
    case AUTOMATIONLREADYEXISTS    : error = "Automation already defined"; break;
    case BADAUTOMATIONNUMBER       : error = "Wrong automation number"; break;
    case BAD_TRACK_INERTIA         : error = "Bad inertia value"; break;
    case BAD_USER_MODE             : error = "Bad user mode"; break;
    default:
        error = QString("Unknown Error Code ").arg(static_cast<unsigned char>(gl_parserErrorCode), 2, 16, QChar('0')).toUpper();
        break;
    }
    prompt(error.append(" ").append(gl_errorContext));
    gl_parserErrorCode = 0;
    gl_errorContext.clear();
}

/////////////////////////////////////////////////////////////////////////////
// getToken
// return value TRUE if parsing correct, otherwise FALSE
/////////////////////////////////////////////////////////////////////////////
char parser::getToken(char* inputString, char* inputToken, char* stringPointer) {

    char carCounter = 0;
    char tokenCarPointer = 0;
    char testToken[MAXSIZETOKEN];

    size_t len = strlen(reinterpret_cast<const char*>(inputString));

    for (carCounter=0; carCounter < len; carCounter++) {
        if (inputString[carCounter]==' ') {
            (*stringPointer)++;
            continue; // remove space
        }
        testToken[tokenCarPointer++] = inputString[carCounter];
        testToken[tokenCarPointer]='\0';

        (*stringPointer)++;
        QByteArray test(reinterpret_cast<const char*>(testToken));
        QByteArray input(reinterpret_cast<const char*>(inputToken));
        test=test.toUpper();
        input=input.toUpper();

        if (test.startsWith(input)) {
            return(TRUE);
        }

        // Error
        size_t lenToken = strlen(reinterpret_cast<const char*>(inputToken));
        if (tokenCarPointer == ((lenToken<MAXSIZETOKEN-1) ? lenToken:MAXSIZETOKEN - 1)) {
            break;

        }
    }
    gl_parserErrorCode = UNKNOWN_TOKEN;
    gl_errorContext=QString::fromUtf8(inputString).append(" not expected => ").append(QString::fromUtf8(testToken));
    return(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// getValue
// return value TRUE if parsing correct, otherwise FALSE
/////////////////////////////////////////////////////////////////////////////
char parser::getValue(char* inputString, char* Value, char* stringPointer) {

    int carCounter = 0;
    char tokenCarPointer = 0;
    unsigned short number;
    unsigned char dataFound;


    errno = 0;
    size_t len = strlen(reinterpret_cast<const char*>(inputString));

    while (inputString[carCounter] == ' ' && carCounter < len) {
        (*stringPointer)++;
        carCounter++;
    }

    QString input = QString::fromUtf8(reinterpret_cast<const char*>(inputString));
    QString sub = input.left(carCounter);  // extrait à partir de carCounter
    number = strtol(reinterpret_cast<const char*>(&inputString[carCounter]), nullptr, 10);

    if (errno == ERANGE || errno ==EINVAL) {
        dataFound = FALSE;
        gl_parserErrorCode = NUMBER_MISSING;
    }
    else {
        if (number >255) {
            errno = ERANGE;
            gl_parserErrorCode = BAD_NUMBER;
        }
        else dataFound = TRUE;
    }

    if (dataFound == FALSE) {
        gl_errorContext=QString::fromUtf8(inputString);
        return(FALSE);
    }

    while (inputString[carCounter] != ' ' && carCounter < len) {
        (*stringPointer)++;
         carCounter++;
     }

     *Value=(char)number;
    return(TRUE);
}
/////////////////////////////////////////////////////////////////////////////
// getIdent
// return value TRUE if parsing correct, otherwise FALSE
/////////////////////////////////////////////////////////////////////////////
char parser::getIdent(char* inputString, char* stringPointer,char *ident) {

    char getFirstSpace;
    char identCounter=0;

    size_t len = strlen(reinterpret_cast<const char*>(inputString));


    // GET AUTOMATION IDENT
    getFirstSpace=FALSE;
    while(identCounter<MAXSIZEIDENT) {
        if (inputString[*stringPointer]!=' ') {
            if(identCounter==0 && getFirstSpace==FALSE) {
                gl_parserErrorCode = MISSING_SPACE;
                gl_errorContext=QString::fromUtf8(inputString);

                return(FALSE);
            }
            else if (*stringPointer==len){
                if (identCounter>0) {
                    ident[identCounter]='\0'; // get end of line
                    break;
                }
                else {
                    gl_parserErrorCode = MISSING_IDENT;
                    gl_errorContext=QString::fromUtf8(inputString);
                    return(FALSE);
                }
            }
            else ident[identCounter++]=inputString[(*stringPointer)++];
        }
        else {
            if (identCounter==0) {
                getFirstSpace=TRUE;
                (*stringPointer)++;
            }
            else {
                ident[identCounter]='\0';
                break; // get second space
            }
        }

        if (identCounter==MAXSIZEIDENT) {
            gl_parserErrorCode = IDENT_TOO_LONG;
            gl_errorContext=QString::fromUtf8(inputString).append(" Not expected => ").append(QString::fromUtf8(ident));;
            return(FALSE);
        }
    }
    return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// parseLine
// return value TRUE if parsing correct, otherwise FALSE
// Semantic analysis on values should be done on char content
/////////////////////////////////////////////////////////////////////////////
char parser::parseLine(char* inputString,parserObject* returnedObject) {

    char stringPointer = 0;
    char keepStringPointer = 0;

    gl_parserErrorCode=0;
    gl_errorContext.clear();

    // Clean returnedObject
    returnedObject->Action.clear();
    returnedObject->BoardNumber.clear();
    returnedObject->Event.clear();
    returnedObject->Name.clear();
    returnedObject->Mode.clear();

    // STOP
    stringPointer = 0;

    if (getToken(&inputString[stringPointer], (char*)STOP, &stringPointer)) {

        // Get board number
        char Number;

        if (!getValue(&inputString[stringPointer], &Number, &stringPointer))return(FALSE);
        returnedObject->BoardNumber=QString::number(Number);
        returnedObject->Action=QString("BOARD ").append(QString::number(Number)).append(QString(" STOP"));
        return(TRUE);
    }

    // STOPALL
    stringPointer = 0;

    if (getToken(&inputString[stringPointer], (char*)STOP, &stringPointer)) {
        returnedObject->Action=QString("STOPALL ");
        return(TRUE);
    }

    // RUNALL
    stringPointer = 0;
    if (getToken(&inputString[stringPointer], (char*)RUNALL, &stringPointer)) {
        returnedObject->Action=QString("RUNALL ");
        return(TRUE);
    }

    // SYNCHRO
    stringPointer = 0;
    if (getToken(&inputString[stringPointer], (char*)SYNCHRO, &stringPointer)) {
        returnedObject->Action=QString("SYNC ");
        return(TRUE);
    }

    // RUN
    stringPointer = 0;
    if (getToken(&inputString[stringPointer], (char*)RUN, &stringPointer)) {

        // Get board number
        char Number;
        if (!getValue(&inputString[stringPointer], &Number, &stringPointer))return(FALSE);
        returnedObject->BoardNumber=QString::number(Number);
        returnedObject->Action=QString("BOARD ").append(QString::number(Number)).append(QString(" RUN"));
        return(TRUE);
    }

    // CALIB
    stringPointer = 0;
    if (getToken(&inputString[stringPointer], (char*)CALIB, &stringPointer)) {

        // Get board number
        char Number;

        if (!getValue(&inputString[stringPointer], &Number, &stringPointer))return(FALSE);
        returnedObject->BoardNumber=QString::number(Number);
        returnedObject->Action=QString("CALIB ").append(QString::number(Number));
        return(TRUE);
    }

    // RESET
    stringPointer = 0;
    if (getToken(&inputString[stringPointer], (char*)RESET, &stringPointer)) {

        // Get board number
        char Number;

        if (!getValue(&inputString[stringPointer], &Number, &stringPointer))return(FALSE);
        returnedObject->BoardNumber=QString::number(Number);
        returnedObject->Action=QString("RESET ").append(QString::number(Number));
        return(TRUE);
    }

    //PROG
    stringPointer = 0;
    if (getToken(&inputString[stringPointer], (char*)PROG, &stringPointer)) {

        // Get board number
        char boardNumber;

        if (!getValue(&inputString[stringPointer], &boardNumber, &stringPointer)) return(FALSE);
        returnedObject->BoardNumber=QString::number(boardNumber);
        returnedObject->Action=QString("BOARD ").append(QString::number(boardNumber)).append(QString(" "));


        // get DCC or ANA or GPIO or AUT or DEL
        keepStringPointer = stringPointer;

        // DCC
        if (getToken(&inputString[stringPointer], (char*)DCC, &stringPointer)) {
            return(TRUE);
        }
        //ANA
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)ANA, &stringPointer)) {
            return(TRUE);
        }
        // GPIO
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)GPIO, &stringPointer)) {

            // get GPIO number
            char Number;

            if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);
            keepStringPointer = stringPointer;

            // IN
            if (getToken(&inputString[stringPointer], (char*)IN, &stringPointer)) {
                return(TRUE);
            }

            // OUT
            else {
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)OUT, &stringPointer)) {
                    return(TRUE);
                }
                else return(FALSE);
            }
        }
        // PUSHCAN
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)PUSHCAN, &stringPointer)) {

            //GPIO
            keepStringPointer = stringPointer;
            if (getToken(&inputString[stringPointer], (char*)GPIO, &stringPointer)) {

                // get GPIO number
                char Number;

                if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);
                keepStringPointer = stringPointer;

                // ONCAN
                if (getToken(&inputString[stringPointer], (char*)ONCAN, &stringPointer)) {
                    return(TRUE);
                }

                // OFFCAN
                else {
                    stringPointer = keepStringPointer;
                    if (getToken(&inputString[stringPointer], (char*)OFFCAN, &stringPointer)) {
                        return(TRUE);
                    }
                    else return(FALSE);
                }
            }
            //TRACK
            stringPointer = keepStringPointer;
            if (getToken(&inputString[stringPointer], (char*)TRACK, &stringPointer)) {

                // get TRACK number
                char Number;

                if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);
                keepStringPointer = stringPointer;

                // ONCAN
                if (getToken(&inputString[stringPointer], (char*)ONCAN, &stringPointer)) {
                    return(TRUE);
                }

                // OFFCAN
                else {
                    stringPointer = keepStringPointer;
                    if (getToken(&inputString[stringPointer], (char*)OFFCAN, &stringPointer)) {
                        return(TRUE);
                    }
                    else return(FALSE);
                }
            }
            //TIMER
            stringPointer = keepStringPointer;
            if (getToken(&inputString[stringPointer], (char*)TIMER, &stringPointer)) {

                // get TIMER number
                char Number;

                if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);
                keepStringPointer = stringPointer;

                // ONCAN
                if (getToken(&inputString[stringPointer], (char*)ONCAN, &stringPointer)) {
                    return(TRUE);
                }

                // OFFCAN
                else {
                    stringPointer = keepStringPointer;
                    if (getToken(&inputString[stringPointer], (char*)OFFCAN, &stringPointer)) {
                        return(TRUE);
                    }
                    else return(FALSE);
                }
            }
            return(FALSE);
        }

        // AUT
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)AUT, &stringPointer)) {

            // GET AUTOMATION IDENT
            char ident[MAXSIZEIDENT];
            if (getIdent(inputString,&stringPointer,ident)==FALSE) {
                return(FALSE);
            }
            returnedObject->Name=QString(ident).append(QString(" BOARD ").append(QString::number(boardNumber)));

            // GET AUTOMATION STATUS FOR MANUAL MODE

            // AUTON
            keepStringPointer = stringPointer;
            if (getToken(&inputString[stringPointer], (char*)AUTON, &stringPointer)) {
                returnedObject->Mode=QString(AUTON);

            }
            else {

                // AUTOFF
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)AUTOFF, &stringPointer)) {
                    returnedObject->Mode=QString(AUTOFF);
                }
                else {          
                    return(FALSE);
                }
            }

            //BOARD
            if (getToken(&inputString[stringPointer], (char*)BOARD, &stringPointer)) {

                // get BOARD number
                char Number;
                if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

                returnedObject->Event = QString("BOARD ").append(QString::number(Number)).append(QString(" "));

                // get GPIO or TIMER or TRACK
                keepStringPointer = stringPointer;

                // GPIO
                if (getToken(&inputString[stringPointer], (char*)GPIO, &stringPointer)) {

                    // get GPIO number
                    char Number;
                    if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

                    // get GPIO level
                    char Level;

                    if (!getValue(&inputString[stringPointer], &Level, &stringPointer)) return(FALSE);

                    returnedObject->Event.append(QString("GPIO ")).append(QString::number(Number)).append(QString(" ").append(QString::number(Level)));
                }

                // TIMER
                else {
                    stringPointer = keepStringPointer;
                    if (getToken(&inputString[stringPointer], (char*)TIMER, &stringPointer)) {

                        // get TIMER number
                        char Number;

                        if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);
                        returnedObject->Event.append(QString("TIMER ")).append(QString::number(Number));

                    }

                    // TRACK
                    else {
                        stringPointer = keepStringPointer;
                        if (getToken(&inputString[stringPointer], (char*)TRACK, &stringPointer)) {

                            // get TRACK number
                            char Number;

                            if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

                            // STA
                            if (!getToken(&inputString[stringPointer], (char*)STA, &stringPointer)) return(FALSE);

                            // get ONTRACK or OFFTRACK
                            keepStringPointer = stringPointer;
                            if (getToken(&inputString[stringPointer], (char*)ONTRACK, &stringPointer)) {
                                returnedObject->Event.append(QString("TRACK ")).append(QString::number(Number)).append(QString(" ONTRACK"));

                            }
                            else {
                                stringPointer = keepStringPointer;
                                if (getToken(&inputString[stringPointer], (char*)OFFTRACK, &stringPointer)) {
                                    returnedObject->Event.append(QString("TRACK ")).append(QString::number(Number)).append(QString(" OFFTRACK"));

                                }
                                else return(FALSE);
                            }
                        }
                        else return(FALSE);
                    }
                }

                // ACT
                if (!getToken(&inputString[stringPointer], (char*)ACT, &stringPointer)) return(FALSE);

                // get GPIO or TIMER or LPO or AUTON or AUTOFF or TRACK or DCC or MANUAL or AUTOMATIC
                keepStringPointer = stringPointer;

                // GPIO
                if (getToken(&inputString[stringPointer], (char*)GPIO, &stringPointer)) {

                    // get GPIO number
                    char Number;
                    if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

                    // get GPIO level
                    char Level;

                    if (!getValue(&inputString[stringPointer], &Level, &stringPointer)) return(FALSE);
                    returnedObject->Action.append(QString("GPIO ")).append(QString::number(Number)).append(QString(" ").append(QString::number(Level)));
                    return(TRUE);
                }

                // TIMER
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)TIMER, &stringPointer)) {

                    // get TIMER number
                    char Number;

                    if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

                    // get TIMER delay
                    char Delay;

                    if (!getValue(&inputString[stringPointer], &Delay, &stringPointer)) return(FALSE);
                    returnedObject->Action.append(QString("TIMER ")).append(QString::number(Number)).append(QString(" ").append(QString::number(Delay)));

                    return(TRUE);
                }


                // LPO
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)LPO, &stringPointer)) {

                    // get LPO number
                    char Number;

                    if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

                    // get LPO level
                    char Level;

                    if (!getValue(&inputString[stringPointer], &Level, &stringPointer)) return(FALSE);
                    returnedObject->Action.append(QString("LPO ")).append(QString::number(Number)).append(QString(" ").append(QString::number(Level)));

                    return(TRUE);
                }

                // AUTON
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)AUTON, &stringPointer)) {

                    // GET AUTOMATION IDENT
                    char ident[MAXSIZEIDENT];

                    if (getIdent(inputString,&stringPointer,ident)==FALSE) {
                        return(FALSE);
                    }
                    returnedObject->Action.append(QString("AUTON ")).append(QString(ident));

                    return(TRUE);
                }

                // AUTOFF
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)AUTOFF, &stringPointer)) {

                    // GET AUTOMATION IDENT
                    char ident[MAXSIZEIDENT];

                    if (getIdent(inputString,&stringPointer,ident)==FALSE) {
                        return(FALSE);
                    }
                    returnedObject->Action.append(QString("AUTOFF ")).append(QString(ident));

                    return(TRUE);
                }

                // TRACK
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)TRACK, &stringPointer)) {

                    // get TRACK number
                    char Number;

                    if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);
                    returnedObject->Action.append(QString("TRACK ")).append(QString::number(Number)).append(QString(" "));


                    // SPEED
                    if (!getToken(&inputString[stringPointer], (char*)SPEED, &stringPointer)) return(FALSE);

                    // KNOB0
                    keepStringPointer = stringPointer;
                    if (getToken(&inputString[stringPointer], (char*)KNOB0, &stringPointer)) {
                        returnedObject->Action.append(QString("SPEED KNOB0 "));

                    }
                    else {
                        // KNOB1
                        stringPointer = keepStringPointer;
                        if (getToken(&inputString[stringPointer], (char*)KNOB1, &stringPointer)) {
                            returnedObject->Action.append(QString("SPEED KNOB1 "));
                        }
                        else {
                            stringPointer = keepStringPointer;
                            // get TRACK speed
                            char Speed;

                            if (!getValue(&inputString[stringPointer], &Speed, &stringPointer)) return(FALSE);
                            returnedObject->Action.append(QString("SPEED ").append(QString::number(Speed)).append(QString(" ")));
                        }
                    }
                    // GET FORW or BACK
                    keepStringPointer = stringPointer;
                    if (getToken(&inputString[stringPointer], (char*)FORW, &stringPointer)) {
                        returnedObject->Action.append(QString("FORW "));
                    }
                    else {
                        stringPointer = keepStringPointer;
                        if (getToken(&inputString[stringPointer], (char*)BACK, &stringPointer)) {
                            returnedObject->Action.append(QString("BACK "));

                        }
                        else return(FALSE);
                    }

                    // get INERTIA
                    if (!getToken(&inputString[stringPointer], (char*)INERTIA, &stringPointer)) return(FALSE);

                    // KNOB0
                    keepStringPointer = stringPointer;
                    if (getToken(&inputString[stringPointer], (char*)KNOB0, &stringPointer)) {
                        returnedObject->Action.append(QString("INERTIA KNOB0 "));

                    }
                    else {
                        // KNOB1
                        stringPointer = keepStringPointer;
                        if (getToken(&inputString[stringPointer], (char*)KNOB1, &stringPointer)) {
                            returnedObject->Action.append(QString("INERTIA KNOB1 "));

                        }
                        else {
                            stringPointer = keepStringPointer;
                            // get INERTIA value
                            char Value;

                            if (!getValue(&inputString[stringPointer], &Value, &stringPointer)) return(FALSE);
                            returnedObject->Action.append(QString("INERTIA ").append(QString::number(Value)));

                        }
                    }
                    return(TRUE);
                }

                // MANUAL0
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)MANUAL0, &stringPointer)) {
                    returnedObject->Action.append(QString("MANUAL0"));
                    return(TRUE);
                }

                // MANUAL1
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)MANUAL1, &stringPointer)) {
                    returnedObject->Action.append(QString("MANUAL1"));
                    return(TRUE);
                }

                // MANUAL2
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)MANUAL2, &stringPointer)) {
                    returnedObject->Action.append(QString("MANUAL2"));
                    return(TRUE);
                }

                // MANUAL3
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)MANUAL3, &stringPointer)) {
                    returnedObject->Action.append(QString("MANUAL3"));
                    return(TRUE);
                }
                // MANUAL
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)MANUAL, &stringPointer)) {
                    returnedObject->Action.append(QString("MANUAL"));

                    return(TRUE);
                }
                // AUTOMATIC
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)AUTOMATIC, &stringPointer)) {
                    returnedObject->Action.append(QString("AUTOMATIC"));
                    return(TRUE);
                }
                // DCC
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)DCC, &stringPointer)) {

                    // Get DCC Address
                    char Address;

                    if (!getValue(&inputString[stringPointer], &Address, &stringPointer))return(FALSE);

                    // get DCC Command
                    char Command;

                    if (!getValue(&inputString[stringPointer], &Command, &stringPointer))return(FALSE);
                    returnedObject->Action.append(QString("DCC ")).append(QString::number(Address)).append(QString(" ")).append(QString::number(Command));

                    return(TRUE);
                }
                return(FALSE);
            }
            else return(FALSE);

        }

        // DEL
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)DEL, &stringPointer)) {

            // get Automation number
            char Number;

            if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);
            return(TRUE);
        }
    }

    //COM
    stringPointer = 0;
    if (getToken(&inputString[stringPointer], (char*)COM, &stringPointer)) {

        // Get board number
        char Number;

        if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

        // GPIO
        keepStringPointer = stringPointer;
        if (getToken(&inputString[stringPointer], (char*)GPIO, &stringPointer)) {

            // get GPIO number
            char Number;

            if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

            // get GPIO level
            char Level;

            if (!getValue(&inputString[stringPointer], &Level, &stringPointer)) return(FALSE);
            return(TRUE);
        }

        // TIMER
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)TIMER, &stringPointer)) {

            // get TIMER number
            char Number;

            if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

            // get TIMER delay
            char Delay;

            if (!getValue(&inputString[stringPointer], &Delay, &stringPointer)) return(FALSE);
            return(TRUE);
        }

        // LPO
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)LPO, &stringPointer)) {

            // get LPO number
            char Number;

            if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

            // get LPO level
            char Level;

            if (!getValue(&inputString[stringPointer], &Level, &stringPointer)) return(FALSE);
            return(TRUE);
        }

        // AUTON
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)AUTON, &stringPointer)) {

            // GET AUTOMATION IDENT
            char ident[MAXSIZEIDENT];

            if (getIdent(inputString,&stringPointer,ident)==FALSE) {
                return(FALSE);
            }
            return(TRUE);
        }

        // AUTOFF
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)AUTOFF, &stringPointer)) {

            // GET AUTOMATION IDENT
            char ident[MAXSIZEIDENT];

            if (getIdent(inputString,&stringPointer,ident)==FALSE) {
                return(FALSE);
            }
            return(TRUE);
        }

        // TRACK
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)TRACK, &stringPointer)) {

            // get TRACK number
            char Number;

            if (!getValue(&inputString[stringPointer], &Number, &stringPointer)) return(FALSE);

            // SPEED
            if (!getToken(&inputString[stringPointer], (char*)SPEED, &stringPointer)) return(FALSE);

            // KNOB0
            keepStringPointer = stringPointer;
            if (getToken(&inputString[stringPointer], (char*)KNOB0, &stringPointer)) {
            }
            else {
                // KNOB1
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)KNOB1, &stringPointer)) {
                }
                else {
                    stringPointer = keepStringPointer;
                    // get TRACK speed
                    char Speed;

                    if (!getValue(&inputString[stringPointer], &Speed, &stringPointer)) return(FALSE);
                }
            }

            // get FORW or BACK
            keepStringPointer = stringPointer;
            if (getToken(&inputString[stringPointer], (char*)FORW, &stringPointer)) {
            }
            else {
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)BACK, &stringPointer)) {
                }
                else return(FALSE);
            }

            // INERTIA
            if (!getToken(&inputString[stringPointer], (char*)INERTIA, &stringPointer)) return(FALSE);

            // KNOB0
            keepStringPointer = stringPointer;
            if (getToken(&inputString[stringPointer], (char*)KNOB0, &stringPointer)) {
            }
            else {
                // KNOB1
                stringPointer = keepStringPointer;
                if (getToken(&inputString[stringPointer], (char*)KNOB1, &stringPointer)) {
                }
                else {
                    stringPointer = keepStringPointer;
                    // get INERTIA value
                    char Value;

                    if (!getValue(&inputString[stringPointer], &Value, &stringPointer)) return(FALSE);
                }
            }
            return(TRUE);
        }

        // MANUAL0
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)MANUAL0, &stringPointer)) {
            return(TRUE);
        }

        // MANUAL
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)MANUAL, &stringPointer)) {
            return(TRUE);
        }

        // AUTOMATIC
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)AUTOMATIC, &stringPointer)) {
            return(TRUE);
        }
        // DCC
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)DCC, &stringPointer)) {

            // Get DCC Address
            char Address;

            if (!getValue(&inputString[stringPointer], &Address, &stringPointer)) return(FALSE);

            // get DCC Command
            char Command;

            if (!getValue(&inputString[stringPointer], &Command, &stringPointer))  {
                return(FALSE);
            }
            return(TRUE);
        }

        // GSTAT
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)GSTAT, &stringPointer)) {
            return(TRUE);
        }
        // LSTAT
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)LSTAT, &stringPointer)) {
            return(TRUE);
        }
        // TSTAT
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)TSTAT, &stringPointer)) {
            return(TRUE);
        }
        // BSTAT
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)BSTAT, &stringPointer)) {
            return(TRUE);
        }
        // AUTLIST
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)AUTLIST, &stringPointer)) {
            return(TRUE);
        }
        // DUMP
        stringPointer = keepStringPointer;
        if (getToken(&inputString[stringPointer], (char*)DUMP, &stringPointer)) {
            return(TRUE);
        }
    }
    return(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// InitRequest
/////////////////////////////////////////////////////////////////////////////
void parser::initRequest(unsigned char* request) {
    unsigned char dataCounter;
    for (dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++)request[dataCounter]=0;
}

/////////////////////////////////////////////////////////////////////////////
// uncompressData
/////////////////////////////////////////////////////////////////////////////
unsigned char parser::uncompressData(unsigned char* data) {
    unsigned char dataCounter;
    unsigned char tmpDataCounter;
    unsigned char repeat;

    // Codage is simply a list of (X,Y) where X is the number of Y. If X = 0 there is no more data
    tmpDataCounter=0;
    for(dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++) gl_tmpBuffer[dataCounter]=0;

    for(dataCounter=0;dataCounter<MAXTRAMESIZE-2;dataCounter+=2) {
        if(data[dataCounter]==0) break;
        for(repeat=0;repeat<data[dataCounter];repeat++) {
            if (tmpDataCounter>=REQUESTSIZE) {
                initRequest(data);
                return(FALSE);
            }
            gl_tmpBuffer[tmpDataCounter++]=data[dataCounter+1];
        }
    }
    for(dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++) data[dataCounter]=gl_tmpBuffer[dataCounter];
    return(TRUE);
}

/*****************************************************************************/
/* getInputRequestFromCAN() */
/*****************************************************************************/
unsigned char parser::getInputRequestFromCAN(unsigned char* request,int* mode) {

    char			requestTrameEnd;
    char			printTrameEnd;

    unsigned char	dataInCounter;
    unsigned char	dataStructureCounter;

    int             tmpDataCounter=0;

    for(int bufferNumber=0;bufferNumber<MAXINPUTCANBUFFER;bufferNumber++) {

        while (gl_getDataCANPointer[bufferNumber]!=gl_InputBufferPointer[bufferNumber]) {

            if (gl_inputBuffer[bufferNumber][gl_getDataCANPointer[bufferNumber]]==(unsigned char)TRAMEREQUESTHEADER) gl_requestHeaderTrameDetected[bufferNumber]++;	else gl_requestHeaderTrameDetected[bufferNumber]=0;
            if (gl_inputBuffer[bufferNumber][gl_getDataCANPointer[bufferNumber]]==(unsigned char)TRAMEREQUESTFOOTER) gl_requestFooterTrameDetected[bufferNumber]++;	else gl_requestFooterTrameDetected[bufferNumber]=0;

            if (gl_inputBuffer[bufferNumber][gl_getDataCANPointer[bufferNumber]]==(unsigned char)TRAMEPRINTHEADER) 	gl_printHeaderTrameDetected[bufferNumber]++;		else gl_printHeaderTrameDetected[bufferNumber]=0;
            if (gl_inputBuffer[bufferNumber][gl_getDataCANPointer[bufferNumber]]==(unsigned char)TRAMEPRINTFOOTER) 	gl_printFooterTrameDetected[bufferNumber]++;		else gl_printFooterTrameDetected[bufferNumber]=0;

            // REQUEST HEADER
            if (gl_requestHeaderTrameDetected[bufferNumber]==8) {
                gl_canMode[bufferNumber]=CAN_REQUEST;
                gl_requestHeaderTrameDetected[bufferNumber]=0;
                gl_requestTrameStart[bufferNumber]=gl_getDataCANPointer[bufferNumber]+1;
                if (gl_requestTrameStart[bufferNumber]>=MAXTRAMESIZE)gl_requestTrameStart[bufferNumber]=0;
            }

            // PRINT HEADER
            else if (gl_printHeaderTrameDetected[bufferNumber]==8) {
                gl_printHeaderTrameDetected[bufferNumber]=0;
                gl_printTrameStart[bufferNumber]=gl_getDataCANPointer[bufferNumber]+1;
                if (gl_printTrameStart[bufferNumber]>=MAXTRAMESIZE)gl_printTrameStart[bufferNumber]=0;
            }

            // REQUEST FOOTER
            else if (gl_requestFooterTrameDetected[bufferNumber]==8 && gl_canMode[bufferNumber]==CAN_REQUEST) {
                requestTrameEnd=gl_getDataCANPointer[bufferNumber]-7;
                if (requestTrameEnd<0)requestTrameEnd+=MAXTRAMESIZE;
                dataInCounter=gl_requestTrameStart[bufferNumber];
                dataStructureCounter=0;
                while(dataInCounter!=requestTrameEnd) {
                    request[dataStructureCounter++]=gl_inputBuffer[bufferNumber][dataInCounter++];
                    if (dataInCounter>=MAXTRAMESIZE)dataInCounter=0;
                    if (dataStructureCounter>=REQUESTSIZE)return(FALSE);
                }
                gl_canMode[bufferNumber]=CAN_FREE; // If more data arrive.... we delete this trame
                gl_getDataCANPointer[bufferNumber]++;
                if (gl_getDataCANPointer[bufferNumber]>=MAXTRAMESIZE)gl_getDataCANPointer[bufferNumber]=0;
                gl_requestHeaderTrameDetected[bufferNumber]=0;
                gl_printHeaderTrameDetected[bufferNumber]=0;
                gl_requestFooterTrameDetected[bufferNumber]=0;
                gl_printFooterTrameDetected[bufferNumber]=0;
                *mode=CAN_REQUEST;
                return(uncompressData(request)); // Mean request available to proceed
            }

            // PRINT FOOTER
            else if (gl_printFooterTrameDetected[bufferNumber]==8 && gl_canMode[bufferNumber]==CAN_PRINT) {
                qDebug() <<"PRINT FOOTER";

                printTrameEnd=gl_getDataCANPointer[bufferNumber]-7;
                if (printTrameEnd<0)printTrameEnd+=MAXTRAMESIZE;
                dataInCounter=gl_printTrameStart[bufferNumber];
                tmpDataCounter=0;
                request[tmpDataCounter]='\0';
                while(dataInCounter!=printTrameEnd) {
                    request[tmpDataCounter++]=gl_inputBuffer[bufferNumber][dataInCounter++];
                    if (dataInCounter>=MAXTRAMESIZE)dataInCounter=0;
                    if (tmpDataCounter>=REQUESTSIZE){
                        gl_requestHeaderTrameDetected[bufferNumber]=0;
                        gl_printHeaderTrameDetected[bufferNumber]=0;
                        gl_requestFooterTrameDetected[bufferNumber]=0;
                        gl_printFooterTrameDetected[bufferNumber]=0;
                        return(FALSE); // ERROR
                    }
                }
                request[tmpDataCounter]='\0';

                gl_canMode[bufferNumber]=CAN_FREE; // If more data arrive.... we continue to get data
                gl_getDataCANPointer[bufferNumber]++;
                if (gl_getDataCANPointer[bufferNumber]>=MAXTRAMESIZE)gl_getDataCANPointer[bufferNumber]=0;
                gl_requestHeaderTrameDetected[bufferNumber]=0;
                gl_printHeaderTrameDetected[bufferNumber]=0;
                gl_requestFooterTrameDetected[bufferNumber]=0;
                gl_printFooterTrameDetected[bufferNumber]=0;
                *mode=CAN_PRINT;
                return(TRUE); // Mean no more data to print
            }

            // read new data
            gl_getDataCANPointer[bufferNumber]++;
            if (gl_getDataCANPointer[bufferNumber]>=MAXTRAMESIZE)gl_getDataCANPointer[bufferNumber]=0;
        }
    }
    return(FALSE); // Nothing to do
}


