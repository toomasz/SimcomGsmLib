#include "Sim900.h"
static int put_data(char c, FILE *f)
{
	Sim900::ser->write(c);
	return 0;
}
Sim900::Sim900(Stream* serial, int powerPin, Stream &debugStream) :
ds(debugStream), parser(debugStream)
{
	_powerPin = powerPin;
	Sim900::ser = serial;
	lastDataWrite = 0;
	dataBufferTail = 0;
	dataBufferHead  = 0;
	pinMode(powerPin, OUTPUT);
	parser.gsm = this;
	parser.ctx = this;
	memset(_dataBuffer,0, DATA_BUFFER_SIZE);
#ifdef fdev_setup_stream
	fdev_setup_stream(&dataStream,put_data, NULL, _FDEV_SETUP_WRITE);
#endif

}
Stream* Sim900::ser = {0};
/*
Get gsm network registration status
Return values S900_ERROR, S900_TIMEOUT, SEARCHING_FOR_NETWORK0, HOME_NETWORK, SEARCHING_FOR_NETWORK, REGISTRATION_DENIED, REGISTRATION_UNKNOWN, ROAMING
*/
int Sim900::GetRegistrationStatus()
{
	parser.SetCommandType(AT_CREG);

	ser->println(F("AT+CREG?"));

	registrationStatus = PopCommandResult(AT_DEFAULT_TIMEOUT);
	return registrationStatus;
}


/*
Get operator name 
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
int Sim900::GetOperatorName()
{
	operatorName[0] = 0;
	parser.SetCommandType(AT_COPS);
	ser->println(F("AT+COPS?"));
	
	int result = PopCommandResult(AT_DEFAULT_TIMEOUT);
	return result;
}
/*
Get signal quality returned by AT+CSQ
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
int Sim900::getSignalQuality()
{
	parser.SetCommandType(AT_CSQ);
	ser->println(F("AT+CSQ"));
	
	int result = PopCommandResult(AT_DEFAULT_TIMEOUT);

	return result;
}


void Sim900::PrintEscapedChar( char c )
{
	if(c=='\r')
		ds.print("\\r");
	else if(c=='\n')
		ds.print("\\n");
	else
		ds.print(c);	
}
/*
Get pdp context status using AT+CIPSTATUS
Return values S900_ERR, S900_TIMEOUT
IP_INITIAL, IP_START, IP_CONFIG, IP_GPRSACT, IP_STATUS, TCP_CONNECTING, TCP_CLOSED, PDP_DEACT, CONNECT_OK
*/
int Sim900::GetIpStatus()
{
	parser.SetCommandType(AT_CIPSTATUS);
	ser->println(F("AT+CIPSTATUS"));

	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}
/*
Get ip address using AT+CIFSR
Return values S900_OK, S900_TIMEOUT
*/
int Sim900::GetIpAddress( )
{
	parser.SetCommandType(AT_CIFSR);
	ser->println(F("AT+CIFSR"));


	int result = PopCommandResult(AT_DEFAULT_TIMEOUT);
	return result;
}
/*
Bring up GRPS connection
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
int Sim900::AttachGprs()
{
	parser.SetCommandType(AT_DEFAULT);
	ser->println(F("AT+CIICR"));
	return PopCommandResult(60000);
}
/*
Start tcp connection to address:port
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
int Sim900::StartTransparentIpConnection(const char *address, int port )
{
	dataBufferHead = dataBufferTail = 0;
	parser.SetCommandType(AT_CIPSTART);
	// Execute command like AT+CIPSTART="TCP","ag.kt29.net","80"
	ser->print(F("AT+CIPSTART=\"TCP\",\"")); ser->print(address);  ser->print(F("\",\"")); ser->print(port); ser->println('"');

	return PopCommandResult(60000);
}
/* close active connection */
int Sim900::CloseConnection()
{
	parser.SetCommandType(AT_CIPCLOSE);
	ser->println(F("AT+CIPCLOSE=1"));
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}


/* returns true if any data is available to read from transparent connection */
bool Sim900::DataAvailable()
{
	if(dataBufferHead != dataBufferTail)
		return true;
	if(ser->available())
		return true;
	return false;
}
void Sim900::PrintDataByte(uint8_t data) // prints 8-bit data in hex
{
	char tmp[3];
	byte first;
	byte second;

	first = (data >> 4) & 0x0f;
	second = data & 0x0f;

	tmp[0] = first+48;
	tmp[1] = second+48;
	if (first > 9) tmp[0] += 39;
	if (second > 9) tmp[1] += 39;
	
	tmp[2] = 0;
	ds.write(' ');
	ds.print(tmp);
	ds.write(' ');
}

