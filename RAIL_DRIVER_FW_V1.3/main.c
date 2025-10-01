/*********************************************************************************
*
* DRIVER RAIL V 1.3
*
* MAIN.C
*
**********************************************************************************


* Processor: PIC18F4680
* Frequency: 32 Mhz
* Compiler: C18
*********************************************************************************/

#include "main.h"

/* ==============================================================================
 * Function: mySprintf
 * Returns: int = the number of characters written to str (excluding the 
 * terminating null character '\0').
 * Description : Replace sprintf()
 * ============================================================================== */
int mySprintf(char *buf, const rom char *fmt, ...)
{
    int n;
    va_list args;
    va_start(args, fmt);
    n = vsprintf(buf, (const far rom char *)fmt, args); // Mandatory
    va_end(args);
    return n;
}


/////////////////////////////////////////////////////////////////////////////////
// EEPROM READ / WRITE FUNCTION 
/////////////////////////////////////////////////////////////////////////////////
///
///
/* ==============================================================================
 * Function: ReadEEPROM
 * Returns: char = SUCCESS on read, ERROR if address invalid.
 * Description: Reads one byte from data EEPROM at the given address into *data. 
 * Checks bounds and selects data EEPROM space.
 * ============================================================================== */
char ReadEEPROM(int adr, char *data){

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
} 
/* ==============================================================================
 * Function: CalibMinMaxKnob
 * Returns: void = no return.
 * Description: Interactive calibration routine for two knobs. Collects min/max 
 * ADC values, displays countdowns, stores results to EEPROM, and restores watchdog.
 * ============================================================================== */
void CalibMinMaxKnob(void) {

	 short adr;
	 char counter;

	// Knob min & max
	adr=(short)KNOB_ADDRESS;
	
	// Min knob values
	gl_mutexLowIsr=1;
	gl_minAdcKnobValue0=0xFFFF;
	gl_minAdcKnobValue1=0xFFFF;
	gl_calibKnob=1;
	gl_mutexLowIsr=0;

	// Wait to get low value
	for(counter=5;counter>0;counter--) {
		mySprintf((char *)gl_message,"CAL0 %d",(int)counter);
		TM1637_displayString((char *)gl_message);
		delayMainLoop(2); 
	}

	// Max knob values
	gl_mutexLowIsr=1;
	gl_maxAdcKnobValue0=0;
	gl_maxAdcKnobValue1=0;
	gl_mutexLowIsr=0;

	// Wait to get high value
	for(counter=5;counter>0;counter--) {
		mySprintf((char *)gl_message,"CAL1 %d",(int)counter);
		TM1637_displayString((char *)gl_message);
		delayMainLoop(2); 
	}

	mySprintf((char *)gl_message,"%S",DONE_STRING);
	TM1637_displayString((char *)gl_message);
	delayMainLoop(2); 

	gl_mutexLowIsr=1;	gl_calibKnob=0; gl_mutexLowIsr=0;

	// Save in EEPROM
	WriteEEPROM(adr++,(gl_minAdcKnobValue0>>8) & 0xFF);
	WriteEEPROM(adr++,gl_minAdcKnobValue0 & 0xFF);

	WriteEEPROM(adr++,(gl_maxAdcKnobValue0>>8) & 0xFF);			
	WriteEEPROM(adr++,gl_maxAdcKnobValue0 & 0xFF);

	WriteEEPROM(adr++,(gl_minAdcKnobValue1>>8) & 0xFF);		
	WriteEEPROM(adr++,gl_minAdcKnobValue1 & 0xFF);

	WriteEEPROM(adr++,(gl_maxAdcKnobValue1>>8) & 0xFF);	
	WriteEEPROM(adr,gl_maxAdcKnobValue1 & 0xFF);

}
/* ==============================================================================
 * Function: ResetEEPROM
 * Returns: void = no return.
 * Description: Initializes EEPROM to default values (magic number, ANA mode, 
 * clears automation, sets GPIO directions) and syncs globals.
 * ============================================================================== */
void ResetEEPROM(void){

	 char 	value;
	 short adr;
	 char  checkMagicNumberCounter;

	// Init EEPROM after flashing the board
	adr=(short)MAGICNUMBER_ADDRESS;
	for(checkMagicNumberCounter=0;checkMagicNumberCounter<MAGICNUMBERSIZE;checkMagicNumberCounter++) {
		WriteEEPROM(adr++,checkMagicNumberCounter);
	}
	// Set ANA mode
	adr=(short)MODE_ADDRESS;
	value=ANAValue;
	WriteEEPROM(adr,value);

	// No automation
	adr=(short)NEXTTAUTOMATION_ADDRESS;
	value=0;
	WriteEEPROM(adr,value);

	gl_mutexLowIsr=1;gl_boardMode = ANAValue;gl_nexAvailableAutomation=0;gl_mutexLowIsr=0;

	// GPIO IN
	for(adr=(short)GPIO0DIR_ADDRESS;adr<=(short)GPIO0DIR_ADDRESS+3;adr++){
		WriteEEPROM(adr,1);
	}
	
	// No push on CAN bus
	for(adr=(short)TRACK_CAN_NOTIFICATION_ADDRESS;adr<(short)TRACK_CAN_NOTIFICATION_ADDRESS+TRACK_SIZE;adr++){
		WriteEEPROM(adr,(char)FALSE);
	}

	// No push on CAN bus
	for(adr=(short)GPIO_CAN_NOTIFICATION_ADDRESS;adr<(short)GPIO_CAN_NOTIFICATION_ADDRESS+GPIO_SIZE;adr++){
		WriteEEPROM(adr,(char)FALSE);
	}

	// No push on CAN bus
	for(adr=(short)TIMER_CAN_NOTIFICATION_ADDRESS;adr<(short)TIMER_CAN_NOTIFICATION_ADDRESS+TIMER_SIZE;adr++){
		WriteEEPROM(adr,(char)FALSE);
	}
}
/* ==============================================================================
 * Function: ReadEEPROMConfig
 * Returns: void = no return.
 * Description: Loads configuration from EEPROM: validates magic, board mode,
 * GPIO directions, knob calibration, and automation cache.
 * ============================================================================== */
void ReadEEPROMConfig(void) {

	 char 	value;
	 char 	dummy;
	 short adr;
	 char  automationCounter;
	 char  automationDataCounter;
	 char  checkMagicNumberCounter;

	// Read MAGIC NUMBER
	adr=(char)MAGICNUMBER_ADDRESS;
	for(checkMagicNumberCounter=0;checkMagicNumberCounter<MAGICNUMBERSIZE;checkMagicNumberCounter++) {
		ReadEEPROM(adr++,&value);
		if (value!=checkMagicNumberCounter) {
			
			ResetEEPROM();
			return;
		}
	}	

	// Read in EEPROM MODE 
	adr=(short)MODE_ADDRESS;
	ReadEEPROM(adr,&value);
	gl_mutexLowIsr=1;gl_boardMode=value;gl_mutexLowIsr=0;

	// Read in GPIO dir
	adr=(short)GPIO0DIR_ADDRESS;
	ReadEEPROM(adr++,&value);
	TRISDbits.RD1=value;
	ReadEEPROM(adr++,&value);
	TRISDbits.RD2=value;
	ReadEEPROM(adr++,&value);
	TRISDbits.RD3=value;
	ReadEEPROM(adr,&value);
	TRISCbits.RC4=value;

	// Knob min & max
	adr=(short)KNOB_ADDRESS;
	ReadEEPROM(adr++,&value);
	gl_mutexLowIsr=1;gl_minAdcKnobValue0=value;gl_mutexLowIsr=0;
	ReadEEPROM(adr++,&value);
	gl_mutexLowIsr=1;gl_minAdcKnobValue0=value+(gl_minAdcKnobValue0<<8);gl_mutexLowIsr=0;
	ReadEEPROM(adr++,&value);
	gl_mutexLowIsr=1;gl_maxAdcKnobValue0=value;gl_mutexLowIsr=0;
	ReadEEPROM(adr++,&value);
	gl_mutexLowIsr=1;gl_maxAdcKnobValue0=value+(gl_maxAdcKnobValue0<<8);gl_mutexLowIsr=0;
	ReadEEPROM(adr++,&value);
	gl_mutexLowIsr=1;gl_minAdcKnobValue1=value;gl_mutexLowIsr=0;
	ReadEEPROM(adr++,&value);
	gl_mutexLowIsr=1;gl_minAdcKnobValue1=value+(gl_minAdcKnobValue1<<8);gl_mutexLowIsr=0;
	ReadEEPROM(adr++,&value);
	gl_mutexLowIsr=1;gl_maxAdcKnobValue1=value;gl_mutexLowIsr=0;
	ReadEEPROM(adr,&value);
	gl_mutexLowIsr=1;gl_maxAdcKnobValue1=value+(gl_maxAdcKnobValue1<<8);gl_mutexLowIsr=0;

	gl_mutexLowIsr=1;gl_deltaKnob0=gl_maxAdcKnobValue0-gl_minAdcKnobValue0;gl_mutexLowIsr=0;
	gl_mutexLowIsr=1;gl_deltaKnob1=gl_maxAdcKnobValue1-gl_minAdcKnobValue1;gl_mutexLowIsr=0;

	// Read in EEPROM last automation
	dummy=getAutomationFromEEPROM();
}
/* ==============================================================================
 * Function: WriteCompletedEEPROM
 * Returns: char = SUCCESS when complete, ERROR otherwise.
 * Description: Reports whether an EEPROM write cycle has completed and clears flags.
 * ============================================================================== */
char WriteCompletedEEPROM(void) {

	if(PIR2bits.EEIF){
		PIR2bits.EEIF=0;		// Clear write complete flag
		EECON1bits.WREN = 0;	// Disable write
		return(SUCCESS);		// Write to EEPROM completed
	}
	else {
		return(ERROR);			// Write to EEPROM not completed
	}
}
/* ==============================================================================
 * Function: WriteRdyEEPROM
 * Returns: char = SUCCESS when ready, ERROR when busy.
 * Description: Indicates if EEPROM is ready for a new write (no write in progress).
 * ============================================================================== */
char WriteRdyEEPROM(void){

	if(!EECON1bits.WR) {
		return(SUCCESS);	// New Write Enabled
	}
	else {
		return(ERROR);		// new Write Disabled
	}
}
/* ==============================================================================
 * Function: WriteEEPROM
 * Returns: char = SUCCESS on write, ERROR if address invalid.
 * Description: Writes one byte to data EEPROM at the given address with unlock 
 * sequence; skips write if data unchanged.
 * ============================================================================== */
char WriteEEPROM(short adr, char data){

	char checkValue;
	if(adr > 0x3FF){
		return(ERROR);
	}
	else{

		// Wait eeprom ready to be written
		while (WriteRdyEEPROM()==(char) ERROR);

		// Check if this value already exists at that address
		ReadEEPROM(adr,&checkValue);
		if (checkValue==data)return(SUCCESS);

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
		while (WriteCompletedEEPROM()==(char) IN_PROGRESS);
		while (WriteRdyEEPROM()==(char) ERROR);

		INTCONbits.GIE		= 1;	// Enable Interrupt
	
		return(SUCCESS);
	}
} 
/* ==============================================================================
 * Function: memAvailable
 * Returns: char = percentage 0..100.
 * Description: Computes approximate free automation memory percentage based on 
 * EEPROM usage.
 * ============================================================================== */
char memAvailable(void) {

	return(getAutomationFromEEPROM());
}
/* ==============================================================================
 * Function: boardStatus
 * Returns: void = no return.
 * Description: Prints board identity, mode, memory usage, user mode and next 
 * automation index to the prompt/LED display.
 * ============================================================================== */
void boardStatus(void) {

	char  value;
	short adr;
	char  timerCounter;
				
	mySprintf((char *)gl_message,"%S",RAIL_DRIVER_HEADER_STRING);
	prompt((char *)gl_message);

	mySprintf((char *)gl_message,(const rom char *)"%S",VERSION_STRING);
	prompt((char *)gl_message);

	mySprintf((char *)gl_message,"%S",BOARD_NUMBER_STRING);
	mySprintf((char *)gl_message,"%s%d",gl_message,(int)gl_boardNumber);
	prompt((char *)gl_message);

	mySprintf((char *)gl_message,"%S",MODE_STRING);
	if(gl_boardMode==ANAValue)mySprintf((char *)gl_message,"%sANA",gl_message);
	else mySprintf((char *)gl_message,"%sDCC",gl_message);
	prompt((char *)gl_message);

	mySprintf((char *)gl_message,"%S",MEMORY_STRING);
	mySprintf((char *)gl_message,"%s%d%%",gl_message,(int)memAvailable());
	prompt((char *)gl_message);

	mySprintf((char *)gl_message,"%S",AUTOMATION_STRING);
	if (gl_userMode!=AUTOMATICValue) mySprintf((char *)gl_message,"%sMANUAL",gl_message);
	else mySprintf((char *)gl_message,"%sAUTOMATIC",gl_message);
	prompt((char *)gl_message);

	mySprintf((char *)gl_message,"%S",AUTOMATION_NUMBER_STRING);
	mySprintf((char *)gl_message,"%s%d",gl_message,(int)gl_nexAvailableAutomation);
	prompt((char *)gl_message);

	for(timerCounter=0;timerCounter<MAXTIMER;timerCounter++) {
		adr=(short)TIMER_CAN_NOTIFICATION_ADDRESS+timerCounter;
		ReadEEPROM(adr,&value);
		if (value==(char)TRUE){
			mySprintf((char *)gl_message,"TIMER %d => CAN",(int)timerCounter);
			prompt((char *)gl_message);
		}
	}

	mySprintf((char *)gl_message,"");
	prompt((char *)gl_message);
}	
/* ==============================================================================
 * Function: getAutomationFromEEPROM
 * Returns: char = percentage 0..100 (100 if none).
 * Description: Reads automation entries from EEPROM into RAM and returns free 
 * space percentage.
 * ============================================================================== */
char getAutomationFromEEPROM(void) {

    char  automationCounter;
    char  automationDataCounter;
	char  value;
	short adr;
	long		   percentage;

	// Get next automation address
	adr=(short)NEXTTAUTOMATION_ADDRESS;
	ReadEEPROM(adr,&value);
	gl_mutexLowIsr=1;gl_nexAvailableAutomation=value;gl_mutexLowIsr=0;
	if (gl_nexAvailableAutomation==0) return((double)100); // nothing to do

	// Read and uncompress automation from EEPROM
	adr=(short)AUTOMATION_ADDRESS;
	for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {	
		for(automationDataCounter=0;automationDataCounter<NEW_AUTOMATIONSIZE;automationDataCounter++) {
			ReadEEPROM(adr++,&value);
			gl_mutexLowIsr=1;gl_automation[automationCounter][automationDataCounter]=value;gl_mutexLowIsr=0;
		}
	}
	percentage=100*(1024-(long)adr)/(1024-(long)AUTOMATION_ADDRESS);
	return((char)percentage);
}
/* ==============================================================================
 * Function: setAutomationToEEPROM
 * Returns: char = TRUE on success, FALSE on overflow.
 * Description: Writes automation entries from RAM to EEPROM and updates the 
 * 'next automation' pointer.
 * ============================================================================== */
char setAutomationToEEPROM(void) {

    char  automationCounter;
    char  automationDataCounter;
	short adr;
	char  value;

	adr=(short)AUTOMATION_ADDRESS;

	for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
		if (adr+NEW_AUTOMATIONSIZE>=1024) {
			mySprintf((char *)gl_errorInfo,"limit is %d",(int)gl_nexAvailableAutomation);
			gl_parserErrorCode=AUTOMATIONSIZELIMIT;
			return(FALSE);
		}	
		for(automationDataCounter=0;automationDataCounter<NEW_AUTOMATIONSIZE;automationDataCounter++) {
			value=gl_automation[automationCounter][automationDataCounter];
			WriteEEPROM(adr++,value);
		}
	}

	// update in EEPROM next automation value
	adr=(short)NEXTTAUTOMATION_ADDRESS;
	value=gl_nexAvailableAutomation;
	WriteEEPROM(adr,value);
	return(TRUE);	
}
/* ==============================================================================
 * Function: isAdigit
 * Returns: char = TRUE or FALSE.
 * Description: Checks if character is an ASCII decimal digit '0'..'9'.
 * ============================================================================== */
char isAdigit(char car) {
	if ((char)car >=(rom char)'0' && (char)car <=(rom char)'9') return TRUE;
	else return FALSE;
}
/* ==============================================================================
 * Function: isAhexaDigit
 * Returns: char = TRUE or FALSE.
 * Description: Checks if character is an uppercase hex digit 'A'..'F'.
 * ============================================================================== */
char isAhexaDigit(char car) {
	if ((char)car >=(rom char)'A' && (char)car <=(rom char) 'F') return TRUE;
	else return FALSE;
}
/* ==============================================================================
 * Function: toUpperCase
 * Returns: char = uppercased character.
 * Description: Converts ASCII letter to uppercase; returns input unchanged otherwise.
 * ============================================================================== */
char toUpperCase(char car) {

    char returnValue;

	if ((char)car >=(rom char)'a' && (char)car <= (rom char)'z') returnValue=car - 0x20;
	else returnValue=car;

	return(returnValue);
}
/* ==============================================================================
 * Function: strtol
 * Returns: short = parsed value (>=0).
 * Description: Parses a positive integer in decimal or 0x/ x-prefixed hex into 
 * a short; sets errno on error.
 * ============================================================================== */
short strtol(const char* nptr) {

    short 	result16;
    short 	result10;
	char 	base;
	short	result;
	int 	digit;

	// init
    result16=0;
    result10=0;
	base=10;
	result=0;

	if ((char)*nptr == (rom char)'0') 	nptr++;
	if ((char)*nptr == (rom char)'\0'|| (char)*nptr == (rom char)' ') return result; // 0
	if (toUpperCase(*nptr) == (rom char)'X') {	
		nptr++;
		base=16;
	}		
	while ((char)*nptr !=(rom char) '\0' && (char)*nptr !=(rom char) ' ') {
		if (isAdigit(*nptr)) {
			digit = *nptr - '0';
		}
		else {
			if (isAhexaDigit(toUpperCase(*nptr))) {
				digit = toUpperCase(*nptr) - 'A' + 10;
 				if (digit>=10)base=16;
			}
			else {
				// end conversion
				errno = ERANGE;
				return result;
			}
		}
		
		result16 = 16 * result16 + digit;
		result10 = 10 * result10 + digit;
		nptr++;
	}
	if (base==10) result=result10;
	else result=result16;

	if (result<0) {
			errno = ERANGE;
			return result;
 		}
	return((short)result);
}
/* ==============================================================================
 * Function: clearError
 * Returns: void = no return.
 * Description: Clears parser error code and associated message buffer.
 * ============================================================================== */
void clearError(void) {
		mySprintf((char *)gl_errorInfo,"");
		gl_parserErrorCode=0;
}
/* ==============================================================================
 * Function: traceError
 * Returns: void = no return.
 * Description: Formats and prints a human-readable error message for the
 * current parser error, then clears it.
 * ============================================================================== */
