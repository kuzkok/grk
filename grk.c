#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "dev/button-sensor.h"

#include "erbium.h"

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

static long sensorCounter = 0;// live in 0 address in eeprom
static long sensorStep = 1; // live in 4 address in eeprom


PROCESS_THREAD(count_sensor, ev, data)
{    
    PROCESS_BEGIN();
    SENSORS_ACTIVATE(button_sensor);

    while(TRUE)
    {
        PROCESS_WAIT_EVENT();
        if(ev == sensors_event && data == &button_sensor)
        {
            sensorCounter+= sensorStep;            
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
    }

    if(REST.get_query_variable(request, "value", &queryString))
    {
        PRINTF("Setting new sensor step: %s\n", queryString);
        sensorStep = atoi(queryString);        
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
