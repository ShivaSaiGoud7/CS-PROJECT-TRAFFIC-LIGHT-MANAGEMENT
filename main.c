/*switches integrated trial 1 - Dhaya **********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "Config_PORT.h"
#include "Config_UART2.h"
#include "Config_CSI11.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
#define DEHIGH() P1 |= (1 << 0);  // GPIO_LED2=HIGH
#define DELOW()  P1 &= ~(1 << 0); 


void delay(long d)
{
	while (d--);
}

void delay_100ns(void)
{
	volatile int i;
	for (i = 0; i < 1; i++);
}

void delay_150ns(void)
{
	volatile int i;
	for (i = 0; i < 2; i++);
}

void LED_BLINK()
{
	P0 |= (1 << 0);  // GPIO_LED1=HIGH
	delay(10000);
	P0 &= ~(1 << 0); // GPIO_LED1=LOW

	P3 |= (1 << 1);  // GPIO_LED2=HIGH
 	delay(10000);
 	P3 &= ~(1 << 1); // GPIO_LED2=LOW
}

void LED1_GLOW()
{
	P0 |= (1 << 0);  // GPIO_LED1=HIGH
	P3 &= ~(1 << 1); // GPIO_LED2=LOW
}

void LED2_GLOW()
{
	P0 &= ~(1 << 0); // GPIO_LED1=LOW
	P3 |= (1 << 1);  // GPIO_LED2=HIGH
}

/* End user code. Do not edit comment generated here */

/* this micro select slave l9825_00 */
#define CS0_LOW()   (P1 &= ~(1 << 5))  // Chip Select Low
#define CS0_HIGH()  (P1 |=  (1 << 5))  // Chip Select High
/* this micro select slave l9825_01 */
#define CS1_LOW()   (P1 &= ~(1 << 7))  // Chip Select Low
#define CS1_HIGH()  (P1 |=  (1 << 7))  // Chip Select High
/* this micro select slave l9825_02 */
#define CS2_LOW()   (P6 &= ~(1 << 0))  // Chip Select Low
#define CS2_HIGH()  (P6 |=  (1 << 0))  // Chip Select High
/* this micro select slave l9825_03 */
#define CS3_LOW()   (P6 &= ~(1 << 1))  // Chip Select Low
#define CS3_HIGH()  (P6 |=  (1 << 1))  // Chip Select High

uint8_t UART_TX_D1[2]={0xFF,0xFF}; 	// UART TX Data byte buffers to send
uint8_t UART_RX_D1[1];			// UART RX Data byte buffers to receive
uint8_t SO[2] = {0x00,0x00}; 		// SPI TX Data byte buffers to send
uint8_t SI[1] = {0x00};			// SPI RX Data byte buffers to receive

uint8_t POLE=0;				// For Pole check	
uint8_t LAMP_COLOUR=0;			// For Pole LAMP colour check

uint8_t POLE_CONFIG = 0;     		// Physical slave pole configuration  --  N
uint8_t POLE_COMMAND = 0;      		// Pole requested by Master  -- N
uint8_t POLE_MATCH = 0;        		// Pole command match confirmation  -- N

uint8_t c=0;				// For ACK Byte check
uint8_t ACK_SUCCESS=0;			// Master ACK Byte confirmation
uint8_t ACK_FAIL=0;			// Master ACK Byte non confirmation

uint8_t TXDATA;				// Current UART TX byte to send
uint8_t RXDATA;				// Current UART RX byte received
uint8_t i=0;				// For repeated SPI output datas transfer

extern uint8_t a;			// For UART TX end confirmation
extern uint8_t b;			// For UART RX end confirmation
extern uint8_t d;			// For UART RX Module receive end confirmation
extern uint8_t e;			// For UART RX Module start confirmation
extern uint8_t start_success;		// For UART Start settings confirmation
extern int SOEND;			// For SPI TX end confirmation
extern int SIEND;			// For SPT RX end confirmation	
extern uint8_t create_success;		// For SPI Module creation confirmation

char LAMP [];				// Confirmed LAMP information (only for user reference through RS485 output

/* End user code. Do not edit comment generated here */