void traceError(void) {

	switch (gl_parserErrorCode) {	
		case UNKNOWN_TOKEN				:		mySprintf((char *)gl_message,"%S",UNKNOWN_TOKEN_STRING);break;
		case NUMBER_MISSING				:		mySprintf((char *)gl_message,"%S",NUMBER_MISSING_STRING);break;
		case INCOMPLETE_REQUEST			:		mySprintf((char *)gl_message,"%S",INCOMPLETE_REQUEST_STRING);break;
		case BAD_NUMBER					:		mySprintf((char *)gl_message,"%S",BAD_NUMBER_STRING);break;
		case MODE_MISSING				:		mySprintf((char *)gl_message,"%S",MODE_MISSING_STRING);break;
		case BAD_GPIO_NUMBER			:		mySprintf((char *)gl_message,"%S",BAD_GPIO_NUMBER_STRING);break;
		case BAD_TIMER_NUMBER			:		mySprintf((char *)gl_message,"%S",BAD_TIMER_NUMBER_STRING);break;
		case BAD_TIMER_VALUE			:		mySprintf((char *)gl_message,"%S",BAD_TIMER_VALUE_STRING);break;
		case BAD_LPO_NUMBER				:		mySprintf((char *)gl_message,"%S",BAD_LPO_NUMBER_STRING);break;
		case BAD_AUT_STATUS				:		mySprintf((char *)gl_message,"%S",BAD_AUT_STATUS_STRING);break;
		case BAD_AUT_IDENT				:		mySprintf((char *)gl_message,"%S",BAD_AUT_IDENT_STRING);break;
		case MISSING_IDENT				:		mySprintf((char *)gl_message,"%S",MISSING_IDENT_STRING);break;
		case BAD_GPIO_DIR				:		mySprintf((char *)gl_message,"%S",BAD_GPIO_DIR_STRING);break;
		case BAD_GPIO_LEVEL				:		mySprintf((char *)gl_message,"%S",BAD_GPIO_LEVEL_STRING);break;
		case BAD_LPO_LEVEL				:		mySprintf((char *)gl_message,"%S",BAD_LPO_LEVEL_STRING);break;
		case BAD_TRACK_SPEED			:		mySprintf((char *)gl_message,"%S",BAD_TRACK_SPEED_STRING);break;
		case BAD_TRACK_DIR				:		mySprintf((char *)gl_message,"%S",BAD_TRACK_DIR_STRING);break;
		case BAD_TRACK_NUMBER			:		mySprintf((char *)gl_message,"%S",BAD_TRACK_NUMBER_STRING);break;
		case BAD_BOARD_MODE					:	mySprintf((char *)gl_message,"%S",BAD_BOARD_MODE_STRING);break;
		case WRONG_BOARD_NUMBER			:		mySprintf((char *)gl_message,"%S",WRONG_BOARD_NUMBER_STRING);break;
		case AUTOMATIONSIZELIMIT		:		mySprintf((char *)gl_message,"%S",AUTOMATIONSIZELIMIT_STRING);break;
		case MISSING_SPACE				:		mySprintf((char *)gl_message,"%S",MISSING_SPACE_STRING);break;
		case IDENT_TOO_LONG				:		mySprintf((char *)gl_message,"%S",IDENT_TOO_LONG_STRING);break;
		case AUTOMATIONLREADYEXISTS		:		mySprintf((char *)gl_message,"%S",AUTOMATIONLREADYEXISTS_STRING);break;
		case BADAUTOMATIONNUMBER		:		mySprintf((char *)gl_message,"%S",BADAUTOMATIONNUMBER_STRING);break;
		case BAD_TRACK_INERTIA			:		mySprintf((char *)gl_message,"%S",BAD_TRACK_INERTIA_STRING);break;
		case BAD_USER_MODE				:		mySprintf((char *)gl_message,"%S",BAD_USER_MODE_STRING);break;
		case BAD_CANPUSH_NUMBER			:		mySprintf((char *)gl_message,"%S",BAD_CANPUSH_NUMBER_STRING);break;
		default : mySprintf((char *)gl_message,"%s 0x%x",UNKNOWN_ERROR_STRING,gl_parserErrorCode);
	}
	mySprintf((char *)gl_message,"%s %s",gl_message,gl_errorInfo);

	prompt((char *)gl_message);
	mySprintf((char *)gl_message,"");
	prompt((char *)gl_message);	
	clearError();
}
/* ==============================================================================
 * Function: getToken
 * Returns: char = TRUE on match, FALSE otherwise.
 * Description: Parses expected token from input string, advancing stringPointer;
 * records offending token on failure.
 * ============================================================================== */
char getToken(char* inputString, char* inputToken, char* stringPointer) {

	 char carCounter;
	 char testToken[MAXSIZETOKEN];
	 int length;
	 int tokenLength;
	 int carTokenCounter;

 	// init
	carCounter = 0;
	carTokenCounter = 0;
	length=strlen(inputString);
	tokenLength=strlen(inputToken);

	// remove space
	while((int)carCounter < length) {
		if ((char)inputString[carCounter]==(rom char)' ') {
			carCounter++;
			continue; 
		}
		else break;
	}

	// Copy token to test
	while((int)carCounter < length && carTokenCounter < tokenLength) {
		testToken[carTokenCounter++]=inputString[carCounter++];
	}
	
	// Test token
	if (!strncmp(testToken, inputToken, tokenLength)) {
		(*stringPointer)+=carCounter;
		return(TRUE);
	}
	
	testToken[carTokenCounter]='\0';
	mySprintf((char *)gl_errorInfo,"%s (testing %s)",testToken,inputToken);
	gl_parserErrorCode = UNKNOWN_TOKEN;
	return(FALSE);
}
/* ==============================================================================
 * Function: getValue
 * Returns: char = TRUE on success, FALSE on error.
 * Description: Parses a numeric value (0..255) from input string, sets
 * errno/globals on errors.
 * ============================================================================== */
char getValue(char* inputString, char* Value, char* stringPointer) {

	char carCounter;
	char tokenCarPointer;
	char dataFound;
	short number;
	int length;

	// Init
	carCounter = 0;
	tokenCarPointer = 0;

	errno = 0;
	length=strlen(inputString);
	while ((char)inputString[carCounter] == (rom char)' ' && carCounter < length) {
		(*stringPointer)++;
		carCounter++;
	}
	//number = strtol_with_atoi(&inputString[carCounter],0);	
	number = strtol(&inputString[carCounter]);

	if (errno == ERANGE || errno ==EINVAL) dataFound = FALSE;
	else dataFound = TRUE;

	if (number >255) {
		errno == ERANGE;
		mySprintf((char *)gl_errorInfo,"%d",(int)number);
		gl_parserErrorCode = BAD_NUMBER;
		return(FALSE);
	}
	
	if ((char)dataFound==(char)TRUE) {

		while ((char)inputString[carCounter] != (rom char)' ' && carCounter < length) {
			(*stringPointer)++;
			carCounter++;
		}

		*Value=(char)number;
		return(TRUE);
	}
	else {
		mySprintf((char *)gl_errorInfo,"");
		gl_parserErrorCode = NUMBER_MISSING;
		return(FALSE);
	}
}
/* ==============================================================================
 * Function: getIdent
 * Returns: char = TRUE on success, FALSE on error.
 * Description: Parses an automation identifier (word after a space), enforcing
 * size limits.
 * ============================================================================== */
char getIdent(char* inputString, char* stringPointer,char *ident) {

	char getFirstSpace;
	char identCounter;

	// init
	identCounter=0;

	// GET AUTOMATION IDENT
	getFirstSpace=FALSE;
	while(identCounter<MAXSIZEIDENT) {
		if ((char)inputString[*stringPointer]!=(char)' ') {
			if(identCounter==0 && (char)getFirstSpace==(char)FALSE) {
				mySprintf((char *)gl_errorInfo,"Column %d =>%c",(int)*stringPointer,(char)inputString[*stringPointer]);
				gl_parserErrorCode = MISSING_SPACE;
				return(FALSE);
			}
			else if (*stringPointer==(int)strlen(inputString)){
				if (identCounter>0) {
					 ident[identCounter]='\0'; // get end of line
					 break;
				}
				else {
					mySprintf((char *)gl_errorInfo,"");
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
			ident[identCounter-1]='\0';
			mySprintf((char *)gl_errorInfo,"%s",ident);
			gl_parserErrorCode = IDENT_TOO_LONG;
			return(FALSE);
		}
	}
	return(TRUE);
}
/* ==============================================================================
 * Function: parser
 * Returns: char = TRUE when parsed, FALSE on syntax error.
 * Description: Top‑level textual command parser. Decodes PROG/COM and subcommands 
 * into the gl_request structure.
 * ============================================================================== */
char parser(char* inputString) {

	char stringPointer;
	char keepStringPointer;
    char token[MAXSIZETOKEN];
	char boardNumber;

	// init
	stringPointer = 0;
	keepStringPointer = 0;

	initRequest();

	// STOP
    mySprintf(token,STOP);
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		gl_request[REQ_GLOBAL_COMMAND] = STOPValue;
		gl_request[REQ_BOARD_NUMBER] = gl_boardNumber;
		clearError();
		return(TRUE);
	}

	// RUNALL 
	mySprintf(token, RUNALL);
	stringPointer = 0;
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		gl_request[REQ_GLOBAL_COMMAND] = RUNALLValue;
		gl_request[REQ_BOARD_NUMBER] = gl_boardNumber;
		clearError();
		return(TRUE);
	}

	// SYNCHRO
	mySprintf(token,SYNCHRO);
	stringPointer = 0;
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		gl_request[REQ_GLOBAL_COMMAND] = SYNCHROValue;
		gl_request[REQ_BOARD_NUMBER] = gl_boardNumber;
		clearError();
		return(TRUE);
	}

	// RUN 
	mySprintf(token,RUN);
	stringPointer = 0;
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		gl_request[REQ_GLOBAL_COMMAND] = RUNValue;

		// Get board number
		if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_BOARD_NUMBER], (char *)&stringPointer))return(FALSE);
		clearError();
		return(TRUE);
	}

	// CALIB 
	mySprintf(token,CALIB);
	stringPointer = 0;
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		gl_request[REQ_GLOBAL_COMMAND] = CALIBValue;

		// Get board number
		if (!getValue((char *)&inputString[stringPointer], (char *)&gl_request[REQ_BOARD_NUMBER],(char *) &stringPointer))return(FALSE);
		clearError();
		return(TRUE);
	}

	// RESET
	mySprintf(token,RESET);
	stringPointer = 0;
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		gl_request[REQ_GLOBAL_COMMAND] = RESETValue;

		// Get board number
		if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_BOARD_NUMBER], (char *) &stringPointer))return(FALSE);
		clearError();
		return(TRUE);
	}

	//PROG
	stringPointer = 0;
	mySprintf(token,PROG);
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		gl_request[REQ_TYPE_ENTRY] = PROGValue;

		// Get board number
		if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_BOARD_NUMBER],(char *) &stringPointer)) return(FALSE);

		// get DCC or ANA or GPIO or AUT or DEL
		keepStringPointer = stringPointer;

		// DCC
		mySprintf(token,DCC);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_PROGRAM_REQUEST_SET_BOARD_MODE] = TRUE;
			gl_request[REQ_PROGRAM_REQUEST_BOARD_MODE] = DCCValue;
			clearError();
			return(TRUE);
		}
		//ANA
		mySprintf(token,ANA);
		stringPointer = keepStringPointer;
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_PROGRAM_REQUEST_SET_BOARD_MODE] = TRUE;
			gl_request[REQ_PROGRAM_REQUEST_BOARD_MODE] = ANAValue;
			clearError();
			return(TRUE);
		}
		// GPIO
		mySprintf(token,GPIO);
		stringPointer = keepStringPointer;
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_PROGRAM_REQUEST_SET_GPIO] = TRUE;

			// get GPIO number
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER],(char *) &stringPointer)) return(FALSE);
			keepStringPointer = stringPointer;

			// IN
			mySprintf(token,IN);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR] = INValue;
				clearError();
				return(TRUE);
			}

			// OUT
			else {
				stringPointer = keepStringPointer;
				mySprintf(token,OUT);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR] = OUTValue;
					clearError();
					return(TRUE);
				}
				else return(FALSE);
			}
		}
		// PUSHCAN
		mySprintf(token,PUSHCAN);
		stringPointer = keepStringPointer;
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH] = TRUE;

			// Get can_push number
			// 0,1,2,3 => Track number
			// 4,5,6,7 => GPIO Number + 4
			// 8 -> 22 => TIMER Number + 8

			// get GPIO or TIMER or TRACK 
			keepStringPointer = stringPointer;

			// TRACK 
			mySprintf(token,TRACK);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {

				// get TRACK number
				if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER],(char *) &stringPointer)) return(FALSE);
				if (gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]<0 || gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]>3) {
					mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]);
					gl_parserErrorCode = BAD_CANPUSH_NUMBER;
					return(FALSE);
				}
			}

			// GPIO 
			else {
				stringPointer = keepStringPointer;
				mySprintf(token,GPIO);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {

					// get GPIO number
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER],(char *) &stringPointer)) return(FALSE);
					else {
						if (gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]<0 || gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]>3) {
							mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]);
							gl_parserErrorCode = BAD_CANPUSH_NUMBER;
							return(FALSE);
						}
						gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]+=TRACK_SIZE;
					}
				}
				// TIMER
				else {
					stringPointer = keepStringPointer;
					mySprintf(token,TIMER);
					if (getToken(&inputString[stringPointer], token, &stringPointer)) {
	
						// get TIMER number
						if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER],(char *) &stringPointer)) return(FALSE);
						else {
							if (gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]<0 || gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]>14) {
								mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]);
								gl_parserErrorCode = BAD_CANPUSH_NUMBER;
								return(FALSE);
							}
							gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]+=TRACK_SIZE+GPIO_SIZE;
						}
					}
				}
			}


			// ONCAN
			mySprintf(token,ONCAN);
			keepStringPointer = stringPointer;
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_ACTIVE] = TRUE;
				clearError();
				return(TRUE);
			}

			// OFFCAN
			else {
				stringPointer = keepStringPointer;
				mySprintf(token,OFFCAN);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_ACTIVE] = FALSE;
					clearError();
					return(TRUE);
				}
				else return(FALSE);
			}
		}
		
		// AUT 
		stringPointer = keepStringPointer;
        mySprintf(token,AUT);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_PROGRAM_REQUEST_SET_AUTOMATION] = TRUE;

			// GET AUTOMATION IDENT
            if (getIdent((char *)inputString,(char *)&stringPointer,(char *)&gl_request[REQ_PROGRAM_REQUEST_IDENT])==(char)FALSE) {
				return(FALSE);
			}

			// GET AUTOMATION STATUS FOR MANUAL MODE
			
			// AUTON
			keepStringPointer = stringPointer;
			mySprintf(token,AUTON);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				gl_request[REQ_PROGRAM_REQUEST_STATUS_MANUAL] = AUTONValue;
			}
			else {

				// AUTOFF
				stringPointer = keepStringPointer;
				mySprintf(token,AUTOFF);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_STATUS_MANUAL] = AUTOFFValue;
				}
				else return(FALSE);
			}

			// By default, automation is active
			gl_request[REQ_PROGRAM_REQUEST_STATUS]=AUTONValue;		

			//BOARD
			mySprintf(token,BOARD);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {

				// get BOARD number
				if (!getValue(&inputString[stringPointer], &boardNumber, &stringPointer)) return(FALSE);

				// get GPIO or TIMER or TRACK 
				keepStringPointer = stringPointer;

				// GPIO 
				mySprintf(token,GPIO);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_GPIO_EVENT] = TRUE;
					gl_request[REQ_PROGRAM_REQUEST_EVENT_BOARD_GPIO_NUMBER] = boardNumber;

					// get GPIO number
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_EVENT_GPIO_NUMBER],(char *) &stringPointer)) return(FALSE);

					// get GPIO level
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_EVENT_GPIO_LEVEL],(char *) &stringPointer)) return(FALSE);
				}

				// TIMER 
				else {
					stringPointer = keepStringPointer;
					mySprintf(token,TIMER);
					if (getToken(&inputString[stringPointer], token, &stringPointer)) {
						gl_request[REQ_PROGRAM_REQUEST_TIMER_EVENT] = TRUE;
						gl_request[REQ_PROGRAM_REQUEST_EVENT_BOARD_TIMER_NUMBER] = boardNumber;

						// get TIMER number
						if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_EVENT_TIMER_NUMBER],(char *) &stringPointer)) return(FALSE);
					}

					// TRACK
					else {
						stringPointer = keepStringPointer;
						mySprintf(token,TRACK);
						if (getToken(&inputString[stringPointer], token, &stringPointer)) {
							gl_request[REQ_PROGRAM_REQUEST_TRACK_EVENT] = TRUE;
							gl_request[REQ_PROGRAM_REQUEST_EVENT_BOARD_TRACK_NUMBER] = boardNumber;

							// get TRACK number
							if ((char *)!getValue(&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_EVENT_TRACK_NUMBER],(char *) &stringPointer)) return(FALSE);

							// STA
							mySprintf(token,STA);
							if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

							// get ONTRACK or OFFTRACK 
							keepStringPointer = stringPointer;
							mySprintf(token,ONTRACK);
							if (getToken(&inputString[stringPointer], token, &stringPointer)) {
								gl_request[REQ_PROGRAM_REQUEST_EVENT_VEHICLE_STATUS] = ONTRACKValue;
							}
							else {
								stringPointer = keepStringPointer;
								mySprintf(token,OFFTRACK);
								if (getToken(&inputString[stringPointer], token, &stringPointer)) {
									gl_request[REQ_PROGRAM_REQUEST_EVENT_VEHICLE_STATUS] = OFFTRACKValue;
								}
								else return(FALSE);
							}
						}
						else return(FALSE);
					}
				}

				// ACT
				mySprintf(token,ACT);
				if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

				// get GPIO or TIMER or LPO or AUTON or AUTOFF or TRACK or DCC or MANUAL or AUTOMATIC
				keepStringPointer = stringPointer;

				// GPIO
				mySprintf(token,GPIO);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_GPIO_SETTING] = TRUE;

					// get GPIO number
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_ACTION_GPIO_SET_NUMBER],(char *) &stringPointer)) return(FALSE);

					// get GPIO level
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_ACTION_GPIO_SET_LEVEL],(char *) &stringPointer)) return(FALSE);
					clearError();
					return(TRUE);
				}

				// TIMER
				stringPointer = keepStringPointer;
				mySprintf(token,TIMER);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_TIMER_SETTING] = TRUE;

					// get TIMER number
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_ACTION_TIMER_SET_NUMBER],(char *) &stringPointer)) return(FALSE);

					// get TIMER delay
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_ACTION_TIMER_SET_DELAY],(char *) &stringPointer)) return(FALSE);
					clearError();
					return(TRUE);
				}


				// LPO
				stringPointer = keepStringPointer;
				mySprintf(token,LPO);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_LPO_SETTING] = TRUE;

					// get LPO number
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGAM_REQUEST_ACTION_LPO_SET_NUMBER],(char *) &stringPointer)) return(FALSE);

					// get LPO level
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_ACTION_LPO_SET_LEVEL],(char *) &stringPointer)) return(FALSE);
					clearError();
					return(TRUE);
				}

				// AUTON
				stringPointer = keepStringPointer;
				mySprintf(token,AUTON);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_AUT_SETTING] = TRUE;

					// GET AUTOMATION IDENT
            		if (getIdent((char *)inputString,(char *)&stringPointer,(char *)&gl_request[REQ_PROGAM_REQUEST_ACTION_AUT_SET_IDENT])==(char)FALSE) {
						return(FALSE);
					}

					// set AUT status
					gl_request[REQ_PROGRAM_REQUEST_ACTION_AUT_SET_STATUS]=AUTONValue;
					clearError();
					return(TRUE);
				}

				// AUTOFF
				stringPointer = keepStringPointer;
				mySprintf(token,AUTOFF);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_AUT_SETTING] = TRUE;

					// GET AUTOMATION IDENT
            		if (getIdent((char *)inputString,(char *)&stringPointer,(char *)&gl_request[REQ_PROGAM_REQUEST_ACTION_AUT_SET_IDENT])==(char)FALSE) {
						return(FALSE);
					}

					// set AUT status
					gl_request[REQ_PROGRAM_REQUEST_ACTION_AUT_SET_STATUS]=AUTOFFValue;
					clearError();
					return(TRUE);
				}

				// TRACK
				stringPointer = keepStringPointer;
				mySprintf(token,TRACK);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_TRACK_SETTING] = TRUE;

					// get TRACK number
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_NUMBER],(char *) &stringPointer)) return(FALSE);

					// SPEED
					mySprintf(token,SPEED);
					if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

					// KNOB0
					keepStringPointer = stringPointer;
					mySprintf(token,KNOB0);
					if (getToken(&inputString[stringPointer], token, &stringPointer)) {
						gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_SPEED]=KNOB0Value; // Value over max value 0xF
					}
					else {
						// KNOB1
						stringPointer = keepStringPointer;
						mySprintf(token,KNOB1);
						if (getToken(&inputString[stringPointer], token, &stringPointer)) {
							gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_SPEED]=KNOB1Value; // Value over max value 0xF
						}
						else {
							stringPointer = keepStringPointer;
							// get TRACK speed 
							if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_SPEED],(char *) &stringPointer)) return(FALSE);
						}
					}
					// GET FORW or BACK
					keepStringPointer = stringPointer;
					mySprintf(token,FORW);
					if (getToken(&inputString[stringPointer], token, &stringPointer)) {
						gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_DIR] = FORWValue;
					}
					else {
						stringPointer = keepStringPointer;
						mySprintf(token,BACK);
						if (getToken(&inputString[stringPointer], token, &stringPointer)) {
							gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_DIR] = BACKValue;
						}
						else return(FALSE);
					}

					// get INERTIA
					mySprintf(token,INERTIA);
					if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

					// KNOB0
					keepStringPointer = stringPointer;
					mySprintf(token,KNOB0);
					if (getToken(&inputString[stringPointer], token, &stringPointer)) {
						gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_INERTIA]=KNOB0Value; // Value over max value 0xF
					}
					else {
						// KNOB1
						stringPointer = keepStringPointer;
						mySprintf(token,KNOB1);
						if (getToken(&inputString[stringPointer], token, &stringPointer)) {
							gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_INERTIA]=KNOB1Value; // Value over max value 0xF
						}
						else {
							stringPointer = keepStringPointer;
							// get INERTIA value
							if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_INERTIA],(char *) &stringPointer)) return(FALSE);
						}
					}					
					clearError();
					return(TRUE);
				}

				// MANUAL0
				stringPointer = keepStringPointer;
				mySprintf(token,MANUAL0);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_SET_USER_MODE] = TRUE;
					gl_request[REQ_PROGRAM_REQUEST_ACTION_USER_MODE] = MANUAL0Value;
					clearError();
					return(TRUE);
				}
				// MANUAL1
				stringPointer = keepStringPointer;
				mySprintf(token,MANUAL1);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_SET_USER_MODE] = TRUE;
					gl_request[REQ_PROGRAM_REQUEST_ACTION_USER_MODE] = MANUAL1Value;
					clearError();
					return(TRUE);
				}
				// MANUAL2
				stringPointer = keepStringPointer;
				mySprintf(token,MANUAL2);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_SET_USER_MODE] = TRUE;
					gl_request[REQ_PROGRAM_REQUEST_ACTION_USER_MODE] = MANUAL2Value;
					clearError();
					return(TRUE);
				}
				// MANUAL3
				stringPointer = keepStringPointer;
				mySprintf(token,MANUAL3);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_SET_USER_MODE] = TRUE;
					gl_request[REQ_PROGRAM_REQUEST_ACTION_USER_MODE] = MANUAL3Value;
					clearError();
					return(TRUE);
				}
				// MANUAL
				stringPointer = keepStringPointer;
				mySprintf(token,MANUAL);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_SET_USER_MODE] = TRUE;
					gl_request[REQ_PROGRAM_REQUEST_ACTION_USER_MODE] = MANUALValue;
					clearError();
					return(TRUE);
				}
				// AUTOMATIC
				stringPointer = keepStringPointer;
				mySprintf(token,AUTOMATIC);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_SET_USER_MODE] = TRUE;
					gl_request[REQ_PROGRAM_REQUEST_ACTION_USER_MODE] = AUTOMATICValue;
					clearError();
					return(TRUE);
				}			
				// DCC
				stringPointer = keepStringPointer;
				mySprintf(token,DCC);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_PROGRAM_REQUEST_DCC_SETTING] = TRUE;

					// Get DCC Address
					if (!getValue((char *)&inputString[stringPointer], (char *)&gl_request[REQ_PROGRAM_REQUEST_ACTION_DCC_ADDRESS_SETTING],(char *) &stringPointer))return(FALSE);

					// get DCC Command
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_ACTION_DCC_COMMAND_SETTING],(char *) &stringPointer))return(FALSE);
					clearError();
					return(TRUE);
				}
				return(FALSE);
			}
			else return(FALSE);

		}

		// DEL
		stringPointer = keepStringPointer;
		mySprintf(token,DEL);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_PROGRAM_REQUEST_DEL_AUTOMATION] = TRUE;

			// get Automation number
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER],(char *) &stringPointer)) return(FALSE);
			clearError();
			return(TRUE);
		}
	}

	//COM
	stringPointer = 0;
	mySprintf(token,COM);
	if (getToken(&inputString[stringPointer], token, &stringPointer)) {
		gl_request[REQ_TYPE_ENTRY] = COMValue;

		// Get board number
		if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_BOARD_NUMBER],(char *) &stringPointer)) return(FALSE);

		// GPIO
		keepStringPointer = stringPointer;
		mySprintf(token,GPIO);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			gl_request[REQ_COMMAND_REQUEST_SET_GPIO] = TRUE;

			// get GPIO number
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER],(char *) &stringPointer)) return(FALSE);

			// get GPIO level
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_GPIO_LEVEL],(char *) &stringPointer)) return(FALSE);
			clearError();
			return(TRUE);
		}

		// TIMER
		stringPointer = keepStringPointer;
		mySprintf(token,TIMER);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			gl_request[REQ_COMMAND_REQUEST_SET_TIMER] = TRUE;

			// get TIMER number
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_TIMER_NUMBER], (char *)&stringPointer)) return(FALSE);

			// get TIMER delay
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_TIMER_DELAY], (char *)&stringPointer)) return(FALSE);
			clearError();
			return(TRUE);
		}

		// LPO 
		stringPointer = keepStringPointer;
		mySprintf(token,LPO);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			gl_request[REQ_COMMAND_REQUEST_SET_LPO] = TRUE;

			// get LPO number
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_LPO_NUMBER],(char *) &stringPointer)) return(FALSE);

			// get LPO level
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_LPO_LEVEL],(char *) &stringPointer)) return(FALSE);
			clearError();
			return(TRUE);
		}

		// AUTON 
		stringPointer = keepStringPointer;
		mySprintf(token,AUTON);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			gl_request[REQ_COMMAND_REQUEST_SET_AUT] = TRUE;

			// GET AUTOMATION IDENT
           	if (getIdent((char *)inputString,(char *)&stringPointer,(char *)&gl_request[REQ_COMMAND_REQUEST_AUT_IDENT])==(char)FALSE) {
				return(FALSE);
			}

			// set AUT status
			gl_request[REQ_COMMAND_REQUEST_AUT_STATUS]=AUTONValue;
			clearError();
			return(TRUE);
		}

		// AUTOFF
		stringPointer = keepStringPointer;
		mySprintf(token,AUTOFF);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			gl_request[REQ_COMMAND_REQUEST_SET_AUT] = TRUE;

			// GET AUTOMATION IDENT
           	if (getIdent((char *)inputString,(char *)&stringPointer,(char *)&gl_request[REQ_COMMAND_REQUEST_AUT_IDENT])==(char)FALSE) {
				return(FALSE);
			}

			// set AUT status
			gl_request[REQ_COMMAND_REQUEST_AUT_STATUS]=AUTOFFValue;
			clearError();
			return(TRUE);
		}

		// TRACK
		stringPointer = keepStringPointer;
		mySprintf(token,TRACK);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {

			gl_request[REQ_COMMAND_REQUEST_SET_TRACK] = TRUE;

			// get TRACK number
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_TRACK_NUMBER],(char *) &stringPointer)) return(FALSE);

			// SPEED
			mySprintf(token,SPEED);
			if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

			// KNOB0
			keepStringPointer = stringPointer;
			mySprintf(token,KNOB0);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]=KNOB0Value; // Value over max value 0xF
			}
			else {
				// KNOB1
				stringPointer = keepStringPointer;
				mySprintf(token,KNOB1);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]=KNOB1Value; // Value over max value 0xF
				}
				else {
					stringPointer = keepStringPointer;
					// get TRACK speed 
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED],(char *) &stringPointer)) return(FALSE);
				}
			}

			// get FORW or BACK 
			keepStringPointer = stringPointer;
			mySprintf(token,FORW);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				gl_request[REQ_COMMAND_REQUEST_TRACK_DIR] = FORWValue;
			}
			else {
				stringPointer = keepStringPointer;
				mySprintf(token,BACK);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_COMMAND_REQUEST_TRACK_DIR] = BACKValue;
				}
				else return(FALSE);
			}

			// INERTIA
			mySprintf(token,INERTIA);
			if (!getToken(&inputString[stringPointer], token, &stringPointer)) return(FALSE);

			// KNOB0
			keepStringPointer = stringPointer;
			mySprintf(token,KNOB0);
			if (getToken(&inputString[stringPointer], token, &stringPointer)) {
				gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]=KNOB0Value; // Value over max value 0xF
			}
			else {
				// KNOB1
				stringPointer = keepStringPointer;
				mySprintf(token,KNOB1);
				if (getToken(&inputString[stringPointer], token, &stringPointer)) {
					gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]=KNOB1Value; // Value over max value 0xF
				}
				else {
					stringPointer = keepStringPointer;
					// get INERTIA value
					if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA],(char *) &stringPointer)) return(FALSE);
				}
			}					

			clearError();
			return(TRUE);
		}

		// MANUAL0
		stringPointer = keepStringPointer;
		mySprintf(token,MANUAL0);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_SET_USER_MODE] = TRUE;
			gl_request[REQ_COMMAND_REQUEST_USER_MODE] = MANUAL0Value;
			clearError();
			return(TRUE);
		}
		// MANUAL1
		stringPointer = keepStringPointer;
		mySprintf(token,MANUAL1);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_SET_USER_MODE] = TRUE;
			gl_request[REQ_COMMAND_REQUEST_USER_MODE] = MANUAL1Value;
			clearError();
			return(TRUE);
		}
		// MANUAL2
		stringPointer = keepStringPointer;
		mySprintf(token,MANUAL2);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_SET_USER_MODE] = TRUE;
			gl_request[REQ_COMMAND_REQUEST_USER_MODE] = MANUAL2Value;
			clearError();
			return(TRUE);
		}
		// MANUAL3
		stringPointer = keepStringPointer;
		mySprintf(token,MANUAL3);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_SET_USER_MODE] = TRUE;
			gl_request[REQ_COMMAND_REQUEST_USER_MODE] = MANUAL3Value;
			clearError();
			return(TRUE);
		}

		// MANUAL
		stringPointer = keepStringPointer;
		mySprintf(token,MANUAL);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_SET_USER_MODE] = TRUE;
			gl_request[REQ_COMMAND_REQUEST_USER_MODE] = MANUALValue;
			clearError();
			return(TRUE);
		}

		// AUTOMATIC
		stringPointer = keepStringPointer;
		mySprintf(token,AUTOMATIC);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_SET_USER_MODE] = TRUE;
			gl_request[REQ_COMMAND_REQUEST_USER_MODE] = AUTOMATICValue;
			clearError();
			return(TRUE);
		}
		// DCC
		stringPointer = keepStringPointer;
		mySprintf(token,DCC);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_SET_DCC]= TRUE;

			// Get DCC Address
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_DCC_ADDRESS],(char *) &stringPointer)) return(FALSE);

			// get DCC Command
			if (!getValue((char *)&inputString[stringPointer],(char *) &gl_request[REQ_COMMAND_REQUEST_DCC_COMMAND],(char *) &stringPointer))  {
				return(FALSE);
			}
			clearError();
			return(TRUE);
		}

		// GSTAT
		stringPointer = keepStringPointer;
		mySprintf(token,GSTAT);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_GET_GPIO_STATUS] = TRUE;
			clearError();
			return(TRUE);
		}
		// LSTAT
		stringPointer = keepStringPointer;
		mySprintf(token,LSTAT);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_GET_LPO_STATUS] = TRUE;
			clearError();
			return(TRUE);
		}
		// TSTAT
		stringPointer = keepStringPointer;
		mySprintf(token,TSTAT);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_GET_TRACK_STATUS] = TRUE;
			clearError();
			return(TRUE);
		}
		// BSTAT
		stringPointer = keepStringPointer;
		mySprintf(token,BSTAT);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_GET_BOARD_STATUS] = TRUE;
			clearError();
			return(TRUE);
		}
		// AUTLIST
		stringPointer = keepStringPointer;
		mySprintf(token,AUTLIST);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_GET_AUTOMATION_LIST] = TRUE;
			clearError();
			return(TRUE);
		}
		// DUMP
		stringPointer = keepStringPointer;
		mySprintf(token,DUMP);
		if (getToken(&inputString[stringPointer], token, &stringPointer)) {
			gl_request[REQ_COMMAND_REQUEST_GET_DUMP] = TRUE;
			clearError();
			return(TRUE);
		}
	}
	return(FALSE);
}
/* ==============================================================================
 * Function: uncompressData
 * Returns: char = TRUE on success, FALSE on overflow.
 * Description: Run‑length decodes gl_request buffer from (count,value) pairs 
 * into raw bytes.
 * ============================================================================== */
