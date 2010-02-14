#include "WProgram.h"

#include <util/delay.h>

#include "DataPacketStructs.hpp"
#include "ArduinoCmds.hpp"

#include "serial_comm_helper.hpp"

#include "diffenc.hpp"
#include "pindefs.hpp"

extern "C" void __cxa_pure_virtual()
{
	for(;;)
	{

	}
}

void genTimestamp(long * sec, long * usec)
{
	//*sec =  global_time_sec + (millis()/1000) - arduino_time_millis/1000;
	//*usec = *sec - (global_time_usec + millis()*1000 - arduino_time_millis*1000);

	*sec = 0;
	*usec = 0;
}

int main()
{	
	init();
	Serial.begin(57600);

	pinMode(left_encoder_pin_A, INPUT);
	pinMode(left_encoder_pin_B, INPUT);

	pinMode(right_encoder_pin_A, INPUT);
	pinMode(right_encoder_pin_B, INPUT);

	attachInterrupt(0, leftenc_event_A, CHANGE);
	attachInterrupt(1, leftenc_event_B, CHANGE);

	//attachInterrupt(2, rightenc_event_A, CHANGE);
	//attachInterrupt(3, rightenc_event_B, CHANGE);

	int32_t tx_num = 0;

	left_ticks = 0;
	right_ticks = 0;

	for(;;)
	{		
		if(!(Serial.available() >= PACKET_HEADER_SIZE))
		{
			continue;
		}

		header_t header;
		byte* indata = NULL;

		if(!serialReadBytesTimeout(PACKET_HEADER_SIZE, (byte*)&header))
		{
			continue;
		}

		if(header.size > 0)
		{
			indata = (byte*)malloc(header.size);
			if(!serialReadBytesTimeout(header.size, indata))
			{
				free(indata);
				indata = NULL;
				continue;
			}
		}

		switch(header.cmd)
		{
			case ARDUINO_RESET:
			{
				//Serial.println("Reset");
				break;
			}
			case ARDUINO_SET_CLOCK:
			{
				//Serial.println("Set Clock");
				break;
			}
			case ARDUINO_GET_ID:
			{
				header_t headerOut;
				genTimestamp(&headerOut.timestamp_sec, &headerOut.timestamp_usec);

				headerOut.packetnum = tx_num;
				headerOut.cmd = ARDUINO_GET_ID;
				headerOut.size = 1;
				char msg = ENCODER_IF_BOARD;

				Serial.write((uint8_t*)&headerOut, PACKET_HEADER_SIZE);
				Serial.print(msg);
				tx_num++;
				break;
			}
			case ENCODER_GET_READING:
			{
				header_t headerOut;
				genTimestamp(&headerOut.timestamp_sec, &headerOut.timestamp_usec);

				headerOut.packetnum = tx_num;
				headerOut.cmd = ENCODER_GET_READING;
				headerOut.size = sizeof(new_encoder_pk_t);

				new_encoder_pk_t body;

				int64_t first_left = left_ticks;
				int64_t first_right = right_ticks;
				//delay(5);
				_delay_ms(5);
				body.pl = left_ticks;
				body.pr = right_ticks;

				body.dl = left_ticks - first_left;
				body.dr = right_ticks - first_right;

				uint8_t* msg = (uint8_t*)&(body);
				Serial.write((uint8_t*)&headerOut, PACKET_HEADER_SIZE);
				Serial.write(msg, sizeof(new_encoder_pk_t));
				tx_num++;
				break;
			}
			case ENCOER_RESET_COUNT:
			{
				header_t headerOut;
				genTimestamp(&headerOut.timestamp_sec, &headerOut.timestamp_usec);

				headerOut.packetnum = tx_num;
				headerOut.cmd = ENCOER_RESET_COUNT;
				headerOut.size = 0;

				left_ticks = 0;
				right_ticks = 0;

				Serial.write((uint8_t*)&headerOut, PACKET_HEADER_SIZE);
				tx_num++;
				break;
			}
			case ARDUINO_HALT_CATCH_FIRE:
			default:
			{
				//HCF();
				break;
			}
		}//end switch

		if(header.size > 0)
		{
			free(indata);
			indata = NULL;
		}
	}//end for

	return 0;
}//end main
