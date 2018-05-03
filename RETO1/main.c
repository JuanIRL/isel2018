#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "fsm.h"

#define TIEMPO_PRESENCIA  2000/portTICK_RATE_MS

#define num_secciones 2
int carga_actual;
int seccion_activa;
int th_up;
int th_down;
int presencias[num_secciones];
int luces[num_secciones];
int vent_ext;
int vent_in;
int secciones[num_secciones];
int temp_ext_ok;
int humedad_ok;
int index_secciones;
int delta_carga = 100/num_secciones;


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
}

#define PERIOD_TICK 100/portTICK_RATE_MS
enum fsm_state {
  OFF,
  ON_INTERIOR,
  ON_EXTERIOR,
};

int carga_up (fsm_t *self) {
  return (carga_actual > th_up);
}

int carga_down (fsm_t *self) {
  return (carga_actual < th_down);
}

int entorno (fsm_t *self) {
  return (temp_ext_ok && humedad_ok);
}

int presencia (fsm_t *self) {
  for(int i=0; i < sizeof(presencias)/sizeof(presencias[0]); i++){
    if(presencias[i]){
      return(1);
    }
  }
  return (0);
}

int timeout (fsm_t *self) {
  static portTickType xLastTickControl = 0;
  int done_timeout = 0;
  portTickType now = xTaskGetTickCount ();
  if (now > xLastTickControl) {
    xLastTickControl = now + TIEMPO_PRESENCIA;
     done_timeout= 1;
  }
  return (done_timeout);
}

void umbral (){
  th_up = delta_carga*seccion_activa;
  th_down = delta_carga*(seccion_activa-1);
}

void on_next_in(fsm_t *self){
  ++seccion_activa;
  secciones[++index_secciones]++;
  vent_in = 1;
  vent_ext = 0;
  umbral();
}

void on_next_ext(fsm_t *self){
  ++seccion_activa;
  secciones[++index_secciones]++;
  vent_in = 0;
  vent_ext = 1;
  umbral();
}

void on_next(fsm_t *self){
  ++seccion_activa;
  secciones[++index_secciones]++;
  umbral();
}

void on_prev (fsm_t *self){
  --seccion_activa;
  --secciones[index_secciones--];
  umbral();
}

void off (fsm_t *self){
  --seccion_activa;
  --secciones[index_secciones--];
  vent_in = 0;
  vent_ext = 0;
  umbral();
}

void ext_open (fsm_t *self){
  vent_in = 0;
  vent_ext = 1;
}

void ext_close (fsm_t *self){
  vent_in = 1;
  vent_ext = 0;
}

/*
 * Máquina de estados: lista de transiciones
 * { EstadoOrigen, CondicionDeDisparo, EstadoFinal, AccionesSiTransicion }
 */
static fsm_trans_t control[] = {
  { OFF, presencia && timeout, OFF,  actualizar_luces },
  { OFF, carga_up && !entorno, ON_INTERIOR,  on_next_in },
  { OFF, carga_up && entorno, ON_EXTERIOR,  on_next_ext },

  { ON_INTERIOR, carga_down && seccion = 1, OFF,  off },
  { ON_INTERIOR, carga_down && !seccion = 1, ON_INTERIOR,  on_prev },
  { ON_INTERIOR, carga_up, ON_INTERIOR,  on_next },
  { ON_INTERIOR, presencia && timeout, ON_INTERIOR,  actualizar_luces },
  { ON_INTERIOR, entorno, ON_EXTERIOR,  ext_open },

  { ON_EXTERIOR, carga_down && seccion = 1, OFF,  off },
  { ON_EXTERIOR, carga_down && !seccion = 1, ON_EXTERIOR,  on_prev },
  { ON_EXTERIOR, carga_up, ON_EXTERIOR,  on_next },
  { ON_EXTERIOR, presencia && timeout, ON_EXTERIOR,  actualizar_luces },
  { ON_EXTERIOR, entorno, ON_INTERIOR,  ext_open },

  {-1, NULL, -1, NULL },
};

void start(void* ignore)
{
    fsm_t* fsm = fsm_new(control);
    led_off(fsm);
    portTickType xLastWakeTime;

    while(true) {
      xLastWakeTime = xTaskGetTickCount ();
      fsm_fire(fsm);
      vTaskDelayUntil(&xLastWakeTime, PERIOD_TICK);
    }
}

void user_init(void)
{
    xTaskCreate (start, "start", 2048, NULL, 1, NULL);
}
