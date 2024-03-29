/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
#define FRAME_SIZE  3
#define FRAME_START 0
#define FLAGS_INDEX 1

#define NORMAL 0
#define ERROR  1

/* Standard identifier id[28:18]*/
#define WRITE_ID(id) (id << 18)
#define READ_ID(id)  (id >> 18)

#define BIT 1
struct status_bitfield_t
{
    uint8_t CAN:BIT;
    uint8_t SPI:BIT;
    uint8_t PIO:BIT;
    uint8_t WATCHDOG:BIT;
}__attribute__((packed));

typedef union
{
    struct status_bitfield_t status_of;
    uint8_t packed;
}report_t;

bool validate_CAN();
bool validate_PIO();
bool validate_SPI();
bool validate_WATCHDOG();

uint8_t Mcan1MessageRAM[MCAN1_MESSAGE_RAM_CONFIG_SIZE] __attribute__((aligned (32)))__attribute__((space(data), section (".ram_nocache")));

int main ( void )
{
    uint8_t buffer[] = "FX\n";
 
    report_t report;
    report.packed = NORMAL;
    report.status_of.WATCHDOG = ERROR;
    
    SYS_Initialize ( NULL );
    
    buffer[FRAME_START] = 'F';
    buffer[FLAGS_INDEX] = report.packed + 'A';
    USART1_Write(&buffer[0], FRAME_SIZE);
    
    WDT_Clear();
    
    MCAN1_MessageRAMConfigSet(Mcan1MessageRAM);
    MCAN1_REGS->MCAN_CCCR |= MCAN_TEST_LBCK(1); // CANBUS external loopback

    while ( true )
    {
        SYS_Tasks ( );
        report.status_of.CAN = validate_CAN();
        report.status_of.PIO = validate_PIO();
        report.status_of.SPI = validate_SPI();
        report.status_of.WATCHDOG = NORMAL;
        
        buffer[FRAME_START] = 'F';
        buffer[FLAGS_INDEX] = report.packed + 'A';
        USART1_Write(&buffer[0], FRAME_SIZE);
        
        WDT_Clear();
    }
    return ( EXIT_FAILURE );
}

bool validate_CAN()
{
    static uint32_t status = 0;

    static uint8_t txFiFo[MCAN1_TX_FIFO_BUFFER_SIZE];
    static uint8_t rxFiFo0[MCAN1_RX_FIFO0_SIZE];
    
    static MCAN_TX_BUFFER *txBuffer = NULL;
    static uint8_t numberOfMessage = 0;
    
    // <SEND>
    memset(txFiFo, 0x00, MCAN1_TX_FIFO_BUFFER_ELEMENT_SIZE);
    txBuffer = (MCAN_TX_BUFFER *)txFiFo;
    txBuffer->id = WRITE_ID(0x469);
    txBuffer->dlc = 8;
    for (uint8_t loop_count = 0; loop_count < 8; loop_count++)
    {
        txBuffer->data[loop_count] = loop_count;
    }
    if (MCAN1_MessageTransmitFifo(1, txBuffer) != true)
    {
        return ERROR; // could not transmit
    }
    // </SEND>
    
    uint32_t delay = 0x00ff;
    while(delay-->0);
    
    // <READ>
    if (MCAN1_InterruptGet(MCAN_INTERRUPT_RF0N_MASK))
    {    
        MCAN1_InterruptClear(MCAN_INTERRUPT_RF0N_MASK);

        /* Check MCAN Status */
        status = MCAN1_ErrorGet();

        if (((status & MCAN_PSR_LEC_Msk) == MCAN_ERROR_NONE) || ((status & MCAN_PSR_LEC_Msk) == MCAN_ERROR_LEC_NO_CHANGE))
        {
            numberOfMessage = MCAN1_RxFifoFillLevelGet(MCAN_RX_FIFO_0);
            if (numberOfMessage != 0)
            {
                memset(rxFiFo0, 0x00, (numberOfMessage * MCAN1_RX_FIFO0_ELEMENT_SIZE));
                if (MCAN1_MessageReceiveFifo(MCAN_RX_FIFO_0, numberOfMessage, (MCAN_RX_BUFFER *)rxFiFo0) == true)
                {
                    for(uint8_t i = 0; i < 8; i++)
                    {
                        if(((MCAN_RX_BUFFER *)rxFiFo0)->data[i] != i)
                        {
                            return ERROR;
                        }
                    }
                    return NORMAL;
                }
                else
                {
                    return ERROR;
                }
            }
        }
        else
        {
            return ERROR;
        }
    }
    // </READ>
    return ERROR;
}

bool validate_PIO()
{
    static bool value = false;
    PIO_PinWrite(PIO_PIN_PB3, value);
    uint32_t delay = 0x00ff;
    while(delay-->0);
    bool readed = PIO_PinRead(PIO_PIN_PA6);
    if(readed != value)
    {
        value = !value;
        return ERROR;
    }
    value = !value;
    return NORMAL;
}

bool validate_SPI()
{
    static uint8_t spi_write[3] = {1, 2, 3};
    static uint8_t spi_read[3] = {4, 5, 6};
    uint8_t status = NORMAL;
    SPI0_WriteRead(&spi_write[0], 3, &spi_read[0], 3);
    for(uint8_t i = 0; i < 3; ++i)
    {
        if(spi_write[i] != spi_read[i])
        {
            status = ERROR;
            break;
        }
    }
    return status;
}

/*******************************************************************************
 End of File
*/
