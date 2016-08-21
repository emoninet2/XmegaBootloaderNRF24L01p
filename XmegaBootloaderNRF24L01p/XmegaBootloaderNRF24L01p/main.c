/*
 * XmegaBootloaderNRF24L01p.c
 *
 * Created: 18-Aug-16 4:49:56 PM
 * Author : Emon

 In the application code, to enter bootloader, use the code below

 #define PROGRAM_ENTERS_BOOTLOADER


 #if defined(PROGRAM_ENTERS_BOOTLOADER)
	cli();
	CCPWrite(&(PMIC.CTRL), PMIC_IVSEL_bm );
	soft_reset();
 #else
	 uint8_t jmpE = (uint8_t)((BOOT_SECTION_START/2)>>16);
	 uint8_t jmpH = (uint8_t)((BOOT_SECTION_START/2)>>8);
	 uint8_t jmpL = (uint8_t)((BOOT_SECTION_START/2)>>0);

	 asm("ldi ZH, %0"::"i" (jmpH):"r30","r31");
	 asm("ldi ZL, %0"::"i" (jmpL):"r30","r31");
	 EIND = jmpE;
	 asm("eijmp");
 #endif

 */ 

#include "main.h"
#include "time.h"
#include "main.h"
#include "xmega_drivers.h"
#include "Component_Drivers/components.h"
#include "XMEGA_API/xmega_api.h"
#include "XMEGA_SERVICES/xmega_services.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

DigitalPin_t led1 = {&PORTR, 0};
DigitalPin_t led2 = {&PORTR, 1};
DigitalPin_t powLedR = {&PORTD, 4};
DigitalPin_t powLedG = {&PORTD, 5};

DigitalPin_t myswitch1 = {&PORTF, 1};
DigitalPin_t myswitch2 = {&PORTF, 2};


//#define  PROGRAM_ENTERS_BOOTLOADER

#define soft_reset()        \
do                          \
{                           \
	wdt_enable(WDTO_15MS);  \
	for(;;)                 \
	{                       \
	}                       \
} while(0)

volatile bool nrf_led_flag  =0;

volatile uint64_t nrfTXaddr  = 0xAABBCCDD01;

// void *command_handler(char **args,int arg_count){
// 
// 	if(!strcmp(args[0], "tst1") ) {
// 		DigitalPin_SetValue(&led);
// 	}
// 	else if(!strcmp(args[0], "tst2") ) {
// 		DigitalPin_ClearValue(&led);
// 	}
// 	
// 	else if(!strcmp(args[0], "rst") ) {
// 		if(!strcmp(args[1], "app") ){
// 
//  			CCPWrite(&(PMIC.CTRL), 0 );
// 
// 			asm("ldi r30, 0");
// 			asm("ldi r31, 0");
// 			asm("IJMP");
// 		}
// 		else if(!strcmp(args[1], "boot") ){
// 			CCPWrite(&(PMIC.CTRL), 0 );
// 			soft_reset();
// 		}
// 	}
// 	else{
// 
// 	}
// 
// 	return 0;
//
void command_parse_execute(char *command){

	int arg_index = 0;
	char *pch;
	char *remotch_args[ 10];
	pch = strtok(command, " ,");
	while(pch != NULL) {
		remotch_args[arg_index] = pch;
		arg_index++;
		if(arg_index >=10) break;
		pch = strtok (NULL, " ,");
	}
	//command_handler(remotch_args,arg_index);
}

uint64_tu8tou64(uint8_t const u8[static 8]){
	uint64_t u64;
	memcpy(&u64, u8, sizeof u64);
	return u64;
}




