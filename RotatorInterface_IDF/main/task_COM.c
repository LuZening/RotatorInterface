#include "COM.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// Start this task in FreeRTOS to enable COM commnication
static void COM_task(void *args)
{
    uart_event_t evtUART;
    while(1)
    {
        /* Check COM receiving Queue*/

        if(xQueueReceive(pCOMEventQueue, &evtUART, portMAX_DELAY))
        {
            switch(evtUART.type)
            {
                // TODO: COM Receiving Task
                case UART_BUFFER_FULL:
                break;
                default:
                break;
            }
        }
    }
}