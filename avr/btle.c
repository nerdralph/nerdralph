/* bluetooth low energy beacon using nrf24l01 modules
 * modeled after:
 * http://dmitry.gr/index.php?r=05.Projects&proj=11.%20Bluetooth%20LE%20fakery
 * https://github.com/floe/BTLE
 */

// #include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define cbi(x,y)    x&= ~(1<<y)
#define sbi(x,y)    x|= (1<<y)

#define DEBUG_PIN 0
#define PIN_CE	1
#define PIN_SCK	5
#define PIN_MI (PIN_SCK - 1)	
#define PIN_MO (PIN_MI - 1)	
#define PIN_nCS	(PIN_MO - 1)	
#define NRF_PORT PORTB
#define NRF_DDR DDRB


#define MY_MAC_0	0xEF
#define MY_MAC_1	0xFF
#define MY_MAC_2	0xC0
#define MY_MAC_3	0xAA
#define MY_MAC_4	0x18
#define MY_MAC_5	0x01


void btLeCrc(const uint8_t* data, uint8_t len, uint8_t* dst){

	uint8_t v, t, d;

	while(len--){
	
		d = *data++;
		for(v = 0; v < 8; v++, d >>= 1){
		
			t = dst[0] >> 7;
			
			dst[0] <<= 1;
			if(dst[1] & 0x80) dst[0] |= 1;
			dst[1] <<= 1;
			if(dst[2] & 0x80) dst[1] |= 1;
			dst[2] <<= 1;
			
		
			if(t != (d & 1)){
			
				dst[2] ^= 0x5B;
				dst[1] ^= 0x06;
			}
		}	
	}
}


uint8_t  avr_swap(uint8_t a){
    return __builtin_avr_insert_bits (0x01234567, a, 0);
}

uint8_t  swapbits(uint8_t a){

	uint8_t v = 0;
	
	if(a & 0x80) v |= 0x01;
	if(a & 0x40) v |= 0x02;
	if(a & 0x20) v |= 0x04;
	if(a & 0x10) v |= 0x08;
	if(a & 0x08) v |= 0x10;
	if(a & 0x04) v |= 0x20;
	if(a & 0x02) v |= 0x40;
	if(a & 0x01) v |= 0x80;

	return v;
}

void btLeWhiten(uint8_t* data, uint8_t len, uint8_t whitenCoeff){

	uint8_t  m;
	
	while(len--){
	
		for(m = 1; m; m <<= 1){
		
			if(whitenCoeff & 0x80){
				
				whitenCoeff ^= 0x11;
				(*data) ^= m;
			}
			whitenCoeff <<= 1;
		}
		data++;
	}
}

static inline uint8_t btLeWhitenStart(uint8_t chan){
	//the value we actually use is what BT'd use left shifted one...makes our life easier

	return swapbits(chan) | 2;	
}

void btLePacketEncode(uint8_t* packet, uint8_t len, uint8_t chan){
	//length is of packet, including crc. pre-populate crc in packet with initial crc value!

	uint8_t i, dataLen = len - 3;
	
	btLeCrc(packet, dataLen, packet + dataLen);
	for(i = 0; i < 3; i++, dataLen++) packet[dataLen] = swapbits(packet[dataLen]);
	btLeWhiten(packet, len, btLeWhitenStart(chan));
	for(i = 0; i < len; i++) packet[i] = swapbits(packet[i]);
	
}

uint8_t spi_byte(uint8_t byte){

	uint8_t i = 8;
	
	do{
		NRF_PORT &=~ (1 << PIN_MO);
		if(byte & 0x80) NRF_PORT |= (1 << PIN_MO);
		NRF_PORT |= (1 << PIN_SCK);
		byte <<= 1;
        // uncomment for full-duplex SPI
		// if(NRF_PORT & PIN_MI) byte++;
		NRF_PORT &= ~(1 << PIN_SCK);
	
	}while(--i);

	return byte;
}

void nrf_cmd(uint8_t cmd, uint8_t data)
{
	cbi(NRF_PORT, PIN_nCS);
	spi_byte(cmd);
	spi_byte(data);
	sbi(NRF_PORT, PIN_nCS); //Deselect chip
}

// just call cmd(cmd, NOP) instead
void nrf_simplebyte(uint8_t cmd)
{
	cbi(NRF_PORT, PIN_nCS);
	spi_byte(cmd);
	sbi(NRF_PORT, PIN_nCS);
}