int Sim900::DataRead()
{
	int ret = ReadDataBuffer();
	if(ret != -1)
	{
		//PrintDataByte(ret);
		return ret;
	}
	ret = ser->read();
	if(ret != -1)
	{
	//	PrintDataByte(ret);
	}
	return ret;
}

/*
Switches to command mode ATO
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
int Sim900::SwitchToCommandMode()
{
	parser.SetCommandType(AT_SWITH_TO_COMMAND);
	commandBeforeRN = true;
	// +++ escape sequence needs to wait 1000ms after last data was sent via transparent connection
	// in the meantime data from tcp connection may arrive so we read it here to dataBuffer
	// ex: lastDataWrite = 1500
	// loop will exit when millis()-1500<1000 so when millis is 2500
	
	delay(1000);
  /* while(ser->available() || (millis()-lastDataWrite) < 1000)
	{
		while(ser->available())
		{
			int c = ser->read();
			pr("\nsc_data: %c\n", (char)c);
			WriteDataBuffer(c);
		}
	}*/
	
	ser->print(F("+++"));
	lastDataWrite = millis();
	return PopCommandResult(500);
}

int Sim900::SwitchToCommandModeDropData()
{
	parser.SetCommandType(AT_SWITH_TO_COMMAND);
		ser->flush();
	while(ser->available())
		ser->read();
	delay(1500);
	while(ser->available())
		ser->read();

	ser->print(F("+++"));

	return PopCommandResult(500);
}

/*
Switches to data mode ATO
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
int Sim900::SwitchToDataMode()
{
	parser.SetCommandType(AT_SWITCH_TO_DATA);

	ser->println(F("ATO"));

	int result = PopCommandResult(AT_DEFAULT_TIMEOUT);
	if(result == S900_OK)
	{
		delay(100);
		while(ser->available())
		{
			int c = ser->read();
			//pr("\nsd_data: %c\n", (char)c);
			//ds.print("s_data: "); ds.println((int)c);
			WriteDataBuffer(c);
		}
		
	}
	lastDataWrite = millis();
	return result;
}
/*
Interets incoming data from serial port as results
Return values S900_OK, S900_ERROR, S900_TIMEOUT, specific function return values, specific errors
*/
int Sim900::PopCommandResult( int timeout )
{
	unsigned long start = millis();
	while(parser.commandReady == false && (millis()-start) < (unsigned long)timeout)
	{
		if(ser->available())
		{
			char c = ser->read();
			parser.FeedChar(c);
		}
	}

	int commandResult = parser.lastResult;
	if (commandResult == S900_NONE)
		commandResult = S900_TIMEOUT;
	parser.SetCommandType(0);
	
	return commandResult;
}
/*
Disables/enables echo on serial port
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
int Sim900::SetEcho( bool echoEnabled )
{
	parser.SetCommandType(AT_DEFAULT);
	if(echoEnabled)
		ser->println(F("ATE1"));
	else
		ser->println(F("ATE0"));

	int r = PopCommandResult(AT_DEFAULT_TIMEOUT);
	delay(100); // without 100ms wait, next command failed, idk wky
	return r;
}
/*
Set sim900 to use transparent mode
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
int Sim900::SetTransparentMode( bool transparentMode )
{
	parser.SetCommandType(AT_DEFAULT);
	
	if(transparentMode)
		ser->println(F("AT+CIPMODE=1"));
	else
		ser->println(F("AT+CIPMODE=0"));
		
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}
/*
Set apn details
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
int Sim900::SetApn(const char *apnName, const char *username,const char *password )
{
	parser.SetCommandType(AT_DEFAULT);
	ser->print(F("AT+CSTT=\"")); ser->print(apnName); ser->print(F("\",\"")); ser->print(username); ser->print(F("\",\"")); ser->print(password); ser->print(F("\"\r\n"));
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}
/*
Execute At command
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
int Sim900::ExecuteCommand_P( const __FlashStringHelper* command )
{
	parser.SetCommandType(AT_DEFAULT);
	ser->println(command);
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}


void Sim900::DataWrite( const __FlashStringHelper* data )
{
	ser->print(data);
	lastDataWrite = millis();
}

void Sim900::DataWrite( char* data )
{
	ser->print(data);
	lastDataWrite = millis();
}

void Sim900::DataWrite( char *data, int length )
{
	ser->write((unsigned char*)data, length);
	lastDataWrite = millis();
}

void Sim900::DataWrite( char c )
{
	ser->write(c);
	lastDataWrite = millis();
}


void Sim900::DataEndl()
{
	ser->print(F("\r\n"));
	ser->flush();
	lastDataWrite = millis();
}
/* makes sure modem is enabled */
/* well this function may take forever */
int Sim900::TurnOn()
{
	// while(!IsPoweredUp())
//	 {
		 ds.println(F("Timeout, trying to turn on"));
		 
		 pinMode(9, OUTPUT);
		 digitalWrite(9,LOW);
		 delay(1000);
		 digitalWrite(9,HIGH);
		 delay(2000);
		 digitalWrite(9,LOW);
		 delay(3000);
//	 }
	 
	 return S900_OK;
}

