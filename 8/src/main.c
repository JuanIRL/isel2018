#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "stdio.h"
#include <fsm.h>

#define ETS_GPIO_INTR_ENABLE() \
    _xt_isr_unmask(1 << ETS_GPIO_INUM)
#define ETS_GPIO_INTR_DISABLE() \
    _xt_isr_mask(1 << ETS_GPIO_INUM)

#define PERIOD_TICK 200/portTICK_RATE_MS

#define GPIO_LED 2
#define GPIO_CODE 13
#define GPIO_PRESENCE 4

long debounceTime = 0;
long code_next_timeout = 0;

volatile int flag_boton = 0;
volatile int flag_presence = 0;

int code_pass [] = {1,2,3};
int code [3] = {-1,-1,-1};
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
  ARMADO
};

int mirar_flags (fsm_t* this){
  if(indice > 2 || code[indice] > 10) {
    return 0;
  }
  return flag_boton;
}

int timeout_valid (fsm_t* this){
  if(code_next_timeout > 0 && xTaskGetTickCount()*portTICK_RATE_MS >= code_next_timeout){
    return 1;
  }
  return 0;
}

int codigo_correcto(fsm_t* this){
  if(indice > 2){
    int j;
    int correcto = 1;
    for(j = 0; j < 3; j++){
      if(code[j] != code_pass[j]){
        correcto = 0;
      }
    }
    code[0] = -1;
    code[1] = -1;
    code[2] = -1;
    if(correcto){
      printf("Codigo correcto!\n");
    }
    return correcto;
  }
  return 0;
}

int codigo_incorrecto(fsm_t* this){
  if(indice > 2){
    int j;
    int correcto = 1;
    for(j = 0; j < 3; j++){
      if(code[j] != code_pass[j]){
        correcto = 0;
      }
    }
    code[0] = -1;
    code[1] = -1;
    code[2] = -1;
    if(!correcto){
      printf("Codigo erroneo!\n");
    }
    return !correcto;
  }
  return 0;
}
int presencia (fsm_t *this) {
  if(flag_presence){
    printf("Detectada presencia\n");
    return 1;
  }
  return 0;
};

void update_code (fsm_t* this){
  code[indice]++;
  flag_boton=0;
  code_next_timeout = xTaskGetTickCount()*portTICK_RATE_MS + 1000;
}

void next_index(fsm_t* this){
  code_next_timeout = 0;
  printf("Indice: %d -> %d\nCodigo: [%d,%d,%d]\n", indice, (indice + 1), code[0], code[1], code[2]);
  indice++;
}

void limpiar_flag(fsm_t* this){
  flag_boton = 0;
  flag_presence = 0;
  if(indice > 2){
    indice = 0;
  }
}
void alarm_sound(fsm_t *this){
  printf("Sonando alarma\n");
  GPIO_OUTPUT_SET(GPIO_LED, 0);
};

void alarm_shut(fsm_t *this){
  printf("Silenciando alarma\n");
  GPIO_OUTPUT_SET(GPIO_LED, 1);
  if(indice > 2){
    indice = 0;
  }
};
static fsm_trans_t alarma[] = {
  {DESARMADO, codigo_correcto, ARMADO, limpiar_flag},
  {DESARMADO, codigo_incorrecto, DESARMADO, limpiar_flag},
  {DESARMADO, mirar_flags, DESARMADO, update_code},
  {DESARMADO, timeout_valid, DESARMADO, next_index},
  {ARMADO, codigo_correcto, DESARMADO, alarm_shut},
  {ARMADO, codigo_incorrecto, ARMADO, limpiar_flag},
  {ARMADO, mirar_flags, ARMADO, update_code},
  {ARMADO, timeout_valid, ARMADO, next_index},
  {ARMADO, presencia, ARMADO, alarm_sound},
  {-1, NULL, -1, NULL }
};

int timeout (fsm_t *this) {
  if(xTaskGetTickCount()*portTICK_RATE_MS > code_next_timeout){
    printf("Tiempo de simbolo terminado\n");
    return 1;
  } else{
    return 0;
  }
}

void isr_gpio (void* arg){
  uint32 status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
  if(xTaskGetTickCount()*portTICK_RATE_MS > debounceTime){
    if(status & BIT(GPIO_CODE)){
      printf("Detectada pulsacion de codigo\n");
      flag_boton = 1;
    }
    debounceTime = xTaskGetTickCount()*portTICK_RATE_MS + 180;
  }
  if(status & BIT(GPIO_PRESENCE)){
    flag_presence = 1;
  }
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status);
}

void alarm(void* ignore)
{
    fsm_t* fsm1 = fsm_new(alarma);
    alarm_shut(fsm1);
    portTickType xLastWakeTime;
    gpio_intr_handler_register((void*)isr_gpio, NULL);

    //gpio_pin_intr_state_set(GPIO_PRESENCE, GPIO_PIN_INTR_POSEDGE);//Pull-Up, conectar a GND
    //gpio_pin_intr_state_set(GPIO_CODE, GPIO_PIN_INTR_POSEDGE);//Pull-Down, conectar a 3.3V

    GPIO_ConfigTypeDef io_conf;
    io_conf.GPIO_IntrType = GPIO_PIN_INTR_POSEDGE;
    io_conf.GPIO_Mode = GPIO_Mode_Input;
    io_conf.GPIO_Pin = BIT(GPIO_CODE);
    io_conf.GPIO_Pullup = GPIO_PullUp_EN;
    gpio_config(&io_conf);

    GPIO_ConfigTypeDef io_conf2;
    io_conf2.GPIO_IntrType = GPIO_PIN_INTR_POSEDGE;
    io_conf2.GPIO_Mode = GPIO_Mode_Input;
    io_conf2.GPIO_Pin = BIT(GPIO_PRESENCE);
    io_conf2.GPIO_Pullup = GPIO_PullUp_EN;
    gpio_config(&io_conf2);

    ETS_GPIO_INTR_ENABLE();
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
