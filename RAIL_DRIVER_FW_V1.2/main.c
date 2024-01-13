/*********************************************************************
*
* DRIVER RAIL V 1.2
*
* MAIN.C
*
*********************************************************************
* Processor: PIC18F4585
* Frequency: 32 Mhz
* Compiler: C18
*********************************************************************/

#include "main.h"
//////////////////////////////////////////////////////////////////////////////
// EEPROM READ / WRITE FUNCTION 
//////////////////////////////////////////////////////////////////////////////
/**********************************************************
	This function reads a byte at given addresse in EEPROM.
	IN:		address
	OUT:	data
	Return Value: ERROR, SUCCESS
**********************************************************/
unsigned char ReadEEPROM(unsigned int adr, unsigned char *data){

	if(adr > 0x3FF){
		return(ERROR);
	}
	else{
		EEADR = adr&0xFF;
		EEADRH = (adr>>8) & 0x3;
		EECON1bits.EEPGD = 0;	// Point to data memory
		EECON1bits.CFGS = 0;	// Access EEPROM
		EECON1bits.RD = 1;		// Read data
		*data = EEDATA;			// Load data
		return(SUCCESS);
	}
} // end of ReadEEPROM()

/****************************************************************************
*
* Function ResetEEPROM
*
****************************************************************************/
void ResetEEPROM(){

	 unsigned char 	value;
	 unsigned short adr;
	 unsigned char  checkMagicNumberCounter;

	// Init EEPROM after flashing the board
	adr=(unsigned short)MAGICNUMBER_ADDRESS;
	for (checkMagicNumberCounter=0;checkMagicNumberCounter<MAGICNUMBERSIZE;checkMagicNumberCounter++) {
		WriteEEPROM(adr++,checkMagicNumberCounter);
	}
	// Set ANA mode
	adr=(unsigned short)MODE_ADDRESS;
	value=ANAValue;
	WriteEEPROM(adr,value);

	// No automation
	adr=(unsigned short)NEXTTAUTOMATION_ADDRESS;
	value=0;
	WriteEEPROM(adr,value);

	gl_mutex=1;gl_boardMode = ANAValue;gl_nexAvailableAutomation=0;gl_mutex=0;

	// GPIO IN
	for(adr=(unsigned short)GPIO0DIR_ADDRESS;adr<=(unsigned short)GPIO0DIR_ADDRESS+3;adr++)WriteEEPROM(adr,1);
	
	// Init
	initSignal();

	// Calibration
	calibration();
}

/****************************************************************************
*
* Function ReadEEPROMConfig
*
****************************************************************************/
void ReadEEPROMConfig(void) {

	 unsigned char 	value;
	 unsigned short adr;
	 unsigned char  automationCounter;
	 unsigned char  automationDataCounter;
	 unsigned char  checkMagicNumberCounter;

	// Read MAGIC NUMBER
	adr=(unsigned char)MAGICNUMBER_ADDRESS;
	for (checkMagicNumberCounter=0;checkMagicNumberCounter<MAGICNUMBERSIZE;checkMagicNumberCounter++) {
		ReadEEPROM(adr++,&value);
		if (value!=checkMagicNumberCounter) {
			
			ResetEEPROM();
			return;
		}
	}	

	// Read in EEPROM MODE 
	adr=(unsigned short)MODE_ADDRESS;
	ReadEEPROM(adr,&value);
	gl_mutex=1;gl_boardMode=value;gl_mutex=0;

	// Read in GPIO dir
	adr=(unsigned short)GPIO0DIR_ADDRESS;
	ReadEEPROM(adr++,&value);
	TRISDbits.RD1=value;
	ReadEEPROM(adr++,&value);
	TRISDbits.RD2=value;
	ReadEEPROM(adr++,&value);
	TRISDbits.RD3=value;
	ReadEEPROM(adr,&value);
	TRISCbits.RC4=value;
	
	// Read in EEPROM last automation
	uncompressAutomation();
}

/**********************************************************
	This function informs on if write to EEPROM is completed.
	IN:		None
	OUT:	None
	Return Value: IN_PROGRESS, SUCCESS
**********************************************************/
unsigned char WriteCompletedEEPROM(void) {

	if(PIR2bits.EEIF){
		PIR2bits.EEIF=0;		// Clear write complete flag
		EECON1bits.WREN = 0;	// Disable write
		return(SUCCESS);		// Write to EEPROM completed
	}
	else {
		return(ERROR);			// Write to EEPROM not completed
	}
}

/**********************************************************
	This function informs on if it is possible to write in EEPROM.
	IN:		None
	OUT:	None
	Return Value: ERROR, SUCCESS
**********************************************************/
unsigned char WriteRdyEEPROM(void){

	if(!EECON1bits.WR) {
		return(SUCCESS);	// New Write Enabled
	}
	else {
		return(ERROR);		// new Write Disabled
	}
}

/**********************************************************
	This function writes a byte at given addresse in EEPROM.
	IN:		addresse, data
	Return Value: ERROR, SUCCESS
**********************************************************/
unsigned char WriteEEPROM(unsigned short adr, unsigned char data){
	if(adr > 0x3FF){
		return(ERROR);
	}
	else{

		// Wait eeprom ready to be written
		while (WriteRdyEEPROM()==(unsigned char) ERROR);

		EEADR = adr&0xFF;			// Address of the data in EEPROM
		EEADRH = (adr>>8) & 0x3;	// Address of the data in EEPROM
		EEDATA = data;				// Data to write in EEPROM
		EECON1bits.EEPGD = 0;		// Point to data memory
		EECON1bits.CFGS = 0;		// Access EEPROM
		EECON1bits.WREN = 1;		// Enable write
		INTCONbits.GIE	= 0;		// Disable Interrupt
		EECON2 = 0x55;
		EECON2 = 0x0AA;
		EECON1bits.WR = 1;			// Begin write

		// Wait data written
		while (WriteCompletedEEPROM()==(unsigned char) IN_PROGRESS);
		while (WriteRdyEEPROM()==(unsigned char) ERROR);

		INTCONbits.GIE		= 1;	// Enable Interrupt
	
		return(SUCCESS);
	}
} // end of WriteEEPROM()

/////////////////////////////////////////////////////////////////////////////
// memAvailable
/////////////////////////////////////////////////////////////////////////////
void memAvailable() {

	sprintf(gl_message,"Memory available %d %",uncompressAutomation());
	prompt(gl_message);
	if (gl_nexAvailableAutomation>0) {
		if (gl_nexAvailableAutomation==1)sprintf(gl_message,"%d automation",gl_nexAvailableAutomation);
		else sprintf(gl_message,"%d automations",gl_nexAvailableAutomation); 
		prompt(gl_message);
	}
	sprintf(gl_message,"");
	prompt(gl_message);
}

/////////////////////////////////////////////////////////////////////////////
// uncompressAutomation
/////////////////////////////////////////////////////////////////////////////
unsigned char uncompressAutomation() {

	unsigned short quantityValue;
    unsigned char  automationCounter;
    unsigned char  automationDataCounter;
	unsigned char  value;
	unsigned short adr;
	long		   percentage;

	// Get next automation address
	adr=(unsigned short)NEXTTAUTOMATION_ADDRESS;
	ReadEEPROM(adr,&value);
	gl_nexAvailableAutomation=value;
	if (gl_nexAvailableAutomation==0) return((double)100); // nothing to do

	// Read and uncompress automation from EEPROM
	adr=(unsigned short)AUTOMATION_ADDRESS;
	ReadEEPROM(adr++,&value);
	quantityValue=value;
	ReadEEPROM(adr++,&value);

	for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {	
		for(automationDataCounter=0;automationDataCounter<AUTOMATIONSIZE;automationDataCounter++) {
			gl_automation[automationCounter][automationDataCounter]=value;
			if (quantityValue>0)quantityValue--;
			if (quantityValue==0) {
				ReadEEPROM(adr++,&value);
				quantityValue=value;
				ReadEEPROM(adr++,&value);
			}
			if (quantityValue==0 || adr>=1024) {
				percentage=100*(1024-(long)adr)/(1024-(long)AUTOMATION_ADDRESS);
				return((unsigned char)percentage);
			}
		}
	}
	percentage=100*(1024-(long)adr)/(1024-(long)AUTOMATION_ADDRESS);
	return((unsigned char)percentage);
}

/////////////////////////////////////////////////////////////////////////////
// compressAutomation
/////////////////////////////////////////////////////////////////////////////
unsigned char compressAutomation() {

    unsigned char  automationCounter;
    unsigned char  automationDataCounter;
	unsigned short dataCounter;
	unsigned short dataEeprom;
	unsigned short quantityValue;
	unsigned char  curValue;
	unsigned short adr;
	unsigned char  value;
	unsigned char  checkValue;


	// Codage is simply a list of (X,Y) where X is the number of Y. 
	dataCounter=0;
	curValue=gl_automation[0][0];
	quantityValue=0;

	// Check size
	for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {	
		for(automationDataCounter=0;automationDataCounter<AUTOMATIONSIZE;automationDataCounter++) {
			if (gl_automation[automationCounter][automationDataCounter]==curValue) {
				quantityValue++;
				if (quantityValue>=256) {
					gl_parserErrorCode=AUTOMATIONSIZELIMIT;
					return(FALSE);
				}
			}
			else {
				if (dataCounter<=(unsigned short)(1021-AUTOMATION_ADDRESS)) {
					dataCounter+=2;
				}
				else {
					gl_parserErrorCode=AUTOMATIONSIZELIMIT;
					return(FALSE);
				}
				quantityValue=1;
				curValue=gl_automation[automationCounter][automationDataCounter];
			}
		}
	}

	// Write to eeprom
	curValue=gl_automation[0][0];
	quantityValue=0;
	adr=(unsigned short)AUTOMATION_ADDRESS;

	for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {	
		for(automationDataCounter=0;automationDataCounter<AUTOMATIONSIZE;automationDataCounter++) {
			if (gl_automation[automationCounter][automationDataCounter]==curValue) {
				quantityValue++;
			}
			else {
				value=quantityValue;
				ReadEEPROM(adr,&checkValue);
				if (checkValue!=value)WriteEEPROM(adr++,value);else adr++;
				value=curValue;
				ReadEEPROM(adr,&checkValue);
				if (checkValue!=value)WriteEEPROM(adr++,value);else adr++;
				quantityValue=1;
				curValue=gl_automation[automationCounter][automationDataCounter];
			}
		}
	}
	// Write end of automation list (no more data)
	if (adr<=1021) {

		value=quantityValue;
		ReadEEPROM(adr,&checkValue);
		if (checkValue!=value)WriteEEPROM(adr++,value);else adr++;
		value=curValue;
		ReadEEPROM(adr,&checkValue);
		if (checkValue!=value)WriteEEPROM(adr++,value);else adr++;

		value=0;
		ReadEEPROM(adr,&checkValue);
		if (checkValue!=value)WriteEEPROM(adr++,value);else adr++;
	}

	// update in EEPROM next automation value
	adr=(unsigned short)NEXTTAUTOMATION_ADDRESS;
	value=gl_nexAvailableAutomation;
	ReadEEPROM(adr,&checkValue);
	if (checkValue!=value)WriteEEPROM(adr++,value);
	return(TRUE);	
}

