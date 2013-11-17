#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"

#include "erbium.h"

//#include "dev/eeprom.h"
typedef unsigned short eeprom_addr_t;
#define eeprom_write(...)
#define eeprom_init()
#define eeprom_read(...)

#define TRUE 1
#define FALSE 0

#define DEBUG 0
#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(count_sensor, "Count sensor");
PROCESS(erbium_server, "Erbium Server");

AUTOSTART_PROCESSES(&count_sensor, &erbium_server);

static long sensorCounter;// live in 0 address in eeprom
static long sensorStep; // live in 4 address in eeprom

static eeprom_addr_t addrCounter = 0;
static eeprom_addr_t addrStep = 4;

static writeLongToEeprom(long val, eeprom_addr_t addr)
{
    unsigned char *eeprom;
    memcpy(eeprom, &val, 4);
    eeprom_write(addr, eeprom, 4);
}

static long readLongFromEeprom(eeprom_addr_t addr)
{
    unsigned char *eeprom;
    eeprom_read(addr, eeprom, 4);
    long data = 0;
    memcpy(&data, eeprom, 4);
    return data;

}

PROCESS_THREAD(count_sensor, ev, data)
{
    static struct etimer timer;
    PROCESS_BEGIN();

    eeprom_init();

    sensorCounter = readLongFromEeprom(addrCounter);
    sensorStep = readLongFromEeprom(addrStep);

    etimer_set(&timer, CLOCK_SECOND);

    while(TRUE)
    {
        PROCESS_WAIT_EVENT();
        if(ev == PROCESS_EVENT_TIMER)//TODO: Change to real event from sensor
        {
            sensorCounter+= sensorStep;
            writeLongToEeprom(sensorCounter, addrCounter);
            etimer_reset(&timer);
        }
    }

    PROCESS_END();
}

RESOURCE(setsensor, METHOD_POST, "setsensor", "title=\"Put sensor value\"");

void
setsensor_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    const char *queryString = NULL;

    if(REST.get_query_variable(request, "value", &queryString))
    {
        PRINTF("Setting new sensor value: %s\n", queryString);
        sensorCounter = atoi(queryString);
        writeLongToEeprom(sensorCounter, addrCounter);
    }

    if(REST.get_query_variable(request, "value", &queryString))
    {
        PRINTF("Setting new sensor step: %s\n", queryString);
        sensorStep = atoi(queryString);
        writeLongToEeprom(sensorStep, addrStep);
    }

}

RESOURCE(getsensor, METHOD_GET, "getsensor", "title=\"Get sensor value\"");

void
getsensor_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%u", sensorCounter);
    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
}

PROCESS_THREAD(erbium_server, ev, data)
{
    PROCESS_BEGIN();

    rest_init_engine();

    rest_activate_resource(&resource_getsensor);
    rest_activate_resource(&resource_setsensor);

    while(TRUE)
    {
        PROCESS_WAIT_EVENT();
    }

    PROCESS_END();
}
