#include <time.h>

enum 
{
	BITMASK_7BITS = 0x7F,
	BITMASK_8BITS = 0xFF,
	BITMASK_HIGH_4BITS = 0xF0,
	BITMASK_LOW_4BITS = 0x0F,

	TYPE_OF_ADDRESS_INTERNATIONAL_PHONE = 0x91,
	TYPE_OF_ADDRESS_NATIONAL_SUBSCRIBER = 0xC8,

	SMS_DELIVER_ONE_MESSAGE = 0x04,
	SMS_SUBMIT = 0x11,

	SMS_MAX_7BIT_TEXT_LENGTH = 160,
};

char hex2dec(char c);

void hex2bin(const char* input, int inputLen, char* output);


unsigned char swap_decimal_nibble(const unsigned char x);

int decode_pdu_message(const unsigned char* buffer, int buffer_length, char* output_sms_text, int sms_text_length);

int decode_phone_number(const unsigned char* buffer, int phone_number_length, char* output_phone_number);

int pdu_decode_bin(const unsigned char* buffer, int buffer_length,
	time_t* output_sms_time,
	char* output_sender_phone_number, int sender_phone_number_size,
	char* output_sms_text, int sms_text_size);