//////////////////////////////////////////////////////////////////////////////
// PARSER AND REQUEST MANAGEMENT
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// isAdigit
/////////////////////////////////////////////////////////////////////////////
unsigned char isAdigit(unsigned char car) {
	if (car >='0' && car <= '9') return TRUE;
	else return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// isAhexaDigit
/////////////////////////////////////////////////////////////////////////////
unsigned char isAhexaDigit(unsigned char car) {
	if ((car >='a' && car <= 'f')  || (car >='A' && car <= 'F')) return TRUE;
	else return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// toUpperCase
/////////////////////////////////////////////////////////////////////////////
unsigned char toUpperCase(unsigned char car) {

    unsigned char returnValue;

	if (car >='a' && car <= 'z') returnValue=car - 0x20;
	else returnValue=car;

	return(returnValue);
}
/////////////////////////////////////////////////////////////////////////////
// strtol_with_atoi for positive number
/////////////////////////////////////////////////////////////////////////////
unsigned short strtol_with_atoi(const char* nptr, short base) {

	unsigned short result=0;
	unsigned char getData=FALSE;

	// Check sign
	if (*nptr == '-') {
		errno = ERANGE;
		return result;
	}
	else if (*nptr == '+') {
		nptr++;
	}

	// Check base 0x for hexadécimal, 0 for octal
	if (base == 0) {
		if (*nptr == '0') {
			nptr++;
			getData=TRUE;
			if (*nptr == '\0'|| *nptr == ' ') return result; // 0
			if (*nptr == 'x' || *nptr == 'X') {
				nptr++;
				base = 16;
				getData=FALSE;
			}
		}
		else {
			base = 10;
		}
	}

	// atoi()
	result = 0;
	while (*nptr != '\0' && *nptr != ' ') {
		int digit;
		if (isAdigit(*nptr)) {
			digit = *nptr - '0';
			getData=TRUE;
		}
		else if (base == 16 && isAhexaDigit(*nptr)) {
			digit = toUpperCase(*nptr) - 'A' + 10;
			getData=TRUE;
		}
		else {
			// end converstion
			break;
		}

		// Check value for base
		if (digit >= base) {
			// Fin de la conversion
			errno = ERANGE;
			return result;
		}

		result = result * base + digit;
		nptr++;
	}
	if (getData==FALSE) errno = EINVAL;
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// traceError 
/////////////////////////////////////////////////////////////////////////////
void traceError() {

	sprintf(gl_message,"Error 0x%x",gl_parserErrorCode);

	switch (gl_parserErrorCode) {	
		case UNKNOWN_TOKEN				:		sprintf(gl_message,"Unknown token");break;
		case NUMBER_MISSING				:		sprintf(gl_message,"Number missing");break;
		case INCOMPLETE_REQUEST			:		sprintf(gl_message,"Incomplete request");break;
		case BAD_NUMBER					:		sprintf(gl_message,"Bad number");break;
		case MODE_MISSING				:		sprintf(gl_message,"Mode missing");break;
		case BAD_GPIO_NUMBER			:		sprintf(gl_message,"Bad GPIO number");break;
		case BAD_TIMER_NUMBER			:		sprintf(gl_message,"Bad TIMER number");break;
		case BAD_TIMER_VALUE			:		sprintf(gl_message,"Bad TIMER value");break;
		case BAD_LPO_NUMBER				:		sprintf(gl_message,"Bad LPO number");break;
		case BAD_AUT_STATUS				:		sprintf(gl_message,"Bad Automation status");break;
		case BAD_AUT_IDENT				:		sprintf(gl_message,"Bad Automation name");break;
		case MISSING_IDENT				:		sprintf(gl_message,"Automation name missing");break;
		case BAD_GPIO_DIR				:		sprintf(gl_message,"Bad GPIO direction");break;
		case BAD_GPIO_LEVEL				:		sprintf(gl_message,"Bad GPIO level");break;
		case BAD_LPO_LEVEL				:		sprintf(gl_message,"Bad LPO level");break;
		case BAD_TRACK_SPEED			:		sprintf(gl_message,"Bad track speed");break;
		case BAD_TRACK_DIR				:		sprintf(gl_message,"Bad track direction");break;
		case BAD_TRACK_NUMBER			:		sprintf(gl_message,"Bad track number");break;
		case BAD_BOARD_MODE					:	sprintf(gl_message,"Bad board mode");break;
		case WRONG_BOARD_NUMBER			:		sprintf(gl_message,"Wrong board number");break;
		case AUTOMATIONSIZELIMIT		:		sprintf(gl_message,"Automation limit reached");break;
		case MISSING_SPACE				:		sprintf(gl_message,"Space missing");break;
		case IDENT_TOO_LONG				:		sprintf(gl_message,"Identifier too long");break;
		case AUTOMATIONLREADYEXISTS		:		sprintf(gl_message,"Automation already defined");break;
		case BADAUTOMATIONNUMBER		:		sprintf(gl_message,"Wrong automation number");break;
		case BAD_TRACK_INERTIA			:		sprintf(gl_message,"Bad inertia value");break;
		case BAD_USER_MODE					:	sprintf(gl_message,"Bad user mode");break;
		default : sprintf(gl_message,"Unknown Error 0x%x",gl_parserErrorCode);
	}

	prompt(gl_message);
	sprintf(gl_message,"");
	prompt(gl_message);	
	gl_parserErrorCode=0;
}

/////////////////////////////////////////////////////////////////////////////
// getToken 
// return value TRUE if parsing correct, otherwise FALSE 
/////////////////////////////////////////////////////////////////////////////
unsigned char getToken(unsigned char* inputString, unsigned char* inputToken, unsigned char* stringPointer) {

	 unsigned char carCounter = 0;
	 unsigned char tokenCarPointer = 0;
	 unsigned char testToken[MAXSIZETOKEN];

	for (carCounter=0; carCounter < strlen(inputString); carCounter++) {
		if (inputString[carCounter]==' ') {
			(*stringPointer)++;
			continue; // remove space
		}
		testToken[tokenCarPointer++] = inputString[carCounter];

		(*stringPointer)++;
		if (!strncmp(testToken, inputToken, strlen(inputToken))) {
			return(TRUE);
		}

		// Error
		if (tokenCarPointer == ((strlen(inputToken)<MAXSIZETOKEN-1) ? strlen(inputToken):MAXSIZETOKEN - 1)) {
			break;

		}
	} 
	gl_parserErrorCode = UNKNOWN_TOKEN;
	return(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// getValue 
// return value TRUE if parsing correct, otherwise FALSE 
/////////////////////////////////////////////////////////////////////////////
unsigned char getValue(unsigned char* inputString, unsigned char* Value, unsigned char* stringPointer) {

	 unsigned char carCounter = 0;
	 unsigned char tokenCarPointer = 0;
	 unsigned char dataFound;
	 unsigned short number;

	errno = 0;
	while (inputString[carCounter] == ' ' && carCounter < strlen(inputString)) {
		(*stringPointer)++;
		carCounter++;
	}
	number = strtol_with_atoi(&inputString[carCounter],0);

	if (errno == ERANGE || errno ==EINVAL) dataFound = FALSE;
	else dataFound = TRUE;

	if (number >255) {
		errno == ERANGE;
		gl_parserErrorCode = BAD_NUMBER;
		return(FALSE);
	}
	
	if (dataFound==TRUE) {

		while (inputString[carCounter] != ' ' && carCounter < strlen(inputString)) {
			(*stringPointer)++;
			carCounter++;
		}

		*Value=(unsigned char)number;
		return(TRUE);
	}
	else {
		gl_parserErrorCode = NUMBER_MISSING;
		return(FALSE);
	}
}
/////////////////////////////////////////////////////////////////////////////
// getIdent 
// return value TRUE if parsing correct, otherwise FALSE 
/////////////////////////////////////////////////////////////////////////////
unsigned char getIdent(unsigned char* inputString, unsigned char* stringPointer,unsigned char *ident) {

	 unsigned char getFirstSpace;
	 unsigned char identCounter=0;

	// GET AUTOMATION IDENT
	getFirstSpace=FALSE;
	while(identCounter<MAXSIZEIDENT) {
		if (inputString[*stringPointer]!=' ') {
			if(identCounter==0 && getFirstSpace==FALSE) {
				gl_parserErrorCode = MISSING_SPACE;
				return(FALSE);
			}
			else if (*stringPointer==strlen(inputString)){
				if (identCounter>0) {
					 ident[identCounter]='\0'; // get end of line
					 break;
				}
				else {
					gl_parserErrorCode = MISSING_IDENT;
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
			return(FALSE);
		}
	}
	return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// Parser 
// return value TRUE if parsing correct, otherwise FALSE 
// Semantic analysis on values should be done on char content
/////////////////////////////////////////////////////////////////////////////
unsigned char parser(unsigned char* inputString, unsigned char* request) {

	 unsigned char stringPointer = 0;
	 unsigned char keepStringPointer = 0;
     unsigned char token[MAXSIZETOKEN];

	initRequest(request);

	// STOP
    sprintf(token,STOP);
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		request[REQ_GLOBAL_COMMAND] = STOPValue;
		request[REQ_BOARD_NUMBER] = gl_boardNumber;
		gl_parserErrorCode=0;
		return(TRUE);
	}

	// RUNALL 
	sprintf(token, RUNALL);
	stringPointer = 0;
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		request[REQ_GLOBAL_COMMAND] = RUNALLValue;
		request[REQ_BOARD_NUMBER] = gl_boardNumber;
		gl_parserErrorCode=0;
		return(TRUE);
	}

	// RUN 
	sprintf(token,RUN);
	stringPointer = 0;
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		request[REQ_GLOBAL_COMMAND] = RUNValue;

		// Get board number
		if (!getValue(&inputString[stringPointer], &request[REQ_BOARD_NUMBER], &stringPointer))return(FALSE);
		gl_parserErrorCode=0;
		return(TRUE);
	}

	// RESET
	sprintf(token,RESET);
	stringPointer = 0;
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		request[REQ_GLOBAL_COMMAND] = RESETValue;

		// Get board number
		if (!getValue(&inputString[stringPointer], &request[REQ_BOARD_NUMBER], &stringPointer))return(FALSE);
		gl_parserErrorCode=0;
		return(TRUE);
	}

	//PROG
	stringPointer = 0;
	sprintf(token,PROG);
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		request[REQ_TYPE_ENTRY] = PROGValue;

		// Get board number
		if (!getValue(&inputString[stringPointer], &request[REQ_BOARD_NUMBER], &stringPointer)) return(FALSE);

		// get DCC or ANA or GPIO or AUT or DEL
		keepStringPointer = stringPointer;

		// DCC
		sprintf(token,DCC);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_PROGRAM_REQUEST_SET_BOARD_MODE] = TRUE;
			request[REQ_PROGRAM_REQUEST_BOARD_MODE] = DCCValue;
			gl_parserErrorCode=0;
			return(TRUE);
		}
		//ANA
		sprintf(token,ANA);
		stringPointer = keepStringPointer;
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_PROGRAM_REQUEST_SET_BOARD_MODE] = TRUE;
			request[REQ_PROGRAM_REQUEST_BOARD_MODE] = ANAValue;
			gl_parserErrorCode=0;
			return(TRUE);
		}
		// GPIO
		sprintf(token,GPIO);
		stringPointer = keepStringPointer;
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_PROGRAM_REQUEST_SET_GPIO] = TRUE;

			// get GPIO number
			if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER], &stringPointer)) return(FALSE);
			keepStringPointer = stringPointer;

			// IN
			sprintf(token,IN);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR] = INValue;
				gl_parserErrorCode=0;
				return(TRUE);
			}

			// OUT
			else {
				stringPointer = keepStringPointer;
				sprintf(token,OUT);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR] = OUTValue;
					gl_parserErrorCode=0;
					return(TRUE);
				}
				else return(FALSE);
			}
		}
		// AUT 
		stringPointer = keepStringPointer;
        sprintf(token,AUT);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_PROGRAM_REQUEST_SET_AUTOMATION] = TRUE;

			// GET AUTOMATION IDENT
            if (getIdent(inputString,&stringPointer,&request[REQ_PROGRAM_REQUEST_IDENT])==FALSE) {
				return(FALSE);
			}

			// GET AUTOMATION STATUS FOR MANUAL MODE
			
			// AUTON
			keepStringPointer = stringPointer;
			sprintf(token,AUTON);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				request[REQ_PROGRAM_REQUEST_STATUS_MANUAL] = AUTONValue;
			}
			else {

				// AUTOFF
				stringPointer = keepStringPointer;
				sprintf(token,AUTOFF);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_STATUS_MANUAL] = AUTOFFValue;
				}
				else {
					gl_parserErrorCode=UNKNOWN_TOKEN;
					return(FALSE);
				}
			}

			// By default, automation is active
			request[REQ_PROGRAM_REQUEST_STATUS]=AUTONValue;		

			//BOARD
			sprintf(token,BOARD);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {

				// get BOARD number
				int boardNumber;
				if (!getValue(&inputString[stringPointer], &boardNumber, &stringPointer)) return(FALSE);

				// get GPIO or TIMER or TRACK 
				keepStringPointer = stringPointer;

				// GPIO 
				sprintf(token,GPIO);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_GPIO_EVENT] = TRUE;
					request[REQ_PROGRAM_REQUEST_EVENT_BOARD_GPIO_NUMBER] = boardNumber;

					// get GPIO number
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_EVENT_GPIO_NUMBER], &stringPointer)) return(FALSE);

					// get GPIO level
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_EVENT_GPIO_LEVEL], &stringPointer)) return(FALSE);
				}

				// TIMER 
				else {
					stringPointer = keepStringPointer;
					sprintf(token,TIMER);
					if (getToken(&inputString[stringPointer], token, &stringPointer)) {
						request[REQ_PROGRAM_REQUEST_TIMER_EVENT] = TRUE;
						request[REQ_PROGRAM_REQUEST_EVENT_BOARD_TIMER_NUMBER] = boardNumber;

						// get TIMER number
						if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_EVENT_TIMER_NUMBER], &stringPointer)) return(FALSE);
					}

					// TRACK
					else {
						stringPointer = keepStringPointer;
						sprintf(token,TRACK);
						if (getToken(&inputString[stringPointer], token, &stringPointer)) {
							request[REQ_PROGRAM_REQUEST_TRACK_EVENT] = TRUE;
							request[REQ_PROGRAM_REQUEST_EVENT_BOARD_TRACK_NUMBER] = boardNumber;

							// get TRACK number
							if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_EVENT_TRACK_NUMBER], &stringPointer)) return(FALSE);

							// STA
							sprintf(token,STA);
							if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

							// get ONTRACK or OFFTRACK 
							keepStringPointer = stringPointer;
							sprintf(token,ONTRACK);
							if (getToken(&inputString[stringPointer], token, &stringPointer)) {
								request[REQ_PROGRAM_REQUEST_EVENT_VEHICLE_STATUS] = ONTRACKValue;
							}
							else {
								stringPointer = keepStringPointer;
								sprintf(token,OFFTRACK);
								if (getToken(&inputString[stringPointer], token, &stringPointer)) {
									request[REQ_PROGRAM_REQUEST_EVENT_VEHICLE_STATUS] = OFFTRACKValue;
								}
								else return(FALSE);
							}
						}
						else return(FALSE);
					}
				}

				// ACT
				sprintf(token,ACT);
				if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

				// get GPIO or TIMER or LPO or AUTON or AUTOFF or TRACK or DCC or MANUAL or AUTOMATIC
				keepStringPointer = stringPointer;

				// GPIO
				sprintf(token,GPIO);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_GPIO_SETTING] = TRUE;

					// get GPIO number
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_ACTION_GPIO_SET_NUMBER], &stringPointer)) return(FALSE);

					// get GPIO level
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_ACTION_GPIO_SET_LEVEL], &stringPointer)) return(FALSE);
					gl_parserErrorCode=0;
					return(TRUE);
				}

				// TIMER
				stringPointer = keepStringPointer;
				sprintf(token,TIMER);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_TIMER_SETTING] = TRUE;

					// get TIMER number
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_ACTION_TIMER_SET_NUMBER], &stringPointer)) return(FALSE);

					// get TIMER delay
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_ACTION_TIMER_SET_DELAY], &stringPointer)) return(FALSE);
					gl_parserErrorCode=0;
					return(TRUE);
				}


				// LPO
				stringPointer = keepStringPointer;
				sprintf(token,LPO);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_LPO_SETTING] = TRUE;

					// get LPO number
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGAM_REQUEST_ACTION_LPO_SET_NUMBER], &stringPointer)) return(FALSE);

					// get LPO level
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_ACTION_LPO_SET_LEVEL], &stringPointer)) return(FALSE);
					gl_parserErrorCode=0;
					return(TRUE);
				}

				// AUTON
				stringPointer = keepStringPointer;
				sprintf(token,AUTON);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_AUT_SETTING] = TRUE;

					// GET AUTOMATION IDENT
            		if (getIdent(inputString,&stringPointer,&request[REQ_PROGAM_REQUEST_ACTION_AUT_SET_IDENT])==FALSE) {
						return(FALSE);
					}

					// set AUT status
					request[REQ_PROGRAM_REQUEST_ACTION_AUT_SET_STATUS]=AUTONValue;
					gl_parserErrorCode=0;
					return(TRUE);
				}

				// AUTOFF
				stringPointer = keepStringPointer;
				sprintf(token,AUTOFF);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_AUT_SETTING] = TRUE;

					// GET AUTOMATION IDENT
            		if (getIdent(inputString,&stringPointer,&request[REQ_PROGAM_REQUEST_ACTION_AUT_SET_IDENT])==FALSE) {
						return(FALSE);
					}

					// set AUT status
					request[REQ_PROGRAM_REQUEST_ACTION_AUT_SET_STATUS]=AUTOFFValue;
					gl_parserErrorCode=0;
					return(TRUE);
				}

				// TRACK
				stringPointer = keepStringPointer;
				sprintf(token,TRACK);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_TRACK_SETTING] = TRUE;

					// get TRACK number
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_NUMBER], &stringPointer)) return(FALSE);

					// SPEED
					sprintf(token,SPEED);
					if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

					// KNOB0
					keepStringPointer = stringPointer;
					sprintf(token,KNOB0);
					if (getToken(&inputString[stringPointer], token, &stringPointer)) {
						request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_SPEED]=KNOB0Value; // Value over max value 0xF
					}
					else {
						// KNOB1
						stringPointer = keepStringPointer;
						sprintf(token,KNOB1);
						if (getToken(&inputString[stringPointer], token, &stringPointer)) {
							request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_SPEED]=KNOB1Value; // Value over max value 0xF
						}
						else {
							stringPointer = keepStringPointer;
							// get TRACK speed 
							if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_SPEED], &stringPointer)) return(FALSE);
						}
					}
					// GET FORW or BACK
					keepStringPointer = stringPointer;
					sprintf(token,FORW);
					if (getToken(&inputString[stringPointer], token, &stringPointer)) {
						request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_DIR] = FORWValue;
					}
					else {
						stringPointer = keepStringPointer;
						sprintf(token,BACK);
						if (getToken(&inputString[stringPointer], token, &stringPointer)) {
							request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_DIR] = BACKValue;
						}
						else return(FALSE);
					}

					// get INERTIA
					sprintf(token,INERTIA);
					if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

					// KNOB0
					keepStringPointer = stringPointer;
					sprintf(token,KNOB0);
					if (getToken(&inputString[stringPointer], token, &stringPointer)) {
						request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_INERTIA]=KNOB0Value; // Value over max value 0xF
					}
					else {
						// KNOB1
						stringPointer = keepStringPointer;
						sprintf(token,KNOB1);
						if (getToken(&inputString[stringPointer], token, &stringPointer)) {
							request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_INERTIA]=KNOB1Value; // Value over max value 0xF
						}
						else {
							stringPointer = keepStringPointer;
							// get INERTIA value
							if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_INERTIA], &stringPointer)) return(FALSE);
						}
					}					
					gl_parserErrorCode=0;
					return(TRUE);
				}

				// MANUAL
				stringPointer = keepStringPointer;
				sprintf(token,MANUAL);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_SET_USER_MODE] = TRUE;
					request[REQ_PROGRAM_REQUEST_ACTION_USER_MODE] = MANUALValue;
					gl_parserErrorCode=0;
					return(TRUE);
				}

				// AUTOMATIC
				stringPointer = keepStringPointer;
				sprintf(token,AUTOMATIC);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_SET_USER_MODE] = TRUE;
					request[REQ_PROGRAM_REQUEST_ACTION_USER_MODE] = AUTOMATICValue;
					gl_parserErrorCode=0;
					return(TRUE);
				}

				// DCC
				stringPointer = keepStringPointer;
				sprintf(token,DCC);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_PROGRAM_REQUEST_DCC_SETTING] = TRUE;

					// Get DCC Address
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_ACTION_DCC_ADDRESS_SETTING], &stringPointer))return(FALSE);

					// get DCC Command
					if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_ACTION_DCC_COMMAND_SETTING], &stringPointer))return(FALSE);
					gl_parserErrorCode=0;
					return(TRUE);
				}
				return(FALSE);
			}
			else return(FALSE);

		}

		// DEL
		stringPointer = keepStringPointer;
		sprintf(token,DEL);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_PROGRAM_REQUEST_DEL_AUTOMATION] = TRUE;

			// get Automation number
			if (!getValue(&inputString[stringPointer], &request[REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER], &stringPointer)) return(FALSE);
			gl_parserErrorCode=0;
			return(TRUE);
		}
	}

	//COM
	stringPointer = 0;
	sprintf(token,COM);
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		request[REQ_TYPE_ENTRY] = COMValue;

		// Get board number
		if (!getValue(&inputString[stringPointer], &request[REQ_BOARD_NUMBER], &stringPointer)) return(FALSE);

		// GPIO
		keepStringPointer = stringPointer;
		sprintf(token,GPIO);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			request[REQ_COMMAND_REQUEST_SET_GPIO] = TRUE;

			// get GPIO number
			if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_GPIO_NUMBER], &stringPointer)) return(FALSE);

			// get GPIO level
			if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_GPIO_LEVEL], &stringPointer)) return(FALSE);
			gl_parserErrorCode=0;
			return(TRUE);
		}

		// TIMER
		stringPointer = keepStringPointer;
		sprintf(token,TIMER);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			request[REQ_COMMAND_REQUEST_SET_TIMER] = TRUE;

			// get TIMER number
			if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_TIMER_NUMBER], &stringPointer)) return(FALSE);

			// get TIMER delay
			if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_TIMER_DELAY], &stringPointer)) return(FALSE);
			gl_parserErrorCode=0;
			return(TRUE);
		}

		// LPO 
		stringPointer = keepStringPointer;
		sprintf(token,LPO);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			request[REQ_COMMAND_REQUEST_SET_LPO] = TRUE;

			// get LPO number
			if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_LPO_NUMBER], &stringPointer)) return(FALSE);

			// get LPO level
			if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_LPO_LEVEL], &stringPointer)) return(FALSE);
			gl_parserErrorCode=0;
			return(TRUE);
		}

		// AUTON 
		stringPointer = keepStringPointer;
		sprintf(token,AUTON);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			request[REQ_COMMAND_REQUEST_SET_AUT] = TRUE;

			// GET AUTOMATION IDENT
           	if (getIdent(inputString,&stringPointer,&request[REQ_COMMAND_REQUEST_AUT_IDENT])==FALSE) {
				return(FALSE);
			}

			// set AUT status
			request[REQ_COMMAND_REQUEST_AUT_STATUS]=AUTONValue;
			gl_parserErrorCode=0;
			return(TRUE);
		}

		// AUTOFF
		stringPointer = keepStringPointer;
		sprintf(token,AUTOFF);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			request[REQ_COMMAND_REQUEST_SET_AUT] = TRUE;

			// GET AUTOMATION IDENT
           	if (getIdent(inputString,&stringPointer,&request[REQ_COMMAND_REQUEST_AUT_IDENT])==FALSE) {
				return(FALSE);
			}

			// set AUT status
			request[REQ_COMMAND_REQUEST_AUT_STATUS]=AUTOFFValue;
			gl_parserErrorCode=0;
			return(TRUE);
		}

		// TRACK
		stringPointer = keepStringPointer;
		sprintf(token,TRACK);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			request[REQ_COMMAND_REQUEST_SET_TRACK] = TRUE;

			// get TRACK number
			if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_TRACK_NUMBER], &stringPointer)) return(FALSE);

			// SPEED
			sprintf(token,SPEED);
			if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

			// KNOB0
			keepStringPointer = stringPointer;
			sprintf(token,KNOB0);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				request[REQ_COMMAND_REQUEST_TRACK_SPEED]=KNOB0Value; // Value over max value 0xF
			}
			else {
				// KNOB1
				stringPointer = keepStringPointer;
				sprintf(token,KNOB1);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_COMMAND_REQUEST_TRACK_SPEED]=KNOB1Value; // Value over max value 0xF
				}
				else {
					stringPointer = keepStringPointer;
					// get TRACK speed 
					if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_TRACK_SPEED], &stringPointer)) return(FALSE);
				}
			}

			// get FORW or BACK 
			keepStringPointer = stringPointer;
			sprintf(token,FORW);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				request[REQ_COMMAND_REQUEST_TRACK_DIR] = FORWValue;
			}
			else {
				stringPointer = keepStringPointer;
				sprintf(token,BACK);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_COMMAND_REQUEST_TRACK_DIR] = BACKValue;
				}
				else return(FALSE);
			}

			// INERTIA
			sprintf(token,INERTIA);
			if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

			// KNOB0
			keepStringPointer = stringPointer;
			sprintf(token,KNOB0);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				request[REQ_COMMAND_REQUEST_TRACK_INERTIA]=KNOB0Value; // Value over max value 0xF
			}
			else {
				// KNOB1
				stringPointer = keepStringPointer;
				sprintf(token,KNOB1);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					request[REQ_COMMAND_REQUEST_TRACK_INERTIA]=KNOB1Value; // Value over max value 0xF
				}
				else {
					stringPointer = keepStringPointer;
					// get INERTIA value
					if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_TRACK_INERTIA], &stringPointer)) return(FALSE);
				}
			}					

			gl_parserErrorCode=0;
			return(TRUE);
		}

		// MANUAL
		stringPointer = keepStringPointer;
		sprintf(token,MANUAL);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_COMMAND_REQUEST_SET_USER_MODE] = TRUE;
			request[REQ_COMMAND_REQUEST_USER_MODE] = MANUALValue;
			gl_parserErrorCode=0;
			return(TRUE);
		}

		// AUTOMATIC
		stringPointer = keepStringPointer;
		sprintf(token,AUTOMATIC);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_COMMAND_REQUEST_SET_USER_MODE] = TRUE;
			request[REQ_COMMAND_REQUEST_USER_MODE] = AUTOMATICValue;
			gl_parserErrorCode=0;
			return(TRUE);
		}

		// DCC
		stringPointer = keepStringPointer;
		sprintf(token,DCC);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_COMMAND_REQUEST_SET_DCC]= TRUE;

			// Get DCC Address
			if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_DCC_ADDRESS], &stringPointer)) return(FALSE);

			// get DCC Command
			if (!getValue(&inputString[stringPointer], &request[REQ_COMMAND_REQUEST_DCC_COMMAND], &stringPointer))  {
				return(FALSE);
			}
			gl_parserErrorCode=0;
			return(TRUE);
		}

		// GSTAT
		stringPointer = keepStringPointer;
		sprintf(token,GSTAT);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_COMMAND_REQUEST_GET_GPIO_STATUS] = TRUE;
			gl_parserErrorCode=0;
			return(TRUE);
		}
		// LSTAT
		stringPointer = keepStringPointer;
		sprintf(token,LSTAT);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_COMMAND_REQUEST_GET_LPO_STATUS] = TRUE;
			gl_parserErrorCode=0;
			return(TRUE);
		}
		// TSTAT
		stringPointer = keepStringPointer;
		sprintf(token,TSTAT);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_COMMAND_REQUEST_GET_TRACK_STATUS] = TRUE;
			gl_parserErrorCode=0;
			return(TRUE);
		}
		// BSTAT
		stringPointer = keepStringPointer;
		sprintf(token,BSTAT);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_COMMAND_REQUEST_GET_BOARD_STATUS] = TRUE;
			gl_parserErrorCode=0;
			return(TRUE);
		}
		// AUTLIST
		stringPointer = keepStringPointer;
		sprintf(token,AUTLIST);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_COMMAND_REQUEST_GET_AUTOMATION_LIST] = TRUE;
			gl_parserErrorCode=0;
			return(TRUE);
		}
		// DUMP
		stringPointer = keepStringPointer;
		sprintf(token,DUMP);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			request[REQ_COMMAND_REQUEST_GET_DUMP] = TRUE;
			gl_parserErrorCode=0;
			return(TRUE);
		}
	}
	return(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// uncompressData 
/////////////////////////////////////////////////////////////////////////////
void uncompressData(unsigned char* data) {
	unsigned char dataCounter;
	unsigned char tmpDataCounter;
	unsigned char repeat;

	// Codage is simply a list of (X,Y) where X is the number of Y. If X = 0 there is no more data

	tmpDataCounter=0;
	for(dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++) gl_tmpBuffer[dataCounter]=0;

	for(dataCounter=0;dataCounter<MAXTRAMESIZE-2;dataCounter+=2) {
		if(data[dataCounter]==0) break;
		for(repeat=0;repeat<data[dataCounter];repeat++) {
			gl_tmpBuffer[tmpDataCounter++]=data[dataCounter+1];
			if (tmpDataCounter>REQUESTSIZE) break;
		}
	}
	for(dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++) data[dataCounter]=gl_tmpBuffer[dataCounter];
}

/////////////////////////////////////////////////////////////////////////////
// compressData 
/////////////////////////////////////////////////////////////////////////////
unsigned char compressData(unsigned char* data) {

	unsigned char tmpDataCounter;
	unsigned char dataCounter;
	unsigned char quantityValue;
	unsigned char curValue;

	dataCounter=0;
	tmpDataCounter=0;
	curValue=data[0];
	quantityValue=0;

	while(dataCounter<REQUESTSIZE) {
		while(data[dataCounter]==curValue) {
			quantityValue++;
			dataCounter++;
			if (dataCounter>=REQUESTSIZE) break;
		}
		if (tmpDataCounter<MAXTRAMESIZE-1) {
			gl_tmpBuffer[tmpDataCounter++]=quantityValue;
			gl_tmpBuffer[tmpDataCounter++]=curValue;
		}
		if (tmpDataCounter>=MAXTRAMESIZE) {
			for(dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++) data[dataCounter]=0;
			return(0); // We loose the trame, because we can't compress it, should never happen.....
		}
		curValue=data[dataCounter];
		quantityValue=0;
	}
	for(dataCounter=0;dataCounter<tmpDataCounter;dataCounter++) data[dataCounter]=gl_tmpBuffer[dataCounter];
	for(;dataCounter<REQUESTSIZE;dataCounter++) data[dataCounter]=0;
	return(tmpDataCounter);
}

/////////////////////////////////////////////////////////////////////////////
// InitRequest 
/////////////////////////////////////////////////////////////////////////////
void initRequest(unsigned char* request) {	
	unsigned char dataCounter;
	for (dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++)request[dataCounter]=0;
}


/////////////////////////////////////////////////////////////////////////////
// Save Automation 
/////////////////////////////////////////////////////////////////////////////
unsigned char saveAutomation(unsigned char automationNumber,unsigned char* data) {

    unsigned char  automationCounter;
    unsigned char  automationDataCounter;
	unsigned short adr;
	unsigned char  value;

	if (automationNumber>=MAXAUTOMATION) {
		gl_parserErrorCode=AUTOMATIONSIZELIMIT;
		return(FALSE);
	}

	// copy new automation content in gl_automation
	for(automationDataCounter=0;automationDataCounter<AUTOMATIONSIZE;automationDataCounter++) {
		gl_automation[automationNumber][automationDataCounter]=data[automationDataCounter];
	}
	if (automationNumber==gl_nexAvailableAutomation)gl_nexAvailableAutomation++;

	// update in EEPROM automation
	return(compressAutomation());
}

/////////////////////////////////////////////////////////////////////////////
// assignAutomation 
/////////////////////////////////////////////////////////////////////////////
void assignAutomation(char *request,unsigned char automationCounter) {

		unsigned char identCounter;

		// set request for command
		initRequest(request);

		request[REQ_TYPE_ENTRY]=COMValue;
		request[REQ_BOARD_NUMBER]=gl_boardNumber;
		request[REQ_COMMAND_REQUEST_SET_GPIO] = gl_automation[automationCounter][AUTOMATION_COMMAND_SET_GPIO];
		request[REQ_COMMAND_REQUEST_GPIO_NUMBER] = gl_automation[automationCounter][AUTOMATION_COMMAND_GPIO_NUMBER];
		request[REQ_COMMAND_REQUEST_GPIO_LEVEL] = gl_automation[automationCounter][AUTOMATION_COMMAND_GPIO_LEVEL];

		request[REQ_COMMAND_REQUEST_SET_TIMER] = gl_automation[automationCounter][AUTOMATION_COMMAND_SET_TIMER];
		request[REQ_COMMAND_REQUEST_TIMER_NUMBER] = gl_automation[automationCounter][AUTOMATION_COMMAND_TIMER_NUMBER];
		request[REQ_COMMAND_REQUEST_TIMER_DELAY] = gl_automation[automationCounter][AUTOMATION_COMMAND_TIMER_DELAY];

		request[REQ_COMMAND_REQUEST_SET_LPO] = gl_automation[automationCounter][AUTOMATION_COMMAND_SET_LPO];
		request[REQ_COMMAND_REQUEST_LPO_NUMBER] = gl_automation[automationCounter][AUTOMATION_COMMAND_LPO_NUMBER];
		request[REQ_COMMAND_REQUEST_LPO_LEVEL] = gl_automation[automationCounter][AUTOMATION_COMMAND_LPO_LEVEL];

		request[REQ_COMMAND_REQUEST_SET_AUT] = gl_automation[automationCounter][AUTOMATION_COMMAND_SET_AUT];
		for(identCounter=0;identCounter<MAXSIZEIDENT;identCounter++) request[REQ_COMMAND_REQUEST_AUT_IDENT+identCounter] = gl_automation[automationCounter][AUTOMATION_COMMAND_AUT_IDENT+identCounter];
		request[REQ_COMMAND_REQUEST_AUT_STATUS] = gl_automation[automationCounter][AUTOMATION_COMMAND_AUT_STATUS];

		request[REQ_COMMAND_REQUEST_SET_TRACK] = gl_automation[automationCounter][AUTOMATION_COMMAND_SET_TRACK];
		request[REQ_COMMAND_REQUEST_TRACK_NUMBER] = gl_automation[automationCounter][AUTOMATION_COMMAND_TRACK_NUMBER];
		request[REQ_COMMAND_REQUEST_TRACK_SPEED] = gl_automation[automationCounter][AUTOMATION_COMMAND_TRACK_SPEED];
		request[REQ_COMMAND_REQUEST_TRACK_DIR] = gl_automation[automationCounter][AUTOMATION_COMMAND_TRACK_DIR];
		request[REQ_COMMAND_REQUEST_TRACK_INERTIA] = gl_automation[automationCounter][AUTOMATION_COMMAND_TRACK_INERTIA];

		request[REQ_COMMAND_REQUEST_SET_USER_MODE]= gl_automation[automationCounter][AUTOMATION_COMMAND_SET_USER_MODE];
		request[REQ_COMMAND_REQUEST_USER_MODE]= gl_automation[automationCounter][AUTOMATION_COMMAND_USER_MODE];

		request[REQ_COMMAND_REQUEST_SET_DCC]= gl_automation[automationCounter][AUTOMATION_COMMAND_SET_DCC];
		request[REQ_COMMAND_REQUEST_DCC_ADDRESS] = gl_automation[automationCounter][AUTOMATION_COMMAND_DCC_ADDRESS];
		request[REQ_COMMAND_REQUEST_DCC_COMMAND] = gl_automation[automationCounter][AUTOMATION_COMMAND_DCC_COMMAND];
}

/////////////////////////////////////////////////////////////////////////////
void setSpeed(char speed,char step,unsigned char trackNumber) {
	gl_setPoint[trackNumber]=((int)speed * (int)MAXINTERNALSPEED);
	gl_setStep[trackNumber]=step;
}

/////////////////////////////////////////////////////////////////////////////
// manageRequest 
/////////////////////////////////////////////////////////////////////////////
unsigned char manageRequest (unsigned char* request,unsigned char sendPrompt) {

	 unsigned char automationCounter;	 
	 unsigned char automationDataCounter;

	 unsigned char eventBoardTrackNumber;
	 unsigned char eventTrackNumber;

	 unsigned char eventVehicleStatus;
	 unsigned char statusCounter;
	 unsigned char dataCounter;

	 unsigned char eventBoardGPIONumber;
	 unsigned char eventGPIONumber;
	 unsigned char eventGPIOLevel;

	 unsigned char eventBoardTIMERNumber;
	 unsigned char eventTIMERNumber;

	 unsigned short adr; 
	 unsigned short adrLast;
	 unsigned char value;
	 unsigned char checkValue;
	 unsigned char writeEEPROMCounter;
 	 static unsigned char onTrack[MAXSIZETOKEN];
	 static unsigned char offTrack[MAXSIZETOKEN];

	 unsigned char length;
	 unsigned char i;
	 unsigned char notEqual;
	
	 unsigned char itemPerLine;

	 char speed;
	 char step;
	 unsigned char trackNumber;

     sprintf(gl_message,"");

	// CHECK EVENT FIRST
	if (request[REQ_EVENT_REQUEST_TRACK_EVENT] == TRUE) {

		// Event from this board is sent to the others 
		if (request[REQ_BOARD_NUMBER] == gl_boardNumber) sendRequestToCAN(request);

		// Keep values as request struct should be reset
		eventBoardTrackNumber=request[REQ_EVENT_REQUEST_EVENT_BOARD_TRACK_NUMBER];
		eventTrackNumber=request[REQ_EVENT_REQUEST_EVENT_TRACK_NUMBER];
		eventVehicleStatus=request[REQ_EVENT_REQUEST_EVENT_VEHICLE_STATUS];

		for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
				if(gl_automation[automationCounter][AUTOMATION_EVENT_TRACK_EVENT]==TRUE && gl_automation[automationCounter][AUTOMATION_STATUS]==AUTONValue) {
				if (gl_automation[automationCounter][AUTOMATION_EVENT_EVENT_BOARD_TRACK_NUMBER]==eventBoardTrackNumber &&
					gl_automation[automationCounter][AUTOMATION_EVENT_EVENT_TRACK_NUMBER]==eventTrackNumber &&
					gl_automation[automationCounter][AUTOMATION_EVENT_EVENT_VEHICLE_STATUS]==eventVehicleStatus) {

						// set request for command
						assignAutomation(request,automationCounter);
						if (manageRequest(request,FALSE)==FALSE) return (FALSE);
						else continue;	
					}
				}	
			}	
			return(TRUE);
	}
	else if (request[REQ_EVENT_REQUEST_GPIO_EVENT] == TRUE) {

		// Event from this board is sent to the others 
		if (request[REQ_BOARD_NUMBER] == gl_boardNumber) sendRequestToCAN(request);

		// Keep values as request struct should be reset
		eventBoardGPIONumber=request[REQ_EVENT_REQUEST_EVENT_BOARD_GPIO_NUMBER];
		eventGPIONumber=request[REQ_EVENT_REQUEST_EVENT_GPIO_NUMBER];
		eventGPIOLevel=request[REQ_EVENT_REQUEST_EVENT_GPIO_LEVEL];

		for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
			if(gl_automation[automationCounter][AUTOMATION_EVENT_GPIO_EVENT]==TRUE && gl_automation[automationCounter][AUTOMATION_STATUS]==AUTONValue) {
				if (gl_automation[automationCounter][AUTOMATION_EVENT_EVENT_BOARD_GPIO_NUMBER]==eventBoardGPIONumber &&
					gl_automation[automationCounter][AUTOMATION_EVENT_EVENT_GPIO_NUMBER]==eventGPIONumber &&
					gl_automation[automationCounter][AUTOMATION_EVENT_EVENT_GPIO_LEVEL]==eventGPIOLevel) {

						// set request for command
						assignAutomation(request,automationCounter);
						if (manageRequest(request,FALSE)==FALSE) return (FALSE);
						else continue;	
					}
				}	
			}	
			return(TRUE);
	}

	else if (request[REQ_EVENT_REQUEST_TIMER_EVENT] == TRUE) {

		// Event from this board is sent to the others 
		if (request[REQ_BOARD_NUMBER] == gl_boardNumber) sendRequestToCAN(request);

		// Keep values as request struct should be reset
		eventBoardTIMERNumber=request[REQ_EVENT_REQUEST_EVENT_BOARD_TIMER_NUMBER];
		eventTIMERNumber=request[REQ_EVENT_REQUEST_EVENT_TIMER_NUMBER];

		for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
			if(gl_automation[automationCounter][AUTOMATION_EVENT_TIMER_EVENT]==TRUE && gl_automation[automationCounter][AUTOMATION_STATUS]==AUTONValue) {
				if (gl_automation[automationCounter][AUTOMATION_EVENT_EVENT_BOARD_TIMER_NUMBER]==eventBoardTIMERNumber &&
					gl_automation[automationCounter][AUTOMATION_EVENT_EVENT_TIMER_NUMBER]==eventTIMERNumber) {

						// set request for command
						assignAutomation(request,automationCounter);
						if (manageRequest(request,FALSE)==FALSE) return (FALSE);
						else continue;		
					}
				}	
			}	
			return(TRUE);
	}

	//Global command

    switch (request[REQ_GLOBAL_COMMAND]) {
		case STOPValue: 
			gl_stopAll=TRUE;
			if (request[REQ_BOARD_NUMBER] == gl_boardNumber) sendRequestToCAN(request);
			if (sendPrompt==TRUE) prompt(gl_message);
			return(TRUE);
		case RUNALLValue :
			gl_stopAll=FALSE;
			if (request[REQ_BOARD_NUMBER] == gl_boardNumber) sendRequestToCAN(request);
			if (sendPrompt==TRUE) prompt(gl_message);
			return(TRUE);
		case RUNValue :
			if (request[REQ_BOARD_NUMBER] != gl_boardNumber && gl_master==TRUE) sendRequestToCAN(request);
			else if (request[REQ_BOARD_NUMBER] == gl_boardNumber) gl_stopAll=FALSE;
			if (sendPrompt==TRUE) prompt(gl_message);
		 	return(TRUE); 
		case RESETValue :
			if (request[REQ_BOARD_NUMBER] != gl_boardNumber && gl_master==TRUE) sendRequestToCAN(request);
			else if (request[REQ_BOARD_NUMBER] == gl_boardNumber) ResetEEPROM();
			if (sendPrompt==TRUE) prompt(gl_message);
		 	return(TRUE); 

		default : 

			// Check board number for command or program, on the master side, we forward the request on CAN bus
			if (request[REQ_BOARD_NUMBER] != gl_boardNumber){
				if (gl_master==TRUE) {

					// Send request to CAN
               	 	sendRequestToCAN(request);
					if (sendPrompt==TRUE) prompt(gl_message);
				}
				return(TRUE); // Not for us
			}

			switch(request[REQ_TYPE_ENTRY]) {
			case PROGValue: 
				if (request[REQ_PROGRAM_REQUEST_SET_BOARD_MODE]==TRUE) {
					if(request[REQ_PROGRAM_REQUEST_BOARD_MODE]==DCCValue) {
							gl_mutex=1;	setDcc(0,0); gl_boardMode=DCCValue; gl_mutex=0;
								
							// Save to EEPROM
							adr=(unsigned short)MODE_ADDRESS;
							ReadEEPROM(adr,&checkValue);
							if (checkValue!=gl_boardMode)WriteEEPROM(adr,gl_boardMode);
							if (sendPrompt==TRUE) prompt(gl_message);
							return(TRUE);
					}
					else if(request[REQ_PROGRAM_REQUEST_BOARD_MODE]==ANAValue) {
							gl_mutex=1; gl_boardMode=ANAValue; gl_mutex=0;

							// Save to EEPROM
							adr=(unsigned short)MODE_ADDRESS;
							ReadEEPROM(adr,&checkValue);
							if (checkValue!=gl_boardMode)WriteEEPROM(adr,gl_boardMode);
							if (sendPrompt==TRUE) prompt(gl_message);
							return(TRUE);
					}
					else {
						gl_parserErrorCode=MODE_MISSING;
						return(FALSE);
					}
				}
				if (request[REQ_PROGRAM_REQUEST_SET_GPIO] == TRUE) {		
					if (request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR]==INValue || request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR]==OUTValue) {		
						if (request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER]>=0 && request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER]<=3) {
							gl_mutex=1;
							switch(request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER]) {
								case 0 :TRISDbits.RD1 = request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR];
										adr=(unsigned short)GPIO0DIR_ADDRESS;
										ReadEEPROM(adr,&checkValue);
										if (checkValue!=TRISDbits.RD1)WriteEEPROM(adr,TRISDbits.RD1);
										if(TRISDbits.RD1==0)gl_GPIOchar[0]=1;
										break;
								case 1 :TRISDbits.RD2 = request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR];
										adr=(unsigned short)GPIO1DIR_ADDRESS;
										ReadEEPROM(adr,&checkValue);
										if (checkValue!=TRISDbits.RD2)WriteEEPROM(adr,TRISDbits.RD2);
										if(TRISDbits.RD2==0)gl_GPIOchar[1]=1;
										break;
								case 2 :TRISDbits.RD3 = request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR];
										adr=(unsigned short)GPIO2DIR_ADDRESS;
										ReadEEPROM(adr,&checkValue);
										if (checkValue!=TRISDbits.RD3)WriteEEPROM(adr,TRISDbits.RD3);
										if(TRISDbits.RD3==0)gl_GPIOchar[2]=1;
										break;
								case 3 :TRISCbits.RC4 = request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR];
										adr=(unsigned short)GPIO3DIR_ADDRESS;
										ReadEEPROM(adr,&checkValue);
										if (checkValue!=TRISCbits.RC4)WriteEEPROM(adr,TRISCbits.RC4);
										if(TRISCbits.RC4==0)gl_GPIOchar[3]=1;
										break;
							}

							gl_mutex=0;
							ReadEEPROM(adr,&checkValue);
							if (checkValue!=request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR])WriteEEPROM(request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR],gl_boardMode);
							if (sendPrompt==TRUE) prompt(gl_message);
							return(TRUE);
						}
						else {
							gl_parserErrorCode=BAD_GPIO_NUMBER;
							return(FALSE);
						}
					}
					else {
						gl_parserErrorCode=BAD_GPIO_DIR;
						return(FALSE);
					}											
				}
				if (request[REQ_PROGRAM_REQUEST_SET_AUTOMATION] == TRUE) {
					length=strlen(&request[REQ_PROGRAM_REQUEST_IDENT]);
					for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
						notEqual=0;
						if (strlen(&(gl_automation[automationCounter][AUTOMATION_IDENT]))!=length) continue; // they are not equal (strncmp doesn't work here)
						for (i=0;i<length;i++)if (request[REQ_PROGRAM_REQUEST_IDENT+i]!=gl_automation[automationCounter][AUTOMATION_IDENT+i]) {
							notEqual=1;
							break;
						}
						if (!notEqual) {								
							gl_parserErrorCode=AUTOMATIONLREADYEXISTS;
							return(FALSE);
						}
					}

					if (saveAutomation(gl_nexAvailableAutomation,&request[START_HERE_TO_COPY_FOR_AUTOMATION])==TRUE){
						if (sendPrompt==TRUE) prompt(gl_message);		
						return(TRUE);
						
					}
					else {
						return(FALSE);
					}
				}		
				if (request[REQ_PROGRAM_REQUEST_DEL_AUTOMATION] == TRUE) {
					 if(request[REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER]<gl_nexAvailableAutomation) {
						gl_nexAvailableAutomation--;
						if (request[REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER]==gl_nexAvailableAutomation) {
							compressAutomation();
							if (sendPrompt==TRUE) prompt(gl_message);		
							return(TRUE);
						}
						else if (gl_nexAvailableAutomation>0) {
							if (saveAutomation(request[REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER],&(gl_automation[gl_nexAvailableAutomation][0]))==TRUE){
								if (sendPrompt==TRUE) prompt(gl_message);		
								return(TRUE);
							}
							else return(FALSE);
						}
						else {
							// reset in EEPROM next automation value
							adr=(unsigned short)NEXTTAUTOMATION_ADDRESS;
							gl_nexAvailableAutomation=0;
							value=0;
							ReadEEPROM(adr,&checkValue);
							if (checkValue!=value)WriteEEPROM(adr,value);
							if (sendPrompt==TRUE) prompt(gl_message);	
							return(TRUE);
						}
					}
					else {
						gl_parserErrorCode=BADAUTOMATIONNUMBER;
						return(FALSE);
					}
				}

				break;
				
			case COMValue : 
				if (request[REQ_COMMAND_REQUEST_SET_GPIO]==TRUE) {
					if (request[REQ_COMMAND_REQUEST_GPIO_NUMBER]==0 && TRISDbits.RD1==1) gl_GPIOcounter[0]=0;
					else if (request[REQ_COMMAND_REQUEST_GPIO_NUMBER]==1 && TRISDbits.RD2==1)gl_GPIOcounter[1]=0;
					else if (request[REQ_COMMAND_REQUEST_GPIO_NUMBER]==2 && TRISDbits.RD3==1)gl_GPIOcounter[2]=0;
					else if (request[REQ_COMMAND_REQUEST_GPIO_NUMBER]==3 && TRISCbits.RC4==1)gl_GPIOcounter[3]=0;
					else if (request[REQ_COMMAND_REQUEST_GPIO_NUMBER]>=0 && request[REQ_COMMAND_REQUEST_GPIO_NUMBER]<=3) {
							if (request[REQ_COMMAND_REQUEST_GPIO_LEVEL]==0 ||request[REQ_COMMAND_REQUEST_GPIO_LEVEL]==1 ) {
								gl_mutex=1;	gl_GPIOchar[request[REQ_COMMAND_REQUEST_GPIO_NUMBER]]=request[REQ_COMMAND_REQUEST_GPIO_LEVEL]; gl_mutex=0;
								if (sendPrompt==TRUE) prompt(gl_message);
								return(TRUE);
							}
							else {
								gl_parserErrorCode=BAD_GPIO_LEVEL;
								return(FALSE);
							}
						}
						else {
							gl_parserErrorCode=BAD_GPIO_NUMBER;
							return(FALSE);
						}				
				}
				else if (request[REQ_COMMAND_REQUEST_SET_TIMER]==TRUE) {
					 if (request[REQ_COMMAND_REQUEST_TIMER_NUMBER]>=0 && request[REQ_COMMAND_REQUEST_TIMER_NUMBER]<=MAXTIMER) {
							if (request[REQ_COMMAND_REQUEST_TIMER_DELAY]>0 && request[REQ_COMMAND_REQUEST_TIMER_DELAY]<=MAXTIMERDELAY ) {
								gl_mutex=1;	gl_TIMERValue[request[REQ_COMMAND_REQUEST_TIMER_NUMBER]]=request[REQ_COMMAND_REQUEST_TIMER_DELAY]; gl_mutex=0;
								if (sendPrompt==TRUE) prompt(gl_message);
								return(TRUE);
							}
							else {
								gl_parserErrorCode=BAD_TIMER_VALUE;
								return(FALSE);
							}
						}
						else {
							gl_parserErrorCode=BAD_TIMER_NUMBER;
							return(FALSE);
						}				
				}
				else if (request[REQ_COMMAND_REQUEST_SET_LPO] == TRUE) {
					if (request[REQ_COMMAND_REQUEST_LPO_NUMBER]>=0 && request[REQ_COMMAND_REQUEST_LPO_NUMBER]<=5) {
						if(request[REQ_COMMAND_REQUEST_LPO_LEVEL]==0 || request[REQ_COMMAND_REQUEST_LPO_LEVEL]==1) {
							gl_mutex=1;gl_OUTchar[request[REQ_COMMAND_REQUEST_LPO_NUMBER]]=!request[REQ_COMMAND_REQUEST_LPO_LEVEL];gl_mutex=0;
							if (sendPrompt==TRUE) prompt(gl_message);
							return(TRUE);
						}
						else {
							gl_parserErrorCode=BAD_LPO_LEVEL;
							return(FALSE);
						}
					}
					else {
						gl_parserErrorCode=BAD_LPO_NUMBER;
						return(FALSE);
					}
				}
				else if (request[REQ_COMMAND_REQUEST_SET_AUT] == TRUE) {
						if(request[REQ_COMMAND_REQUEST_AUT_STATUS]==AUTONValue || request[REQ_COMMAND_REQUEST_AUT_STATUS]==AUTOFFValue) {

							length=strlen(&request[REQ_COMMAND_REQUEST_AUT_IDENT]);
							for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
								notEqual=0;
								if (strlen(&(gl_automation[automationCounter][AUTOMATION_IDENT]))!=length) continue; // They are not equal (strncmp doesn't work here)
								for (i=0;i<length;i++)if (request[REQ_COMMAND_REQUEST_AUT_IDENT+i]!=gl_automation[automationCounter][AUTOMATION_IDENT+i]) {
									notEqual=1;
									break;
								}
								if (!notEqual) {								
									// update automation status
									gl_automation[automationCounter][AUTOMATION_STATUS]=request[REQ_COMMAND_REQUEST_AUT_STATUS];
									if (sendPrompt==TRUE) prompt(gl_message);
									return(TRUE);
								}
							}
							gl_parserErrorCode=BAD_AUT_IDENT;
							return(FALSE);
						}
						else {
							gl_parserErrorCode=BAD_AUT_STATUS;
							return(FALSE);
						}
				}
				else if (request[REQ_COMMAND_REQUEST_SET_TRACK] == TRUE) {
						if (gl_boardMode==DCCValue) {
							gl_parserErrorCode=BAD_BOARD_MODE;
							return(FALSE);
						}
													
						if (request[REQ_COMMAND_REQUEST_TRACK_NUMBER]>=0 && request[REQ_COMMAND_REQUEST_TRACK_NUMBER]<=4) {
							if (((request[REQ_COMMAND_REQUEST_TRACK_SPEED]>=0 && request[REQ_COMMAND_REQUEST_TRACK_SPEED]<=MAXSPEEDVALUE)|| request[REQ_COMMAND_REQUEST_TRACK_SPEED]==KNOB0Value || request[REQ_COMMAND_REQUEST_TRACK_SPEED]==KNOB1Value) &&
							    (request[REQ_COMMAND_REQUEST_TRACK_DIR]==FORWValue || request[REQ_COMMAND_REQUEST_TRACK_DIR]==BACKValue) &&
								((request[REQ_COMMAND_REQUEST_TRACK_INERTIA]>=0 && request[REQ_COMMAND_REQUEST_TRACK_INERTIA]<=MAXINERTIAVALUE)|| request[REQ_COMMAND_REQUEST_TRACK_INERTIA]==KNOB0Value || request[REQ_COMMAND_REQUEST_TRACK_INERTIA]==KNOB1Value)) {
								gl_mutex=1;

								if (request[REQ_COMMAND_REQUEST_TRACK_SPEED]==KNOB0Value) speed=gl_knobValue0;
								else if (request[REQ_COMMAND_REQUEST_TRACK_SPEED]==KNOB1Value) speed=gl_knobValue1;
								else speed=request[REQ_COMMAND_REQUEST_TRACK_SPEED];

								if (request[REQ_COMMAND_REQUEST_TRACK_INERTIA]==KNOB0Value) step=gl_knobValue0;
								else if (request[REQ_COMMAND_REQUEST_TRACK_INERTIA]==KNOB1Value) step=gl_knobValue1;
								else step=request[REQ_COMMAND_REQUEST_TRACK_INERTIA];
								
								if (step<0)step=-step;
	
								if (request[REQ_COMMAND_REQUEST_TRACK_NUMBER]==4) { // ALL
									for(trackNumber=0;trackNumber<4;trackNumber++) {
										setSpeed(speed,step,trackNumber);
										if (request[REQ_COMMAND_REQUEST_TRACK_DIR]==BACKValue) gl_setPoint[trackNumber]=-gl_setPoint[trackNumber];
									}
								}
								else {
										trackNumber=request[REQ_COMMAND_REQUEST_TRACK_NUMBER];
										setSpeed(speed,step,trackNumber);
										if (request[REQ_COMMAND_REQUEST_TRACK_DIR]==BACKValue) gl_setPoint[trackNumber]=-gl_setPoint[trackNumber];
								}
								gl_mutex=0;
								if (sendPrompt==TRUE) prompt(gl_message);
								return(TRUE);
							}
							else {
								if (!(request[REQ_COMMAND_REQUEST_TRACK_SPEED]>=0 && request[REQ_COMMAND_REQUEST_TRACK_SPEED]<=MAXSPEEDVALUE)) {
									gl_parserErrorCode=BAD_TRACK_SPEED;
									return(FALSE);
								}
								if (!(request[REQ_COMMAND_REQUEST_TRACK_DIR]==FORWValue || request[REQ_COMMAND_REQUEST_TRACK_DIR]==BACKValue)) {
									gl_parserErrorCode=BAD_TRACK_DIR;
									return(FALSE);
								}
								if (!(request[REQ_COMMAND_REQUEST_TRACK_INERTIA]>=0 && request[REQ_COMMAND_REQUEST_TRACK_INERTIA]<=MAXINERTIAVALUE)) {
									gl_parserErrorCode=BAD_TRACK_INERTIA;
									return(FALSE);
								}
							}	
						}
						else {								
							gl_parserErrorCode=BAD_TRACK_NUMBER;
							return(FALSE);
						}
				}
				else if (request[REQ_COMMAND_REQUEST_SET_USER_MODE]== TRUE) {
					if (request[REQ_COMMAND_REQUEST_USER_MODE]==MANUALValue ||request[REQ_COMMAND_REQUEST_USER_MODE]==AUTOMATICValue ) {
						gl_userMode=request[REQ_COMMAND_REQUEST_USER_MODE];

						// Set all automation status
						for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
							if (request[REQ_COMMAND_REQUEST_USER_MODE]==MANUALValue) gl_automation[automationCounter][AUTOMATION_STATUS]=gl_automation[automationCounter][AUTOMATION_STATUS_MANUAL];
							else gl_automation[automationCounter][AUTOMATION_STATUS]=AUTONValue;
						}

						if (sendPrompt==TRUE) prompt(gl_message);
						return(TRUE);
					}
					else {
						gl_parserErrorCode=BAD_USER_MODE;
						return(FALSE);
					}
				}				
				else if (request[REQ_COMMAND_REQUEST_SET_DCC]== TRUE) {
						if (gl_boardMode==ANAValue) {
							gl_parserErrorCode=BAD_BOARD_MODE;
							return(FALSE);
						}
						gl_mutex=1;
						setDcc(request[REQ_COMMAND_REQUEST_DCC_ADDRESS],request[REQ_COMMAND_REQUEST_DCC_COMMAND]);
						gl_mutex=0;
						if (sendPrompt==TRUE) prompt(gl_message);
						return(TRUE);
				}
				else if (request[REQ_COMMAND_REQUEST_GET_AUTOMATION_LIST] == TRUE) {
					sprintf(gl_message,"");
					if (sendPrompt==TRUE) prompt(gl_message);
					for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
						if(gl_automation[automationCounter][AUTOMATION_STATUS]==AUTONValue) sprintf(gl_message,"%d %s ON",automationCounter,&(gl_automation[automationCounter][AUTOMATION_IDENT]));
						else if(gl_automation[automationCounter][AUTOMATION_STATUS]==AUTOFFValue) sprintf(gl_message,"%d %s OFF",automationCounter,&(gl_automation[automationCounter][AUTOMATION_IDENT]));
						else sprintf(gl_message,"%d %s UNKNOWN STATE",automationCounter,&(gl_automation[automationCounter][AUTOMATION_IDENT]));					
						if (sendPrompt==TRUE) prompt(gl_message);
					}
					sprintf(gl_message,"");
					if (sendPrompt==TRUE) prompt(gl_message);
					return(TRUE);
				}
				else if (request[REQ_COMMAND_REQUEST_GET_DUMP] == TRUE) {
					for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
						sprintf(gl_message,"");
						itemPerLine=0;
						for(automationDataCounter=0;automationDataCounter<AUTOMATIONSIZE;automationDataCounter++) {
							sprintf(gl_message,"%s(%d,%d,%d)",gl_message,automationCounter,automationDataCounter,gl_automation[automationCounter][automationDataCounter]);		
							if (gl_automation[automationCounter][automationDataCounter]>=32 && gl_automation[automationCounter][automationDataCounter]<=127)sprintf(gl_message,"%s[%c]",gl_message,gl_automation[automationCounter][automationDataCounter]); 
							itemPerLine++;
							if (itemPerLine>=5) {
								if (sendPrompt==TRUE) prompt(gl_message);
								itemPerLine=0;
								sprintf(gl_message,"");
							}
						}
						if (itemPerLine>0) {
							if (sendPrompt==TRUE) prompt(gl_message);
							itemPerLine=0;
							sprintf(gl_message,"");
						}
					}
					return(TRUE);
				}
				else if (request[REQ_COMMAND_REQUEST_GET_BOARD_STATUS] == TRUE) {
					if(gl_boardMode==ANAValue)sprintf(gl_message,"Mode ANA");
					else sprintf(gl_message,"Mode DCC");
					if (sendPrompt==TRUE) prompt(gl_message);
					if (sendPrompt==TRUE) memAvailable();			
					return(TRUE);
				}
				else if (request[REQ_COMMAND_REQUEST_GET_GPIO_STATUS] == TRUE) {
					if (sendPrompt==TRUE) {
						if(TRISDbits.RD1==0)sprintf(gl_message,"GPIO 0 OUT VAL %d",gl_GPIOchar[0]);else sprintf(gl_message,"GPIO 0 IN VAL %d COUNT %d",gl_GPIOchar[0],gl_GPIOcounter[0]);
						prompt(gl_message);
						if(TRISDbits.RD2==0)sprintf(gl_message,"GPIO 1 OUT VAL %d",gl_GPIOchar[1]);else sprintf(gl_message,"GPIO 1 IN VAL %d COUNT %d",gl_GPIOchar[1],gl_GPIOcounter[1]);
						prompt(gl_message);
						if(TRISDbits.RD3==0)sprintf(gl_message,"GPIO 2 OUT VAL %d",gl_GPIOchar[2]);else sprintf(gl_message,"GPIO 2 IN VAL %d COUNT %d",gl_GPIOchar[2],gl_GPIOcounter[2]);
						prompt(gl_message);
						if(TRISCbits.RC4==0)sprintf(gl_message,"GPIO 3 OUT VAL %d",gl_GPIOchar[3]);else sprintf(gl_message,"GPIO 3 IN VAL %d COUNT %d",gl_GPIOchar[3],gl_GPIOcounter[3]);
						prompt(gl_message);
						sprintf(gl_message,"KNOB 0 VAL %d",gl_knobValue0);
						prompt(gl_message);
						sprintf(gl_message,"KNOB 1 VAL %d",gl_knobValue1);
						prompt(gl_message);
	
						sprintf(gl_message,"");
						prompt(gl_message);
					}				
					return(TRUE);
				}
				else if (request[REQ_COMMAND_REQUEST_GET_LPO_STATUS] == TRUE) {
					for(statusCounter=0;statusCounter<6;statusCounter++) {
						if (gl_OUTchar[statusCounter]==0) sprintf(gl_message,"LPO %d 1",statusCounter);else sprintf(gl_message,"LPO %d 0",statusCounter);
						if (sendPrompt==TRUE) prompt(gl_message);
					}
					sprintf(gl_message,"");
					if (sendPrompt==TRUE) prompt(gl_message);	
					return(TRUE);
				}
				else if (request[REQ_COMMAND_REQUEST_GET_TRACK_STATUS] == TRUE) {
					for(statusCounter=0;statusCounter<4;statusCounter++) {
						sprintf(onTrack,"ONTRACK");
						sprintf(offTrack,"OFFTRACK");
						if (gl_setPoint[statusCounter]>0) sprintf(gl_message,"TRACK %d FORW SPEED %d %s",statusCounter,gl_setPoint[statusCounter]/(MAXINTERNALSPEED),gl_OUTSTATchar[statusCounter]==0 ? offTrack : onTrack );
						else sprintf(gl_message,"TRACK %d BACK SPEED %d %s",statusCounter,-gl_setPoint[statusCounter]/(MAXINTERNALSPEED),gl_OUTSTATchar[statusCounter]==0 ? offTrack : onTrack );
						if (sendPrompt==TRUE) prompt(gl_message);
					}
					sprintf(gl_message,"");
					if (sendPrompt==TRUE) prompt(gl_message);	
					return(TRUE);
				}		
				break;

			// NO ACTION TO PERFORM
			default : 
				if (sendPrompt==TRUE) prompt(gl_message);
				return(TRUE);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// UART
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// initUSART
//////////////////////////////////////////////////////////////////////////////
void initUSART() {

 	unsigned char 	readUSARTPointer;
	if (gl_master==FALSE)return;

    //RX and TX pin configuration
	TRISC=0b10110000; //  RC4, RC5, RC7 (RX) in (RC6 TX RS232 out, could be updated as standard GPIO in initSignal())

    // USART module configuration
    TXSTAbits.TXEN = 1; // Activate USART transmitter
    TXSTAbits.SYNC = 0; // Asynchronous mode
    TXSTAbits.BRGH = 1; // High Baud Rate Select bit
    RCSTAbits.SPEN = 1; // Enable serial module (TX and RX)
    RCSTAbits.CREN = 1; // Enable USART receiver
   
    // Speed register configuration
    SPBRG = ((_XTAL_FREQ / 16) / _BAUD) - 1; // _BAUD defined in main.h

	// init RS232 input buffer, event if it's not mandatory at all
	for(readUSARTPointer=0;readUSARTPointer<USARTBUFFERSIZE;readUSARTPointer++) {
		gl_receivedUSARTData[readUSARTPointer]=0;
	}
	gl_receivedUSARTPointer=0;
	gl_getDataUSARTPointer=0;

    PIE1bits.RCIE = 1;      // Enable USART interrupt reception
}
/*****************************************************************************/
/* sendUSART */
/*****************************************************************************/
void sendUSART(unsigned char data){

	if (gl_master==FALSE)return;

    // wait until transmission buffer is empty
    while (!TXSTAbits.TRMT);
	TXREG = data;
	while(!PIR1bits.TXIF);
}
/*****************************************************************************/
/* getInputRequestFromUSART() */
/*****************************************************************************/
unsigned char getInputRequestFromUSART(unsigned char *inputString,char *inputCounter) {

   unsigned char  getData;

	if (gl_master==FALSE)return;

	// Get Data from RS232
	if (gl_getDataUSARTPointer!=gl_receivedUSARTPointer) {
		getData=gl_receivedUSARTData[gl_getDataUSARTPointer];
		gl_receivedUSARTData[gl_getDataUSARTPointer++] = 0;

		if (gl_getDataUSARTPointer>=USARTBUFFERSIZE)gl_getDataUSARTPointer=0;

		//Echo
        if (getData!=0xD && *inputCounter<MAXINPUTSTRING-1) {
		
			if (getData==0x7F) {
				(*inputCounter)--;
				if (*inputCounter<0) *inputCounter=0;
				else sendUSART(getData); // Echo
			}
			else {
                inputString[(*inputCounter)++]=(unsigned char)toUpperCase(getData);
				sendUSART(getData); // Echo
			}
       }
        else {
			inputString[*inputCounter]='\0';	
			*inputCounter=0;
			return(TRUE);
		}
	}
	else {
		return(FALSE);
	}	
}
/*****************************************************************************/
/* prompt*/
/*****************************************************************************/
void prompt(unsigned char* gl_message) {

	if (gl_master==TRUE) printf("\n\rMaster (%d) > %s",gl_boardNumber,gl_message);
	else if(strlen(gl_message)>0){
		printf("Board %d : %s",gl_boardNumber,gl_message);
		if (gl_master==TRUE)prompt("");
	}
	flushOut();	
}
//////////////////////////////////////////////////////////////////////////////
// CAN
//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/* flushOut*/
/*****************************************************************************/
void flushOut() {
	_user_putc(ENDOFPRINTFTRAME);
}

