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

DigitalPin_t led = {&PORTR, 0};
DigitalPin_t led2 = {&PORTR, 1};


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



void *command_handler(char **args,int arg_count){

	if(!strcmp(args[0], "tst1") ) {
		DigitalPin_SetValue(&led);
	}
	else if(!strcmp(args[0], "tst2") ) {
		DigitalPin_ClearValue(&led);
	}
	
	else if(!strcmp(args[0], "rst") ) {
		if(!strcmp(args[1], "app") ){

 			CCPWrite(&(PMIC.CTRL), 0 );

			asm("ldi r30, 0");
			asm("ldi r31, 0");
			asm("IJMP");
		}
		else if(!strcmp(args[1], "boot") ){
			CCPWrite(&(PMIC.CTRL), 0 );
			soft_reset();
		}
	}
	else{

	}

	return 0;
}

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
	command_handler(remotch_args,arg_index);
}


void package_handler(uint8_t *command){

	if(command[0]=='x'){
		DigitalPin_SetValue(&led);
	}
	else if (command[0]=='y'){
		DigitalPin_ClearValue(&led);
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
		uint32_t address = (command[1]<<24) | (command[2]<<16) |  (command[3]<<8)  | command[4] ;
		uint8_t data;
		data =  SP_ReadByte(address);
 	}
	else if (command[0]=='R'){//read word
		uint32_t address = (command[1]<<24) | (command[2]<<16) |  (command[3]<<8)  | command[4] ;
		uint16_t data;
		data =  SP_ReadWord(address);
	}
	else if (command[0]=='L'){
		uint32_t address = (command[3]<<24) | (command[4]<<16) |  (command[5]<<8)  | command[6] ;
		uint16_t data = (command[1]<<24) | (command[2]<<16);
		SP_LoadFlashWord(address, data);
	}
	else if (command[0]=='C'){
		SP_EraseFlashBuffer();
	}
	else if (command[0]=='E'){
		SP_EraseApplicationSections();
	}
	else if (command[0]=='e'){
		uint32_t address = (command[1]<<24) | (command[2]<<16) |  (command[3]<<8)  | command[4] ;
		SP_EraseApplicationPage( address);
	}
	else if (command[0]=='m'){
		uint32_t address = (command[1]<<24) | (command[2]<<16) |  (command[3]<<8)  | command[4] ;
		SP_WriteApplicationPage(address);
	}
	else if (command[0]=='M'){
		uint32_t address = (command[1]<<24) | (command[2]<<16) |  (command[3]<<8)  | command[4] ;
		SP_EraseWriteApplicationPage(address);
	}
	else if (command[0]=='d'){
		uint32_t address = (command[1]<<24) | (command[2]<<16) |  (command[3]<<8)  | command[4] ;
		SP_EraseBootPage(address);
	}
	else if (command[0]=='n'){
		uint32_t address = (command[1]<<24) | (command[2]<<16) |  (command[3]<<8)  | command[4] ;
		SP_WriteBootPage(address);
	}
	else if (command[0]=='N'){
		uint32_t address = (command[1]<<24) | (command[2]<<16) |  (command[3]<<8)  | command[4] ;
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

// 	volatile uint8_t x = SP_ReadWord(0);
// 	
// 	SP_EraseApplicationSections();
// 	SP_LoadFlashPage(myflashpagedata);
// 	SP_WriteApplicationPage(0);
// 
// 	asm("nop");
// 
// 	SP_ReadFlashPage(myflashpagedata2, 0);
// 	NVM.CMD = 0x2B;
// 
// 
// 
// 	 
// 	
// 
// 	//asm("ldi r17, 0x3f");
// 
// 	asm("lpm r6,Z");
// 	_LPM_toReg(7);
// 
// 	//_MOV(3, 18);
// 	_MOV(16, 17);
// 	_TEST(19, 54);



	
	uint64_t nrfaddress = 0x4C4C4C4C31;
	_nrf24l01p_init();

	//LEFT
	_nrf24l01p_set_RX_pipe_address(_NRF24L01P_PIPE_P1, 0x4C4C4C4C31);
	_nrf24l01p_set_RX_pipe_address(_NRF24L01P_PIPE_P2, nrfaddress+1);
	_nrf24l01p_set_RX_pipe_address(_NRF24L01P_PIPE_P3, nrfaddress+2);
	_nrf24l01p_set_RX_pipe_address(_NRF24L01P_PIPE_P4, nrfaddress+3);
	_nrf24l01p_set_RX_pipe_address(_NRF24L01P_PIPE_P5, nrfaddress+4);
	char rxData[35];
	DigitalPin_SetDir(&led,1);
	DigitalPin_ClearValue(&led);
	while(1){

		if((_nrf24l01p_readable(_NRF24L01P_PIPE_P1))){
			asm("nop");
			nrf_led_flag = 1;
			int width = _nrf24l01p_read_dyn_pld(_NRF24L01P_PIPE_P1, (uint8_t*) rxData);
			rxData[width] = '\0';
			package_handler(rxData);
		}
	
	}


}