char uncompressData(void) {
	char dataCounter;
	char tmpDataCounter;
	char repeat;

	// Codage is simply a list of (X,Y) where X is the number of Y. If X = 0 there is no more data

	tmpDataCounter=0;
	for(dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++){
		 gl_tmpBuffer[dataCounter]=0;
	}

	for(dataCounter=0;dataCounter<MAXTRAMESIZE-2;dataCounter+=2) {
		if(gl_request[dataCounter]==0) break;
		for(repeat=0;repeat<gl_request[dataCounter];repeat++) {
			if (tmpDataCounter>=REQUESTSIZE) {
				initRequest();
				return(FALSE);
			}
			gl_tmpBuffer[tmpDataCounter++]=gl_request[dataCounter+1];
		}
	}
	for(dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++){
		gl_request[dataCounter]=gl_tmpBuffer[dataCounter];
	}
	return(TRUE);
}
/* ==============================================================================
 * Function: compressData
 * Returns: char = number of bytes produced (0 on failure).
 * Description: Run‑length encodes gl_request into (count,value) pairs for 
 * CAN transmission.
 * ============================================================================== */
char compressData(void) {

	char tmpDataCounter;
	char dataCounter;
	char quantityValue;
	char curValue;

	// Init
	dataCounter=0;
	tmpDataCounter=0;
	curValue=gl_request[0];
	quantityValue=0;

	while(dataCounter<REQUESTSIZE) {
		while(gl_request[dataCounter]==curValue) {
			quantityValue++;
			dataCounter++;
			if (dataCounter>=REQUESTSIZE) break;
		}
		if (tmpDataCounter<MAXTRAMESIZE-1 && tmpDataCounter<REQUESTSIZE-2) {
			gl_tmpBuffer[tmpDataCounter++]=quantityValue;
			gl_tmpBuffer[tmpDataCounter++]=curValue;
		}
		if (tmpDataCounter>=MAXTRAMESIZE) {
			for(dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++){
				gl_request[dataCounter]=0;
			}
			return(0); // We loose the trame, because we can't compress it, should never happen.....
		}
		curValue=gl_request[dataCounter];
		quantityValue=0;
	}
	for(dataCounter=0;dataCounter<tmpDataCounter;dataCounter++){
		gl_request[dataCounter]=gl_tmpBuffer[dataCounter];
	}
	for(;dataCounter<REQUESTSIZE;dataCounter++) {
		gl_request[dataCounter]=0;
	}
	return(tmpDataCounter);
}
/* ==============================================================================
 * Function: initRequest
 * Returns: void = no return.
 * Description: Zeros the gl_request buffer to a clean state.
 * ============================================================================== */
void initRequest(void) {	
	unsigned char dataCounter;
	for(dataCounter=0;dataCounter<(unsigned char)REQUESTSIZE;dataCounter++){
		gl_request[dataCounter]=0;
	}
}
/* ==============================================================================
 * Function: removeAutomation
 * Returns: char = TRUE on success, FALSE on error.
 * Description: Deletes an automation by index by compacting the array and
 * persisting to EEPROM.
 * ============================================================================== */
char removeAutomation(char automationNumber) {

    char  automationDataCounter;

	if (automationNumber>=gl_nexAvailableAutomation) {
		mySprintf((char *)gl_errorInfo,"%d",(int)automationNumber);
		gl_parserErrorCode=BADAUTOMATIONNUMBER;
		return(FALSE);
	}
	for(automationDataCounter=0;automationDataCounter<NEW_AUTOMATIONSIZE;automationDataCounter++) {
		gl_mutexLowIsr=1;gl_automation[automationNumber][automationDataCounter]=gl_automation[gl_nexAvailableAutomation][automationDataCounter];gl_mutexLowIsr=0;
	}
	// update in EEPROM automation
	return(setAutomationToEEPROM());
}
/* ==============================================================================
 * Function: saveAutomation
 * Returns: char = TRUE on success, FALSE on error.
 * Description: Copies the currently parsed program request into an automation
 * slot and persists it.
 * ============================================================================== */
char saveAutomation(char automationNumber) {

    char  automationCounter;
    char  automationDataCounter;
	short adr;
	char  value;

	if (automationNumber>=MAXAUTOMATION) {
		mySprintf((char *)gl_errorInfo,"%d",automationNumber);
		gl_parserErrorCode=AUTOMATIONSIZELIMIT;
		return(FALSE);
	}

	gl_mutexLowIsr=1;

	// copy new automation content in gl_automation
	if((char)gl_request[REQ_PROGRAM_REQUEST_GPIO_EVENT]==(char)TRUE) {
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_TYPE]=GPIO_EVENT;
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_BOARD_NUMBER]=gl_request[REQ_PROGRAM_REQUEST_EVENT_BOARD_GPIO_NUMBER];
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_NUMBER]=gl_request[REQ_PROGRAM_REQUEST_EVENT_GPIO_NUMBER];
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_VALUE]=gl_request[REQ_PROGRAM_REQUEST_EVENT_GPIO_LEVEL];
	}
	else if((char)gl_request[REQ_PROGRAM_REQUEST_TIMER_EVENT]==(char)TRUE) {
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_TYPE]=TIMER_EVENT;
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_BOARD_NUMBER]=gl_request[REQ_PROGRAM_REQUEST_EVENT_BOARD_TIMER_NUMBER];
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_NUMBER]=gl_request[REQ_PROGRAM_REQUEST_EVENT_TIMER_NUMBER];
	}
	else if((char)gl_request[REQ_PROGRAM_REQUEST_TRACK_EVENT]==(char)TRUE) {
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_TYPE]=TRACK_EVENT;
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_BOARD_NUMBER]=gl_request[REQ_PROGRAM_REQUEST_EVENT_BOARD_TRACK_NUMBER];
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_NUMBER]=gl_request[REQ_PROGRAM_REQUEST_EVENT_TRACK_NUMBER];
		gl_automation[automationNumber][NEW_AUTOMATION_EVENT_VALUE]=gl_request[REQ_PROGRAM_REQUEST_EVENT_VEHICLE_STATUS];
	}

	if ((char)gl_request[REQ_PROGRAM_REQUEST_GPIO_SETTING]==(char)TRUE) {
		gl_automation[automationNumber][NEW_AUTOMATION_SET_COMMAND]=SET_GPIO;
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_1]=gl_request[REQ_PROGRAM_REQUEST_ACTION_GPIO_SET_NUMBER];
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_2]=gl_request[REQ_PROGRAM_REQUEST_ACTION_GPIO_SET_LEVEL];

	}
	else if ((char)gl_request[REQ_PROGRAM_REQUEST_TIMER_SETTING]==(char)TRUE) {
		gl_automation[automationNumber][NEW_AUTOMATION_SET_COMMAND]=SET_TIMER;
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_1]=gl_request[REQ_PROGRAM_REQUEST_ACTION_TIMER_SET_NUMBER];
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_2]=gl_request[REQ_PROGRAM_REQUEST_ACTION_TIMER_SET_DELAY];
	}
	else if ((char)gl_request[REQ_PROGRAM_REQUEST_LPO_SETTING]==(char)TRUE) {
		gl_automation[automationNumber][NEW_AUTOMATION_SET_COMMAND]=SET_LPO;
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_1]=gl_request[REQ_PROGAM_REQUEST_ACTION_LPO_SET_NUMBER];
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_2]=gl_request[REQ_PROGRAM_REQUEST_ACTION_LPO_SET_LEVEL];
	}

	else if ((char)gl_request[REQ_PROGRAM_REQUEST_AUT_SETTING]==(char)TRUE) {
		gl_automation[automationNumber][NEW_AUTOMATION_SET_COMMAND]=SET_AUT;
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_1]=gl_request[REQ_PROGAM_REQUEST_ACTION_AUT_SET_IDENT];
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_2]=gl_request[REQ_PROGAM_REQUEST_ACTION_AUT_SET_IDENT+1];
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_3]=gl_request[REQ_PROGRAM_REQUEST_ACTION_AUT_SET_STATUS];
	}
	else if ((char)gl_request[REQ_PROGRAM_REQUEST_TRACK_SETTING]==(char)TRUE) {
		gl_automation[automationNumber][NEW_AUTOMATION_SET_COMMAND]=SET_TRACK;
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_1]=gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_NUMBER];
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_2]=gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_SPEED];
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_3]=gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_DIR];
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_4]=gl_request[REQ_PROGRAM_REQUEST_ACTION_TRACK_SET_INERTIA];
	}
	else if ((char)gl_request[REQ_PROGRAM_REQUEST_SET_USER_MODE]==(char)TRUE) {
		gl_automation[automationNumber][NEW_AUTOMATION_SET_COMMAND]=SET_USER_MODE;
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_1]=gl_request[REQ_PROGRAM_REQUEST_ACTION_USER_MODE];
	}
	else if ((char)gl_request[REQ_PROGRAM_REQUEST_DCC_SETTING]==(char)TRUE) {
		gl_automation[automationNumber][NEW_AUTOMATION_SET_COMMAND]=SET_DCC;
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_1]=gl_request[REQ_PROGRAM_REQUEST_ACTION_DCC_ADDRESS_SETTING];
		gl_automation[automationNumber][NEW_AUTOMATION_SET_PARAM_2]=gl_request[REQ_PROGRAM_REQUEST_ACTION_DCC_COMMAND_SETTING];
	}

	gl_automation[automationNumber][NEW_AUTOMATION_STATUS]=gl_request[REQ_PROGRAM_REQUEST_STATUS];
	gl_automation[automationNumber][NEW_AUTOMATION_STATUS_MANUAL]=gl_request[REQ_PROGRAM_REQUEST_STATUS_MANUAL];
	gl_automation[automationNumber][NEW_AUTOMATION_IDENT]=gl_request[REQ_PROGRAM_REQUEST_IDENT];
	gl_automation[automationNumber][NEW_AUTOMATION_IDENT+1]=gl_request[REQ_PROGRAM_REQUEST_IDENT+1];
	gl_automation[automationNumber][NEW_AUTOMATION_IDENT+2]='\0';
	if (automationNumber==gl_nexAvailableAutomation)gl_nexAvailableAutomation++;

	gl_mutexLowIsr=0;

	// update in EEPROM automation
	return(setAutomationToEEPROM());
}
/* ==============================================================================
 * Function: assignAutomation
 * Returns: void = no return.
 * Description: Builds a COM request in gl_request from a stored automation entry.
 * ============================================================================== */