/*****************************************************************************/
/* CANsendDelay*/
/*****************************************************************************/
void CANsendDelay() {
	unsigned short	delay;
	for(delay=0;delay<WAITDELAYTRAMECAN;delay++);
}

/*****************************************************************************/
/* _user_putc*/
/*****************************************************************************/
int _user_putc (char c) {

	unsigned long 	id;			// Id of sender
    unsigned char 	dataOut[8];	// DATA to CAN	
	unsigned char 	dataCounter;
	unsigned char	dataOutCounter;	
	unsigned char	trameComplete;

	BYTE dataLen; 				// Number of bytes transmitted in the gl_message
	ECAN_RX_MSG_FLAGS flags; 	// Flags

	// On master board send to UART via standart putc()
	if (gl_master==TRUE){
		if (gl_inputCounter==0)sendUSART(c);
	}
	else {
		if (gl_outputBufferCounter<MAXMESSAGESIZE)gl_outputBuffer[gl_outputBufferCounter++]=c;

		// Send on CAN bus
	

		if (gl_outputBufferCounter==MAXMESSAGESIZE || c==ENDOFPRINTFTRAME) {
			dataLen=8;
			flags=ECAN_TX_STD_FRAME;
			id=gl_boardNumber;

			// header trame
			for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++) dataOut[dataOutCounter]=TRAMEPRINTHEADER;

			// Synchro send
			while(gl_synchroSend >0);

			while(!ECANSendMessage(id,dataOut,dataLen,flags));
			CANsendDelay();

			// trame
			trameComplete=FALSE;
			dataCounter=0;
			while(dataCounter<MAXMESSAGESIZE) {
				for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++) {
					if (dataCounter<gl_outputBufferCounter) {
						dataOut[dataOutCounter]=gl_outputBuffer[dataCounter++];
						if (dataOut[dataOutCounter]==ENDOFPRINTFTRAME) trameComplete=TRUE;
					}
					else {
						dataOut[dataOutCounter]=ENDOFPRINTFTRAME;
						if (dataCounter<MAXMESSAGESIZE)dataCounter++;
						trameComplete=TRUE;
					}
				}
				while(!ECANSendMessage(id,dataOut,dataLen,flags));
				CANsendDelay();

				if (trameComplete==TRUE) break;
			}
			gl_outputBufferCounter=0;

			// footer trame
			for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++) dataOut[dataOutCounter]=TRAMEPRINTFOOTER;
			while(!ECANSendMessage(id,dataOut,dataLen,flags));
			CANsendDelay();
		}
	}
	return(c);
}