void L9825TR_INIT(void)
{
// Reset assertion for L9825TR
	P2 &= ~(1 << 1); // nRES = LOW
    	delay(1000);
// Reset release for L9825TR
    	P2 |= (1 << 1);  // nRES = HIGH
    	delay(1000);
// Set IN0 and IN1 to LOW (which configures OUT1 and OUT2 - hence only SPI can control)
	P14 |= (1 << 7);
   	P2  |= (1 << 2);
// Set all CS signals into Logic High
	CS0_HIGH();
  	CS1_HIGH();
	CS2_HIGH();
	CS3_HIGH();
}

// Function to receive byte through RS485 from Master

void UART_RX()
{
	while(d == 0)
	{
	R_Config_UART2_Receive(UART_RX_D1, 1);
	}
	d=0;
	while(b == 0)
	{
		// WAIT UNTILL RECEIVING A BYTE
	}
	UART_RX_D1[0] = RXDATA;
		
	a = 0;
	b = 0;
}

// Function to send byte through RS485 to Master

void UART_SEND()
{
	DEHIGH();
	UART_TX_D1[1]=UART_TX_D1[0];
	R_Config_UART2_Send(UART_TX_D1, 1);
	while(a == 0)
	{
		// WAIT UNTILL TRANSMITTING A BYTE
	}	
	a = 0;
	b = 0;
	DELOW();
}	
	
// Checking for POLE 1 - RED LAMP condition ACK Byte Checking from Master

void UART_ACK_CHECK_RED_POLE1(uint8_t ACK_SUCCESS, uint8_t ACK_FAIL)
{ 

// Beginning of UART_ACK_CHECK_RED function

	// UART module section
	UART_RX();
	
	if(UART_RX_D1[0] == ACK_SUCCESS) 
	{
		c = 0x01;
		//SO[0]=0x01;
		//SO[1]=0x01;
	}
	else if(UART_RX_D1[0] == ACK_FAIL) 
	{
		c = 0x00; 
		//SO[0]=0x00;
		//SO[1]=0x00;
	}
		
	// SPI module section
	// L9825TR_INIT(); 
	
	while(create_success == 0)
	{
		R_Config_CSI11_Create(); // Configure SPI
	}
	create_success = 0;
		
	SOEND = 0;
	SIEND = 0;

	delay_100ns();  // Setup time before CLK
		
	if(c = 0x01)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS1_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS1_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{
		SO[0]=0x01;
		SO[1]=0x01;
	
		CS0_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS0_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	}
	else if(c = 0x00)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS1_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS1_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{	
		SO[0]=0x00;
		SO[1]=0x00;
	
		CS0_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS0_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	}
		
a = 0;
b = 0;
c = 0;

// End of UART_ACK_CHECK_RED function
}

// Checking for POLE 1 - AMBER LAMP condition ACK Byte Checking from Master

void UART_ACK_CHECK_RED_POLE2(uint8_t ACK_SUCCESS, uint8_t ACK_FAIL)
{ 

// Beginning of UART_ACK_CHECK_RED function

	// UART module section
	UART_RX();
	
	if(UART_RX_D1[0] == ACK_SUCCESS) 
	{
		c = 0x01;
		// SO[0]=0x01;
		// SO[1]=0x01;
	}
	else if(UART_RX_D1[0] == ACK_FAIL) 
	{
		c = 0x00; 
		// SO[0]=0x00;
		// SO[1]=0x00;
	}
		
	// SPI module section
	// L9825TR_INIT(); 
	
	while(create_success == 0)
	{
		R_Config_CSI11_Create(); // Configure SPI
	}
	create_success = 0;
		
	SOEND = 0;
	SIEND = 0;

	delay_100ns();  // Setup time before CLK
		
	if(c = 0x01)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS3_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS3_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{
		SO[0]=0x01;
		SO[1]=0x01;
	
		CS2_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS2_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	}
	else if(c = 0x00)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS3_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS3_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{	
		SO[0]=0x00;
		SO[1]=0x00;
	
		CS2_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS2_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	}
		
a = 0;
b = 0;
c = 0;

// End of UART_ACK_CHECK_RED function
}

