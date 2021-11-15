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
#include "definitions.h"                // SYS function prototypes

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
#define FRAME_SIZE  6
#define FRAME_START 0
#define FLAGS_INDEX 1
#define COUNT_INDEX 2

#define NORMAL 0
#define ERROR  1

#define COUNTER_INIT 0
#define COUNTER_SIZE 4

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

typedef union
{
    uint8_t as_bytes[COUNTER_SIZE];
    uint32_t packed;
}reportCounter_t;

bool validate_CAN();
bool validate_PIO();
bool validate_SPI();
bool validate_WATCHDOG();

int main ( void )
{
    uint8_t buffer[] = "RFCCCC";
    
    reportCounter_t counter;
    counter.packed = COUNTER_INIT;
 
    report_t report;
    report.packed = NORMAL;
    report.status_of.WATCHDOG = ERROR;
    
    SYS_Initialize ( NULL );
    
    buffer[FRAME_START] = 'R';
    buffer[FLAGS_INDEX] = report.packed;
    buffer[COUNT_INDEX + 0] = counter.as_bytes[0];
    buffer[COUNT_INDEX + 1] = counter.as_bytes[1];
    buffer[COUNT_INDEX + 2] = counter.as_bytes[2];
    buffer[COUNT_INDEX + 3] = counter.as_bytes[3];
    USART1_Write(&buffer[0], FRAME_SIZE);
    
    counter.packed++;
    WDT_Clear();
    
    while ( true )
    {
        SYS_Tasks ( );
        report.status_of.CAN = validate_CAN();
        report.status_of.PIO = validate_PIO();
        report.status_of.SPI = validate_SPI();
        report.status_of.WATCHDOG = NORMAL;
        
        buffer[FRAME_START] = 'R';
        buffer[FLAGS_INDEX] = report.packed;
        buffer[COUNT_INDEX + 0] = counter.as_bytes[0];
        buffer[COUNT_INDEX + 1] = counter.as_bytes[1];
        buffer[COUNT_INDEX + 2] = counter.as_bytes[2];
        buffer[COUNT_INDEX + 3] = counter.as_bytes[3];
        USART1_Write(&buffer[0], FRAME_SIZE);
        
        counter.packed++;
        WDT_Clear();
    }
    return ( EXIT_FAILURE );
}

bool validate_CAN()
{
    return NORMAL;
}

bool validate_PIO()
{
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