/*****************************************************************************/
/* sendRequestToCAN() */
/*****************************************************************************/
void sendRequestToCAN(unsigned char* request) {
	
	 unsigned long 	id;			// Id of sender
     unsigned char 	dataOut[8];	// DATA to CAN	
	 unsigned char 	dataCounter;
	 unsigned char	dataOutCounter;
	BYTE dataLen; 				// Number of bytes transmitted in the gl_message
	ECAN_RX_MSG_FLAGS flags; 	// Flags
	unsigned char trameSize;

	// Convert request to dataOut using gl_dataStructure
	trameSize=compressData(request);

	dataLen=8;
	flags=ECAN_TX_STD_FRAME;
	dataCounter=0;
	id=gl_boardNumber;

	// header trame
	for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++) {
		dataOut[dataOutCounter]=TRAMEREQUESTHEADER;
	}
			
	// Synchro send
	while(gl_synchroSend >0);
	CANsendDelay(); // Delay to avoid sending to fast after receiving a trame

	while(!ECANSendMessage(id,dataOut,dataLen,flags));
	CANsendDelay();

	// trame
	while(dataCounter<trameSize) {
		for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++) {
			if (dataCounter<trameSize) {
				dataOut[dataOutCounter]=request[dataCounter++];
				if(dataCounter>=MAXTRAMESIZE) return; // Something wrong happened
			}
			else {
				dataOut[dataOutCounter]=0;
			}
		}
		while(!ECANSendMessage(id,dataOut,dataLen,flags));
		CANsendDelay();
	}

	// footer trame
	for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++) {
		dataOut[dataOutCounter]=TRAMEREQUESTFOOTER;
	}
	while(!ECANSendMessage(id,dataOut,dataLen,flags));
	uncompressData(request); // get back to initial data for other action in manageRequest()
}
/*****************************************************************************/
/* getInputRequestFromCAN() */
/*****************************************************************************/
unsigned char getInputRequestFromCAN(unsigned char* request) {
	
	unsigned char	requestHeaderTrameDetected=0;
	unsigned char	printHeaderTrameDetected=0;	
	unsigned char	requestFooterTrameDetected=0;
	unsigned char	printFooterTrameDetected=0;
	char			requestTrameEnd;
	char			printTrameEnd;

	unsigned char	dataInCounter;
	unsigned char	dataStructureCounter;

	sprintf(gl_message,"");
	while (gl_getDataCANPointer!=gl_InputBufferPointer) {
	
			if (gl_inputBuffer[gl_getDataCANPointer]==TRAMEREQUESTHEADER) requestHeaderTrameDetected++;	else requestHeaderTrameDetected=0;
			if (gl_inputBuffer[gl_getDataCANPointer]==TRAMEREQUESTFOOTER)	requestFooterTrameDetected++;	else requestFooterTrameDetected=0;

			if (gl_inputBuffer[gl_getDataCANPointer]==TRAMEPRINTHEADER) 	printHeaderTrameDetected++;		else printHeaderTrameDetected=0;				
			if (gl_inputBuffer[gl_getDataCANPointer]==TRAMEPRINTFOOTER) 	printFooterTrameDetected++;		else printFooterTrameDetected=0;

			// REQUEST HEADER
			if (requestHeaderTrameDetected==8) {
				gl_canMode=CAN_REQUEST;
				requestHeaderTrameDetected=0;
				gl_requestTrameStart=gl_getDataCANPointer+1;
				if (gl_requestTrameStart>=MAXTRAMESIZE)gl_requestTrameStart=0;
			}
			
			// PRINT HEADER
			else if (printHeaderTrameDetected==8) {
				gl_canMode=CAN_PRINT;
				printHeaderTrameDetected=0;
				gl_printTrameStart=gl_getDataCANPointer+1;
				if (gl_printTrameStart>=MAXTRAMESIZE)gl_printTrameStart=0;
			}

			// REQUEST FOOTER
			else if (requestFooterTrameDetected==8 && gl_canMode==CAN_REQUEST) {
				requestTrameEnd=gl_getDataCANPointer-7;
				if (requestTrameEnd<0)requestTrameEnd+=MAXTRAMESIZE;
				dataInCounter=gl_requestTrameStart;
				dataStructureCounter=0;
				while(dataInCounter!=requestTrameEnd) {
					request[dataStructureCounter++]=gl_inputBuffer[dataInCounter++];
					if (dataInCounter>=MAXTRAMESIZE)dataInCounter=0;
				}
				gl_canMode=CAN_UNKNOWN; // If more data arrive.... we delete this trame
				gl_getDataCANPointer++;		
				if (gl_getDataCANPointer>=MAXTRAMESIZE)gl_getDataCANPointer=0;
				uncompressData(request);
				return(TRUE); // Mean request available to proceed
			}
				
			// PRINT FOOTER	
			else if (printFooterTrameDetected==8 && gl_canMode==CAN_PRINT) {
				printTrameEnd=gl_getDataCANPointer-7;
				if (printTrameEnd<0)printTrameEnd+=MAXTRAMESIZE;
				dataInCounter=gl_printTrameStart;

				while(dataInCounter!=printTrameEnd) {
					if (gl_master==TRUE)printf("%c",gl_inputBuffer[dataInCounter]);
					dataInCounter++;
					if (dataInCounter>=MAXTRAMESIZE)dataInCounter=0;
				}
				if (gl_master==TRUE) prompt(gl_message);
				gl_canMode=CAN_UNKNOWN; // If more data arrive.... we continue to get data
				gl_getDataCANPointer++;		
				if (gl_getDataCANPointer>=MAXTRAMESIZE)gl_getDataCANPointer=0;
				return(FALSE); // Mean no more data to print
			}	
		
			// read new data
			gl_getDataCANPointer++;		
			if (gl_getDataCANPointer>=MAXTRAMESIZE)gl_getDataCANPointer=0;					
	}
	return(FALSE); // Nothing to do
}