void assignAutomation(char automationCounter) {

	char counterCommand;
	char command;

	// set request for command
	initRequest();

	gl_request[REQ_TYPE_ENTRY]=COMValue;
	gl_request[REQ_BOARD_NUMBER]=gl_boardNumber;

	// Init
	for(counterCommand=REQ_COMMAND_REQUEST_SET_GPIO;counterCommand<=REQ_COMMAND_REQUEST_DCC_COMMAND;counterCommand++) {
		gl_request[counterCommand]=0;
	}	

	// Copy from automation

	command=gl_automation[automationCounter][NEW_AUTOMATION_SET_COMMAND];
	switch (command) {
		case SET_GPIO : gl_request[REQ_COMMAND_REQUEST_SET_GPIO]=TRUE;
						gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_1];
					    gl_request[REQ_COMMAND_REQUEST_GPIO_LEVEL]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_2];
						break;

		case SET_TIMER :gl_request[REQ_COMMAND_REQUEST_SET_TIMER]=TRUE;
						gl_request[REQ_COMMAND_REQUEST_TIMER_NUMBER]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_1];
					    gl_request[REQ_COMMAND_REQUEST_TIMER_DELAY]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_2];
						break;

		case SET_LPO :	gl_request[REQ_COMMAND_REQUEST_SET_LPO]=TRUE;
						gl_request[REQ_COMMAND_REQUEST_LPO_NUMBER]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_1];
					    gl_request[REQ_COMMAND_REQUEST_LPO_LEVEL]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_2];
						break;
	
		case SET_AUT :	gl_request[REQ_COMMAND_REQUEST_SET_AUT]=TRUE;
						gl_request[REQ_COMMAND_REQUEST_AUT_IDENT]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_1];
						gl_request[REQ_COMMAND_REQUEST_AUT_IDENT+1]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_2];
					    gl_request[REQ_COMMAND_REQUEST_AUT_STATUS]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_3];
						break;

		case SET_TRACK :gl_request[REQ_COMMAND_REQUEST_SET_TRACK]=TRUE;
						gl_request[REQ_COMMAND_REQUEST_TRACK_NUMBER]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_1];
					    gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_2];
					    gl_request[REQ_COMMAND_REQUEST_TRACK_DIR]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_3];
					    gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_4];
						break;

		case SET_USER_MODE :gl_request[REQ_COMMAND_REQUEST_SET_USER_MODE]=TRUE;
						gl_request[REQ_COMMAND_REQUEST_USER_MODE]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_1];
						break;

		case SET_DCC :	gl_request[REQ_COMMAND_REQUEST_SET_DCC]=TRUE;
						gl_request[REQ_COMMAND_REQUEST_DCC_ADDRESS]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_1];
					    gl_request[REQ_COMMAND_REQUEST_DCC_COMMAND]=gl_automation[automationCounter][NEW_AUTOMATION_SET_PARAM_2];
						break;

	}
}
/* ==============================================================================
 * Function: setSpeed
 * Returns: void = no return.
 * Description: Sets target speed and inertia step for a track index (internal
 * fixed‑point).
 * ============================================================================== */
void setSpeed(char speed,char step,char trackNumber) {
	gl_mutexLowIsr=1;gl_setPoint[trackNumber]=((int)speed * (int)MAXINTERNALSPEED);	gl_mutexLowIsr=0;
		gl_mutexLowIsr=1;gl_setStep[trackNumber]=step;	gl_mutexLowIsr=0;
}
/* ==============================================================================
 * Function: manageRequest
 * Returns: char = TRUE on handled, FALSE on validation error.
 * Description: Executes the request described by gl_request: events, global
 * commands, or COM/PROG actions; may send/receive via CAN.
 * ============================================================================== */
