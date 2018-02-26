enum fsm2_state {
  LED_ON,
  LED_OFF
};

int button_pressed(fsm_t* this);
int timeout(fsm_t* this);
void led_on(fsm_t* this);
void led_off(fsm_t* this);


static fsm_trans_t interruptor[] = {
  {LED_OFF,button_pressed,LED_ON,led_on},
  {LED_ON,timeout,LED_OFF,led_off},
  {-1, NULL, -1, NULL }
};