//////////////////////////////////////////////////////////////////////////////
// INTERRUPT AND SIGNAL MANAGEMENT
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// setDcc
/////////////////////////////////////////////////////////////////////////////
void setDcc(unsigned char address, unsigned char command) {

	 unsigned char i;			
	 unsigned char bitNumber; 	
	 unsigned char control; 		

	control=address ^ command; // Control
	bitNumber=0;

	// PREAMBULE
    for(i=0;i<PREAMBLE_SIZE;i++) {
		gl_dcc[bitNumber++]=1;
	}
	// 0
	gl_dcc[bitNumber++]=0;

	// ADDRESS
    for(i=0;i<8;i++) {
		gl_dcc[bitNumber++]=address >> (7-i) & 1;
	}
	// 0
	gl_dcc[bitNumber++]=0;

	// COMMAND
    for(i=0;i<8;i++) {
		gl_dcc[bitNumber++]=command >> (7-i) & 1;
	}
	// 0
	gl_dcc[bitNumber++]=0;

	// CONTROL
    for(i=0;i<8;i++) {
		gl_dcc[bitNumber++]=control >> (7-i) & 1;
	}
	// 1
	gl_dcc[bitNumber++]=1;			
	gl_dcc[bitNumber++]=1; // Only one is enough, but in case of....
}
//////////////////////////////////////////////////////////////////////////////
// function set7segmentPort
//////////////////////////////////////////////////////////////////////////////
void set7segmentPort(unsigned char CLK, unsigned char DIO) {

	unsigned char myPortE; // used to better synchronised output updates
	unsigned char delay;

    myPortE= (CLK<<1) + (DIO<<2);
    LATE=myPortE;
}

