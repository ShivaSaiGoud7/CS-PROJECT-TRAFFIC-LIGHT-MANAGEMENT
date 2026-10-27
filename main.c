/* ===========================================================
   main.c
   1 Pole - 4 Lamps
   RS485 command based control with ACK
   =========================================================== */

#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "Config_UART2.h"

/* -----------------------------------------------------------
   1-pole, 4-lamp arrangement driven from low-side outputs.
   Update these GPIO mappings if the board uses different pins.
   ----------------------------------------------------------- */

#define DRIVER_ACTIVE_LOW     1   /* 1 = active low driver, 0 = active high driver */

#define LAMP1   P0_bit.no0
#define LAMP2   P0_bit.no1
#define LAMP3   P0_bit.no2
#define LAMP4   P0_bit.no3

/* RS485 DE/RE control */
#define DEHIGH()    P1 |= (1u << 0)
#define DELOW()     P1 &= ~(1u << 0)

/* UART buffers */
volatile uint8_t UART_RX_D1[1];
volatile uint8_t UART_TX_D1[2];

/* External UART variables used by generated Renesas config */
extern uint8_t a;
extern uint8_t b;
extern uint8_t d;
extern uint8_t e;

/* -----------------------------------------------------------
   Command map for 1-pole, 4-lamp control
   ----------------------------------------------------------- */
#define CMD_ALL_OFF         0x00
#define CMD_LAMP1_ON        0x01
#define CMD_LAMP2_ON        0x02
#define CMD_LAMP3_ON        0x03
#define CMD_LAMP4_ON        0x04
#define CMD_LAMP1_OFF       0x05
#define CMD_LAMP2_OFF       0x06
#define CMD_LAMP3_OFF       0x07
#define CMD_LAMP4_OFF       0x08
#define CMD_LAMP1_2_ON      0x09
#define CMD_LAMP3_4_ON      0x0A
#define CMD_RED_AMBER_ON    0x0B
#define CMD_GREEN_ON        0x0C
#define CMD_TRAFFIC_RUN     0x0D
#define CMD_STOP           0x0E

#define ACK_VALID           0xAA
#define ACK_INVALID         0x55

/* -----------------------------------------------------------
   Delay function
   ----------------------------------------------------------- */
void delay_ms(uint32_t ms)
{
    volatile uint32_t i;
    for (i = 0; i < (ms * 1000UL); i++)
    {
        __nop();
    }
}

/* -----------------------------------------------------------
   Set one lamp ON/OFF
   ----------------------------------------------------------- */
void setLamp(uint8_t lamp, uint8_t state)
{
    uint8_t outputState;

    if (DRIVER_ACTIVE_LOW == 1)
    {
        outputState = (state == 0) ? 1u : 0u;
    }
    else
    {
        outputState = state;
    }

    switch (lamp)
    {
        case 1:
            LAMP1 = outputState;
            break;

        case 2:
            LAMP2 = outputState;
            break;

        case 3:
            LAMP3 = outputState;
            break;

        case 4:
            LAMP4 = outputState;
            break;

        default:
            break;
    }
}

/* -----------------------------------------------------------
   All lamps OFF
   ----------------------------------------------------------- */
void allLampsOff(void)
{
    setLamp(1, 0);
    setLamp(2, 0);
    setLamp(3, 0);
    setLamp(4, 0);
}

/* -----------------------------------------------------------
   Traffic sequence for 1 pole - 4 lamps
   Lamp mapping:
   1 = RED
   2 = AMBER
   3 = GREEN 1
   4 = GREEN 2
   ----------------------------------------------------------- */
void trafficSequence(void)
{
    allLampsOff();
    setLamp(1, 1);
    delay_ms(5000);

    setLamp(2, 1);
    delay_ms(2000);

    allLampsOff();
    setLamp(3, 1);
    delay_ms(5000);

    allLampsOff();
    setLamp(2, 1);
    delay_ms(2000);

    allLampsOff();
    setLamp(4, 1);
    delay_ms(5000);

    allLampsOff();
    delay_ms(1000);
}

