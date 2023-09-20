#include "user_input.h"

#include "Arduino.h"

#define USERINPUT_BIT_I_LEFT     (_BV(0))
#define USERINPUT_BIT_I_RIGHT    (_BV(1))
#define USERINPUT_BIT_I_CLICK    (_BV(2))
#define USERINPUT_BIT_I_FAST     (_BV(3))

#define USERINPUT_BIT_V_LEFT     (_BV(4))
#define USERINPUT_BIT_V_RIGHT    (_BV(5))
#define USERINPUT_BIT_V_CLICK    (_BV(6))
#define USERINPUT_BIT_V_FAST     (_BV(7))

#define USERINPUT_PIN_I_A       PD2
#define USERINPUT_PIN_I_B       PD3
#define USERINPUT_PIN_I_CLICK   PD4
#define USERINPUT_PIN_V_A       PD5
#define USERINPUT_PIN_V_B       PD6
#define USERINPUT_PIN_V_CLICK   PD7
#define USERINPUT_PIN_ONOFF     8

volatile uint8_t user_input_flags = 0;
volatile bool user_input_onoff_button_pressed = false;

volatile uint8_t userinput_i_lastStateA = LOW;
volatile bool userinput_i_turnFlag = false;

volatile uint8_t userinput_v_lastStateA = LOW;
volatile bool userinput_v_turnFlag = false;

volatile bool userinput_i_clickFlag = false;
volatile bool userinput_v_clickFlag = false;

#define USERINPUT_SETFLAG(flag)   user_input_flags |= flag
#define USERINPUT_RESETFLAG(flag) user_input_flags &= ~(flag)

void isrcall_process_encoder(bool first);

ISR(PCINT0_vect) {
  if (digitalRead(USERINPUT_PIN_ONOFF) == HIGH) {
    user_input_onoff_button_pressed = true;
  }
}

ISR(PCINT2_vect) {
  if (digitalRead(USERINPUT_PIN_I_CLICK) == HIGH) {
    userinput_i_clickFlag = true;
  } else if (userinput_i_clickFlag) {
    USERINPUT_SETFLAG(USERINPUT_BIT_I_CLICK);
    userinput_i_clickFlag = false; 
  }

  if (digitalRead(USERINPUT_PIN_V_CLICK) == HIGH) {
    userinput_v_clickFlag = true;
  } else if (userinput_v_clickFlag) {
    USERINPUT_SETFLAG(USERINPUT_BIT_V_CLICK);
    userinput_v_clickFlag = false; 
  }

  uint8_t stateiA = digitalRead(USERINPUT_PIN_I_A);
  uint8_t statevA = digitalRead(USERINPUT_PIN_V_A);

  if (stateiA != userinput_i_lastStateA) {
    userinput_i_lastStateA = stateiA;
    userinput_i_turnFlag   = !userinput_i_turnFlag;
    if (userinput_i_turnFlag) {
      if (digitalRead(USERINPUT_PIN_I_B) != stateiA) {
        if (user_input_flags & USERINPUT_BIT_I_LEFT) {
          USERINPUT_SETFLAG(USERINPUT_BIT_I_FAST);
        } else {
          USERINPUT_SETFLAG(USERINPUT_BIT_I_LEFT);
          USERINPUT_RESETFLAG(USERINPUT_BIT_I_RIGHT & USERINPUT_BIT_I_FAST);
        }
      } else {
        if (user_input_flags & USERINPUT_BIT_I_RIGHT) {
          USERINPUT_SETFLAG(USERINPUT_BIT_I_FAST);
        } else {
          USERINPUT_SETFLAG(USERINPUT_BIT_I_RIGHT);
          USERINPUT_RESETFLAG(USERINPUT_BIT_I_LEFT & USERINPUT_BIT_I_FAST);
        }
      }
    }
  }

  if (statevA != userinput_v_lastStateA) {
    userinput_v_lastStateA = statevA;
    userinput_v_turnFlag   = !userinput_v_turnFlag;
    if (userinput_v_turnFlag) {
      if (digitalRead(USERINPUT_PIN_V_B) != statevA) {
        if (user_input_flags & USERINPUT_BIT_V_LEFT) {
          USERINPUT_SETFLAG(USERINPUT_BIT_V_FAST);
        } else {
          USERINPUT_SETFLAG(USERINPUT_BIT_V_LEFT);
          USERINPUT_RESETFLAG(USERINPUT_BIT_V_RIGHT & USERINPUT_BIT_V_FAST);
        }
      } else {
        if (user_input_flags & USERINPUT_BIT_V_RIGHT) {
          USERINPUT_SETFLAG(USERINPUT_BIT_V_FAST);
        } else {
          USERINPUT_SETFLAG(USERINPUT_BIT_V_RIGHT);
          USERINPUT_RESETFLAG(USERINPUT_BIT_V_LEFT & USERINPUT_BIT_V_FAST);
        }
      }
    }
  }
}