int Sim900::SetBaudRate(uint32_t baud)
{
	parser.SetCommandType(AT_DEFAULT);
	ser->print(F("AT+IPR="));  ser->println(baud);
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}
int Sim900::GetIMEI()
{
	parser.SetCommandType(AT_GSN);

	ser->println(F("AT+GSN"));

	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}

bool Sim900::IsPoweredUp()
{
	return GetIMEI() == S900_OK;
	
	//int result = S900_TIMEOUT;
//
	//parser.SetCommandType(AT_DEFAULT);
	//ser->println(F("AT+GSN"));
	//result = PopCommandResult(400);
//
//
	//if(result == S900_TIMEOUT)
		//return false;
	//return true;
}

void Sim900::wait(int ms)
{
	unsigned long start = millis();
	while((millis()-start) <= (unsigned long)ms)
		if(ser->available())
			parser.FeedChar(ser->read());
}

int Sim900::ExecuteFunction(FunctionBase &function)
{
	parser.SetCommandType(&function);
	ser->println(function.getCommand());
	
	int initialResult = PopCommandResult(function.functionTimeout);
	
	if(initialResult == S900_OK)
		return initialResult;
	if(function.GetInitSequence() == NULL)
		return initialResult;
		
	char *p= (char*)function.GetInitSequence();
	while(pgm_read_byte(p) != 0)
	{
			
	//	ds.print(F("Exec: "));
			
		//printf("Exec: %s\n", p);
		ds.println((__FlashStringHelper*)p);
		delay(100);
		int r = ExecuteCommand_P((__FlashStringHelper*)p);
		
		if(r < 0)
		{
				
			ds.println(F("Fail"));
			return r;
		}
		ds.println(" __Fin");		

		while(pgm_read_byte(p) != 0)			
			p++;
			
		p++;
	}
	delay(500);
	parser.SetCommandType(&function);
	ser->println(function.getCommand());
	return PopCommandResult(function.functionTimeout);
}

int Sim900::SendSms(char *number, char *message)
{
	parser.SetCommandType(AT_DEFAULT);
	ser->print(F("AT+CMGS=\""));
	ser->print(number);
	ser->println(F("\""));
	
	while(ser->available() == false && ser->read() != '>');
	
	ser->println(message);
	ser->println(F("\x2a"));
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}
int Sim900::UnwriteDataBuffer()
{
	if (dataBufferTail == 0)
		dataBufferTail = DATA_BUFFER_SIZE - 1;
	else
		dataBufferTail--;
	return _dataBuffer[dataBufferTail];
}

void Sim900::WriteDataBuffer(char c)
{
	int tmp = dataBufferTail+1;
	if(tmp == DATA_BUFFER_SIZE)
		tmp = 0;
	if(tmp == dataBufferHead)
	{
		ds.println(F("Buffer overflow"));
		return;
	}
	//ds.print(F("Written ")); this->PrintDataByte(c);
	_dataBuffer[dataBufferTail] = c;
	dataBufferTail = tmp;
}

int Sim900::ReadDataBuffer()
{
	if(dataBufferHead != dataBufferTail)
	{
		int ret= _dataBuffer[dataBufferHead];
		dataBufferHead++;
		if(dataBufferHead==DATA_BUFFER_SIZE)
			dataBufferHead = 0;
		return ret;
	}
	return -1;
}

int Sim900::Cipshut()
{
	parser.SetCommandType(AT_CIPSHUT);
	ser->println(F("AT+CIPSHUT"));
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}

void Sim900::DataWriteNumber(int c)
{
	ser->print(c);
	lastDataWrite = millis();
}
void Sim900::DataWriteNumber(uint16_t c)
{
	ser->print(c);
	lastDataWrite = millis();
}

void Sim900::data_printf(const __FlashStringHelper *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf_P(&dataStream, (const char*)fmt, args);
	va_end(args);
	lastDataWrite = millis();
}

int Sim900::Call(char *number)
{
	parser.SetCommandType(AT_DEFAULT);
	ser->print(F("ATD"));
	ser->print(number);
	ser->println(F(";"));
		
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}

void Sim900::Shutdown()
{
	ser->println(F("AT+CPOWD=0"));
}







