char manageRequest (char sendPrompt) {

	 char automationCounter;	 
	 char automationDataCounter;

	 char eventBoardTrackNumber;
	 char eventTrackNumber;

	 char eventVehicleStatus;
	 char statusCounter;
	 char dataCounter;

	 char eventBoardGPIONumber;
	 char eventGPIONumber;
	 char eventGPIOLevel;

	 char eventBoardTIMERNumber;
	 char eventTIMERNumber;

	 short adr; 
	 short adrLast;
	 char value;
	 char writeEEPROMCounter;
	 char length;
	 char i;
	 char notEqual;
	
	 char itemPerLine;

	 char speed;
	 char step;
	 char trackNumber;
	 char TIMERCounter;

	 char command;
	 char type;	
	 char gpio;

	// CHECK EVENT FIRST
	if ((char)gl_request[REQ_EVENT_REQUEST_TRACK_EVENT] == (char) TRUE) {

		// Event from this board is sent to the others 
		if (((char)gl_request[REQ_BOARD_NUMBER] == (char) gl_boardNumber) && ((char)gl_request[REQ_EVENT_REQUEST_EVENT_CAN_NOTIFICATION] == (char)TRUE))  sendRequestToCAN();

		// Keep values as request struct should be reset
		eventBoardTrackNumber=gl_request[REQ_EVENT_REQUEST_EVENT_BOARD_TRACK_NUMBER];
		eventTrackNumber=gl_request[REQ_EVENT_REQUEST_EVENT_TRACK_NUMBER];
		eventVehicleStatus=gl_request[REQ_EVENT_REQUEST_EVENT_VEHICLE_STATUS];

		for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
				if(gl_automation[automationCounter][NEW_AUTOMATION_EVENT_TYPE]==TRACK_EVENT && gl_automation[automationCounter][NEW_AUTOMATION_STATUS]==AUTONValue) {
				if (gl_automation[automationCounter][NEW_AUTOMATION_EVENT_BOARD_NUMBER]==eventBoardTrackNumber &&
					gl_automation[automationCounter][NEW_AUTOMATION_EVENT_NUMBER]==eventTrackNumber &&
					gl_automation[automationCounter][NEW_AUTOMATION_EVENT_VALUE]==eventVehicleStatus) {

						// set request for command
						assignAutomation(automationCounter);
						if (manageRequest(FALSE)==(char)FALSE) return (FALSE);
						else continue;	
					}
				}	
			}	
			return(TRUE);
	}
	else if ((char)gl_request[REQ_EVENT_REQUEST_GPIO_EVENT] == (char) TRUE) {

		// Event from this board is sent to the others 
		if (((char)gl_request[REQ_BOARD_NUMBER] == (char) gl_boardNumber) && ((char)gl_request[REQ_EVENT_REQUEST_EVENT_CAN_NOTIFICATION] == (char)TRUE)) sendRequestToCAN();

		// Keep values as request struct should be reset
		eventBoardGPIONumber=gl_request[REQ_EVENT_REQUEST_EVENT_BOARD_GPIO_NUMBER];
		eventGPIONumber=gl_request[REQ_EVENT_REQUEST_EVENT_GPIO_NUMBER];
		eventGPIOLevel=gl_request[REQ_EVENT_REQUEST_EVENT_GPIO_LEVEL];

		for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
			if(gl_automation[automationCounter][NEW_AUTOMATION_EVENT_TYPE]==GPIO_EVENT && gl_automation[automationCounter][NEW_AUTOMATION_STATUS]==AUTONValue) {
				if (gl_automation[automationCounter][NEW_AUTOMATION_EVENT_BOARD_NUMBER]==eventBoardGPIONumber &&
					gl_automation[automationCounter][NEW_AUTOMATION_EVENT_NUMBER]==eventGPIONumber &&
					gl_automation[automationCounter][NEW_AUTOMATION_EVENT_VALUE]==eventGPIOLevel) {

						// set request for command
						assignAutomation(automationCounter);
						if (manageRequest(FALSE)==(char)FALSE) return (FALSE);
						else continue;	
					}
				}	
			}	
			return(TRUE);
	}
	else if ((char)gl_request[REQ_EVENT_REQUEST_TIMER_EVENT] == (char) TRUE) {

		// Event from this board is sent to the others 
		if (((char)gl_request[REQ_BOARD_NUMBER] == (char) gl_boardNumber) && ((char)gl_request[REQ_EVENT_REQUEST_EVENT_CAN_NOTIFICATION] == (char)TRUE)) sendRequestToCAN();

		// Keep values as request struct should be reset
		eventBoardTIMERNumber=gl_request[REQ_EVENT_REQUEST_EVENT_BOARD_TIMER_NUMBER];
		eventTIMERNumber=gl_request[REQ_EVENT_REQUEST_EVENT_TIMER_NUMBER];

		for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
			if(gl_automation[automationCounter][NEW_AUTOMATION_EVENT_TYPE]==TIMER_EVENT && gl_automation[automationCounter][NEW_AUTOMATION_STATUS]==AUTONValue) {
				if (gl_automation[automationCounter][NEW_AUTOMATION_EVENT_BOARD_NUMBER]==eventBoardTIMERNumber &&
					gl_automation[automationCounter][NEW_AUTOMATION_EVENT_NUMBER]==eventTIMERNumber) {

						// set request for command
						assignAutomation(automationCounter);
						if (manageRequest(FALSE)==(char)FALSE) return (FALSE);
						else continue;		
					}
				}	
			}	
			return(TRUE);
	}
	
    //Global command
	command=gl_request[REQ_GLOBAL_COMMAND];
    switch (command) {

		case STOPValue: 
			gl_mutexLowIsr=1;gl_stopAll=TRUE;	gl_mutexLowIsr=0;
			if ((char)gl_request[REQ_BOARD_NUMBER] == (char)gl_boardNumber) sendRequestToCAN();
			mySprintf((char *)gl_message,STOP_STRING);
			TM1637_displayString((char *)gl_message);
			if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
			return(TRUE);

		case RUNALLValue :
			gl_mutexLowIsr=1;gl_stopAll=FALSE;gl_mutexLowIsr=0;
			if ((char)gl_request[REQ_BOARD_NUMBER] == (char)gl_boardNumber) sendRequestToCAN();
			mySprintf((char *)gl_message,RUN_STRING);
			TM1637_displayString((char *)gl_message);
			if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
			return(TRUE);

		case SYNCHROValue:
			CAN_SendSync();
			if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
			return(TRUE);

		case RUNValue :
			if ((char)gl_request[REQ_BOARD_NUMBER] != (char)gl_boardNumber && (char)gl_master==(char)TRUE) sendRequestToCAN();
			else if ((char)gl_request[REQ_BOARD_NUMBER] == (char)gl_boardNumber){
				gl_mutexLowIsr=1;gl_stopAll=FALSE;gl_mutexLowIsr=0;
				mySprintf((char *)gl_message,RUN_STRING);
				TM1637_displayString((char *)gl_message);
			}

			if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
		 	return(TRUE); 

		case CALIBValue :
			if ((char)gl_request[REQ_BOARD_NUMBER] != (char)gl_boardNumber && (char)gl_master==(char)TRUE) sendRequestToCAN();
			else if ((char)gl_request[REQ_BOARD_NUMBER] == (char)gl_boardNumber) {
				gl_mutexLowIsr=1;gl_stopAll=TRUE;gl_mutexLowIsr=0;
			    CalibMinMaxKnob();
				gl_mutexLowIsr=1;gl_stopAll=FALSE;gl_mutexLowIsr=0;			
			}
			if ((char)sendPrompt==(char)TRUE) {
				prompt((char *)gl_message);
			}
		 	return(TRUE);
 
		case RESETValue :
			if ((char)gl_request[REQ_BOARD_NUMBER] != (char)gl_boardNumber && (char)gl_master==(char)TRUE) sendRequestToCAN();
			else if ((char)gl_request[REQ_BOARD_NUMBER] == (char)gl_boardNumber) {
				mySprintf((char *)gl_message,RESET_STRING);
				TM1637_displayString((char *)gl_message);
				gl_mutexLowIsr=1;gl_stopAll=TRUE;gl_mutexLowIsr=0;
				ResetEEPROM();
				delayMainLoop(2);
				init();
			}
			if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
		 	return(TRUE); 

		default : 

			// Check board number for command or program, on the master side, we forward the request on CAN bus
			if ((char)gl_request[REQ_BOARD_NUMBER] != (char)gl_boardNumber){
				if ((char)gl_master==(char)TRUE) {

					// Send request to CAN
               	 	sendRequestToCAN();
					if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
				}
				return(TRUE); // Not for us
			}

			type=gl_request[REQ_TYPE_ENTRY];
			switch(type) {
			case PROGValue: 
				if ((char)gl_request[REQ_PROGRAM_REQUEST_SET_BOARD_MODE]==(char)TRUE) {
					if((char)gl_request[REQ_PROGRAM_REQUEST_BOARD_MODE]==(char)DCCValue) {
							gl_mutexLowIsr=1;	setDcc(0,0); gl_boardMode=DCCValue; gl_mutexLowIsr=0;
								
							// Save to EEPROM
							adr=(short)MODE_ADDRESS;
							WriteEEPROM(adr,gl_boardMode);
							if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
							return(TRUE);
					}
					else if((char)gl_request[REQ_PROGRAM_REQUEST_BOARD_MODE]==(char)ANAValue) {
							gl_mutexLowIsr=1; gl_boardMode=ANAValue; gl_mutexLowIsr=0;

							// Save to EEPROM
							adr=(short)MODE_ADDRESS;
							WriteEEPROM(adr,gl_boardMode);
							if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
							return(TRUE);
					}
					else {
						mySprintf((char *)gl_errorInfo,"0x%x",gl_request[REQ_PROGRAM_REQUEST_BOARD_MODE]);
						gl_parserErrorCode=MODE_MISSING;
						return(FALSE);
					}
				}
				if ((char)gl_request[REQ_PROGRAM_REQUEST_SET_GPIO] == (char)TRUE) {		
					if ((char)gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR]==(char)INValue || (char)gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR]==(char)OUTValue) {		
						if ((int)gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER]>=0 && (int)gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER]<=3) {
							
							gpio=gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER];
							switch(gpio) {
								case 0 :TRISDbits.RD1 = gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR];
										adr=(short)GPIO0DIR_ADDRESS;
										WriteEEPROM(adr,TRISDbits.RD1);
										if((char)TRISDbits.RD1==(char)0){gl_mutexLowIsr=1;gl_GPIOchar[0]=1;gl_mutexLowIsr=0;}
										break;
								case 1 :TRISDbits.RD2 = gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR];
										adr=(short)GPIO1DIR_ADDRESS;
										WriteEEPROM(adr,TRISDbits.RD2);
										if((char)TRISDbits.RD2==(char)0){gl_mutexLowIsr=1;gl_GPIOchar[1]=1;gl_mutexLowIsr=0;}
										break;
								case 2 :TRISDbits.RD3 = gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR];
										adr=(short)GPIO2DIR_ADDRESS;
										WriteEEPROM(adr,TRISDbits.RD3);
										if((char)TRISDbits.RD3==(char)0){gl_mutexLowIsr=1;gl_GPIOchar[2]=1;gl_mutexLowIsr=0;}
										break;
								case 3 :TRISCbits.RC4 = gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR];
										adr=(short)GPIO3DIR_ADDRESS;
										WriteEEPROM(adr,TRISCbits.RC4);
										if((char)TRISCbits.RC4==(char)0){gl_mutexLowIsr=1;gl_GPIOchar[3]=1;gl_mutexLowIsr=0;}
										break;
							}


							if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
							return(TRUE);
						}
						else {
							mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER]);
							gl_parserErrorCode=BAD_GPIO_NUMBER;
							return(FALSE);
						}
					}
					else {
						mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR]);
						gl_parserErrorCode=BAD_GPIO_DIR;
						return(FALSE);
					}											
				}
				if ((char)gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH] == (char)TRUE) {	
					if ((int)gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]>=0 && (int)gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]<=TRACK_SIZE+GPIO_SIZE+TIMER_SIZE) {
						adr=(short)CAN_NOTIFICATION_ADDRESS+gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER];
						if ((char)gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_ACTIVE] == (char)TRUE) WriteEEPROM(adr,(char)TRUE);
						else WriteEEPROM(adr,(char)FALSE);
					}
					else {
							mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_PROGRAM_REQUEST_SET_CAN_PUSH_NUMBER]);
						gl_parserErrorCode=BAD_CANPUSH_NUMBER;
						return(FALSE);
					}
				}	

				if ((char)gl_request[REQ_PROGRAM_REQUEST_SET_AUTOMATION] == (char)TRUE) {
					length=strlen(&gl_request[REQ_PROGRAM_REQUEST_IDENT]);
					for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
						notEqual=0;
						if ((int)strlen(&(gl_automation[automationCounter][NEW_AUTOMATION_IDENT]))!=(int)length) continue; // they are not equal (strncmp doesn't work here)
						for(i=0;i<length;i++)if ((char)gl_request[REQ_PROGRAM_REQUEST_IDENT+i]!=(char)gl_automation[automationCounter][NEW_AUTOMATION_IDENT+i]) {
							notEqual=1;
							break;
						}
						if (!notEqual) {
							if (length<MAXERRORINFO) mySprintf((char *)gl_errorInfo,"%s",&gl_request[REQ_PROGRAM_REQUEST_IDENT]);								
							gl_parserErrorCode=AUTOMATIONLREADYEXISTS;
							return(FALSE);
						}
					}

					if (saveAutomation(gl_nexAvailableAutomation)==(char)TRUE){
						if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);		
						return(TRUE);
						
					}
					else {
						return(FALSE);
					}
				}		
				if ((char)gl_request[REQ_PROGRAM_REQUEST_DEL_AUTOMATION] == (char) TRUE) {
					 if(gl_request[REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER]<gl_nexAvailableAutomation) {
						gl_mutexLowIsr=1;gl_nexAvailableAutomation--;gl_mutexLowIsr=0;
						if ((char)gl_request[REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER]==(char)gl_nexAvailableAutomation) {
							setAutomationToEEPROM();
							if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);		
							return(TRUE);
						}
						else if (gl_nexAvailableAutomation>0) {
							if (removeAutomation(gl_request[REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER])==(char)TRUE){
								if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);		
								return(TRUE);
							}
							else return(FALSE);
						}
						else {
							// reset in EEPROM next automation value
							adr=(short)NEXTTAUTOMATION_ADDRESS;
							gl_mutexLowIsr=1;gl_nexAvailableAutomation=0;gl_mutexLowIsr=0;
							value=0;
							WriteEEPROM(adr,value);
							if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);	
							return(TRUE);
						}
					}
					else {
						mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER]);								
						gl_parserErrorCode=BADAUTOMATIONNUMBER;
						return(FALSE);
					}
				}

				break;
				
			case COMValue : 
				if ((char)gl_request[REQ_COMMAND_REQUEST_SET_GPIO]==(char)TRUE) {
					if ((char)gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER]==(char)0 && (char)TRISDbits.RD1==(char)1) {gl_mutexLowIsr=1;gl_GPIOcounter[0]=0;gl_mutexLowIsr=0;}
					else if ((char)gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER]==(char)1 && (char)TRISDbits.RD2==(char)1){gl_mutexLowIsr=1;gl_GPIOcounter[1]=0;gl_mutexLowIsr=0;}
					else if ((char)gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER]==(char)2 && (char)TRISDbits.RD3==(char)1){gl_mutexLowIsr=1;gl_GPIOcounter[2]=0;gl_mutexLowIsr=0;}
					else if ((char)gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER]==(char)3 && (char)TRISCbits.RC4==(char)1){gl_mutexLowIsr=1;gl_GPIOcounter[3]=0;gl_mutexLowIsr=0;}
					else if ((char)gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER]>=(char)0 && (char)gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER]<=(char)3) {
							if ((char)gl_request[REQ_COMMAND_REQUEST_GPIO_LEVEL]==(char)0 ||(char)gl_request[REQ_COMMAND_REQUEST_GPIO_LEVEL]==(char)1 ) {
								gl_mutexLowIsr=1;gl_GPIOchar[gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER]]=gl_request[REQ_COMMAND_REQUEST_GPIO_LEVEL]; gl_mutexLowIsr=0;
								if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
								return(TRUE);
							}
							else {
								mySprintf((char *)gl_errorInfo,"GPIO %d => %d",(int)gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER],(int)gl_request[REQ_COMMAND_REQUEST_GPIO_LEVEL]);								
								gl_parserErrorCode=BAD_GPIO_LEVEL;
								return(FALSE);
							}
						}
						else {
							mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_COMMAND_REQUEST_GPIO_NUMBER]);								
							gl_parserErrorCode=BAD_GPIO_NUMBER;
							return(FALSE);
						}				
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_SET_TIMER]==(char)TRUE) {
					 if ((int)gl_request[REQ_COMMAND_REQUEST_TIMER_NUMBER]>= 0 && (int)gl_request[REQ_COMMAND_REQUEST_TIMER_NUMBER]<=MAXTIMER) {
							if ((int)gl_request[REQ_COMMAND_REQUEST_TIMER_DELAY]>0 && (int)gl_request[REQ_COMMAND_REQUEST_TIMER_DELAY]<=MAXTIMERDELAY ) {
								gl_mutexLowIsr=1;	gl_TIMERValue[gl_request[REQ_COMMAND_REQUEST_TIMER_NUMBER]]=gl_request[REQ_COMMAND_REQUEST_TIMER_DELAY]; gl_mutexLowIsr=0;
								if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
								return(TRUE);
							}
							else {
								mySprintf((char *)gl_errorInfo,"TIMER %d => %d",(int)gl_request[REQ_COMMAND_REQUEST_TIMER_NUMBER],(int)gl_request[REQ_COMMAND_REQUEST_TIMER_DELAY]);								
								gl_parserErrorCode=BAD_TIMER_VALUE;
								return(FALSE);
							}
						}
						else {
							mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_COMMAND_REQUEST_TIMER_NUMBER]);								
							gl_parserErrorCode=BAD_TIMER_NUMBER;
							return(FALSE);
						}				
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_SET_LPO] == (char) TRUE) {
					if ((int)gl_request[REQ_COMMAND_REQUEST_LPO_NUMBER]>=0 && (int)gl_request[REQ_COMMAND_REQUEST_LPO_NUMBER]<=5) {
						if((char)gl_request[REQ_COMMAND_REQUEST_LPO_LEVEL]==(char)0 || (char)gl_request[REQ_COMMAND_REQUEST_LPO_LEVEL]==(char)1) {
							gl_mutexLowIsr=1;gl_OUTchar[gl_request[REQ_COMMAND_REQUEST_LPO_NUMBER]]=!gl_request[REQ_COMMAND_REQUEST_LPO_LEVEL];gl_mutexLowIsr=0;
							if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
							return(TRUE);
						}
						else if((char)gl_request[REQ_COMMAND_REQUEST_LPO_LEVEL]==(char)2) { // flashing mode
							gl_mutexLowIsr=1;gl_OUTchar[gl_request[REQ_COMMAND_REQUEST_LPO_NUMBER]]=gl_request[REQ_COMMAND_REQUEST_LPO_LEVEL];gl_mutexLowIsr=0;
							if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
							return(TRUE);
						}
						else {
							mySprintf((char *)gl_errorInfo,"LPO %d => %d",(int)gl_request[REQ_COMMAND_REQUEST_LPO_NUMBER],(int)gl_request[REQ_COMMAND_REQUEST_LPO_LEVEL]);									
							gl_parserErrorCode=BAD_LPO_LEVEL;
							return(FALSE);
						}
					}
					else {
						mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_COMMAND_REQUEST_LPO_NUMBER]);								
						gl_parserErrorCode=BAD_LPO_NUMBER;
						return(FALSE);
					}
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_SET_AUT] == (char) TRUE) {
						if((char)gl_request[REQ_COMMAND_REQUEST_AUT_STATUS]==(char)AUTONValue || (char)gl_request[REQ_COMMAND_REQUEST_AUT_STATUS]==(char)AUTOFFValue) {

							length=strlen(&gl_request[REQ_COMMAND_REQUEST_AUT_IDENT]);
							for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
								notEqual=0;
								if ((int)strlen(&(gl_automation[automationCounter][NEW_AUTOMATION_IDENT]))!=(int)length) continue; // They are not equal (strncmp doesn't work here)
								for(i=0;i<length;i++)if ((char)gl_request[REQ_COMMAND_REQUEST_AUT_IDENT+i]!=(char)gl_automation[automationCounter][NEW_AUTOMATION_IDENT+i]) {
									notEqual=1;
									break;
								}
								if (!notEqual) {								
									// update automation status
									gl_mutexLowIsr=1;gl_automation[automationCounter][NEW_AUTOMATION_STATUS]=gl_request[REQ_COMMAND_REQUEST_AUT_STATUS];gl_mutexLowIsr=0;
									if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
									return(TRUE);
								}
							}
							if (length<MAXERRORINFO) mySprintf((char *)gl_errorInfo,"%s",&gl_request[REQ_COMMAND_REQUEST_AUT_IDENT]);								
							gl_parserErrorCode=BAD_AUT_IDENT;
							return(FALSE);
						}
						else {
							mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_COMMAND_REQUEST_AUT_STATUS]);								
							gl_parserErrorCode=BAD_AUT_STATUS;
							return(FALSE);
						}
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_SET_TRACK] == (char) TRUE) {
						if (gl_boardMode==DCCValue) {
							mySprintf((char *)gl_errorInfo,"Current mode DCC");								
							gl_parserErrorCode=BAD_BOARD_MODE;
							return(FALSE);
						}
													
						if ((int)gl_request[REQ_COMMAND_REQUEST_TRACK_NUMBER]>=0 && (int)gl_request[REQ_COMMAND_REQUEST_TRACK_NUMBER]<=31) {
							if ((((int)gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]>=0 && (int)gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]<=MAXSPEEDVALUE)|| (char)gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]==(char)KNOB0Value || (char)gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]==(char)KNOB1Value) &&
							    ((char)gl_request[REQ_COMMAND_REQUEST_TRACK_DIR]==(char)FORWValue || (char)gl_request[REQ_COMMAND_REQUEST_TRACK_DIR]==(char)BACKValue) &&
								(((int)gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]>=0 && (int)gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]<=100)|| (char)gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]==(char)KNOB0Value || (char)gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]==(char)KNOB1Value)) {

								if ((char)gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]==(char)KNOB0Value) speed=gl_knobValue0;
								else if ((char)gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]==(char)KNOB1Value) speed=gl_knobValue1;
								else speed=gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED];

								if ((char)gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]==(char)KNOB0Value) step=gl_knobValue0;
								else if ((char)gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]==(char)KNOB1Value) step=gl_knobValue1;
								else step=gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA];
							
								if (step<0)step=-step;
								if ((char)gl_request[REQ_COMMAND_REQUEST_TRACK_DIR]==(char)FORWValue) TM1637_display(speed,step);
								else TM1637_display(-speed,step);
	
								// Manage all groups of tracks
								// 17 = 0
								// 18 = 1
								// 19 = 1 0
								// 20 = 2
								// 21 = 2 0
								// 22 = 2 1
								// 23 = 2 1 0
								// 24 = 3
								// 25 = 3 0
								// 26 = 3 1
								// 27 = 3 1 0
								// 28 = 3 2 
								// 29 = 3 2 0
								// 30 = 3 2 1
								// 31 = 3 2 1 0

								if ((int)gl_request[REQ_COMMAND_REQUEST_TRACK_NUMBER]>=17) {
									for(trackNumber=0;trackNumber<4;trackNumber++) {
										if (((char)(gl_request[REQ_COMMAND_REQUEST_TRACK_NUMBER] & 0x0F & (1<<trackNumber)))== (char)(1<<trackNumber)) {
											setSpeed(speed,step,trackNumber);
											if ((char)gl_request[REQ_COMMAND_REQUEST_TRACK_DIR]==(char)BACKValue) {
												gl_mutexLowIsr=1;gl_setPoint[trackNumber]=-gl_setPoint[trackNumber];gl_mutexLowIsr=0;
											}
										}
										else {
											setSpeed(0,0,trackNumber);
											if ((char)gl_request[REQ_COMMAND_REQUEST_TRACK_DIR]==(char)BACKValue){
												gl_mutexLowIsr=1;gl_setPoint[trackNumber]=-gl_setPoint[trackNumber];gl_mutexLowIsr=0;
											}
										}
									}
								}
								else if ((int)gl_request[REQ_COMMAND_REQUEST_TRACK_NUMBER]<=3) {
										trackNumber=gl_request[REQ_COMMAND_REQUEST_TRACK_NUMBER];
										setSpeed(speed,step,trackNumber);
										if ((char)gl_request[REQ_COMMAND_REQUEST_TRACK_DIR]==(char)BACKValue) {
											gl_mutexLowIsr=1;gl_setPoint[trackNumber]=-gl_setPoint[trackNumber];gl_mutexLowIsr=0;
										}
								}
								else {	
									mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_COMMAND_REQUEST_TRACK_NUMBER]);															
									gl_parserErrorCode=BAD_TRACK_NUMBER;
									return(FALSE);
								}
								
								if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
								return(TRUE);
							}
							else {
								if (!((char)gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]>=(char)0 && (int)gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]<=MAXSPEEDVALUE)) {
									mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_COMMAND_REQUEST_TRACK_SPEED]);															
									gl_parserErrorCode=BAD_TRACK_SPEED;
									return(FALSE);
								}
								if (!((char)gl_request[REQ_COMMAND_REQUEST_TRACK_DIR]==(char)FORWValue || (char)gl_request[REQ_COMMAND_REQUEST_TRACK_DIR]==(char)BACKValue)) {
									mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_COMMAND_REQUEST_TRACK_DIR]);															
									gl_parserErrorCode=BAD_TRACK_DIR;
									return(FALSE);
								}
								if (!((int)gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]>=0 && (int)gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]<=100)) {
									mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_COMMAND_REQUEST_TRACK_INERTIA]);															
									gl_parserErrorCode=BAD_TRACK_INERTIA;
									return(FALSE);
								}
							}	
						}
						else {
							mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_COMMAND_REQUEST_TRACK_NUMBER]);															
							gl_parserErrorCode=BAD_TRACK_NUMBER;
							return(FALSE);
						}
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_SET_USER_MODE]== (char) TRUE) {
					if ((char)gl_request[REQ_COMMAND_REQUEST_USER_MODE]==(char)MANUALValue ||
						(char)gl_request[REQ_COMMAND_REQUEST_USER_MODE]==(char)MANUAL0Value ||
						(char)gl_request[REQ_COMMAND_REQUEST_USER_MODE]==(char)MANUAL1Value ||
						(char)gl_request[REQ_COMMAND_REQUEST_USER_MODE]==(char)MANUAL2Value ||
						(char)gl_request[REQ_COMMAND_REQUEST_USER_MODE]==(char)MANUAL3Value ||
						(char)gl_request[REQ_COMMAND_REQUEST_USER_MODE]==(char)AUTOMATICValue ) {

						// Reset all timer if AUTOMATIC mode is set
						for(TIMERCounter=0;TIMERCounter<MAXTIMER;TIMERCounter++) {
							gl_mutexLowIsr=1;gl_TIMERValue[TIMERCounter]=0;gl_mutexLowIsr=0;
							gl_mutexLowIsr=1;gl_TIMERNotification[TIMERCounter]=FALSE;gl_mutexLowIsr=0;
						}

						gl_mutexLowIsr=1;gl_userMode=gl_request[REQ_COMMAND_REQUEST_USER_MODE];gl_mutexLowIsr=0;

						// Set all automation status
						for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
							if ((char)gl_request[REQ_COMMAND_REQUEST_USER_MODE]!=(char)AUTOMATICValue){gl_mutexLowIsr=1;gl_automation[automationCounter][NEW_AUTOMATION_STATUS]=gl_automation[automationCounter][NEW_AUTOMATION_STATUS_MANUAL];gl_mutexLowIsr=0;}
							else {
								gl_mutexLowIsr=1;gl_automation[automationCounter][NEW_AUTOMATION_STATUS]=AUTONValue;gl_mutexLowIsr=0;
							}
						}
						if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
						return(TRUE);
					}
					else {
						mySprintf((char *)gl_errorInfo,"%d",(int)gl_request[REQ_COMMAND_REQUEST_USER_MODE]);															
						gl_parserErrorCode=BAD_USER_MODE;
						return(FALSE);
					}
				}				
				else if ((char)gl_request[REQ_COMMAND_REQUEST_SET_DCC]== (char) TRUE) {
						if (gl_boardMode==ANAValue) {
							mySprintf((char *)gl_errorInfo,"Current mode ANA");								
							gl_parserErrorCode=BAD_BOARD_MODE;
							return(FALSE);
						}
						gl_mutexLowIsr=1;setDcc(gl_request[REQ_COMMAND_REQUEST_DCC_ADDRESS],gl_request[REQ_COMMAND_REQUEST_DCC_COMMAND]);gl_mutexLowIsr=0;
						if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
						return(TRUE);
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_GET_AUTOMATION_LIST] == (char) TRUE) {

				 	 gl_mutexLowIsr=1;gl_stopAll=TRUE;	gl_mutexLowIsr=0;

					mySprintf((char *)gl_message,"");
					if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
					for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
						if(gl_automation[automationCounter][NEW_AUTOMATION_STATUS]==AUTONValue) mySprintf((char *)gl_message,"%d %s ON",(int)automationCounter,&(gl_automation[automationCounter][NEW_AUTOMATION_IDENT]));
						else if(gl_automation[automationCounter][NEW_AUTOMATION_STATUS]==AUTOFFValue) mySprintf((char *)gl_message,"%d %s OFF",(int)automationCounter,&(gl_automation[automationCounter][NEW_AUTOMATION_IDENT]));
						else mySprintf((char *)gl_message,"%d %s UNKNOWN STATE",(int)automationCounter,&(gl_automation[automationCounter][NEW_AUTOMATION_IDENT]));					
						if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
					}
					mySprintf((char *)gl_message,"");
					if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);

				 	gl_mutexLowIsr=1;gl_stopAll=FALSE;	gl_mutexLowIsr=0;

					return(TRUE);
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_GET_DUMP] == (char) TRUE) {

				 	gl_mutexLowIsr=1;gl_stopAll=TRUE;	gl_mutexLowIsr=0;

					for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {
						mySprintf((char *)gl_message,"");
						itemPerLine=0;
						for(automationDataCounter=0;automationDataCounter<NEW_AUTOMATIONSIZE;automationDataCounter++) {
							mySprintf((char *)gl_message,"%s(%d,%d,%d)",(int)gl_message,automationCounter,(int)automationDataCounter,(int)gl_automation[automationCounter][automationDataCounter]);		
							if (gl_automation[automationCounter][automationDataCounter]>=32 && gl_automation[automationCounter][automationDataCounter]<=127)mySprintf((char *)gl_message,"%s[%c]",gl_message,gl_automation[automationCounter][automationDataCounter]); 
							itemPerLine++;
							if (itemPerLine>=5) {
								if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
								itemPerLine=0;
								mySprintf((char *)gl_message,"");
							}
						}
						if (itemPerLine>0) {
							if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
							itemPerLine=0;
							mySprintf((char *)gl_message,"");
						}
					}
			 	 	gl_mutexLowIsr=1;gl_stopAll=FALSE;	gl_mutexLowIsr=0; 

					return(TRUE);
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_GET_BOARD_STATUS] == (char) TRUE) {
					if ((char)sendPrompt==(char)TRUE) {
						boardStatus();	
						return(TRUE);
					}
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_GET_GPIO_STATUS] == (char) TRUE) {
					if ((char)sendPrompt==(char)TRUE) {
						if((char)TRISDbits.RD1==(char)0)mySprintf((char *)gl_message,"GPIO 0 OUT VAL %d",(int)gl_GPIOchar[0]);else mySprintf((char *)gl_message,"GPIO 0 IN VAL %d COUNT %d",(int)gl_GPIOchar[0],(int)gl_GPIOcounter[0]);
						adr=(short)GPIO_CAN_NOTIFICATION_ADDRESS;
						ReadEEPROM(adr,&value);
						if (value==(char)TRUE)mySprintf((char *)gl_message,"%s => CAN",gl_message);
						prompt((char *)gl_message);

						if((char)TRISDbits.RD2==(char)0)mySprintf((char *)gl_message,"GPIO 1 OUT VAL %d",(int)gl_GPIOchar[1]);else mySprintf((char *)gl_message,"GPIO 1 IN VAL %d COUNT %d",(int)gl_GPIOchar[1],(int)gl_GPIOcounter[1]);
						adr=(short)GPIO_CAN_NOTIFICATION_ADDRESS+1;
						ReadEEPROM(adr,&value);
						if (value==(char)TRUE)mySprintf((char *)gl_message,"%s => CAN",gl_message);
						prompt((char *)gl_message);

						if((char)TRISDbits.RD3==(char)0)mySprintf((char *)gl_message,"GPIO 2 OUT VAL %d",(int)gl_GPIOchar[2]);else mySprintf((char *)gl_message,"GPIO 2 IN VAL %d COUNT %d",(int)gl_GPIOchar[2],(int)gl_GPIOcounter[2]);
						adr=(short)GPIO_CAN_NOTIFICATION_ADDRESS+2;
						ReadEEPROM(adr,&value);
						if (value==(char)TRUE)mySprintf((char *)gl_message,"%s => CAN",gl_message);						prompt((char *)gl_message);

						if((char)TRISCbits.RC4==(char)0)mySprintf((char *)gl_message,"GPIO 3 OUT VAL %d",(int)gl_GPIOchar[3]);else mySprintf((char *)gl_message,"GPIO 3 IN VAL %d COUNT %d",(int)gl_GPIOchar[3],(int)gl_GPIOcounter[3]);
						adr=(short)GPIO_CAN_NOTIFICATION_ADDRESS+3;
						ReadEEPROM(adr,&value);
						if (value==(char)TRUE)mySprintf((char *)gl_message,"%s => CAN",gl_message);
						prompt((char *)gl_message);

						mySprintf((char *)gl_message,"KNOB 0 VAL %d",(int)gl_knobValue0);
						prompt((char *)gl_message);
						mySprintf((char *)gl_message,"KNOB 1 VAL %d",(int)gl_knobValue1);
						prompt((char *)gl_message);
	
						mySprintf((char *)gl_message,"");
						prompt((char *)gl_message);
					}				
					return(TRUE);
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_GET_LPO_STATUS] == (char) TRUE) {
					for(statusCounter=0;statusCounter<6;statusCounter++) {
						if ((char)gl_OUTchar[statusCounter]==(char)0) mySprintf((char *)gl_message,"LPO %d 1",(int)statusCounter);
						if ((char)gl_OUTchar[statusCounter]==(char)1) mySprintf((char *)gl_message,"LPO %d 0",(int)statusCounter);
						if ((char)gl_OUTchar[statusCounter]>=(char)2) mySprintf((char *)gl_message,"LPO %d flashing",(int)statusCounter);
						if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
					}
					mySprintf((char *)gl_message,"");
					if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);	
					return(TRUE);
				}
				else if ((char)gl_request[REQ_COMMAND_REQUEST_GET_TRACK_STATUS] == (char) TRUE) {
					for(statusCounter=0;statusCounter<4;statusCounter++) {
						if (gl_setPoint[statusCounter]>0) mySprintf((char *)gl_message,"TRACK %d FORW SPEED %d %S (%d / %d)",
							(int)statusCounter,
							(int)gl_setPoint[statusCounter]/(MAXINTERNALSPEED),
							gl_OUTSTATchar[statusCounter]==0 ? OFFTRACK_STRING : ONTRACK_STRING,
							(int)gl_average[statusCounter],
							(int)gl_noVehicule[statusCounter]);
						else mySprintf((char *)gl_message,"TRACK %d BACK SPEED %d %S (%d / %d)",
							 (int)statusCounter,
							 (int)-gl_setPoint[statusCounter]/(MAXINTERNALSPEED),
							 gl_OUTSTATchar[statusCounter]==0 ? OFFTRACK_STRING : ONTRACK_STRING,
							(int)gl_average[statusCounter],
							(int)gl_noVehicule[statusCounter]);
						adr=(short)TRACK_CAN_NOTIFICATION_ADDRESS+statusCounter;
						ReadEEPROM(adr,&value);
						if (value==(char)TRUE)mySprintf((char *)gl_message,"%s => CAN",gl_message);
						prompt((char *)gl_message);
					}
					mySprintf((char *)gl_message,"");
					prompt((char *)gl_message);	
					return(TRUE);
				}		
				break;

			// NO ACTION TO PERFORM
			default : 
				if ((char)sendPrompt==(char)TRUE) prompt((char *)gl_message);
				return(TRUE);
		}
	}
}
/* ==============================================================================
 * Function: initUSART
 * Returns: void = no return.
 * Description: Initializes the UART (pins, baud rate, interrupts) and input 
 * buffer on master board.
 * ============================================================================== */