void UART_ACK_CHECK_AMBER_POLE1(uint8_t ACK_SUCCESS, uint8_t ACK_FAIL)
{ 
// Beginning of UART_ACK_CHECK_AMBER function

	// UART module section
	UART_RX();
	
	if(UART_RX_D1[0] == ACK_SUCCESS) 
	{
		c = 0x01;
		//SO[0]=0x10;
		//SO[1]=0x10;
	}
	else if(UART_RX_D1[0] == ACK_FAIL) 
	{
		c = 0x00; 
		//SO[0]=0x00;
		//SO[1]=0x00;
	}
		
	// SPI module section
	// L9825TR_INIT(); 
	
	while(create_success == 0)
	{
		R_Config_CSI11_Create(); // Configure SPI
	}
	create_success = 0;
		
	SOEND = 0;
	SIEND = 0;

	delay_100ns();  // Setup time before CLK
	
	if(c == 0x01)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS1_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS1_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{
		SO[0]=0x10;
		SO[1]=0x10;
	
		CS0_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS0_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	}
	else if(c == 0x00)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS1_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS1_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{	
		SO[0]=0x00;
		SO[1]=0x00;
	
		CS0_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS0_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	}
		
a = 0;
b = 0;
c = 0;

// End of UART_ACK_CHECK_AMBER function
}

void UART_ACK_CHECK_AMBER_POLE2(uint8_t ACK_SUCCESS, uint8_t ACK_FAIL)
{ 
// Beginning of UART_ACK_CHECK_AMBER function

	// UART module section
	UART_RX();
	
	if(UART_RX_D1[0] == ACK_SUCCESS) 
	{
		c = 0x01;
		// SO[0]=0x10;
		// SO[1]=0x10;
	}
	else if(UART_RX_D1[0] == ACK_FAIL) 
	{
		c = 0x00; 
		// SO[0]=0x00;
		// SO[1]=0x00;
	}
		
	// SPI module section
	// L9825TR_INIT(); 
	
	while(create_success == 0)
	{
		R_Config_CSI11_Create(); // Configure SPI
	}
	create_success = 0;
		
	SOEND = 0;
	SIEND = 0;

	delay_100ns();  // Setup time before CLK
		
	if(c = 0x01)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS3_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS3_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{
		SO[0]=0x10;
		SO[1]=0x10;
	
		CS2_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS2_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	}
	else if(c = 0x00)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS3_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS3_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{	
		SO[0]=0x00;
		SO[1]=0x00;
	
		CS2_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS2_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	}
		
a = 0;
b = 0;
c = 0;

// End of UART_ACK_CHECK_RED function
}

// Checking for POLE 1 - GREEN-1 LAMP condition ACK Byte Checking from Master

void UART_ACK_CHECK_GREEN1_POLE1(uint8_t ACK_SUCCESS, uint8_t ACK_FAIL)
{ 

// Beginning of UART_ACK_CHECK_GREEN function

	// UART module section
	UART_RX();
	
	if(UART_RX_D1[0] == ACK_SUCCESS) 
	{
		c = 0x01;
		// SO[0]=0x11;
		// SO[1]=0x11;
	}
	else if(UART_RX_D1[0] == ACK_FAIL) 
	{
		c = 0x00; 
		// SO[0]=0x00;
		// SO[1]=0x00;
	}
		
	// SPI module section
	// L9825TR_INIT(); 
	
	while(create_success == 0)
	{
		R_Config_CSI11_Create(); // Configure SPI
	}
	create_success = 0;
		
	SOEND = 0;
	SIEND = 0;

	delay_100ns();  // Setup time before CLK
		
	if(c = 0x01)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS0_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS0_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{
		SO[0]=0x11;
		SO[1]=0x11;
	
		CS1_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS1_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	}
	else if(c = 0x00)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS0_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS0_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{	
		SO[0]=0x00;
		SO[1]=0x00;
	
		CS1_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS1_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	}
		
a = 0;
b = 0;
c = 0;

// End of UART_ACK_CHECK_GREEN function
}