/////////////////////////////////////////////////////////////////////////////
// function twoWire_init()
/////////////////////////////////////////////////////////////////////////////
void twoWire_init() {
	set7segmentPort(0,0);
}
/////////////////////////////////////////////////////////////////////////////
// function twoWire_start()
/////////////////////////////////////////////////////////////////////////////
void twoWire_start(){
    
	set7segmentPort(1,1);
	set7segmentPort(1,0);
}
/////////////////////////////////////////////////////////////////////////////
// function twoWire_stop()
/////////////////////////////////////////////////////////////////////////////
void twoWire_stop(){
	set7segmentPort(0,0);
	set7segmentPort(1,0);
	set7segmentPort(1,1);
}
/////////////////////////////////////////////////////////////////////////////
// function twoWire_ack()
/////////////////////////////////////////////////////////////////////////////
void twoWire_ack(){

	set7segmentPort(0,0);
	set7segmentPort(1,0);
}
/////////////////////////////////////////////////////////////////////////////
// function twoWire_write(char data)
/////////////////////////////////////////////////////////////////////////////
char twoWire_write(char data){

	unsigned char tx;
	unsigned char DIO;
	for(tx = 0 ; tx < 8 ; tx++) {
		DIO = ((data >> tx) & 0x01) ? 1 : 0 ; //LSB first (Real 12c sends MSB first)
		set7segmentPort(0,DIO);
		set7segmentPort(1,DIO);
		set7segmentPort(0,DIO);
	}
}
/////////////////////////////////////////////////////////////////////////////
// function 
/////////////////////////////////////////////////////////////////////////////
void TM1637_init(void){
    twoWire_init();    
}
/////////////////////////////////////////////////////////////////////////////
// function TM1637_write(unsigned char number1, unsigned char number2)
/////////////////////////////////////////////////////////////////////////////
void TM1637_write(short number1,short number2){
		

    char str1Num[4];
    char str2Num[4];
    char strNum[8];
    char size;

	sprintf(str1Num,"%3d",number1); 
	sprintf(str2Num,"%3d",number2); // 3 characters
	sprintf(strNum,"%s%s",str2Num,str1Num);

    for(size=5;size>=0;size--) {
		if (strNum[size]==' ')twoWire_write(digits[11]);
		else if (strNum[size]=='-')twoWire_write(digits[10]);
        else {
			unsigned char i = strNum[size] - '0';  //Get index 0 - 9 
        	twoWire_write(digits[i]);
		}
        twoWire_ack();
    }
}