void initUSART(void) {

 	char 	readUSARTPointer;
	if ((char)gl_master==(char)FALSE)return;

	// init RS232 input buffer
	for(readUSARTPointer=0;readUSARTPointer<USARTBUFFERSIZE;readUSARTPointer++) {
		gl_receivedUSARTData[readUSARTPointer]=0;
	}
	gl_receivedUSARTPointer=0;
	gl_getDataUSARTPointer=0;

    //RX and TX pin configuration
	TRISC=0b10110000; //  RC4, RC5, RC7 (RX) in (RC6 TX RS232 out, could be updated as standard GPIO in initEnvironment())

     // USART module configuration
    TXSTAbits.TXEN = 1; // Activate USART transmitter
    TXSTAbits.SYNC = 0; // Asynchronous mode
    TXSTAbits.BRGH = 1; // High Baud Rate Select bit
    RCSTAbits.SPEN = 1; // Enable serial module (TX and RX)
    RCSTAbits.CREN = 1; // Enable USART receiver
    BAUDCONbits.BRG16 = 1;  // Mode 16 bits

    SPBRGH = 0;
    SPBRG  = 68; // 115200 bauds
   
    // Speed register configuration

    PIE1bits.RCIE = 1;      // Enable USART interrupt reception
}
/* ==============================================================================
 * Function: sendUSART
 * Returns: void = no return.
 * Description: Blocking transmit of one byte over UART on master board.
 * ============================================================================== */
void sendUSART(char data){

	if ((char)gl_master==(char)FALSE)return;

    // wait until transmission buffer is empty
    while (!TXSTAbits.TRMT);
	TXREG = data;
	while(!PIR1bits.TXIF);
}
/* ==============================================================================
 * Function: getInputRequestFromUSART
 * Returns: char = TRUE when a full line is ready, FALSE otherwise.
 * Description: Consumes bytes from the UART ring buffer to assemble a line‑based 
 * input string (CR‑terminated).
 * ============================================================================== */
