#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "stdio.h"
#include <fsm.h>


#define PERIOD_TICK 200/portTICK_RATE_MS

#define GPIO_LED 2
#define GPIO_CODE 4
#define GPIO_PRESENCE 0

long debounceTime = 0;
long code_timeout = 0;

int flag_armed = 0;

int code_pass [] = {1,2,3};
int code [3] = {0,0,0};
int indice = 0;
int num = 0;

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
  ARMADO,
  IDLE,
  WAIT2,
  WAIT3,
  NUM1_WRITE,
  NUM2_WRITE,
  NUM3_WRITE,
  CODE_CHECK,
};


int presencia (fsm_t *this) {
  if(GPIO_INPUT_GET(GPIO_PRESENCE)){
    printf("Detectada presencia\n");
    return 1;
  }
  return 0;
};
int alarm_on(fsm_t *this){
  return(flag_armed);

};
int alarm_off(fsm_t *this){
  return(!flag_armed);

};

void alarm_sound(fsm_t *this){
  printf("Sonando alarma\n");
  GPIO_OUTPUT_SET(GPIO_LED, 0);

};

void alarm_shut(fsm_t *this){
  printf("Silenciando alarma\n");
  GPIO_OUTPUT_SET(GPIO_LED, 1);

};
static fsm_trans_t alarma[] = {
  {DESARMADO,alarm_on,ARMADO,alarm_shut},
  {ARMADO, alarm_off, DESARMADO, alarm_shut},
  {ARMADO, presencia, ARMADO, alarm_sound},
  {-1, NULL, -1, NULL }
};

int button_pressed (fsm_t *this){
  if(GPIO_INPUT_GET(GPIO_PRESENCE)){
    if(xTaskGetTickCount()*portTICK_RATE_MS > debounceTime){
      debounceTime = xTaskGetTickCount()*portTICK_RATE_MS + 180;
      code_timeout = xTaskGetTickCount()*portTICK_RATE_MS + 1000;
      return 1;
    } else {
      return 0;
    }
  }
  return 0;
};

int timeout (fsm_t *this) {
  if(xTaskGetTickCount()*portTICK_RATE_MS > code_timeout){
    printf("Tiempo de simbolo terminado\n");
    return 1;
  } else{
    return 0;
  }
}

void num_save (fsm_t *this){
  code[indice] = num;
  indice++;
  num = 0;
  printf("Simbolo guardado\n");
}

void num_add (fsm_t *this){
  num++;
  printf("Valor de simbolo: %d\n", num);
}

void alarm_arm (fsm_t *this) {
  if(code == code_pass){
    flag_armed = !flag_armed;
    if(flag_armed){
      printf("Alarma armada\n");
    }else {
      printf("Alarma desarmada\n");
    }
  }
  int j;
  for(j = 0; j<3; j++){
    code[j] = 0;
  }
  num = 0;
  indice = 0;
  printf("Codigo reseteado\n");
}
static fsm_trans_t codigo[] = {
  {IDLE,button_pressed,NUM1_WRITE,NULL},
  {NUM1_WRITE,button_pressed,NUM1_WRITE,num_add},
  {NUM1_WRITE,timeout,WAIT2,num_save},
  {WAIT2,button_pressed,NUM2_WRITE,NULL},
  {NUM2_WRITE,button_pressed,NUM2_WRITE,num_add},
  {NUM2_WRITE,timeout,WAIT3,num_save},
  {WAIT3,button_pressed,NUM3_WRITE,NULL},
  {NUM3_WRITE,button_pressed,NUM3_WRITE,num_add},
  {NUM3_WRITE,timeout,CODE_CHECK,num_save},
  {CODE_CHECK,button_pressed,IDLE,alarm_arm},
  {-1, NULL, -1, NULL }
};

void alarm(void* ignore)
{
    fsm_t* fsm1 = fsm_new(alarma);
    fsm_t* fsm2 = fsm_new(codigo);
    alarm_shut(fsm1);
    alarm_arm(fsm2);
    portTickType xLastWakeTime;

    while(true) {
      xLastWakeTime = xTaskGetTickCount ();
      fsm_fire(fsm1);
      fsm_fire(fsm2);
      printf("FSM Alarma: %d\nFSM Codigo: %d\n", fsm1->current_state, fsm2->current_state);
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