void UART_ACK_CHECK_GREEN1_POLE2(uint8_t ACK_SUCCESS, uint8_t ACK_FAIL)
{ 

// Beginning of UART_ACK_CHECK_GREEN function

	// UART module section
	UART_RX();
	
	if(UART_RX_D1[0] == ACK_SUCCESS) 
	{
		c = 0x01;
		// SO[0]=0x11;
		// SO[1]=0x11;
	}
	else if(UART_RX_D1[0] == ACK_FAIL) 
	{
		c = 0x00; 
		// SO[0]=0x00;
		// SO[1]=0x00;
	}
		
	// SPI module section
	//L9825TR_INIT(); 
	
	while(create_success == 0)
	{
		R_Config_CSI11_Create(); // Configure SPI
	}
	create_success = 0;
		
	SOEND = 0;
	SIEND = 0;

	delay_100ns();  // Setup time before CLK
		
	if(c = 0x01)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS2_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS2_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{
		SO[0]=0x11;
		SO[1]=0x11;
	
		CS3_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS3_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	}
	else if(c = 0x00)
	{
	while(i<3)
	{
		
		SO[0]=0x00;
		SO[1]=0x00;
		
		CS2_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS2_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
	}
	i=0;
	
	delay(100);  
	
	while(i<3)
	{	
		SO[0]=0x00;
		SO[1]=0x00;
	
		CS3_LOW();
		delay_100ns();
		while(start_success == 0)
		{
			R_Config_CSI11_Start();
		}
		start_success = 0;
		while(SOEND == 0)
		{
			R_Config_CSI11_Send(SO, 1);
		}
		SOEND = 0;
		SIEND = 0;
		
		CS3_HIGH(); 
		delay_150ns();      // Hold time after NCS HIGH
		
		R_Config_CSI11_Stop();
		i = i+1;
	}
	i=0;
	
	}
		
a = 0;
b = 0;
c = 0;

// End of UART_ACK_CHECK_GREEN1 function
}


/***********************************************************************************************************************
* Pole Configuration Read
* For identifying the Poles
* For example if master sends 0x09 , POLE = 0x09 & 0x38 = 0x08 which is POLE 2 
***********************************************************************************************************************/

void POLE_CONFIG_READ(void)
{
    POLE_CONFIG = 0;

    /* Pole 1 */
    if(((P12 & (1 << 0)) != 0) &&
       ((P13 & (1 << 7)) == 0) &&
       ((P2  & (1 << 0)) == 0) &&
       ((P0  & (1 << 1)) == 0))
    {
        POLE_CONFIG = 1;
    }

    /* Pole 2 */
    else if(((P12 & (1 << 0)) == 0) &&
            ((P13 & (1 << 7)) != 0) &&
            ((P2  & (1 << 0)) == 0) &&
            ((P0  & (1 << 1)) == 0))
    {
        POLE_CONFIG = 2;
    }

    /* Pole 3 */
    else if(((P12 & (1 << 0)) == 0) &&
            ((P13 & (1 << 7)) == 0) &&
            ((P2  & (1 << 0)) != 0) &&
            ((P0  & (1 << 1)) == 0))
    {
        POLE_CONFIG = 3;
    }

    /* Pole 4 */
    else if(((P12 & (1 << 0)) == 0) &&
            ((P13 & (1 << 7)) == 0) &&
            ((P2  & (1 << 0)) == 0) &&
            ((P0  & (1 << 1)) != 0))
    {
        POLE_CONFIG = 4;
    }

    /* Invalid configuration */
    else
    {
        POLE_CONFIG = 0;
    }
}
/***********************************************************************************************************************
* Pole Command Check
***********************************************************************************************************************/

void POLE_COMMAND_CHECK(void)
{
	POLE_MATCH = 0;

	/*
	 * Master pole command:
	 *
	 * Pole 1 = 0x00
	 * Pole 2 = 0x08
	 * Pole 3 = 0x10
	 * Pole 4 = 0x18
	 */

	POLE_COMMAND = UART_RX_D1[0] & 0x38;

	if((POLE_CONFIG == 1) && (POLE_COMMAND == 0x00))
	{
		POLE_MATCH = 1;
	}

	else if((POLE_CONFIG == 2) && (POLE_COMMAND == 0x08))
	{
		POLE_MATCH = 1;
	}

	else if((POLE_CONFIG == 3) && (POLE_COMMAND == 0x10))
	{
		POLE_MATCH = 1;
	}

	else if((POLE_CONFIG == 4) && (POLE_COMMAND == 0x18))
	{
		POLE_MATCH = 1;
	}

	else
	{
		POLE_MATCH = 0;
	}
}

