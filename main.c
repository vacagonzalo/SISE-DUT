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
#define FRAME_START 0
#define FLAGS 5
#define FLAG_LENGTH 1

#define NORMAL 0
#define ERROR 1

#define COUNTER_INITIAL_VALUE 0
#define COUNTER_LENGTH 4

#define BUFFER_OFFSET 1
#define BUFFER_LENGTH 6

struct status_bitfield_t
{
    uint8_t CAN:FLAG_LENGTH;
    uint8_t SPI:FLAG_LENGTH;
    uint8_t PIO:FLAG_LENGTH;
    uint8_t WATCHDOG:FLAG_LENGTH;
}__attribute__((packed));

typedef union
{
    struct status_bitfield_t status_of;
    uint8_t packed;
}report_t;

typedef union
{
    uint8_t as_bytes[COUNTER_LENGTH];
    uint32_t packed;
}reportCounter_t;

bool validate_CAN();
bool validate_PIO();
bool validate_SPI();
bool validate_WATCHDOG();

void update_report(uint8_t *buffer, report_t *report, reportCounter_t *counter);
void send_report(uint8_t *buffer, reportCounter_t *counter);

int main ( void )
{
    uint8_t buffer[] = "RCCCCF";
    
    reportCounter_t counter;
    counter.packed = COUNTER_INITIAL_VALUE;
 
    report_t report;
    report.packed = NORMAL;
    report.status_of.WATCHDOG = ERROR;
    
    SYS_Initialize ( NULL );
    
    update_report(buffer, &report, &counter);
    send_report(buffer, &counter);
    
    while ( true )
    {
        SYS_Tasks ( );
        report.status_of.CAN = validate_CAN();
        report.status_of.PIO = validate_PIO();
        report.status_of.SPI = validate_SPI();
        report.status_of.WATCHDOG = validate_WATCHDOG();
        update_report(buffer, &report, &counter);
        send_report(buffer, &counter);
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
    static uint8_t spi_write[] = "SPI";
    uint8_t spi_read[] = "???";
    uint8_t status = NORMAL;
    SPI0_WriteRead(
            &spi_write[0], 
            sizeof(spi_write),
            &spi_read[0], 
            sizeof(spi_read));
    for(uint8_t i = 0; i < 4; ++i)
    {
        if(spi_write[i] != spi_read[i])
        {
            status = ERROR;
            break;
        }
    }
    return status;
}

bool validate_WATCHDOG()
{
    WDT_Clear();
    return NORMAL;
}

void update_report(uint8_t *buffer, report_t *report, reportCounter_t *counter)
{
    for(uint8_t byte = 0; byte < COUNTER_LENGTH; ++byte)
    {
        buffer[byte + BUFFER_OFFSET] = counter->as_bytes[byte];
    }
    buffer[FLAGS] = report->packed;
}

void send_report(uint8_t *buffer, reportCounter_t *counter)
{
    USART1_Write(buffer, BUFFER_LENGTH);
    counter->packed++;
}
/*******************************************************************************
 End of File
*/

