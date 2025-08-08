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

	for(automationCounter=0;automationCounter<gl_nexAvailableAutomation;automationCounter++) {	
		for(automationDataCounter=0;automationDataCounter<AUTOMATIONSIZE;automationDataCounter++) {
			ReadEEPROM(adr++,&value);

			// Case X Codage
			if (value!=0) {
				gl_automation[automationCounter][automationDataCounter]=value;
			}
			// Case 0 <Nb>0> X Codage
			else {
				ReadEEPROM(adr++,&value);
				quantityValue=value;
				ReadEEPROM(adr++,&value);
				gl_automation[automationCounter][automationDataCounter]=value;
				quantityValue--;
				while (quantityValue>0) {
					gl_automation[automationCounter][automationDataCounter++]=value;
					quantityValue--;
				}
			}

			if (adr>=1024) {
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
	// Codage ::= <X!=0> Codage | 0 0 Codage | 0 <Nb>3> X Codage 
	dataCounter=0;
	curValue=gl_automation[0][0];
	quantityValue=0;


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
				// Case X Codage
				if (quantityValue<=3) {
					for(counter=0;counter<quantityValue;counter++) {
					  	value=curValue;

						// if curValue is 0, need to add the same symbol once
						if (value==0) {	
							ReadEEPROM(adr,&checkValue);
							if (checkValue!=value)if(WriteEEPROM(adr++,value);else adr++;
							ReadEEPROM(adr,&checkValue);
							if (checkValue!=value)WriteEEPROM(adr++,value);else adr++;
						}
					}
				}
				// Case 0 <Nb>0> X Codage
		
				value=0;	
				ReadEEPROM(adr,&checkValue);
				if (checkValue!=value)WriteEEPROM(adr++,value);else adr++;

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
		value=0;	
		ReadEEPROM(adr,&checkValue);
		if (checkValue!=value)WriteEEPROM(adr++,value);else adr++;

		value=quantityValue;
		ReadEEPROM(adr,&checkValue);
		if (checkValue!=value)WriteEEPROM(adr++,value);else adr++;

		value=curValue;
		ReadEEPROM(adr,&checkValue);
		if (checkValue!=value)WriteEEPROM(adr++,value);else adr++;
	}

	// update in EEPROM next automation value
	adr=(unsigned short)NEXTTAUTOMATION_ADDRESS;
	value=gl_nexAvailableAutomation;
	ReadEEPROM(adr,&checkValue);
	if (checkValue!=value)WriteEEPROM(adr,value);
	return(TRUE);	
}

