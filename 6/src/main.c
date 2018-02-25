#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include <fsm.h>


#define PERIOD_TICK 100/portTICK_RATE_MS
#define GPIO_LED 2
#define GPIO_ARMED 4
#define GPIO_PRESENCE 0

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
  DESARMADO,
  ARMADO
};


int presencia (fsm_t *this) {
  if(GPIO_INPUT_GET(GPIO_PRESENCE)) {
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
int alarm_on(fsm_t *this){
  return(!GPIO_INPUT_GET(GPIO_ARMED));

};
int alarm_off(fsm_t *this){
  return(GPIO_INPUT_GET(GPIO_ARMED));

};

void alarm_sound(fsm_t *this){
  GPIO_OUTPUT_SET(GPIO_LED, 0);

};

void alarm_shut(fsm_t *this){
  GPIO_OUTPUT_SET(GPIO_LED, 1);

};
static fsm_trans_t interruptor[] = {
  {DESARMADO,alarm_on,ARMADO,alarm_shut},
  {ARMADO, alarm_off, DESARMADO, alarm_shut},
  {ARMADO, presencia, ARMADO, alarm_sound},
  {-1, NULL, -1, NULL }
};

void alarm(void* ignore)
{
    fsm_t* fsm = fsm_new(interruptor);
    alarm_shut(fsm);
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
    xTaskCreate(&alarm, "startup", 2048, NULL, 1, NULL);
}
