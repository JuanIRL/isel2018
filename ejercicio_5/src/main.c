#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include <fsm.h>


#define PERIOD_TICK 100/portTICK_RATE_MS

long debounceTime = 0;
long timeout_time = 0;

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
};



enum fsm_state {
  LED_ON,
  LED_OFF
};


int button_pressed (fsm_t *this) {
  if(GPIO_INPUT_GET(0)) {
    if(xTaskGetTickCount()*portTICK_RATE_MS > debounceTime){
      debounceTime = xTaskGetTickCount()*portTICK_RATE_MS + 300;
      return 1;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
};

int timeout (fsm_t *this) {
  if(xTaskGetTickCount()*portTICK_RATE_MS >= timeout_time){
    return 1;
  } else {
    return 0;
  }

};

void led_on (fsm_t *this) {
  GPIO_OUTPUT_SET(2, 0);
  timeout_time = xTaskGetTickCount()*portTICK_RATE_MS + 60000; //Primero 2s
};
void led_off (fsm_t *this) {
  GPIO_OUTPUT_SET(2, 1);
};

static fsm_trans_t interruptor[] = {
  {LED_OFF,button_pressed,LED_ON,led_on},
  {LED_ON,timeout,LED_OFF,led_off},
  {-1, NULL, -1, NULL }
};

void inter(void* ignore)
{
    fsm_t* fsm = fsm_new(interruptor);
    led_off(fsm);
    portTickType xLastWakeTime;

    while(true) {
      xLastWakeTime = xTaskGetTickCount ();
      fsm_fire(fsm);
      vTaskDelayUntil(&xLastWakeTime, PERIOD_TICK);
    }
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    xTaskCreate(&inter, "startup", 2048, NULL, 1, NULL);
}
