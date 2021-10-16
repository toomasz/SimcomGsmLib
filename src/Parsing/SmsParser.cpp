#include "SmsParser.h"

char hex2dec(char c)
{
	if (c >= '0' && c <= '9') return c - '0';	
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;	
	if (c >= 'a' && c <= 'f') return c - 'A' + 10;	
	return 0;
}

void hex2bin(const char* input, int inputLen, char* output)
{
	for (int i = 0; i < inputLen; i+=2)	
		output[i / 2] = 16 * hex2dec(input[i]) + hex2dec(input[i + 1]);	
}

unsigned char swap_decimal_nibble(const unsigned char x)
{
	return (x / 16) + ((x % 16) * 10);
}

int decode_pdu_message(const unsigned char* buffer, int buffer_length, char* output_sms_text, int sms_text_length)
{
	int output_text_length = 0;
	if (buffer_length > 0)
		output_sms_text[output_text_length++] = BITMASK_7BITS & buffer[0];

	int carry_on_bits = 1;
	int i = 1;
	for (; i < buffer_length; ++i)
	{

		output_sms_text[output_text_length++] = BITMASK_7BITS & ((buffer[i] << carry_on_bits) | (buffer[i - 1] >> (8 - carry_on_bits)));

		if (output_text_length == sms_text_length) break;

		carry_on_bits++;

		if (carry_on_bits == 8) 
		{
			carry_on_bits = 1;
			output_sms_text[output_text_length++] = buffer[i] & BITMASK_7BITS;
			if (output_text_length == sms_text_length) break;
		}

	}
	if (output_text_length < sms_text_length)  // Add last remainder.
		output_sms_text[output_text_length++] = buffer[i - 1] >> (8 - carry_on_bits);

	return output_text_length;
}

int decode_phone_number(const unsigned char* buffer, int phone_number_length, char* output_phone_number)
{
	int i = 0;
	for (; i < phone_number_length; ++i) 
	{
		if (i % 2 == 0)
			output_phone_number[i] = (buffer[i / 2] & BITMASK_LOW_4BITS) + '0';
		else
			output_phone_number[i] = ((buffer[i / 2] & BITMASK_HIGH_4BITS) >> 4) + '0';
	}
	output_phone_number[phone_number_length] = '\0';  // Terminate C string.
	return phone_number_length;
}

int pdu_decode_bin(const unsigned char* buffer, int buffer_length,
	time_t* output_sms_time,
	char* output_sender_phone_number, int sender_phone_number_size,
	char* output_sms_text, int sms_text_size)
{

	if (buffer_length <= 0)
		return -1;

	const int sms_deliver_start = 1 + buffer[0];
	if (sms_deliver_start + 1 > buffer_length) return -1;
	if ((buffer[sms_deliver_start] & SMS_DELIVER_ONE_MESSAGE) != SMS_DELIVER_ONE_MESSAGE) return -1;

	const int sender_number_length = buffer[sms_deliver_start + 1];
	if (sender_number_length + 1 > sender_phone_number_size) return -1;

	decode_phone_number(buffer + sms_deliver_start + 3, sender_number_length, output_sender_phone_number);

	const int sms_pid_start = sms_deliver_start + 3 + (buffer[sms_deliver_start + 1] + 1) / 2;

	// Decode timestamp.
	struct tm sms_broken_time;
	sms_broken_time.tm_year = 100 + swap_decimal_nibble(buffer[sms_pid_start + 2]);
	sms_broken_time.tm_mon = swap_decimal_nibble(buffer[sms_pid_start + 3]) - 1;
	sms_broken_time.tm_mday = swap_decimal_nibble(buffer[sms_pid_start + 4]);
	sms_broken_time.tm_hour = swap_decimal_nibble(buffer[sms_pid_start + 5]);
	sms_broken_time.tm_min = swap_decimal_nibble(buffer[sms_pid_start + 6]);
	sms_broken_time.tm_sec = swap_decimal_nibble(buffer[sms_pid_start + 7]);
	const char gmt_offset = swap_decimal_nibble(buffer[sms_pid_start + 8]);
	// GMT offset is expressed in 15 minutes increments.
	(*output_sms_time) = mktime(&sms_broken_time) - gmt_offset * 15 * 60;

	const int sms_start = sms_pid_start + 2 + 7;
	if (sms_start + 1 > buffer_length) return -1;

	const int output_sms_text_length = buffer[sms_start];
	if (sms_text_size < output_sms_text_length) return -1; 

	const int decoded_sms_text_size = decode_pdu_message(buffer + sms_start + 1, buffer_length - (sms_start + 1),
		output_sms_text, output_sms_text_length);

	if (decoded_sms_text_size != output_sms_text_length) return -1; 

	if (output_sms_text_length < sms_text_size)
		output_sms_text[output_sms_text_length] = 0;
	else
		output_sms_text[sms_text_size - 1] = 0;

	return output_sms_text_length;
}