void nrf_manybytes(uint8_t* data, uint8_t len){

	cbi(NRF_PORT, PIN_nCS);
	do{
	
		spi_byte(*data++);
	
	}while(--len);
	sbi(NRF_PORT, PIN_nCS);
}

void SPI_init (void)
{
	sbi(NRF_PORT, PIN_nCS);
    NRF_DDR = (1<<PIN_nCS) | (1<<PIN_CE) | (1<<PIN_SCK) | (1<<PIN_MO) \
        | (1<<DEBUG_PIN);
	//TCCR0B = (1<<CS00);
	//MCUCR = (1<<SM1)|(1<<SE);
	//sei();
}


void main() __attribute__ ((noreturn));
void main (void)
{
	static const uint8_t chRf[] = {2, 26,80};
	static const uint8_t chLe[] = {37,38,39};
	uint8_t i, L, ch = 0;
    // initialize buffer with set tx address command
    // address 8E 89 BE D6 bit-reversed
    // static rather than local saves 18 bytes
	static uint8_t buf[32]; //= {0x30, 0x6B, 0x7D, 0x91, 0x71};
	
	SPI_init();

	nrf_cmd(0x20, 0x12);	//on, no crc, int on RX/TX done
	_delay_ms(2);           // Tpd2stby
	//nrf_cmd(0x22, 0x00);	//no RX
	nrf_cmd(0x21, 0x00);	//no auto-acknowledge
	//nrf_cmd(0x22, 0x01);	//RX on pipe 0
	nrf_cmd(0x23, 0x02);	//4-byte address
	nrf_cmd(0x24, 0x00);	//no auto-retransmit
	//nrf_cmd(0x26, 0x06);	//1MBps at 0dBm
	nrf_cmd(0x26, 0x00);	//1MBps at -18dBm
	//nrf_cmd(0x27, 0x3E);	//clear various flags
	//nrf_cmd(0x31, 32);	    //always RX 32 bytes
	
	buf[0] = 0x30;			//set tx address
	buf[1] = swapbits(0x8E);
	buf[2] = swapbits(0x89);
	buf[3] = swapbits(0xBE);
	buf[4] = swapbits(0xD6);
	nrf_manybytes(buf, 5);
	//buf[0] = 0x2A;        // rx address P0
	//nrf_manybytes(buf, 5);
	
	
	while(1){
		if(++ch == sizeof(chRf)){
            ch = 0;
            sbi(NRF_PORT, DEBUG_PIN);
		    _delay_ms(109); // wait between advertisements
            cbi(NRF_PORT, DEBUG_PIN);
	    }

		L = 0;
		
		buf[L++] = 0x02;	//PDU type, given address is random
        buf[L++] = 11;      //11 + 6 = 17 bytes of payload
		
		buf[L++] = MY_MAC_0;
		buf[L++] = MY_MAC_1;
		buf[L++] = MY_MAC_2;
		buf[L++] = MY_MAC_3;
		buf[L++] = MY_MAC_4;
		buf[L++] = MY_MAC_5;
		
		buf[L++] = 2;		//flags (LE-only, limited discovery mode)
		buf[L++] = 0x01;
		buf[L++] = 0x05;
		
		buf[L++] = 7;
		buf[L++] = 0x08;
		buf[L++] = 'n';
		buf[L++] = 'R';
		buf[L++] = 'F';
		buf[L++] = ' ';
		buf[L++] = 'L';
		buf[L++] = 'E';
		
		buf[L++] = 0x55;	//CRC start value: 0x555555
		buf[L++] = 0x55;
		buf[L++] = 0x55;
		
		btLePacketEncode(buf, L, chLe[ch]);
		
		nrf_cmd(0x25, chRf[ch]);
		nrf_cmd(0x27, 0x6E);	//clear flags

		//nrf_simplebyte(0xE2); //Clear RX Fifo
		//nrf_simplebyte(0xE1); //Clear TX Fifo
	
		cbi(NRF_PORT, PIN_nCS);
		spi_byte(0xA0);
		for(i = 0 ; i < L ; i++) spi_byte(buf[i]);
		sbi(NRF_PORT, PIN_nCS);
	
		//nrf_cmd(0x20, 0x12);	//tx on
        // no CE pulse required when CE tied high 
		sbi(NRF_PORT, PIN_CE);	 //do tx
		_delay_us(15);
		cbi(NRF_PORT, PIN_CE);

        // only delay for debugging - i.e. check IRQ trigger
		//_delay_ms(1);           // let tx finish
	}
	
}