/* -----------------------------------------------------------
   RS485 receive
   ----------------------------------------------------------- */
void UART_RX(void)
{
    while (d == 0)
    {
        R_Config_UART2_Receive(UART_RX_D1, 1);
    }
    d = 0;

    while (b == 0)
    {
        /* wait until receive complete */
    }

    b = 0;
    a = 0;
}

/* -----------------------------------------------------------
   RS485 transmit ACK
   valid command -> 0xAA
   invalid command -> 0x55
   ----------------------------------------------------------- */
void UART_SEND_ACK(uint8_t ackByte)
{
    DEHIGH();
    UART_TX_D1[0] = ackByte;
    UART_TX_D1[1] = ackByte;

    R_Config_UART2_Send(UART_TX_D1, 1);

    while (a == 0)
    {
        /* wait until transmit complete */
    }

    a = 0;
    b = 0;
    DELOW();
}

/* -----------------------------------------------------------
   Process RS485 command
   ----------------------------------------------------------- */
void processCommand(uint8_t cmd)
{
    uint8_t ack = ACK_INVALID;
    uint8_t valid = 0;

    switch (cmd)
    {
        case CMD_ALL_OFF:
        case CMD_LAMP1_ON:
        case CMD_LAMP2_ON:
        case CMD_LAMP3_ON:
        case CMD_LAMP4_ON:
        case CMD_LAMP1_OFF:
        case CMD_LAMP2_OFF:
        case CMD_LAMP3_OFF:
        case CMD_LAMP4_OFF:
        case CMD_LAMP1_2_ON:
        case CMD_LAMP3_4_ON:
        case CMD_RED_AMBER_ON:
        case CMD_GREEN_ON:
        case CMD_TRAFFIC_RUN:
        case CMD_STOP:
            valid = 1;
            break;

        default:
            valid = 0;
            break;
    }

    if (valid == 1)
    {
        switch (cmd)
        {
            case CMD_ALL_OFF:
                allLampsOff();
                break;

            case CMD_LAMP1_ON:
                allLampsOff();
                setLamp(1, 1);
                break;

            case CMD_LAMP2_ON:
                allLampsOff();
                setLamp(2, 1);
                break;

            case CMD_LAMP3_ON:
                allLampsOff();
                setLamp(3, 1);
                break;

            case CMD_LAMP4_ON:
                allLampsOff();
                setLamp(4, 1);
                break;

            case CMD_LAMP1_OFF:
                setLamp(1, 0);
                break;

            case CMD_LAMP2_OFF:
                setLamp(2, 0);
                break;

            case CMD_LAMP3_OFF:
                setLamp(3, 0);
                break;

            case CMD_LAMP4_OFF:
                setLamp(4, 0);
                break;

            case CMD_LAMP1_2_ON:
                allLampsOff();
                setLamp(1, 1);
                setLamp(2, 1);
                break;

            case CMD_LAMP3_4_ON:
                allLampsOff();
                setLamp(3, 1);
                setLamp(4, 1);
                break;

            case CMD_RED_AMBER_ON:
                allLampsOff();
                setLamp(1, 1);
                setLamp(2, 1);
                break;

            case CMD_GREEN_ON:
                allLampsOff();
                setLamp(3, 1);
                setLamp(4, 1);
                break;

            case CMD_TRAFFIC_RUN:
                trafficSequence();
                break;

            case CMD_STOP:
                allLampsOff();
                break;

            default:
                break;
        }

        ack = ACK_VALID;
    }

    UART_SEND_ACK(ack);
}

/* -----------------------------------------------------------
   Main
   ----------------------------------------------------------- */
void main(void)
{
    EI();

    allLampsOff();

    R_Config_UART2_Create();
    delay_ms(1000);

    while (e == 0)
    {
        R_Config_UART2_Start();
    }
    e = 0;

    while (1U)
    {
        UART_RX();
        processCommand(UART_RX_D1[0]);
    }
}
