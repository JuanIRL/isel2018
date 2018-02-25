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

volatile int flag_armed = 0;
volatile int flag_boton = 0;
volatile int flag_presence = 0;

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
  if(flag_presence){
    printf("Detectada presencia\n");
    return 1;
  }
  return 0;
};

int alarm_on(fsm_t *this){
  return(flag_armed);
};

void alarm_sound(fsm_t *this){
  printf("Sonando alarma\n");
  GPIO_OUTPUT_SET(GPIO_LED, 0);

};

void alarm_shut(fsm_t *this){
  printf("Silenciando alarma\n");
  GPIO_OUTPUT_SET(GPIO_LED, 1);
  flag_armed = 0;

};
static fsm_trans_t alarma[] = {
  {DESARMADO,alarm_on,ARMADO,alarm_shut},
  {ARMADO, alarm_on, DESARMADO, alarm_shut},
  {ARMADO, presencia, ARMADO, alarm_sound},
  {-1, NULL, -1, NULL }
};

int button_pressed (fsm_t *this){
  return (flag_boton);
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
    flag_armed = 1;
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

//Quitar rebotes
void isr_gpio (void* arg){
  if(xTaskGetTickCount()*portTICK_RATE_MS > debounceTime){
    uint32 status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    if(status & BIT(0)){ //Mirar que bit indica el gpio que ha generado interrupcion
      /**-------*/
    }
    debounceTime = xTaskGetTickCount()*portTICK_RATE_MS + 180;
  }
}

void alarm(void* ignore)
{
    fsm_t* fsm1 = fsm_new(alarma);
    alarm_shut(fsm1);
    portTickType xLastWakeTime;
    while(true) {
      xLastWakeTime = xTaskGetTickCount ();
      fsm_fire(fsm1);
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