void user_input_init() {
  pinMode(USERINPUT_PIN_I_A,      INPUT_PULLUP);
  pinMode(USERINPUT_PIN_I_B,      INPUT_PULLUP);
  pinMode(USERINPUT_PIN_I_CLICK,  INPUT_PULLUP);

  pinMode(USERINPUT_PIN_V_A,      INPUT_PULLUP);
  pinMode(USERINPUT_PIN_V_B,      INPUT_PULLUP);
  pinMode(USERINPUT_PIN_V_CLICK,  INPUT_PULLUP);

  pinMode(USERINPUT_PIN_ONOFF, INPUT);

  PCICR =  _BV(PCIE0) | _BV(PCIE2);
  PCMSK0 = _BV(PCINT0);
  PCMSK2 = _BV(PCINT18) | _BV(PCINT19) | _BV(PCINT20) | _BV(PCINT21) | _BV(PCINT22) | _BV(PCINT23);
}

bool user_input_onoff_pressed() {
  bool result = false;

  uint8_t oldSREG = SREG;
  cli();

  result = user_input_onoff_button_pressed;
  user_input_onoff_button_pressed = false;
  
  SREG = oldSREG;
  
  return result;
}

uint8_t user_input_encoder_i_status() {
  uint8_t result = USER_INPUT_STATUS_NONE;

  uint8_t oldSREG = SREG;
  cli();

  if (user_input_flags & USERINPUT_BIT_I_CLICK) {
    USERINPUT_RESETFLAG(USERINPUT_BIT_I_CLICK);
    result = USER_INPUT_STATUS_CLICK;
  } else if (user_input_flags & USERINPUT_BIT_I_LEFT) {
    if (user_input_flags & USERINPUT_BIT_I_FAST) {
      USERINPUT_RESETFLAG(USERINPUT_BIT_I_FAST);
      result = USER_INPUT_STATUS_LEFT_FAST;
    } else {
      USERINPUT_RESETFLAG(USERINPUT_BIT_I_LEFT);
      result = USER_INPUT_STATUS_LEFT;
    }
  } else if (user_input_flags & USERINPUT_BIT_I_RIGHT) {
    if (user_input_flags & USERINPUT_BIT_I_FAST) {
      USERINPUT_RESETFLAG(USERINPUT_BIT_I_FAST);
      result = USER_INPUT_STATUS_RIGHT_FAST;
    } else {
      USERINPUT_RESETFLAG(USERINPUT_BIT_I_LEFT);
      result = USER_INPUT_STATUS_RIGHT;
    }
  }

  SREG = oldSREG;
  
  return result;
}

uint8_t user_input_encoder_v_status() {
  uint8_t result = USER_INPUT_STATUS_NONE;

  uint8_t oldSREG = SREG;
  cli();

  if (user_input_flags & USERINPUT_BIT_V_CLICK) {
    USERINPUT_RESETFLAG(USERINPUT_BIT_V_CLICK);
    result = USER_INPUT_STATUS_CLICK;
  } else if (user_input_flags & USERINPUT_BIT_V_LEFT) {
    if (user_input_flags & USERINPUT_BIT_V_FAST) {
      USERINPUT_RESETFLAG(USERINPUT_BIT_V_FAST);
      result = USER_INPUT_STATUS_LEFT_FAST;
    } else {
      USERINPUT_RESETFLAG(USERINPUT_BIT_V_LEFT);
      result = USER_INPUT_STATUS_LEFT;
    }
  } else if (user_input_flags & USERINPUT_BIT_V_RIGHT) {
    if (user_input_flags & USERINPUT_BIT_V_FAST) {
      USERINPUT_RESETFLAG(USERINPUT_BIT_V_FAST);
      result = USER_INPUT_STATUS_RIGHT_FAST;
    } else {
      USERINPUT_RESETFLAG(USERINPUT_BIT_V_LEFT);
      result = USER_INPUT_STATUS_RIGHT;
    }
  }

  SREG = oldSREG;
  
  return result;
}