void main(void)
{
	EI();
	delay (1000);
	
	L9825TR_INIT();
	
	POLE_CONFIG_READ();   //Need to check the position ... 
	
	R_Config_UART2_Create();
	delay (1000);
	
	while (e == 0)
	{
		R_Config_UART2_Start();	
		//delay (1000);	
	}
	e = 0;
		
	while(1U)
	{
		
		// Waiting for data byte from Master 
		
		UART_RX();

		// Find Pole and Lamp colour information 
		
		POLE = UART_RX_D1[0] & 0x38;
		LAMP_COLOUR = UART_RX_D1[0] & 0x07;
		
		POLE_COMMAND_CHECK();   //Position check while debugging ...
		// Need to execute if the lamp command only when POLE_MATCH == 1

		if((POLE_MATCH == 1) && (POLE == 0x00)) // CHECKING FOR POLE 1 
		{
			if(LAMP_COLOUR == 0x00) // Beginning of Red Lamp colour commands
			{
				if(UART_RX_D1[0] == 0x0)
				{
					UART_TX_D1[0] = 0x0F;
					UART_SEND(UART_TX_D1[0]);
					UART_ACK_CHECK_RED_POLE1(0x06, 0x07);
				}
				
			}			// End of Red Lamp colour commands

			else if(LAMP_COLOUR == 0x01) 	// Beginning of Amber Lamp colour commands
			{
				if(UART_RX_D1[0] == 0x01)
				{
					UART_TX_D1[0] = 0x8E;
					UART_SEND(UART_TX_D1[0]);
					UART_ACK_CHECK_AMBER_POLE1(0x06, 0x07);
				}
				
			}	// End of Amber Lamp colour commands

			else if(LAMP_COLOUR == 0x02) // Beginning of Green Lamp colour commands
			{	
				if(UART_RX_D1[0] == 0x02)
				{
					UART_TX_D1[0] = 0x8D;
					UART_SEND(UART_TX_D1[0]);
					UART_ACK_CHECK_GREEN1_POLE1(0x06, 0x07);
				}
				
			}	// End of Green1 Lamp colour commands
			
//			if(LAMP_COLOUR == 0x05) // Beginning of Auto Blink ambur colour commands
//			{
//				if(UART_RX_D1[0] == 0x0)
//				{
//					UART_TX_D1[0] = 0x0A;
//					UART_SEND(UART_TX_D1[1]);
//					UART_ACK_CHECK_AUTO_BLINK_POLE1(0x06, 0x07);
//				}
				
//			}	

		} // End of Pole 1 commands check
		
		else if((POLE_MATCH == 1) && (POLE == 0x08)) // CHECKING FOR POLE 2 
		{
			if(LAMP_COLOUR == 0x00) // Beginning of Red Lamp colour commands
			{
				if(UART_RX_D1[0] == 0x08)
				{
					UART_TX_D1[0] = 0x17;
					UART_SEND(UART_TX_D1[0]);
					UART_ACK_CHECK_RED_POLE2(0x0E, 0x0F);
				}
				
			}			// End of Red Lamp colour commands

			else if(LAMP_COLOUR == 0x01) 	// Beginning of Amber Lamp colour commands
			{
				if(UART_RX_D1[0] == 0x09)
				{
					UART_TX_D1[0] = 0x96;
					UART_SEND(UART_TX_D1[0]);
					UART_ACK_CHECK_AMBER_POLE2(0x0E, 0x0F);
				}
				
			}	// End of Amber Lamp colour commands

			else if(LAMP_COLOUR == 0x02) // Beginning of Green Lamp colour commands
			{	
				if(UART_RX_D1[0] == 0x0A)
				{
					UART_TX_D1[0] = 0x95;
					UART_SEND(UART_TX_D1[0]);
					UART_ACK_CHECK_GREEN1_POLE2(0x0E, 0x0F);
				}
				
			}	// End of Green1 Lamp colour commands
			// similarly need to add for pole 3 (if pole == 0x10) and pole 4 (if pole == 0x18)
			//Need to include for POLE_MATCH ==0 
			
//			if(LAMP_COLOUR == 0x05) // Beginning of Auto Blink ambur colour commands
//			{
//				if(UART_RX_D1[0] == 0x0D)
//				{
//					UART_TX_D1[0] = 0x12;
//					UART_SEND(UART_TX_D1[1]);
//					UART_ACK_CHECK_AUTO_BLINK_POLE1(0x0E, 0x0F);
//				}
				
//			}	

		} // End of Pole 2 commands check

	} // End of while loop

} // End of main loop