void package_handler(uint8_t *command){

	if (command[0]=='a'){
		uint8_t *pt = &nrfTXaddr;
		*pt++ = (command[5]);
		*pt++ = (command[4]);
		*pt++ = (command[3]);
		*pt++ = (command[2]);
		*pt = (command[1]);
	}
	else if(command[0]=='x'){//led ON
		DigitalPin_SetValue(&led1);
		char str[20];
		sprintf(str,"light is off\r\n");
		_nrf24l01p_send_to_address(nrfTXaddr, str, strlen(str));
	}
	else if (command[0]=='y'){//led OFF
		DigitalPin_ClearValue(&led1);
		char str[20];
		sprintf(str,"light is on\r\n");
		_nrf24l01p_send_to_address(nrfTXaddr, str, strlen(str));
	}

	else if (command[0]=='s'){//led OFF
		EEPROM_WriteByte( command[1], command[2], command[3] );
	}

	else if (command[0]=='t'){//led OFF
		
		uint8_t val = EEPROM_ReadByte(  command[1], command[2] );
		char str[20];
		sprintf(str,"EEPROM value : %x\r\n",val);
		_nrf24l01p_send_to_address(nrfTXaddr, str, strlen(str));
	}
	else if (command[0]=='u'){//led OFF
		EEPROM_EraseAll();
	}
	else if (command[0]=='q'){
 		
#if defined(PROGRAM_ENTERS_BOOTLOADER)
		CCPWrite(&(PMIC.CTRL), 0 );
		asm("ldi r30, 0");
		asm("ldi r31, 0");
		EIND = 0;
		asm("eijmp");
#else
		CCPWrite(&(PMIC.CTRL), 0 );
		soft_reset();
#endif
	}
	else if (command[0]=='Q'){
		soft_reset();
	}
	else if (command[0]=='r'){//read byte
		uint32_t address;

		uint8_t *pt = &address;
		*pt++ = (command[4]);
		*pt++ = (command[3]);
		*pt++ = (command[2]);
		*pt = (command[1]);

		uint8_t data;
		data =  SP_ReadByte(address);
		char str[20];
		sprintf(str,"the data is %x\r\n",data);
		_nrf24l01p_send_to_address(nrfTXaddr, str, strlen(str));
 	}
	else if (command[0]=='R'){//read word
		uint32_t address;

		uint8_t *pt = &address;
		*pt++ = (command[4]);
		*pt++ = (command[3]);
		*pt++ = (command[2]);
		*pt = (command[1]);

		uint16_t data;
		data =  SP_ReadWord(address);
		_nrf24l01p_send_to_address(nrfTXaddr, &data, sizeof(data));
	}
	else if (command[0]=='L'){
		uint32_t address;

		uint8_t *pt = &address;
		*pt++ = (command[6]);
		*pt++ = (command[5]);
		*pt++ = (command[4]);
		*pt = (command[3]);

		uint16_t data;

		uint8_t *ptd = &data;
		*ptd++ = (command[2]);
		*ptd = (command[1]);

		SP_LoadFlashWord(address, data);
	}
	else if (command[0]=='C'){
		SP_EraseFlashBuffer();
	}
	else if (command[0]=='E'){//erase application section
		SP_EraseApplicationSections();
	}
	else if (command[0]=='e'){
		uint32_t address;

		uint8_t *pt = &address;
		*pt++ = (command[6]);
		*pt++ = (command[5]);
		*pt++ = (command[4]);
		*pt = (command[3]);

		SP_EraseApplicationPage( address);
	}
	else if (command[0]=='m'){
		uint32_t address;

		uint8_t *pt = &address;
		*pt++ = (command[6]);
		*pt++ = (command[5]);
		*pt++ = (command[4]);
		*pt = (command[3]);

		SP_WriteApplicationPage(address);
	}
	else if (command[0]=='M'){
		uint32_t address;

		uint8_t *pt = &address;
		*pt++ = (command[6]);
		*pt++ = (command[5]);
		*pt++ = (command[4]);
		*pt = (command[3]);
		SP_EraseWriteApplicationPage(address);
	}
	else if (command[0]=='d'){
		uint32_t address;

		uint8_t *pt = &address;
		*pt++ = (command[6]);
		*pt++ = (command[5]);
		*pt++ = (command[4]);
		*pt = (command[3]);
		SP_EraseBootPage(address);
	}
	else if (command[0]=='n'){
		uint32_t address;

		uint8_t *pt = &address;
		*pt++ = (command[6]);
		*pt++ = (command[5]);
		*pt++ = (command[4]);
		*pt = (command[3]);
		SP_WriteBootPage(address);
	}
	else if (command[0]=='N'){
		uint32_t address;

		uint8_t *pt = &address;
		*pt++ = (command[6]);
		*pt++ = (command[5]);
		*pt++ = (command[4]);
		*pt = (command[3]);
		SP_EraseWriteBootPage(address);
	}

	else{

	}
}

uint8_t myflashpagedata[512];
uint8_t myflashpagedata2[512];

volatile bool _boot_mode =0;


int main(void)
{


	cli();

	CLKSYS_Enable( OSC_RC32MEN_bm );
	do {} while ( CLKSYS_IsReady( OSC_RC32MRDY_bm ) == 0 );
	CLKSYS_Main_ClockSource_Select( CLK_SCLKSEL_RC32M_gc );
	CLKSYS_Disable( OSC_RC2MEN_bm );



	for(int i=0;i<512;i++){
		myflashpagedata[i] = i;
	}


	CCPWrite(&(PMIC.CTRL), PMIC_IVSEL_bm );

	DigitalPin_SetValue(&led1);
	DigitalPin_SetValue(&led2);
	DigitalPin_SetDir(&led1,1);
	DigitalPin_SetDir(&led2,1);
	DigitalPin_SetDir(&powLedR,1);
	DigitalPin_SetDir(&powLedG,1);

	//DigitalPin_SetValue(&powLedR);//off
	DigitalPin_ClearValue(&powLedR);//on
	DigitalPin_SetValue(&powLedG);//on
	//DigitalPin_ClearValue(&powLedG);//off


	nrfTXaddr = 0xAABBCCDD01;


	_nrf24l01p_init();

	//LEFT
	_nrf24l01p_set_RX_pipe_address(_NRF24L01P_PIPE_P1, 0x4C4C4C4C31);
	_nrf24l01p_set_RX_pipe_address(_NRF24L01P_PIPE_P2, 0x4C4C4C4C32);
	_nrf24l01p_set_RX_pipe_address(_NRF24L01P_PIPE_P3, 0x4C4C4C4C33);
	_nrf24l01p_set_RX_pipe_address(_NRF24L01P_PIPE_P4, 0x4C4C4C4C34);
	_nrf24l01p_set_RX_pipe_address(_NRF24L01P_PIPE_P5, 0x4C4C4C4C35);
	char rxData[35];
	bool package_handler_signal = 0;

	while(1){



		if((_nrf24l01p_readable(_NRF24L01P_PIPE_P1))){
			asm("nop");
			nrf_led_flag = 1;
			int width = _nrf24l01p_read_dyn_pld(_NRF24L01P_PIPE_P1, (uint8_t*) rxData);
			rxData[width] = '\0';
			package_handler_signal = 1;
			
		}
		
		if(package_handler_signal){
			_delay_ms(100);
			package_handler(rxData);
			package_handler_signal = 0;
		}


	}


}