/////////////////////////////////////////////////////////////////////////////
// main function for display TM1637_display(unsigned char number1, unsigned char number2)
/////////////////////////////////////////////////////////////////////////////
void TM1637_display(short number1,short number2){  

    twoWire_start();
    twoWire_write(0x40);
    twoWire_ack();
    twoWire_stop();
    
    twoWire_start();
    twoWire_write(0xC0);
    twoWire_ack();
    TM1637_write(number1,number2);
    
    twoWire_stop();

}
/////////////////////////////////////////////////////////////////////////////
// Valid brightness values: 0 - 8.
// 0 = display off.
// main function for display TM1637_setBrightness(char level)
/////////////////////////////////////////////////////////////////////////////
void TM1637_setBrightness(char level){  

	gl_mutex=1;
    twoWire_start();
    twoWire_write(0x87 + level);
    twoWire_ack();
    twoWire_stop();
	gl_mutex=0;
}

//////////////////////////////////////////////////////////////////////////////
// function SetPort
//////////////////////////////////////////////////////////////////////////////
void setPort(){

    unsigned char OUTCounter;
	unsigned char myPortA; // used to better synchronised output updates
	unsigned char myPortB; // used to better synchronised output updates
	unsigned char myPortC; // used to better synchronised output updates
	unsigned char myPortD; // used to better synchronised output updates


	if (gl_stopAll==TRUE) {
		gl_S1T0char=0; gl_S2T0char=0;
		gl_S1T1char=0; gl_S2T1char=0;
		gl_S1T2char=0; gl_S2T2char=0;
		gl_S1T3char=0; gl_S2T3char=0;	
		for(OUTCounter=0;OUTCounter<6;OUTCounter++) gl_OUTchar[OUTCounter]=1;
	}

    myPortA=(gl_S1T0char<<4) + (gl_S1T1char<<6) + (gl_S2T0char<<7);
    myPortB=(gl_OUTchar[2]) + (gl_OUTchar[3]<<1) + (gl_OUTchar[4]<<4) + (gl_OUTchar[5]<<5);
	myPortC=(gl_S2T1char) + (gl_S1T2char<<1) + (gl_S2T2char<<2) + (gl_S1T3char<<3) +(TRISCbits.RC4==0 ? gl_GPIOchar[3] <<4 : 0);
	myPortD=(gl_S2T3char) + (TRISDbits.RD1==0 ? gl_GPIOchar[0] <<1 :0) + (TRISDbits.RD2==0 ? gl_GPIOchar[1] <<2:0) +(TRISDbits.RD3==0 ? gl_GPIOchar[2] <<3:0) + (gl_OUTchar[0]<<6) +  (gl_OUTchar[1]<<7);



	LATA=myPortA;
    LATB=myPortB;
    LATC=myPortC;
    LATD=myPortD;

}

/*****************************************************************************/
/* interrupt_at_high_vector */
/*****************************************************************************/
#pragma code high_vector=0x08
void interrupt_at_high_vector(void){
    _asm goto high_isr _endasm
}
#pragma code
/****************************************************************************/
/* high_isr */
/****************************************************************************/
#pragma interrupt high_isr
void high_isr(void){

	BYTE dataLen; 				// Number of bytes transmitted in the message
	ECAN_RX_MSG_FLAGS flags; 	// Flags
	unsigned long 	id;			// Id of sender	

	if(PIR3bits.RXB0IF ||PIR3bits.RXB1IF) {
		while(ECANReceiveMessage(&id, &gl_inputBuffer[gl_InputBufferPointer], &dataLen, &flags)) {	
			gl_InputBufferPointer+=dataLen;
			if(gl_InputBufferPointer>=MAXTRAMESIZE)gl_InputBufferPointer-=MAXTRAMESIZE;
		}
		gl_synchroSend=(gl_boardNumber+1)*SYNCHROSENDDELAY;
	}
	
	if (gl_master==FALSE)return;

    // Check if interrupt originates from USART reception
    if (PIR1bits.RCIF)
    {
        // Read received data
        gl_receivedUSARTData[gl_receivedUSARTPointer++] = RCREG;
		if (gl_receivedUSARTPointer>=USARTBUFFERSIZE)gl_receivedUSARTPointer=0;
    }
}
#pragma code
/*****************************************************************************/
/* low_interrupt */
/*****************************************************************************/
#pragma code low_vector=0x18
void low_interrupt (){
    _asm goto low_isr _endasm
}
#pragma code
/*****************************************************************************/
/* low_isr */
/*****************************************************************************/
#pragma interruptlow low_isr
void low_isr(void){
	  unsigned char 	bitStateCounter;
	  unsigned char 	delay;
	  unsigned char 	selectBitDelay;
	  unsigned char 	bitNumber;
	  unsigned char 	bitValue;
	  short 			ADC;

	// Synchro send
	if(gl_synchroSend>0)gl_synchroSend--;
	
	if (!gl_mutex) {

		// KNOB VALUE
		ADCON0=INKNOB0;
		ADCON0bits.GO = 1;                            // ADCON0.GODONE = 1 
		while(ADCON0bits.GO == 1);                    // wait till GODONE bit is zero
		ADC = ADRESH;    //Read converted result 
		ADC = (ADC<<8) + ADRESL;
		gl_adcKnobValue0=((9*gl_adcKnobValue0)+ADC)/10;
		gl_knobValue0=(9*gl_knobValue0+(gl_adcKnobValue0/20)-25)/10;

 		if (gl_knobValue0<-15)gl_knobValue0=-15;
 		if (gl_knobValue0>15)gl_knobValue0=15;


		ADCON0=INKNOB1;
		ADCON0bits.GO = 1;                            // ADCON0.GODONE = 1 
		while(ADCON0bits.GO == 1);                    // wait till GODONE bit is zero
		ADC = ADRESH;    //Read converted result 
		ADC = (ADC<<8) + ADRESL;
		gl_adcKnobValue1=((9*gl_adcKnobValue1)+ADC)/10; 
		gl_knobValue1=(9*gl_knobValue1+(gl_adcKnobValue1/9))/10;

		if (gl_knobValue1<0)gl_knobValue1=0;
		if (gl_knobValue1>MAXINERTIAVALUE)gl_knobValue1=MAXINERTIAVALUE;

		// GPIO IN Detection
		if(TRISDbits.RD1==1 && PORTDbits.RD1!=gl_GPIOchar[0]){
			gl_GPIOstabilized[0]++;
			if (gl_GPIOstabilized[0]>GPIOTHRESHOLD) {
				gl_GPIOchar[0]=PORTDbits.RD1;
				if (gl_GPIOchar[0]==1)gl_GPIOcounter[0]++;
				gl_GPIONotification[0]=TRUE;
				gl_GPIOstabilized[0]=0;
			}
		}
		else gl_GPIOstabilized[0]=0;

		if(TRISDbits.RD2==1 &&PORTDbits.RD2!=gl_GPIOchar[1]){
			gl_GPIOstabilized[1]++;
			if (gl_GPIOstabilized[1]>GPIOTHRESHOLD) {
				gl_GPIOchar[1]=PORTDbits.RD2;
				if (gl_GPIOchar[1]==1)gl_GPIOcounter[1]++;
				gl_GPIONotification[1]=TRUE;
				gl_GPIOstabilized[1]=0;
			}
		}
		else gl_GPIOstabilized[1]=0;

		if(TRISDbits.RD3==1 & PORTDbits.RD3!=gl_GPIOchar[2]){
			gl_GPIOstabilized[2]++;
			if (gl_GPIOstabilized[2]>GPIOTHRESHOLD) {
				gl_GPIOchar[2]=PORTDbits.RD3;
				if (gl_GPIOchar[2]==1)gl_GPIOcounter[2]++;
				gl_GPIONotification[2]=TRUE;
				gl_GPIOstabilized[2]=0;
			}
		}
		else gl_GPIOstabilized[2]=0;

		if(TRISCbits.RC4==1 & PORTCbits.RC4!=gl_GPIOchar[3]){
			gl_GPIOstabilized[3]++;
			if (gl_GPIOstabilized[3]>GPIOTHRESHOLD) {
				gl_GPIOchar[3]=PORTCbits.RC4;
				if (gl_GPIOchar[3]==1)gl_GPIOcounter[3]++;
				gl_GPIONotification[3]=TRUE;
				gl_GPIOstabilized[3]=0;
			}
		}
		else gl_GPIOstabilized[3]=0;

		if (gl_stopAll==FALSE) {

			// TIMER
			gl_timer--;
			if (gl_timer==0) {
				gl_timer=INITTIMERVALUE;
				if (gl_TIMERValue[gl_timerNumber]>0){
					gl_TIMERValue[gl_timerNumber]--;
					if (gl_TIMERValue[gl_timerNumber]==0)gl_TIMERNotification[gl_timerNumber]=TRUE;
				}
				gl_timerNumber++;
				if(gl_timerNumber>MAXTIMER)gl_timerNumber=0;
			}

			gl_trackNumber=gl_trackNumber+1;
			if (gl_trackNumber>3)gl_trackNumber=0;
	
			//////////// MODE ANALOG ////////////////
			if(gl_boardMode==ANAValue) {
				gl_speedCounter++;
				if (gl_speedCounter>MAX_INERTIA_COUNTER) {
		 			gl_speedCounter=1;
				}

				if (gl_curSpeed[gl_trackNumber]!=gl_setPoint[gl_trackNumber]) {
					if (gl_curSpeed[gl_trackNumber]>gl_setPoint[gl_trackNumber]){
						gl_curSpeed[gl_trackNumber]-=(MAXINERTIAVALUE-gl_setStep[gl_trackNumber]+1)/5;
						if(gl_curSpeed[gl_trackNumber]<gl_setPoint[gl_trackNumber])gl_curSpeed[gl_trackNumber]=gl_setPoint[gl_trackNumber];
					}
					else {
						gl_curSpeed[gl_trackNumber]+=(MAXINERTIAVALUE-gl_setStep[gl_trackNumber]+1)/5;
						if(gl_curSpeed[gl_trackNumber]>gl_setPoint[gl_trackNumber])gl_curSpeed[gl_trackNumber]=gl_setPoint[gl_trackNumber];
					}

					if (gl_curSpeed[gl_trackNumber]>0) {
						gl_speed[gl_trackNumber]=gl_curSpeed[gl_trackNumber]/(MAXINTERNALSPEED);
						gl_direction[gl_trackNumber]=TRACK_BACKWARD;
					}
					else if (gl_curSpeed[gl_trackNumber]<0) {
						gl_speed[gl_trackNumber]=-gl_curSpeed[gl_trackNumber]/(MAXINTERNALSPEED);
						gl_direction[gl_trackNumber]=TRACK_FORWARD;
					}
					else {
						gl_direction[gl_trackNumber]==TRACK_STOP;
						gl_speed[gl_trackNumber]=0;
					}
				}

				if (gl_speed[gl_trackNumber]>=gl_speedCounter) {
					if (gl_direction[gl_trackNumber]==TRACK_FORWARD) {
			    		switch (gl_trackNumber) {
			           		case 0:gl_S1T0char=1; gl_S2T0char=0;break;
			          	 	case 1:gl_S1T1char=1; gl_S2T1char=0;break;
			           	 	case 2:gl_S1T2char=1; gl_S2T2char=0;break;
			           		case 3:gl_S1T3char=1; gl_S2T3char=0;break;
			        	}
			    	}
			    	if (gl_direction[gl_trackNumber]==TRACK_BACKWARD) {
			        	switch (gl_trackNumber) {
			            	case 0:gl_S1T0char=0; gl_S2T0char=1;break;
			            	case 1:gl_S1T1char=0; gl_S2T1char=1;break;
			            	case 2:gl_S1T2char=0; gl_S2T2char=1;break;
			            	case 3:gl_S1T3char=0; gl_S2T3char=1;break;
			        	}
			    	}
			    	if (gl_direction[gl_trackNumber]==TRACK_STOP) {
			        	switch (gl_trackNumber) {
			            	case 0:gl_S1T0char=0; gl_S2T0char=0;break;
			            	case 1:gl_S1T1char=0; gl_S2T1char=0;break;
			            	case 2:gl_S1T2char=0; gl_S2T2char=0;break;
			            	case 3:gl_S1T3char=0; gl_S2T3char=0;break;
			        	}
			    	}
				}
				else {
					switch (gl_trackNumber) {
			        	case 0:gl_S1T0char=0; gl_S2T0char=0;break;
			        	case 1:gl_S1T1char=0; gl_S2T1char=0;break;
			        	case 2:gl_S1T2char=0; gl_S2T2char=0;break;
			        	case 3:gl_S1T3char=0; gl_S2T3char=0;break;
			    	}
				}

				setPort();
			}

			//////////// MODE DIGITAL ////////////////

			if(gl_boardMode==DCCValue && gl_dcc_ready==0) {		

				for (bitStateCounter=0;bitStateCounter<FRAME_SIZE;bitStateCounter++) {

					if (gl_dcc[bitStateCounter]==0) selectBitDelay=DCC_0;
					else selectBitDelay=DCC_1;
							
					gl_S1T0char=0;gl_S1T1char=0;
					gl_S1T2char=0;gl_S1T3char=0;
					gl_S2T0char=1;gl_S2T1char=1;
					gl_S2T2char=1;gl_S2T3char=1;
            		setPort();
					for (delay=0;delay<selectBitDelay;delay++);
	
		    		gl_S2T0char=0;gl_S2T1char=0;
					gl_S2T2char=0;gl_S2T3char=0;
					gl_S1T0char=1;gl_S1T1char=1;
					gl_S1T2char=1;gl_S1T3char=1;
            		setPort();
					for (delay=0;delay<selectBitDelay;delay++);
				}

				gl_S1T0char=0; gl_S2T0char=1;	
				gl_S1T1char=0; gl_S2T1char=1;
				gl_S1T2char=0; gl_S2T2char=1;	
				gl_S1T3char=0; gl_S2T3char=1;
        		setPort();
				for (delay=0;delay<selectBitDelay;delay++);	
				for (bitStateCounter=0;bitStateCounter<FRAME_SIZE;bitStateCounter++)gl_dcc[bitStateCounter]=1;
			}
			gl_dcc_ready--;
			if (gl_dcc_ready<0) gl_dcc_ready=INITWAITDCCCOUNTER;
			setPort();

			// TRACK DETECTION 

			switch(gl_trackNumber) {
				case 0 : ADCON0=CURT0;break;
		    	case 1 : ADCON0=CURT1;break;
		    	case 2 : ADCON0=CURT2;break;
		   		case 3 : ADCON0=CURT3;break;
			}
			// NEED TO GET LOW VOLTAGE VALUE WHEN TRACK IS OFF FOR CALIBRATION AT POWER ON
	 		if (gl_calibration==TRUE) {
				ADCON0bits.GO = 1;                            // ADCON0.GODONE = 1 
				while(ADCON0bits.GO == 1);                    // wait till GODONE bit is zero
				ADC = 0;
				ADC = ADRESH;    //Read converted result 
				ADC = (ADC<<8) + ADRESL;
				gl_average[gl_trackNumber]=(SAMPLEFORCALIBRATION*gl_average[gl_trackNumber]+ADC)/(SAMPLEFORCALIBRATION+1);
				if (gl_noVehicule[gl_trackNumber]>gl_average[gl_trackNumber])gl_noVehicule[gl_trackNumber]=gl_average[gl_trackNumber];
			}

			if((gl_speed[gl_trackNumber]==gl_speedCounter && gl_boardMode==ANAValue) || (gl_dcc_ready==INITWAITDCCCOUNTER && gl_boardMode==DCCValue)) { // ONLY WHEN POWER ON
				ADCON0bits.GO = 1;                            // ADCON0.GODONE = 1 
				while(ADCON0bits.GO == 1);                    // wait till GODONE bit is zero
				ADC = 0;
				ADC = ADRESH;    //Read converted result 
				ADC = (ADC<<8) + ADRESL;

				if (gl_average[gl_trackNumber]<ADC) gl_average[gl_trackNumber]=ADC; // TRAP THE EVENT
				else gl_average[gl_trackNumber]=(SAMPLEFORAVERAGE*gl_average[gl_trackNumber]+ADC)/(SAMPLEFORAVERAGE+1);

				if ((10*gl_average[gl_trackNumber]>(10+HYSTERERISHIGH)*gl_noVehicule[gl_trackNumber]) && (gl_OUTSTATchar[gl_trackNumber]==0)  && (gl_trackNotification[gl_trackNumber]==FALSE)) {
					gl_OUTSTATchar[gl_trackNumber]=1;
					gl_trackNotification[gl_trackNumber]=TRUE;
				}
				else if ((10*gl_average[gl_trackNumber]<(10+HYSTERERISLOW)*gl_noVehicule[gl_trackNumber]) && (gl_OUTSTATchar[gl_trackNumber]==1) && (gl_trackNotification[gl_trackNumber]==FALSE)) {
					gl_OUTSTATchar[gl_trackNumber]=0;
					gl_trackNotification[gl_trackNumber]=TRUE;
				}
			} 
		}
		else setPort();
	}

    // INTERRUPT RESET
    if(INTCONbits.TMR0IF==1){
	    INTCONbits.TMR0IF = 0;
		T0CONbits.PSA			= 0;   // Timer0 prescaler is assigned
		T0CONbits.T0PS0			= 0;   // Prescale value 
		T0CONbits.T0PS1			= 0;   // Prescale value 
		T0CONbits.T0PS2			= 0;   // Prescale value 
    }
}
#pragma code