char getInputRequestFromUSART(char *inputString,char *inputCounter) {

   char  getData;

	if ((char)gl_master==(char)FALSE)return(FALSE);

	// Get Data from RS232
	if (gl_getDataUSARTPointer!=gl_receivedUSARTPointer) {
		getData=gl_receivedUSARTData[gl_getDataUSARTPointer];
		gl_receivedUSARTData[gl_getDataUSARTPointer++] = 0;

		if (gl_getDataUSARTPointer>=USARTBUFFERSIZE)gl_getDataUSARTPointer=0;

		//Echo
        if (getData!=0xD && *inputCounter<USARTBUFFERSIZE-1) {
		
			if (getData==0x7F) {
				(*inputCounter)--;
				if (*inputCounter<0) *inputCounter=0;
				else sendUSART(getData); // Echo
			}
			else {
                inputString[(*inputCounter)++]=(char)toUpperCase(getData);
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
/* ==============================================================================
 * Function: prompt
 * Returns: void = no return.
 * Description: Prints a shell‑like prompt 'BRD>' + message via _user_putc and 
 * flushes.
 * ============================================================================== */
void prompt(char* gl_message) {

	int i;
	static char number[3];
	int	length;

	// Prompt
	for(i = 0; BOARD_PROMPT_STRING[i]!=(const char)'\0'; i++) {
       	_user_putc(BOARD_PROMPT_STRING[i]);
    }
		
	// Board number
	mySprintf((char *)number,"%d",(int)gl_boardNumber);
	length=strlen(number);
	for(i = 0; i < length; i++) {
       	_user_putc(number[i]);
    }

	// close parenthesis
	_user_putc('>');

	// space
	_user_putc(' ');

	// message
	length=strlen(gl_message);
	for(i = 0; i < length; i++) {
       	_user_putc(gl_message[i]);
    }
	flushOut();	
}
/* ==============================================================================
 * Function: flushOut
 * Returns: void = no return.
 * Description: Emits end‑of‑print marker on output channel.
 * ============================================================================== */
void flushOut(void) {
	_user_putc(ENDOFPRINTFTRAME);
}
/* ==============================================================================
 * Function: CANsendDelay
 * Returns: void = no return.
 * Description: Small busywait delay between CAN frames.
 * ============================================================================== */
void CANsendDelay(void) {
	int	delay;
	for(delay=0;delay<WAITDELAYTRAMECAN;delay++);
}
/* ==============================================================================
 * Function: sendPrintToCAN
 * Returns: void = no return.
 * Description: Packages the output buffer into CAN 'PRINT' frames with header/footer
 * and sends.
 * ============================================================================== */
void sendPrintToCAN(void){

	long 	id;			// Id of sender
    BYTE 	dataOut[8];	// DATA to CAN	
	char 	dataCounter;
	char	dataOutCounter;	
	char	trameComplete;

	BYTE dataLen; 				// Number of bytes transmitted in the gl_message
	ECAN_TX_MSG_FLAGS flags; 	// Flags

	dataLen=8;
	flags=ECAN_TX_STD_FRAME;
	id=gl_boardNumber;

	// header trame
	for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++){
		dataOut[dataOutCounter]=TRAMEPRINTHEADER;
	}

	while(!ECANSendMessage(id,dataOut,dataLen,flags));
	CANsendDelay();

	// trame
	trameComplete=FALSE;
	dataCounter=0;
	while(dataCounter<MAXMESSAGESIZE) {
		for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++) {
			if (dataCounter<gl_outputCANbufferCounter) {
				dataOut[dataOutCounter]=gl_outputCANbuffer[dataCounter++];
				if ((char)dataOut[dataOutCounter]==(char)ENDOFPRINTFTRAME) trameComplete=TRUE;
			}
			else {
				dataOut[dataOutCounter]=ENDOFPRINTFTRAME;
				if (dataCounter<MAXMESSAGESIZE)dataCounter++;
				trameComplete=TRUE;
			}
		}
		while(!ECANSendMessage(id,dataOut,dataLen,flags));
		CANsendDelay();

		if ((char)trameComplete==(char)TRUE) break;
	}
	gl_outputCANbufferCounter=0;

	// footer trame
	for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++){
		dataOut[dataOutCounter]=TRAMEPRINTFOOTER;
	}
	while(!ECANSendMessage(id,dataOut,dataLen,flags));
	CANsendDelay();

}
/* ==============================================================================
 * Function: _user_putc
 * Returns: int = the character written.
 * Description: Writes a character to UART on master or queues to CAN on slave; 
 * triggers CAN send on buffer/full or terminator.
 * ============================================================================== */
int _user_putc (char c) {

	// On master board send to UART via standart putc()
	if ((char)gl_master==(char)TRUE){
		if (gl_inputCounter==0)sendUSART(c);
	}

	// Send on CAN bus
	else {
		if (gl_outputCANbufferCounter<MAXMESSAGESIZE)gl_outputCANbuffer[gl_outputCANbufferCounter++]=c;
		if ((gl_outputCANbufferCounter==MAXMESSAGESIZE || c==ENDOFPRINTFTRAME)) {
			sendPrintToCAN();
		}
	}
	return(c);
}
/* ==============================================================================
 * Function: sendRequestToCAN
 * Returns: void = no return.
 * Description: Compresses gl_request and transmits it as a CAN 'REQUEST' frame 
 * sequence; restores request via RLE decode.
 * ============================================================================== */
void sendRequestToCAN(void) {
	
	long 	id;			// Id of sender
    BYTE 	dataOut[8];	// DATA to CAN	
	char 	dataCounter;
	char	dataOutCounter;
	BYTE dataLen; 				// Number of bytes transmitted in the gl_message
	ECAN_TX_MSG_FLAGS flags; 	// Flags
	char trameSize;

	// Convert request to dataOut using gl_dataStructure
	trameSize=compressData();

	dataLen=8;
	flags=ECAN_TX_STD_FRAME;
	dataCounter=0;
	id=gl_boardNumber;

	// header trame
	for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++) {
		dataOut[dataOutCounter]=TRAMEREQUESTHEADER;
	}

	while(!ECANSendMessage(id,dataOut,dataLen,flags));
	CANsendDelay();

	// trame
	while(dataCounter<trameSize) {
		for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++) {
			if (dataCounter<trameSize) {
				dataOut[dataOutCounter]=gl_request[dataCounter++];
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
	uncompressData(); // get back to initial data for other action in manageRequest()
}
/* ==============================================================================
 * Function: CAN_SendSync
 * Returns: void = no return.
 * Description: Broadcasts a CAN SYNC frame and re‑phases timers and counters 
 * across boards.
 * ============================================================================== */
void CAN_SendSync(void){

	BYTE dataLen; 				// Number of bytes transmitted in the gl_message
	ECAN_TX_MSG_FLAGS flags; 	// Flags
    BYTE 	dataOut[8];	// DATA to CAN	
	char	dataOutCounter;

	dataLen=8;
	flags=ECAN_TX_STD_FRAME;

	// sync trame
	for(dataOutCounter=0;dataOutCounter<8;dataOutCounter++) {
		dataOut[dataOutCounter]=TRAMESYNCTRACE;
	}

    while (!ECANSendMessage(SYNC_ID, dataOut, dataLen, flags));

    gl_mutexLowIsr = 1;
    INTCONbits.TMR0IE       = 0;   // Disable Timer0 Interrupt
    gl_speedCounter			= 1;   // Same value on each board
	gl_syncRequested 		= 1;   // For printing SYNC message on LED
	gl_flashingCounter		= 0;
    INTCONbits.TMR0IF       = 0;   // T0 int flag bit cleared before starting
	TMR0H					= 0;   // re-phase Timer0
	TMR0L 					= 6;   // re-phase Timer0
    INTCONbits.TMR0IE       = 1;   // Enable Timer0 Interrupt
    T0CONbits.TMR0ON        = 1;   // timer0 START
    gl_mutexLowIsr = 0;
	CANsendDelay();
}
/* ==============================================================================
 * Function: getInputRequestFromCAN
 * Returns: char = TRUE when a request was assembled, FALSE otherwise.
 * Description: Parses incoming CAN buffers, extracting REQUEST or PRINT trames; 
 * fills gl_request or prints content.
 * ============================================================================== */
char getInputRequestFromCAN(void) {
	
	char	requestHeaderTrameDetected;
	char	printHeaderTrameDetected;	
	char	requestFooterTrameDetected;
	char	printFooterTrameDetected;
	char	requestTrameEnd;
	char	printTrameEnd;

	char	dataInCounter;
	char	dataStructureCounter;

	char 	bufferNumber;
	char	data;

	mySprintf((char *)gl_message,"");
	for(bufferNumber=0;bufferNumber<MAXINPUTCANBUFFER;bufferNumber++) {
		requestHeaderTrameDetected=0;
		printHeaderTrameDetected=0;	
		requestFooterTrameDetected=0;
		printFooterTrameDetected=0;

		while (gl_inputCANReadBufferPointer[bufferNumber]!=gl_inputCANWriteBufferPointer[bufferNumber] && gl_canReceivedDataReady[bufferNumber]==READY) {
			data=gl_inputCANbuffer[bufferNumber][gl_inputCANReadBufferPointer[bufferNumber]];
		
			if ((char)data==(char)TRAMEREQUESTHEADER) requestHeaderTrameDetected++;	else requestHeaderTrameDetected=0;
			if ((char)data==(char)TRAMEPRINTHEADER) printHeaderTrameDetected++;		else printHeaderTrameDetected=0;				

			if ((char)data==(char)TRAMEREQUESTFOOTER) requestFooterTrameDetected++;	else requestFooterTrameDetected=0;
			if ((char)data==(char)TRAMEPRINTFOOTER) printFooterTrameDetected++;		else printFooterTrameDetected=0;


			// REQUEST HEADER
			if (requestHeaderTrameDetected==8) {
				gl_inputCANmode[bufferNumber]=CAN_REQUEST;
				requestHeaderTrameDetected=0;
				gl_requestInputCANtrameStart[bufferNumber]=gl_inputCANReadBufferPointer[bufferNumber]+1;
				if ((int)gl_requestInputCANtrameStart[bufferNumber]>=MAXTRAMESIZE)gl_requestInputCANtrameStart[bufferNumber]=0;
			}
			// PRINT HEADER
			else if (printHeaderTrameDetected==8) {
				gl_inputCANmode[bufferNumber]=CAN_PRINT;
				printHeaderTrameDetected=0;
				gl_printInputCANtrameStart[bufferNumber]=gl_inputCANReadBufferPointer[bufferNumber]+1;
				if ((char)gl_printInputCANtrameStart[bufferNumber]>=(char)MAXTRAMESIZE)gl_printInputCANtrameStart[bufferNumber]=0;
			}
			// REQUEST READY
			else if (requestFooterTrameDetected==8) {
				if (gl_inputCANmode[bufferNumber]==CAN_REQUEST) {
					gl_inputCANmode[bufferNumber]=CAN_GET_FOOTER; 
					requestTrameEnd=gl_inputCANReadBufferPointer[bufferNumber]-7;
					if (requestTrameEnd<0)requestTrameEnd+=MAXTRAMESIZE;
					dataInCounter=gl_requestInputCANtrameStart[bufferNumber];
					dataStructureCounter=0;
					while(dataInCounter!=requestTrameEnd) {
						gl_request[dataStructureCounter++]=gl_inputCANbuffer[bufferNumber][dataInCounter++];
						if (dataInCounter>=MAXTRAMESIZE)dataInCounter=0;
					}
					gl_inputCANReadBufferPointer[bufferNumber]++;		
					if (gl_inputCANReadBufferPointer[bufferNumber]>=MAXTRAMESIZE)gl_inputCANReadBufferPointer[bufferNumber]=0;
					gl_inputCANmode[bufferNumber]=CAN_FREE;
					gl_canReceivedDataReady[bufferNumber]=WAITING_FOR_DATA;
					return(uncompressData()); // Mean request available to proceed
				}
				// Incomplete trame received
				else {
					gl_inputCANReadBufferPointer[bufferNumber]++;
					if (gl_inputCANReadBufferPointer[bufferNumber]>=MAXTRAMESIZE)gl_inputCANReadBufferPointer[bufferNumber]=0;
					gl_inputCANmode[bufferNumber]=CAN_FREE;
					gl_canReceivedDataReady[bufferNumber]=WAITING_FOR_DATA;
					return(FALSE);
				}
			}
			// PRINT READY
			else if (printFooterTrameDetected==8) {
				if(gl_inputCANmode[bufferNumber]==CAN_PRINT) {
					if ((char)gl_master==(char)TRUE) {
						gl_inputCANmode[bufferNumber]=CAN_GET_FOOTER; 
						printTrameEnd=gl_inputCANReadBufferPointer[bufferNumber]-7;
						if (printTrameEnd<0)printTrameEnd+=MAXTRAMESIZE;
						dataInCounter=gl_printInputCANtrameStart[bufferNumber];	
						while(dataInCounter!=printTrameEnd) {
							if ((char)gl_inputCANbuffer[bufferNumber][dataInCounter]!=(char)0) _user_putc(gl_inputCANbuffer[bufferNumber][dataInCounter]);
							dataInCounter++;
							if (dataInCounter>=MAXTRAMESIZE)dataInCounter=0;
						}
						gl_inputCANReadBufferPointer[bufferNumber]++;		
						if (gl_inputCANReadBufferPointer[bufferNumber]>=MAXTRAMESIZE)gl_inputCANReadBufferPointer[bufferNumber]=0;
						gl_inputCANmode[bufferNumber]=CAN_FREE;			
						gl_canReceivedDataReady[bufferNumber]=WAITING_FOR_DATA;
						return(FALSE); // Mean no more data to print
					}
					else {
						gl_inputCANReadBufferPointer[bufferNumber]++;
						if (gl_inputCANReadBufferPointer[bufferNumber]>=MAXTRAMESIZE)gl_inputCANReadBufferPointer[bufferNumber]=0;
						gl_inputCANmode[bufferNumber]=CAN_FREE;
						gl_canReceivedDataReady[bufferNumber]=WAITING_FOR_DATA;
						return(FALSE);
					}
				}
				// Incomplete trame received
				else {
					gl_inputCANReadBufferPointer[bufferNumber]++;
					if (gl_inputCANReadBufferPointer[bufferNumber]>=MAXTRAMESIZE)gl_inputCANReadBufferPointer[bufferNumber]=0;
					gl_inputCANmode[bufferNumber]=CAN_FREE;
					gl_canReceivedDataReady[bufferNumber]=WAITING_FOR_DATA;
					return(FALSE);
				}
			}
			else {	
				// read new data
				gl_inputCANReadBufferPointer[bufferNumber]++;		
				if (gl_inputCANReadBufferPointer[bufferNumber]>=MAXTRAMESIZE)gl_inputCANReadBufferPointer[bufferNumber]=0;
			}				
		}
	}
	return(FALSE); // Nothing to do
}
/* ==============================================================================
 * Function: set7segmentPort
 * Returns: void = no return.
 * Description: Sets the TM1637 lines (CLK,DIO) on a shared port mapping.
 * ============================================================================== */
void set7segmentPort(char CLK, char DIO) {

	char myPortE; // used to better synchronised output updates
	char delay;

    myPortE= (CLK<<1) + (DIO<<2);
    LATE=myPortE;
}
/* ==============================================================================
 * Function: twoWire_init
 * Returns: void = no return.
 * Description: Initializes pseudo two‑wire interface for TM1637.
 * ============================================================================== */
void twoWire_init(void) {
	set7segmentPort(0,0);
}
/* ==============================================================================
 * Function: twoWire_start
 * Returns: void = no return.
 * Description: Generates TM1637 start sequence.
 * ============================================================================== */
void twoWire_start(void){
    
	set7segmentPort(1,1);
	set7segmentPort(1,0);
}
/* ==============================================================================
 * Function: twoWire_stop
 * Returns: void = no return.
 * Description: Generates TM1637 stop sequence.
 * ============================================================================== */
void twoWire_stop(void){
	set7segmentPort(0,0);
	set7segmentPort(1,0);
	set7segmentPort(1,1);
}
/* ==============================================================================
 * Function: twoWire_ack
 * Returns: void = no return.
 * Description: Generates a TM1637 dummy ACK pulse.
 * ============================================================================== */
void twoWire_ack(void){

	set7segmentPort(0,0);
	set7segmentPort(1,0);
}
/* ==============================================================================
 * Function: twoWire_write
 * Returns: char = unspecified / not used.
 * Description: Clocks out 8 bits LSB‑first on TM1637 (bit‑bang).
 * ============================================================================== */
char twoWire_write(char data){

	char tx;
	char DIO;
	for(tx = 0 ; tx < 8 ; tx++) {
		DIO = ((data >> tx) & 0x01) ? 1 : 0 ; //LSB first (Real 12c sends MSB first)
		set7segmentPort(0,DIO);
		set7segmentPort(1,DIO);
		set7segmentPort(0,DIO);
	}
}
/* ==============================================================================
 * Function: TM1637_init
 * Returns: void = no return.
 * Description: Initializes the TM1637 display driver.
 * ============================================================================== */
void TM1637_init(void){
    twoWire_init();    
}
/* ==============================================================================
 * Function: TM1637_write
 * Returns: void = no return.
 * Description: Converts two 3‑digit numbers to 6 digits and writes segment bytes.
 * ============================================================================== */
void TM1637_write(short number1,short number2){

    char str1Num[4];
    char str2Num[4];
    char strNum[8];
    char size;

	mySprintf(str1Num,"%3d",number1); 
	mySprintf(str2Num,"%3d",number2); // 3 characters
	mySprintf(strNum,"%s%s",str2Num,str1Num);

    for(size=5;size>=0;size--) {
		if ((char)strNum[size]==(char)' ')twoWire_write(digits[11]);
		else if ((char)strNum[size]==(char)'-')twoWire_write(digits[10]);
        else {
			char i = strNum[size] - '0';  //Get index 0 - 9 
        	twoWire_write(digits[i]);
		}
        twoWire_ack();
    }
}
/* ==============================================================================
 * Function: TM1637_display
 * Returns: void = no return.
 * Description: Configures TM1637 and displays two numbers with given brightness.
 * ============================================================================== */
void TM1637_display(short number1,short number2){  

	TM1637_setBrightness(3);

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
/* ==============================================================================
 * Function: charToSegments
 * Returns: char = segment bitmap.
 * Description: Maps ASCII char to 7‑segment encoding for common‑anode TM1637.
 * ============================================================================== */
char charToSegments(char c) {

    if((char)c >= (char)'0' && (char)c <= (char)'9')        return digits[c - '0'];
    else if((char)c == (char)'-')               return digits[10];
    else if((char)c == (char)' ')               return digits[11];
    else if((char)c >= (char)'A' && (char)c <= (char)'Z')   return digits[12 + (c - 'A')];
    else if((char)c >= (char)'a' && (char)c <= (char)'z')   return digits[12 + (c - 'a')];
    else                            return 0x00;
}
/* ==============================================================================
 * Function: TM1637_writeStringWindow
 * Returns: void = no return.
 * Description: Writes exactly 6 characters to TM1637 using physical position mapping.
 * ============================================================================== */
void TM1637_writeStringWindow(const char *s, char start, char len) {

	// physical order of the positions on screen (0 = left, 5 = right)
	// adjust this array if the order is different

    const char mapping[6] = {2,1,0,5,4,3};
	char idx;
	char ch;

    char i;
    for(i = 0; i < 6; i++) {
        idx = start + mapping[i];
        if (idx < len) ch = s[idx];
        else ch = ' ';
        twoWire_write(charToSegments(ch));
        twoWire_ack();
    }
}
/* ==============================================================================
 * Function: TM1637_displayString
 * Returns: void = no return.
 * Description: Displays a string on 6 digits; scrolls if longer than 6.
 * ============================================================================== */
void TM1637_displayString(char *string) {

    char len;
    char start;

	// init
	len = strlen(string);

	TM1637_setBrightness(3);

    if (len <= 6) {
        twoWire_start();
        twoWire_write(0x40);
        twoWire_ack();
        twoWire_stop();

        twoWire_start();
        twoWire_write(0xC0);
        twoWire_ack();
        TM1637_writeStringWindow(string, 0, len);
        twoWire_stop();
    } 
    else {
        for(start = 0; start <= (len - 6); start++) {
            twoWire_start();
            twoWire_write(0x40);
            twoWire_ack();
            twoWire_stop();

            twoWire_start();
            twoWire_write(0xC0);
            twoWire_ack();
            TM1637_writeStringWindow(string, start, len);
            twoWire_stop();

            delayMainLoop(1);
        }
    }
}
/* ==============================================================================
 * Function: TM1637_setBrightness
 * Returns: void = no return.
 * Description: Sets TM1637 brightness (0..8). 0 turns display off.
 * ============================================================================== */
void TM1637_setBrightness(char level){  

    twoWire_start();
    twoWire_write(0x87 + level);
    twoWire_ack();
    twoWire_stop();
}
/* ==============================================================================
 * Function: setDcc
 * Returns: void = no return.
 * Description: Builds a full DCC packet from address and command into gl_dcc buffer.
 * ============================================================================== */
void setDcc(char address, char command) {

	 char i;			
	 char bitNumber; 	
	 char control; 		

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
/* ==============================================================================
 * Function: setPort
 * Returns: void = no return.
 * Description: Updates microcontroller output latches (A..D) from current state
 * variables; handles stopAll.
 * ============================================================================== */
void setPort(void){

	char myPortA; // used to better synchronised output updates
	char myPortB; // used to better synchronised output updates
	char myPortC; // used to better synchronised output updates
	char myPortD; // used to better synchronised output updates

	if ((char)gl_stopAll==(char)TRUE) {
		gl_S1T0char=0; gl_S2T0char=0;
		gl_S1T1char=0; gl_S2T1char=0;
		gl_S1T2char=0; gl_S2T2char=0;
		gl_S1T3char=0; gl_S2T3char=0;	
		LATA=0xFF;
  	 	LATB=0xFF;
  	 	LATC=0xFF;
    	LATD=0xFF;
	}
	else {
	    myPortA=(gl_S1T0char<<4) + (gl_S1T1char<<6) + (gl_S2T0char<<7);
	    myPortB=(gl_OUTchar[2]&1) + ((gl_OUTchar[3]&1)<<1) + ((gl_OUTchar[4]&1)<<4) + ((gl_OUTchar[5]&1)<<5);
		myPortC=(gl_S2T1char) + (gl_S1T2char<<1) + (gl_S2T2char<<2) + (gl_S1T3char<<3) +((char)TRISCbits.RC4==(char)0 ? gl_GPIOchar[3] <<4 : 0);
		myPortD=(gl_S2T3char) + ((char)TRISDbits.RD1==(char)0 ? gl_GPIOchar[0] <<1 :0) + ((char)TRISDbits.RD2==(char)0 ? gl_GPIOchar[1] <<2:0) +((char)TRISDbits.RD3==(char)0 ? gl_GPIOchar[2] <<3:0) + ((gl_OUTchar[0]&1)<<6) +  ((gl_OUTchar[1]&1)<<7);
		LATA=myPortA;
	    LATC=myPortC;
	    LATD=myPortD;
	    LATB=myPortB;
	}

}

/******************************************************************************** 
 * interrupt_at_high_vector 
 ********************************************************************************/

#pragma code high_vector=0x08
/* ==============================================================================
 * Function: interrupt_at_high_vector
 * Returns: void = no return.
 * Description: Call high interrupt code
 * ============================================================================== */
void interrupt_at_high_vector(void){
    _asm goto high_isr _endasm
}
#pragma code

#pragma interrupt high_isr
/* ==============================================================================
 * Function: high_isr
 * Returns: void = ISR, no return.
 * Description: Hight priority interrupt service routine: receives CAN messages, 
 * sync handling, buffers data, and UART RX.
 * ============================================================================== */
void high_isr(void){

	BYTE dataLen; 				// Number of bytes transmitted in the message
	BYTE dataCounter;
	ECAN_RX_MSG_FLAGS flags; 	// Flags
	unsigned long 	id;			// Id of sender	
	char	dataBuffered;
	char	bufferNumber;	
	char	requestFooterTrameDetected;
	char	printFooterTrameDetected;

	// Low voltage detection
    if (PIR2bits.HLVDIF) {
		gl_low=1;				
        PIR2bits.HLVDIF = 0; 			// Clear flag HLVD
    }	

	dataBuffered=FALSE;

	if(PIR3bits.RXB0IF ||PIR3bits.RXB1IF) {
		while(ECANReceiveMessage((unsigned long *)&id,(BYTE *)&gl_data[0], (BYTE *) &dataLen,(ECAN_RX_MSG_FLAGS *) &flags));

   	   // BOARD SYNC
        if (id == SYNC_ID) {
    		INTCONbits.TMR0IF       = 0;   // T0 int flag bit cleared before starting
			TMR0H					= 0;   // re-phase Timer0
			TMR0L 					= 6;   // re-phase Timer0
           	gl_speedCounter 		= 1;   // Same value on each board
			gl_syncRequested 		= 1;   // For printing SYNC message on LED
			gl_flashingCounter		= 0;   // Flashing
		}
		// OTHER TRAMES
		else {

			// Try to assign this trame to an existing buffer
			for(bufferNumber=0;bufferNumber<MAXINPUTCANBUFFER;bufferNumber++) {
				if ((unsigned long)gl_currentCANid[bufferNumber]==(unsigned long)id && gl_inputCANmode[bufferNumber]!=CAN_FREE && gl_canReceivedDataReady[bufferNumber]==WAITING_FOR_DATA) {
					requestFooterTrameDetected=0;
					printFooterTrameDetected=0;
					for(dataCounter=0;dataCounter<dataLen;dataCounter++) {
						if ((char)gl_data[dataCounter]==(char)TRAMEREQUESTFOOTER) 	requestFooterTrameDetected++;	else requestFooterTrameDetected=0;
						if ((char)gl_data[dataCounter]==(char)TRAMEPRINTFOOTER) 	printFooterTrameDetected++;		else printFooterTrameDetected=0;
						gl_inputCANbuffer[bufferNumber][gl_inputCANWriteBufferPointer[bufferNumber]++]=gl_data[dataCounter];
						if(gl_inputCANWriteBufferPointer[bufferNumber]>=MAXTRAMESIZE)gl_inputCANWriteBufferPointer[bufferNumber]=0;

						// Lock this buffer until managed
						if(requestFooterTrameDetected==8 || printFooterTrameDetected==8) gl_canReceivedDataReady[bufferNumber]=READY;	
					}
					dataBuffered=TRUE;
					break;
				}		
			}
	
			// Assign a new buffer to this trame
			if ((char)dataBuffered==(char)FALSE) {
				for(bufferNumber=0;bufferNumber<MAXINPUTCANBUFFER;bufferNumber++) {
					if (gl_inputCANmode[bufferNumber]==CAN_FREE) {
						for(dataCounter=0;dataCounter<dataLen;dataCounter++) {
							gl_inputCANbuffer[bufferNumber][gl_inputCANWriteBufferPointer[bufferNumber]++]=gl_data[dataCounter];
							if(gl_inputCANWriteBufferPointer[bufferNumber]>=MAXTRAMESIZE)gl_inputCANWriteBufferPointer[bufferNumber]=0;
						}
						gl_inputCANmode[bufferNumber]=CAN_GET_DATA;	
						gl_currentCANid[bufferNumber]=id;
						dataBuffered=TRUE;
						break;
					}
				}
			}
			// Flashing light for buffer overflow !	
			if (dataBuffered==FALSE) {
				gl_OUTchar[3]=2;
			}
		}
	}


	if ((char)gl_master==(char)FALSE)return;

    // Check if interrupt originates from USART reception to read input data
    while (PIR1bits.RCIF) {
        gl_receivedUSARTData[gl_receivedUSARTPointer++] = RCREG;
		if (gl_receivedUSARTPointer>=USARTBUFFERSIZE)gl_receivedUSARTPointer=0;
    }
}
#pragma code

/********************************************************************************
 * low_interrupt 
 ********************************************************************************/
#pragma code low_vector=0x18
/* ==============================================================================
 * Function: low_interrupt
 * Returns: void = vector stub.
 * Description: Low‑priority interrupt vector stub that jumps to low_isr.
 * ============================================================================== */
void low_interrupt (void){
    _asm goto low_isr _endasm
}
#pragma code

#pragma interruptlow low_isr
/* ==============================================================================
 * Function: low_isr_task
 * Returns: void = no return.
 * Description: Low‑priority background task called from ISR: manages flashing, 
 * reads knobs/ADC and computes averages.
 * ============================================================================== */
void low_isr_task(void) {

char 	bitStateCounter;
char 	delay;
char 	selectBitDelay;
char 	bitNumber;
char 	bitValue;
short 	ADC;
int 	knobValue1;

	if (gl_goFlashingCounter==1)gl_flashingCounter++;
	if (((gl_flashingCounter & 0xFFF) == 0) || ((gl_flashingCounter & 0xFFF) == 0x7FF))gl_goFlashingCounter=0;

	// KNOBS MANAGEMENT
	gl_getKnobValue++;
	if (gl_getKnobValue==0) {

		gl_numberKnobData++;

		// KNOB VALUE
		ADCON0=INKNOB0;
		ADCON0bits.GO = 1;                            // ADCON0.GODONE = 1 
		while((char)ADCON0bits.GO == (char)1);        // wait till GODONE bit is zero
		ADC = ADRESH;    //Read converted result 
		ADC = (ADC<<8) + ADRESL;

		gl_adcKnobValue0+=ADC;

		ADCON0=INKNOB1;
		ADCON0bits.GO = 1;                            // ADCON0.GODONE = 1 
		while((char)ADCON0bits.GO == (char)1);        // wait till GODONE bit is zero
		ADC = ADRESH;    //Read converted result 
		ADC = (ADC<<8) + ADRESL;

		gl_adcKnobValue1+=ADC;

		if(gl_numberKnobData>=20) {
			gl_numberKnobData=0;
			gl_adcKnobValue0=gl_adcKnobValue0/10;
			gl_adcKnobValue1=gl_adcKnobValue1/10;

			if (gl_calibKnob==1) {
				if (gl_minAdcKnobValue0>gl_adcKnobValue0)gl_minAdcKnobValue0=gl_adcKnobValue0;
				if (gl_minAdcKnobValue1>gl_adcKnobValue1)gl_minAdcKnobValue1=gl_adcKnobValue1;
				if (gl_maxAdcKnobValue0<gl_adcKnobValue0)gl_maxAdcKnobValue0=gl_adcKnobValue0;
				if (gl_maxAdcKnobValue1<gl_adcKnobValue1)gl_maxAdcKnobValue1=gl_adcKnobValue1;
				gl_deltaKnob0=gl_maxAdcKnobValue0-gl_minAdcKnobValue0;
				gl_deltaKnob1=gl_maxAdcKnobValue1-gl_minAdcKnobValue1;
				
			}

			// Speed
			gl_knobValue0=(31*((long)(gl_adcKnobValue0-gl_minAdcKnobValue0))/(long)(gl_deltaKnob0))-15;
			if (gl_knobValue0>15)gl_knobValue0=15;
			else if (gl_knobValue0==-1 || gl_knobValue0==1)gl_knobValue0=0; // For stability around 0

			// Inertia
			knobValue1=(101*(long)(gl_adcKnobValue1-gl_minAdcKnobValue1))/(long)(gl_deltaKnob1);
			if (knobValue1>100)knobValue1=100;
			if(gl_knobValue1==0 && knobValue1==100)knobValue1=0; // Should happen if potentiometer lost contact
			gl_knobValue1=knobValue1;
			
			gl_adcKnobValue0=0;
			gl_adcKnobValue1=0;
		}
	}

	// GPIO IN Detection
	if((char)TRISDbits.RD1==(char)1 && (char)PORTDbits.RD1!=(char)gl_GPIOchar[0]){
		gl_GPIOstabilized[0]++;
		if (gl_GPIOstabilized[0]>(unsigned int)GPIOTHRESHOLD) {
			gl_GPIOchar[0]=PORTDbits.RD1;
			if (gl_GPIOchar[0]==1)gl_GPIOcounter[0]++;
			gl_GPIONotification[0]=TRUE;
			gl_GPIOstabilized[0]=0;
		}
	}
	else gl_GPIOstabilized[0]=0;

	if((char)TRISDbits.RD2==(char)1 && (char)PORTDbits.RD2!=(char)gl_GPIOchar[1]){
		gl_GPIOstabilized[1]++;
		if (gl_GPIOstabilized[1]>(unsigned int)GPIOTHRESHOLD) {
			gl_GPIOchar[1]=PORTDbits.RD2;
			if (gl_GPIOchar[1]==1)gl_GPIOcounter[1]++;
			gl_GPIONotification[1]=TRUE;
			gl_GPIOstabilized[1]=0;
		}
	}
	else gl_GPIOstabilized[1]=0;

	if((char)TRISDbits.RD3==(char)1 && (char)PORTDbits.RD3!=(char)gl_GPIOchar[2]){
		gl_GPIOstabilized[2]++;
		if (gl_GPIOstabilized[2]>(unsigned int)GPIOTHRESHOLD) {
			gl_GPIOchar[2]=PORTDbits.RD3;
			if (gl_GPIOchar[2]==1)gl_GPIOcounter[2]++;
			gl_GPIONotification[2]=TRUE;
			gl_GPIOstabilized[2]=0;
		}
	}
	else gl_GPIOstabilized[2]=0;

	if((char)TRISCbits.RC4==(char)1 && (char)PORTCbits.RC4!=(char)gl_GPIOchar[3]){
		gl_GPIOstabilized[3]++;
		if (gl_GPIOstabilized[3]>(unsigned int)GPIOTHRESHOLD) {
			gl_GPIOchar[3]=PORTCbits.RC4;
			if (gl_GPIOchar[3]==1)gl_GPIOcounter[3]++;
			gl_GPIONotification[3]=TRUE;
			gl_GPIOstabilized[3]=0;
		}
	}
	else gl_GPIOstabilized[3]=0;

	if ((char)gl_stopAll==(char)FALSE) {

		gl_trackNumber=gl_trackNumber+1;
		if (gl_trackNumber>3)gl_trackNumber=0;

		// TIMER
		gl_timer--;
		if ((unsigned short)gl_timer==(unsigned short)0) {
			gl_timer=INITTIMERVALUE;
			if (gl_TIMERValue[gl_timerNumber]>0){
				gl_TIMERValue[gl_timerNumber]--;
				if (gl_TIMERValue[gl_timerNumber]==0)gl_TIMERNotification[gl_timerNumber]=TRUE;
			}
			gl_timerNumber++;
			if(gl_timerNumber>MAXTIMER)gl_timerNumber=0;
		}

		//////////// MODE ANALOG ////////////////
		if(gl_boardMode==ANAValue) {
			gl_speedCounter++;
			if (gl_speedCounter>MAX_INERTIA_COUNTER) {
	 			gl_speedCounter=1;
			}

			if (gl_curSpeed[gl_trackNumber]!=gl_setPoint[gl_trackNumber]) {
				gl_setStepCounter[gl_trackNumber]--;
				if (gl_setStepCounter[gl_trackNumber]<=0) {
					gl_setStepCounter[gl_trackNumber]=gl_setStep[gl_trackNumber];
					if (gl_curSpeed[gl_trackNumber]>gl_setPoint[gl_trackNumber]){
						gl_curSpeed[gl_trackNumber]-=MAX_STEP;
						if(gl_curSpeed[gl_trackNumber]<gl_setPoint[gl_trackNumber])gl_curSpeed[gl_trackNumber]=gl_setPoint[gl_trackNumber];
					}
					else {
						gl_curSpeed[gl_trackNumber]+=MAX_STEP;
						if(gl_curSpeed[gl_trackNumber]>gl_setPoint[gl_trackNumber])gl_curSpeed[gl_trackNumber]=gl_setPoint[gl_trackNumber];
					}
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
					gl_direction[gl_trackNumber]=TRACK_STOP;
					gl_speed[gl_trackNumber]=0;
				}
			}

			// Set Port Value when all tracks have been updated
			if (gl_trackNumber==0) {
				for(gl_trackNumber=0;gl_trackNumber<4;gl_trackNumber++) {
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
				}
				gl_trackNumber=0;
				setPort();
			}
		}
		else if(gl_boardMode==DCCValue && gl_dcc_ready==0) {		
			for(bitStateCounter=0;bitStateCounter<FRAME_SIZE;bitStateCounter++) {
				if (gl_dcc[bitStateCounter]==0) selectBitDelay=DCC_0;
				else selectBitDelay=DCC_1;
						
				gl_S1T0char=0;gl_S1T1char=0;
				gl_S1T2char=0;gl_S1T3char=0;
				gl_S2T0char=1;gl_S2T1char=1;
				gl_S2T2char=1;gl_S2T3char=1;
           		setPort();
				for(delay=0;delay<selectBitDelay;delay++);

	    		gl_S2T0char=0;gl_S2T1char=0;
				gl_S2T2char=0;gl_S2T3char=0;
				gl_S1T0char=1;gl_S1T1char=1;
				gl_S1T2char=1;gl_S1T3char=1;
           		setPort();
				for(delay=0;delay<selectBitDelay;delay++);
			}
			gl_S1T0char=0; gl_S2T0char=1;	
			gl_S1T1char=0; gl_S2T1char=1;
			gl_S1T2char=0; gl_S2T2char=1;	
			gl_S1T3char=0; gl_S2T3char=1;
       		setPort();
			for(delay=0;delay<selectBitDelay;delay++);	
			for(bitStateCounter=0;bitStateCounter<FRAME_SIZE;bitStateCounter++)gl_dcc[bitStateCounter]=1;
		}
		gl_dcc_ready--;
		if (gl_dcc_ready<0) gl_dcc_ready=INITWAITDCCCOUNTER;


		// TRACK DETECTION 

		switch(gl_trackNumber) {
			case 0 : ADCON0=CURT0;break;
	    	case 1 : ADCON0=CURT1;break;
	    	case 2 : ADCON0=CURT2;break;
	   		case 3 : ADCON0=CURT3;break;
		}
		// NEED TO GET LOW VOLTAGE VALUE WHEN TRACK IS OFF FOR CALIBRATION AT POWER ON
 		if ((char)gl_trackCalibration==(char)TRUE) {
			ADCON0bits.GO = 1;                            // ADCON0.GODONE = 1 
			while((char)ADCON0bits.GO ==(char) 1);                    // wait till GODONE bit is zero
			ADC = 0;
			ADC = ADRESH;    //Read converted result 
			ADC = (ADC<<8) + ADRESL;
			gl_average[gl_trackNumber]=(SAMPLEFORCALIBRATION*gl_average[gl_trackNumber]+ADC)/(SAMPLEFORCALIBRATION+1);
			if (gl_noVehicule[gl_trackNumber]>gl_average[gl_trackNumber])gl_noVehicule[gl_trackNumber]=gl_average[gl_trackNumber];
		}

		if(((char)gl_speed[gl_trackNumber]==(char)gl_speedCounter && (char)gl_boardMode==(char)ANAValue) || ((char)gl_dcc_ready==(char)INITWAITDCCCOUNTER && (char)gl_boardMode==(char)DCCValue)) { // ONLY WHEN POWER ON
			ADCON0bits.GO = 1;                            // ADCON0.GODONE = 1 
			while((char)ADCON0bits.GO == (char)1);                    // wait till GODONE bit is zero
			ADC = 0;
			ADC = ADRESH;    //Read converted result 
			ADC = (ADC<<8) + ADRESL;

			if (gl_average[gl_trackNumber]<ADC) gl_average[gl_trackNumber]=ADC; // TRAP THE EVENT
			else gl_average[gl_trackNumber]=(SAMPLEFORAVERAGE*gl_average[gl_trackNumber]+ADC)/(SAMPLEFORAVERAGE+1);

			if (((int)(10*gl_average[gl_trackNumber])>(int)((10+HYSTERERISHIGH)*gl_noVehicule[gl_trackNumber])) && ((char)gl_OUTSTATchar[gl_trackNumber]==(char)0)  && ((char)gl_trackNotification[gl_trackNumber]==(char)FALSE)) {
				gl_OUTSTATchar[gl_trackNumber]=1;
				gl_trackNotification[gl_trackNumber]=TRUE;
			}
			else if (((int)(10*gl_average[gl_trackNumber])<(int)((10+HYSTERERISLOW)*gl_noVehicule[gl_trackNumber])) && ((char)gl_OUTSTATchar[gl_trackNumber]==(char)1)  && ((char)gl_trackNotification[gl_trackNumber]==(char)FALSE)) {
				gl_OUTSTATchar[gl_trackNumber]=0;
				gl_trackNotification[gl_trackNumber]=TRUE;
			}
		} 
	}
	else setPort();
}
/* ==============================================================================
 * Function: low_isr
 * Returns: void = no return.
 * Description: Call low interrupt code
 * ============================================================================== */
void low_isr(void){
	
	if (!gl_mutexLowIsr) {
		low_isr_task();
	}

    // INTERRUPT RESET
    if((char)INTCONbits.TMR0IF==(char)1){
	    INTCONbits.TMR0IF = 0;
		TMR0L = 6;
    }
}
#pragma code
/* ==============================================================================
 * Function: initEnvironment
 * Returns: void = no return.
 * Description: Init all the structures and global variables
 * Warning : Don't call this function without disabling access to low_isr
 * ============================================================================== */
void initEnvironment(void) {

    char trackNumber;
	char OUTCounter;	
	char GPIOCounter;
	char TIMERCounter;

	gl_mutexLowIsr=1;gl_stopAll=TRUE;;gl_mutexLowIsr=0;

	// GPIO //////////////////////////
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

	for(GPIOCounter=0;GPIOCounter<4;GPIOCounter++) {
		gl_mutexLowIsr=1; 
		gl_GPIOchar[GPIOCounter]=0;
		gl_GPIOcounter[GPIOCounter]=0;
		gl_GPIOstabilized[GPIOCounter]=0;
		gl_GPIONotification[GPIOCounter]=TRUE; // To read current value on TCO
		gl_mutexLowIsr=0; 
	}
	gl_mutexLowIsr=1;gl_goFlashingCounter=1;gl_mutexLowIsr=0;

	// Internal setting initialisation //////////////////////////
    for(trackNumber=0;trackNumber<4;trackNumber++) {
		gl_mutexLowIsr=1;
		gl_average[trackNumber]=0;
		gl_noVehicule[trackNumber]=0;
		gl_speed[trackNumber]=0;
		gl_direction[trackNumber]=0;
		gl_setPoint[trackNumber]=0;
		gl_setStep[trackNumber]=0;
		gl_setStepCounter[trackNumber]=0;
		gl_curSpeed[trackNumber]=0;
		gl_OUTSTATchar[trackNumber]=0;
		gl_trackNotification[trackNumber]=FALSE;
		gl_mutexLowIsr=0;
    }

	// LPO  //////////////////////////
	for(OUTCounter=0;OUTCounter<6;OUTCounter++) {
		gl_mutexLowIsr=1;gl_OUTchar[OUTCounter]=1;gl_mutexLowIsr=0;
	}	

	// TRACK //////////////////////////
	gl_mutexLowIsr=1;
	gl_S1T0char=0; gl_S2T0char=0;
	gl_S1T1char=0; gl_S2T1char=0;
	gl_S1T2char=0; gl_S2T2char=0;
	gl_S1T3char=0; gl_S2T3char=0;	
	gl_trackNumber=0;
	gl_speedCounter=0;
	gl_syncRequested=0;
	gl_trackCalibration=FALSE;
	gl_mutexLowIsr=0;

	// TIMER //////////////////////////
	for(TIMERCounter=0;TIMERCounter<MAXTIMER;TIMERCounter++) {
		gl_mutexLowIsr=1;
		gl_TIMERValue[TIMERCounter]=0;
		gl_TIMERNotification[TIMERCounter]=FALSE;
		gl_mutexLowIsr=0;
	}
	gl_mutexLowIsr=1;
	gl_timerNumber=0;
	gl_timer=0;
	gl_mutexLowIsr=0;

	// DCC TEMPO BETWEEN TWO TRAMES
    gl_mutexLowIsr=1;gl_dcc_ready=INITWAITDCCCOUNTER;gl_mutexLowIsr=0;

	// Command and Program input request
	initRequest();

	// Knob
	gl_mutexLowIsr=1;
	gl_adcKnobValue0=0;
	gl_adcKnobValue1=0;
	gl_getKnobValue=0;
	gl_calibKnob=0;
	gl_knobValue0=0;
	gl_knobValue1=0;
	gl_numberKnobData=0;
	gl_mutexLowIsr=0;

	// Default user mode is automatic 
	gl_mutexLowIsr=1;
	gl_userMode=AUTOMATICValue;
	gl_mutexLowIsr=0;

	// Update form EEPROM
	ReadEEPROMConfig();

	gl_mutexLowIsr=1;
	if((char)TRISDbits.RD1==(char)0)gl_GPIOchar[0]=1; else gl_GPIOchar[0]=0xFF; // out default value is 1 
	if((char)TRISDbits.RD2==(char)0)gl_GPIOchar[1]=1; else gl_GPIOchar[1]=0xFF; // out default value is 1 
	if((char)TRISDbits.RD3==(char)0)gl_GPIOchar[2]=1; else gl_GPIOchar[2]=0xFF; // out default value is 1 
	if((char)TRISCbits.RC4==(char)0)gl_GPIOchar[3]=1; else gl_GPIOchar[3]=0xFF; // out default value is 1 	
	gl_mutexLowIsr=0;

	

	// Error clean
	clearError();

}
/* ==============================================================================
 * Function: delayMainLoop
 * Returns: void = no return.
 * Description: Deliver a delay for the main loop
 * ============================================================================== */
void delayMainLoop(int delay) {

    long i;

    for(i = 0; i <(long)(delay*5000UL); i++);
}

/* ==============================================================================
 * Function: PIC18FMainSettings
 * Returns: void = no return.
 * Description: Set PIC18F4680 
 * ============================================================================== */
void PIC18FMainSettings (void){

    // PIC setup and enable interrupts

    gl_mutexLowIsr = 1; 

    OSCCON                  = 0x70; // no pre-divider => 8 MHz 
    OSCTUNE                 = 0x40; // PLL x4 => 32 MHz 
    T0CONbits.T08BIT        = 1;    // 8-bit timer 
    T0CONbits.T0CS          = 0;    // increment on instruction cycle (Fosc/4)
    T0CONbits.T0SE          = 0;    // (no effect when T0CS=0)

    T0CONbits.PSA           = 0;    // prescaler assigned to TMR0  
    T0CONbits.T0PS0         = 0;    // 1:2 prescaler
    T0CONbits.T0PS1         = 0;
    T0CONbits.T0PS2         = 0;

    INTCONbits.TMR0IF       = 0;    // clear TMR0 interrupt flag
    TMR0H                   = 0;
    TMR0L                   = 6;    // 16 kHz (62.5 �s)

    RCONbits.IPEN           = 1;    // enable interrupt priority levels
    INTCONbits.GIEL         = 1;    // enable low-priority interrupts
    INTCONbits.GIEH         = 1;    // enable high-priority interrupts
    INTCONbits.PEIE         = 1;    // enable peripheral interrupts

    INTCONbits.TMR0IE       = 1;    // enable Timer0 interrupt
    INTCON2bits.TMR0IP      = 0;    // Timer0 on low priority

	gl_low					= 0;

    HLVDCON                 = 0;    // reset the register
    HLVDCONbits.HLVDL       = LOWTH; // threshold 
    HLVDCONbits.VDIRMAG     = 0;    // interrupt if VDD < threshold
    HLVDCONbits.HLVDEN      = 1;    // enable HLVD
    
    while(!HLVDCONbits.IRVST);      // wait for the reference to stabilize
    
    PIR2bits.HLVDIF         = 0;    // clear HLVD interrupt flag
	IPR2bits.HLVDIP 		= 1;
    PIE2bits.HLVDIE         = 1;    // enable HLVD interrupt

    T0CONbits.TMR0ON        = 1;    // start Timer0

    gl_mutexLowIsr = 0; 
}


/* ==============================================================================
 * Function: init
 * Returns: void = no return.
 * Description: Init PIC18F4680
 * ============================================================================== */
void init(void) {
 
	char bufferNumber;
	char automationCounter;

    // Environment
     initEnvironment();

    // Main settings + Start timer
    PIC18FMainSettings(); 

	gl_mutexLowIsr=1; 

    // RS232
	initUSART(); // Serial USART init on master board only
	gl_inputCounter=0; // for UART
	mySprintf((char *)gl_inputUartString,"");

	// Use user putc function
	stdout = _H_USER;
    
    // ECAN
    TRISBbits.TRISB3 = 1; // CANRX input setting
	gl_outputCANbufferCounter=0;
	for(bufferNumber=0;bufferNumber<MAXINPUTCANBUFFER;bufferNumber++) {
		gl_inputCANWriteBufferPointer[bufferNumber]=0;
		gl_inputCANReadBufferPointer[bufferNumber]=0;
		gl_inputCANmode[bufferNumber]=CAN_FREE;
		gl_canReceivedDataReady[bufferNumber]=WAITING_FOR_DATA;
	}
    ECANInitialize(); // init ECAN
	PIE3bits.RXB0IE=1; // enable interrupt for CAN
	PIE3bits.RXB1IE=1; // enable interrupt for CAN

	// Display
    TM1637_init();

	// Get board number
	gl_boardNumber=IN4 + 2*IN3 + 4*IN0 + 8*IN1 + 16*IN2;

	// Detect master board
	if (gl_boardNumber==31) gl_master=TRUE;
	else gl_master=FALSE;

	gl_mutexLowIsr=0; 

	mySprintf((char *)gl_message,INIT_STRING);
	TM1637_displayString((char *)gl_message);

	gl_mutexLowIsr=1;gl_stopAll=FALSE;gl_mutexLowIsr=0;

	// CALIBRATION FOR TRACK DETECTION
    trackCalibration();

	// MESSAGE ON DISPLAY
	mySprintf((char *)gl_message,START_STRING);
	TM1637_displayString((char *)gl_message);
}
/* ==============================================================================
 * Function: trackCalibration
 * Returns: void = no return.
 * Description: Evaluate track voltage at power on for later train detection
 * No calibration after a reset due to watchdog event
 * ============================================================================== */
void trackCalibration(void) {

    short trackNumberCalibration;
	char 	trackNumber;

	gl_mutexLowIsr=1;	gl_trackCalibration=TRUE; gl_mutexLowIsr = 0;
	delayMainLoop(TRACKCALIBRATIONDELAY);

    for(trackNumber=0;trackNumber<4;trackNumber++) {
	  gl_noVehicule[trackNumber]=0xFF;
	}
	delayMainLoop(TRACKCALIBRATIONDELAY);
	gl_mutexLowIsr=1;	gl_trackCalibration=FALSE;  gl_mutexLowIsr = 0;
}
/* ==============================================================================
 * Function: getEventRequestFromTrack
 * Returns: char = implementation-defined.
 * Description: Notify when a train or a car arrives or leaves at a track.
 * ============================================================================== */
char getEventRequestFromTrack(void) {

	 char trackNumber;
	 char 	value;
	 short adr;

	for(trackNumber=0;trackNumber<4;trackNumber++) {
		if ((char)gl_trackNotification[trackNumber]==(char)TRUE) {
			initRequest();
			gl_request[REQ_BOARD_NUMBER]=gl_boardNumber;
			gl_request[REQ_EVENT_REQUEST_TRACK_EVENT]=TRUE;
			gl_request[REQ_EVENT_REQUEST_EVENT_BOARD_TRACK_NUMBER]=gl_boardNumber;
			gl_request[REQ_EVENT_REQUEST_EVENT_TRACK_NUMBER]=trackNumber;
			gl_request[REQ_EVENT_REQUEST_EVENT_VEHICLE_STATUS]=gl_OUTSTATchar[trackNumber]==1 ? ONTRACKValue : OFFTRACKValue;

			// Read status to push notification on CAN bus
			adr=(short)TRACK_CAN_NOTIFICATION_ADDRESS+trackNumber;
			ReadEEPROM(adr,&value);
			gl_request[REQ_EVENT_REQUEST_EVENT_CAN_NOTIFICATION]=value;

			gl_mutexLowIsr=1;gl_trackNotification[trackNumber]=FALSE;gl_mutexLowIsr=0;
			return (TRUE);
		}
	}
	return(FALSE);	
}
/* ==============================================================================
 * Function: getEventRequestFromGPIO
 * Returns: char = implementation-defined.
 * Description: Notify when a GPIO value has changed
 * ============================================================================== */
char getEventRequestFromGPIO(void) {

	 char GPIONumber;
	 char 	value;
	 short adr;

	for(GPIONumber=0;GPIONumber<4;GPIONumber++) {
		if ((char)gl_GPIONotification[GPIONumber]==(char)TRUE) {
			initRequest();
			gl_request[REQ_BOARD_NUMBER]=gl_boardNumber;
			gl_request[REQ_EVENT_REQUEST_GPIO_EVENT]=TRUE;
			gl_request[REQ_EVENT_REQUEST_EVENT_BOARD_GPIO_NUMBER]=gl_boardNumber;
			gl_request[REQ_EVENT_REQUEST_EVENT_GPIO_NUMBER]=GPIONumber;
			if (gl_GPIOchar[GPIONumber]==0)gl_request[REQ_EVENT_REQUEST_EVENT_GPIO_LEVEL]=0;
			else gl_request[REQ_EVENT_REQUEST_EVENT_GPIO_LEVEL]=gl_GPIOcounter[GPIONumber];

			// Read status to push notification on CAN bus
			adr=(short)GPIO_CAN_NOTIFICATION_ADDRESS+GPIONumber;
			ReadEEPROM(adr,&value);
			gl_request[REQ_EVENT_REQUEST_EVENT_CAN_NOTIFICATION]=value;

			gl_mutexLowIsr=1;gl_GPIONotification[GPIONumber]=FALSE;gl_mutexLowIsr=0;
			return(TRUE);
		}
	}
	return(FALSE);	
}
/* ==============================================================================
 * Function: getEventRequestFromTIMER
 * Returns: char = implementation-defined.
 * Description: Notify when a timer has triggered
 * ============================================================================== */
char getEventRequestFromTIMER(void) {

	 char TIMERNumber;
	 char 	value;
	 short adr;

	for(TIMERNumber=0;TIMERNumber<MAXTIMER;TIMERNumber++) {
		if ((char)gl_TIMERNotification[TIMERNumber]==(char)TRUE) {
			initRequest();
			gl_request[REQ_BOARD_NUMBER]=gl_boardNumber;
			gl_request[REQ_EVENT_REQUEST_TIMER_EVENT]=TRUE;
			gl_request[REQ_EVENT_REQUEST_EVENT_BOARD_TIMER_NUMBER]=gl_boardNumber;
			gl_request[REQ_EVENT_REQUEST_EVENT_TIMER_NUMBER]=TIMERNumber;

			// Read status to push notification on CAN bus
			adr=(short)TIMER_CAN_NOTIFICATION_ADDRESS+TIMERNumber;
			ReadEEPROM(adr,&value);
			gl_request[REQ_EVENT_REQUEST_EVENT_CAN_NOTIFICATION]=value;

			gl_mutexLowIsr=1;gl_TIMERNotification[TIMERNumber]=FALSE;gl_mutexLowIsr=0;
			return(TRUE);
		}
	}
	return(FALSE);	
}
/* ==============================================================================
 * Function: getEventFromKNOB
 * Returns: void = no return.
 * Description: get current value of knobs
 * ============================================================================== */
void getEventFromKNOB(void) {

	char trackNumber;

	// Manage knob value

	if (gl_lastKnobValue0!=gl_knobValue0||gl_lastKnobValue1!=gl_knobValue1) {

		if (gl_userMode==AUTOMATICValue) {
	
			if (gl_lastKnobValue0!=gl_knobValue0) {
					mySprintf((char *)gl_message,"%S%d",SPEED_STRING,gl_knobValue0);
					TM1637_displayString((char *)gl_message);
					gl_lastKnobValue0=gl_knobValue0;
			}
			if (gl_lastKnobValue1!=gl_knobValue1) {
					mySprintf((char *)gl_message,"%S%d",INERTIA_STRING,gl_knobValue1);
					TM1637_displayString((char *)gl_message);
					gl_lastKnobValue1=gl_knobValue1;
			}
		}				
		else {
			gl_lastKnobValue0=gl_knobValue0;
			gl_lastKnobValue1=gl_knobValue1;
			TM1637_display(gl_lastKnobValue0,gl_lastKnobValue1);
	
			// MANUAL
			if (gl_userMode==MANUALValue) {
				for(trackNumber=0;trackNumber<4;trackNumber++) {
					setSpeed(gl_knobValue0,gl_knobValue1,trackNumber);
				}
			}
			else if (gl_userMode==MANUAL0Value) {
				setSpeed(gl_knobValue0,gl_knobValue1,0);
			}
			else if (gl_userMode==MANUAL1Value) {
				setSpeed(gl_knobValue0,gl_knobValue1,1);
			}
			else if (gl_userMode==MANUAL2Value) {
				setSpeed(gl_knobValue0,gl_knobValue1,2);
			}
			else if (gl_userMode==MANUAL3Value) {
				setSpeed(gl_knobValue0,gl_knobValue1,3);
			}
		}
	}
}
/* ==============================================================================
 * Function main() 
 * ============================================================================== */
 
void main(void)
{

    char OUTCounter;

	// Full init of PIC18F
    init();

	// START MAIN LOOP
    while (1){

		// In this loop we wait to get something to manage
		while(1) {	
				
			if ((gl_flashingCounter & 0xFFF) == 0) {
				for(OUTCounter=0;OUTCounter<6;OUTCounter++) {
					if (gl_OUTchar[OUTCounter]==3) {
						gl_mutexLowIsr=1;gl_OUTchar[OUTCounter]=2;gl_mutexLowIsr=0;
					}
				}
				gl_goFlashingCounter=1;
			}
			else if ((gl_flashingCounter & 0xFFF) == 0x7FF) {
				for(OUTCounter=0;OUTCounter<6;OUTCounter++) {
					if (gl_OUTchar[OUTCounter]==2) {
						gl_mutexLowIsr=1;gl_OUTchar[OUTCounter]=3;gl_mutexLowIsr=0;
					}
				}
				gl_goFlashingCounter=1;
			}

			// Board synchronisation information
			if (gl_syncRequested) {
				mySprintf((char *)gl_message,SYNC_STRING);
				TM1637_displayString((char *)gl_message);
				gl_syncRequested=0;
			}

			// Low voltage detection
			if(gl_low)  {
				mySprintf((char *)gl_message,VOLT_STRING);
				TM1637_displayString((char *)gl_message);
				gl_low=0;
			}

			// Manage knob value (for MANUAL and AUTOMATIC mode)
			getEventFromKNOB();

			// Get current status on tracks
			if (getEventRequestFromTrack()==(char)TRUE) {
				break;
			}

			// Get current status on GPIO
			if (getEventRequestFromGPIO()==(char)TRUE) {
				break;
			}

			// Get current status on TIMER	
			if (getEventRequestFromTIMER()==(char)TRUE) {
				break;
			}

			// Get Request from CAN Bus
			if (getInputRequestFromCAN()==(char)TRUE) {
				break;
			}

			// Get Request from RS232 input
			if (getInputRequestFromUSART((char *)gl_inputUartString,(char *)&gl_inputCounter)==(char)TRUE) {

				if ((int)strlen(gl_inputUartString)!=0) {

					// Call parser to analyse input request
	       			if (parser((char *)gl_inputUartString)==(char)TRUE) {
						mySprintf((char *)gl_inputUartString,"");						
						break;
					}
					else { // parsing error
						mySprintf((char *)gl_inputUartString,"");
						traceError();
					}		
				}
				else {
					mySprintf((char *)gl_message,"");
					prompt((char *)gl_message);
				}
			}
		}

		// Manage other requests than request from CAN bus
		if (gl_parserErrorCode!=0) traceError();
		else {
			if (manageRequest(TRUE)==(char)FALSE){
					traceError();
			}
			initRequest();
		}
	}
}
