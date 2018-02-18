#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"


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

// Number of milliseconds for a dot in morse code.
const int punto_ms = 250;
// Dash is 3 times the length of a dot.
const int raya_ms = 750;
// Delay between parts of a character is the length of a dot.
const int espera_simbolos = 250;
// Delay between letters in a word is the length three dots.
const int espera_entre_letras = 750;
// Delay between words is seven dots.
const int espera_entre_palabras = 1250;

/*
  static const struct {const char letter, *code;} MorseMap[] =
  {
  	{ 'A', ".-" },
  	{ 'B', "-..." },
  	{ 'C', "-.-." },
  	{ 'D', "-.." },
  	{ 'E', "." },
  	{ 'F', "..-." },
  	{ 'G', "--." },
  	{ 'H', "...." },
  	{ 'I', ".." },
  	{ 'J', ".---" },
  	{ 'K', ".-.-" },
  	{ 'L', ".-.." },
  	{ 'M', "--" },
  	{ 'N', "-." },
  	{ 'O', "---" },
  	{ 'P', ".--." },
  	{ 'Q', "--.-" },
  	{ 'R', ".-." },
  	{ 'S', "..." },
  	{ 'T', "-" },
  	{ 'U', "..-" },
  	{ 'V', "...-" },
  	{ 'W', ".--" },
  	{ 'X', "-..-" },
  	{ 'Y', "-.--" },
  	{ 'Z', "--.." },
  	{ ' ', "     " },
    { '1', ".----" },
  	{ '2', "..---" },
  	{ '3', "...--" },
  	{ '4', "....-" },
  	{ '5', "....." },
  	{ '6', "-...." },
  	{ '7', "--..." },
  	{ '8', "---.." },
  	{ '9', "----." },
  	{ '0', "-----" },

  };
*/
  void loop() {
    // Letra H
    while (1){


    for (int j=0;j<4;j++)
    {
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(punto_ms);
    }
    vTaskDelay(espera_entre_letras);
    // Letra O
    for (int i=0;i<3;i++)
    {
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(raya_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(raya_ms);
    }
    vTaskDelay(espera_entre_letras);
    // Letra L
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(raya_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(raya_ms);
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(punto_ms);

    vTaskDelay(espera_entre_letras);
    //Letra A

      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(raya_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(raya_ms);

      vTaskDelay(espera_entre_letras);
      //Espacio entre palabras
      for (int k=0;k<5;k++)
      {
        GPIO_OUTPUT_SET(2, 1);
        vTaskDelay(punto_ms);
        GPIO_OUTPUT_SET(2, 0);
        vTaskDelay(punto_ms);
      }
        vTaskDelay(espera_entre_letras);
      //Letra M
      for (int l=0;l<2;l++)
      {
        GPIO_OUTPUT_SET(2, 1);
        vTaskDelay(raya_ms);
        GPIO_OUTPUT_SET(2, 0);
        vTaskDelay(raya_ms);
      }
      vTaskDelay(espera_entre_letras);
      //Letra u
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(raya_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(raya_ms);

      vTaskDelay(espera_entre_letras);
      //letra n
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(raya_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(raya_ms);
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(punto_ms);

      vTaskDelay(espera_entre_letras);
      // letra d
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(raya_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(raya_ms);
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 1);
      vTaskDelay(punto_ms);
      GPIO_OUTPUT_SET(2, 0);
      vTaskDelay(punto_ms);

            vTaskDelay(espera_entre_letras);


      // Letra O
      for (int p=0;p<3;p++)
      {
        GPIO_OUTPUT_SET(2, 1);
        vTaskDelay(raya_ms);
        GPIO_OUTPUT_SET(2, 0);
        vTaskDelay(raya_ms);
      }
      vTaskDelay(espera_entre_letras);
  }

}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

void user_init(void){
    xTaskCreate(loop, "startup", 2048, NULL, 1, NULL);
}