//////////////////////////////////////////////////////////////////////////////
// CODE INITIALISATION
//////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/
/* initSignal */
/*****************************************************************************/
void initSignal() {

    unsigned char trackNumber;
	unsigned char OUTCounter;	
	unsigned char GPIOCounter;
	unsigned char TIMERCounter;

	gl_mutex=1; // should not be necessary because interrupt not yet enabled

    ADCON1 	= 0x9; // AN0 to AN5 
    ADCON2 	= 0x9D; 
	TRISA 	= 0b00101111; // PORTA AN0 to AN4 in 
	LATA 	= 0;

    TRISB	= 0b00001000; // RB3 in for CAN bus
	LATB	= 0;

    TRISC	= 0b11110000; //  RC4, RC6, RC5, RC7 (RX) in (RC6 TX RS232 could be updated in initUSART() to be out)
	LATC	= 0;

    TRISE	= 0b00000001; // Port A5 in + PORTD setting
	LATE	= 0;
	CMCON 	= 7;

    TRISD	= 0b00111110; //  RD1, RD2, RD3, RD4, RD5 in
	ECCP1CON = 0; // Disable ENHANCED CAPTURE/COMPARE/PWM (ECCP1) MODULE
	LATD	= 0;


    for(trackNumber=0;trackNumber<4;trackNumber++) {
      gl_average[trackNumber]=0;
	  gl_noVehicule[trackNumber]=0xFF;
      gl_speed[trackNumber]=0;
      gl_direction[trackNumber]=0;
      gl_setPoint[trackNumber]=0;
      gl_setStep[trackNumber]=0;
      gl_curSpeed[trackNumber]=0;
	  gl_OUTSTATchar[trackNumber]=0;
	  gl_trackNotification[trackNumber]=FALSE;
    }

	for(OUTCounter=0;OUTCounter<6;OUTCounter++) gl_OUTchar[OUTCounter]=1;
	
	for(GPIOCounter=0;GPIOCounter<4;GPIOCounter++) {
		gl_GPIOchar[GPIOCounter]=0;
		gl_GPIOcounter[GPIOCounter]=0;
		gl_GPIOstabilized[GPIOCounter]=0;
		gl_GPIONotification[GPIOCounter]=FALSE;
	}
	for(TIMERCounter=0;TIMERCounter<MAXTIMER;TIMERCounter++) {
		gl_TIMERValue[TIMERCounter]=0;
		gl_TIMERNotification[TIMERCounter]=FALSE;
	}
	gl_timerNumber=0;
	gl_timer=0;
		

	// DCC TEMPO BETWEEN TWO TRAMES
    gl_dcc_ready=INITWAITDCCCOUNTER;

	// Start
	gl_calibration=FALSE;
	gl_stopAll=FALSE;
	gl_trackNumber=0;
	
	// Get board number
	gl_boardNumber=IN4 + 2*IN3 + 4*IN0 + 8*IN1 + 16*IN2;

	// Detect master board
	if (gl_boardNumber==31) gl_master=TRUE;
	else gl_master=FALSE;

	// Init structure 
	gl_outputBufferCounter=0;
	initRequest(&gl_request);

	// Update form EEPROM
	gl_mutex=0;
	ReadEEPROMConfig();

	if(TRISDbits.RD1==0)gl_GPIOchar[0]=1; else gl_GPIOchar[0]=0xFF; // out default value is 1 
	if(TRISDbits.RD2==0)gl_GPIOchar[1]=1; else gl_GPIOchar[1]=0xFF; // out default value is 1 
	if(TRISDbits.RD3==0)gl_GPIOchar[2]=1; else gl_GPIOchar[2]=0xFF; // out default value is 1 
	if(TRISCbits.RC4==0)gl_GPIOchar[3]=1; else gl_GPIOchar[3]=0xFF; // out default value is 1 	

}

/*****************************************************************************/
/* PIC18FMainSettings */
/*****************************************************************************/
void PIC18FMainSettings (){

    // PIC setting and enable interrupts
    OSCCON                  = 0x70;  // no pre-divider => 8MHz 
    OSCTUNE                 = 0x40;  // PLL *4 => 32MHz 

    T0CONbits.T08BIT        = 1;   // 8-bit timer 
    T0CONbits.T0CS          = 0;   // increment on instruction cycle input
    T0CONbits.T0SE          = 0;   // increment on low--> high transition of clock
    T0CONbits.PSA           = 1;   // T0 prescaler  assigned to 1:1
    RCONbits.IPEN           = 1;   // Enable Interrupt Priorities
    INTCONbits.GIEL         = 1;   // Enable Low Priority Interrupt
    INTCONbits.GIEH         = 0;   // Enable high priority interrupts
    INTCONbits.GIE          = 1;   // Enable Global Interrupts            
    INTCONbits.PEIE         = 1;   // Enable device interrupts
    INTCONbits.TMR0IE       = 1;   // Enable Timer0 Interrupt
    INTCON2bits.TMR0IP      = 0;   // TMR0 set to low Priority Interrupt
    INTCONbits.TMR0IF       = 0;   // T0 int flag bit cleared before starting
    T0CONbits.TMR0ON        = 1;   // timer0 START
}

/*****************************************************************************/
/* init */
/*****************************************************************************/
void init() {
 

    // Signal
     initSignal();

    // RS232
	initUSART(); // Serial USART init on master board only
    
    // ECAN
    TRISBbits.TRISB3 = 1; // CANRX input setting
	gl_InputBufferPointer=0;
	gl_getDataCANPointer=0;
	gl_canMode=CAN_UNKNOWN;

    ECANInitialize(); // init ECAN
	PIE3bits.RXB0IE=1; // enable interrupt for CAN
	PIE3bits.RXB1IE=1; // enable interrupt for CAN

    // Main settings + Start timer
    PIC18FMainSettings(); 

	// Use user putc function
	stdout = _H_USER;

	// Default user mode is automatic 
	gl_userMode=AUTOMATICValue;

    TM1637_init();
   	TM1637_setBrightness(4);

}

/*****************************************************************************/
/* calibration */
/*****************************************************************************/
void calibration() {

	 unsigned short trackNumberCalibration;
	 unsigned char 	trackNumber;

	gl_mutex=1;	gl_calibration=TRUE; gl_mutex = 0;
	for(trackNumberCalibration=0;trackNumberCalibration<TIMECALIBRATION;trackNumberCalibration++);

    for(trackNumber=0;trackNumber<4;trackNumber++) {
	  gl_noVehicule[trackNumber]=0xFF;
	}
	for(trackNumberCalibration=0;trackNumberCalibration<TIMECALIBRATION;trackNumberCalibration++);	
	gl_mutex=1;	gl_calibration=FALSE;  gl_mutex = 0;
}

//////////////////////////////////////////////////////////////////////////////
// EVENT MANAGEMENT
//////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/
/* getEventRequestFromTrack() */
/*****************************************************************************/
unsigned char getEventRequestFromTrack(unsigned char* request) {

	 unsigned char trackNumber;

	for (trackNumber=0;trackNumber<4;trackNumber++) {
		if (gl_trackNotification[trackNumber]==TRUE) {
			initRequest(request);
			request[REQ_BOARD_NUMBER]=gl_boardNumber;
			request[REQ_EVENT_REQUEST_TRACK_EVENT]=TRUE;
			request[REQ_EVENT_REQUEST_EVENT_BOARD_TRACK_NUMBER]=gl_boardNumber;
			request[REQ_EVENT_REQUEST_EVENT_TRACK_NUMBER]=trackNumber;
			request[REQ_EVENT_REQUEST_EVENT_VEHICLE_STATUS]=gl_OUTSTATchar[trackNumber]==1 ? ONTRACKValue : OFFTRACKValue;
			gl_mutex=1;gl_trackNotification[trackNumber]=FALSE;gl_mutex=0;
			return (TRUE);
		}
	}
	return(FALSE);	
}

/*****************************************************************************/
/* getEventRequestFromGPIO() */
/*****************************************************************************/
unsigned char getEventRequestFromGPIO(unsigned char* request) {

	 unsigned char GPIONumber;

	for (GPIONumber=0;GPIONumber<4;GPIONumber++) {
		if (gl_GPIONotification[GPIONumber]==TRUE) {
			initRequest(request);
			request[REQ_BOARD_NUMBER]=gl_boardNumber;
			request[REQ_EVENT_REQUEST_GPIO_EVENT]=TRUE;
			request[REQ_EVENT_REQUEST_EVENT_BOARD_GPIO_NUMBER]=gl_boardNumber;
			request[REQ_EVENT_REQUEST_EVENT_GPIO_NUMBER]=GPIONumber;
			if (gl_GPIOchar[GPIONumber]==0)request[REQ_EVENT_REQUEST_EVENT_GPIO_LEVEL]=0;
			else request[REQ_EVENT_REQUEST_EVENT_GPIO_LEVEL]=gl_GPIOcounter[GPIONumber];
			gl_mutex=1;gl_GPIONotification[GPIONumber]=FALSE;gl_mutex=0;
			return(TRUE);
		}
	}
	return(FALSE);	
}

/*****************************************************************************/
/* getEventRequestFromTIMER() */
/*****************************************************************************/
unsigned char getEventRequestFromTIMER(unsigned char* request) {

	 unsigned char TIMERNumber;

	for (TIMERNumber=0;TIMERNumber<MAXTIMER;TIMERNumber++) {
		if (gl_TIMERNotification[TIMERNumber]==TRUE) {
			initRequest(request);
			request[REQ_BOARD_NUMBER]=gl_boardNumber;
			request[REQ_EVENT_REQUEST_TIMER_EVENT]=TRUE;
			request[REQ_EVENT_REQUEST_EVENT_BOARD_TIMER_NUMBER]=gl_boardNumber;
			request[REQ_EVENT_REQUEST_EVENT_TIMER_NUMBER]=TIMERNumber;
			gl_mutex=1;gl_TIMERNotification[TIMERNumber]=FALSE;gl_mutex=0;
			return(TRUE);
		}
	}
	return(FALSE);	
}

/*****************************************************************************/
/* getEventFromKNOB() */
/*****************************************************************************/
void getEventFromKNOB() {

	unsigned char trackNumber;

	// Manage knob value
	if (gl_lastKnobValue0!=gl_knobValue0||gl_lastKnobValue1!=gl_knobValue1) {
		gl_lastKnobValue0=gl_knobValue0;
		gl_lastKnobValue1=gl_knobValue1;
		TM1637_display(gl_lastKnobValue0,gl_lastKnobValue1);
		if (gl_userMode==MANUALValue) {
			for(trackNumber=0;trackNumber<4;trackNumber++) {
				setSpeed(gl_knobValue0,gl_knobValue1,trackNumber);
			}
		}
	}
}
/*****************************************************************************/
/* MAIN */
/*****************************************************************************/
void main()
{
	gl_inputCounter=0; // for UART
	sprintf(gl_inputUartString,"");
	gl_parserErrorCode=0;

	// Full init of PIC18F
    init();

	// CALIBRATION
    calibration();

	// START MAIN LOOP

	sprintf(gl_message,"Board %d",gl_boardNumber);
	prompt(gl_message);
	sprintf(gl_message,RAIL_DRIVER_HEADER);
	prompt(gl_message);
	sprintf(gl_message,VERSION);
	prompt(gl_message);
	memAvailable();
	
    while (1){
		while(1) {	

			// Manage knob value (for MANUAL and AUTOMATIC mode)
			getEventFromKNOB();

			// Get current status on tracks
			if (getEventRequestFromTrack(&gl_request)==TRUE) {
				break;
			}

			// Get current status on GPIO
			if (getEventRequestFromGPIO(&gl_request)==TRUE) {
				break;
			}

			// Get current status on TIMER	
			if (getEventRequestFromTIMER(&gl_request)==TRUE) {
				break;
			}

			// Get Request from CAN Bus
			if (getInputRequestFromCAN(&gl_request)==TRUE) {
				break;		
			}

			// Get Request from RS232 input
			if (getInputRequestFromUSART(gl_inputUartString,&gl_inputCounter)==TRUE) {
				if (strlen(gl_inputUartString)!=0) {

					// Call parser to analyse input request
	       			if (parser(gl_inputUartString,&gl_request)==TRUE) {
						sprintf(gl_inputUartString,"");
						break;
					}
					else { // parsing error
						sprintf(gl_inputUartString,"");
						traceError();
					}		
				}
				else {
					sprintf(gl_message,"");
					prompt(gl_message);
				}
			}
		}

		// Manage request
		if (gl_parserErrorCode!=0) traceError();
		else {
			if (manageRequest(&gl_request,TRUE)==FALSE){
					traceError();
			}
			initRequest(&gl_request);
		}
	}
}
