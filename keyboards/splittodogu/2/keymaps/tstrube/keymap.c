// Copyright 2025 Timo Strube (@tstrube)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "analog.h"
#include "eeprom.h"
//#include "joystick.h"
//#include "split_util.h"
#include "transactions.h"

// Default
#define TS_CUT C(KC_X)
#define TS_CPY C(KC_C)
#define TS_PST C(KC_V)
#define TS_LCK G(KC_L)
#define TS_MIC C(S(KC_M))

// Fruit
#define TSF_CUT G(KC_X)
#define TSF_CPY G(KC_C)
#define TSF_PST G(KC_V)
#define TSF_LCK G(C(KC_Q))
#define TSF_MIC G(S(KC_M))

// Joysticks
#define EEPROM_ADDR_JOY_LEFT  0
#define EEPROM_ADDR_JOY_RIGHT 1
#define LED_INDEX_JOY_LEFT    44
#define LED_INDEX_JOY_RIGHT   89
#define JOY_MIN               0
#define JOY_CENTER            512
#define JOY_MAX               1023
#define JOYSTICK_THRESHOLD    200
#define RPC_JOYSTICK_AXES     44

#define LJOY_UP_POS     (keypos_t){5, 0}
#define LJOY_DOWN_POS   (keypos_t){5, 1}
#define LJOY_LEFT_POS   (keypos_t){5, 2}
#define LJOY_RIGHT_POS  (keypos_t){5, 3}
#define LJOY_BTN_POS    (keypos_t){5, 4}

#define RJOY_UP_POS     (keypos_t){5, 5}
#define RJOY_DOWN_POS   (keypos_t){5, 6}
#define RJOY_LEFT_POS   (keypos_t){5, 7}
#define RJOY_RIGHT_POS  (keypos_t){5, 8}
#define RJOY_BTN_POS    (keypos_t){5, 9}

#define JOY_PIN_X GP27
#define JOY_PIN_Y GP28
#define JOY_PIN_B GP29

typedef struct {
    uint16_t x;
    uint16_t y;
} joystick_axes_t;

typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
    bool btn;
} joystick_state_t;

static joystick_axes_t local_joystick = { JOY_CENTER, JOY_CENTER };
static joystick_axes_t remote_joystick = { JOY_CENTER, JOY_CENTER };
static joystick_state_t left_digital_state = {0};
static joystick_state_t right_digital_state = {0};

bool joystick_digital_left = false;
bool joystick_digital_right = false;

enum joy_keycodes {
    TS_JOYL = QK_USER,
    TS_JOYR
};

enum layers {
    DF,
    MC,
    FN
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [DF] = LAYOUT_split_5x8_7(
        KC_ESC ,KC_GRV ,KC_1   ,KC_2   ,KC_3   ,KC_4   ,KC_5   ,KC_6   ,KC_MPLY,KC_MPLY,KC_7   ,KC_8   ,KC_9   ,KC_0   ,KC_MINS,KC_EQL ,KC_BSPC,KC_DEL ,
        TS_CUT ,KC_TAB ,KC_Q   ,KC_W   ,KC_E   ,KC_R   ,KC_T   ,KC_Y   ,KC_MNXT,KC_MNXT,KC_PGUP,KC_PGDN,KC_U   ,KC_I   ,KC_O   ,KC_P   ,KC_LBRC,KC_RBRC,
        TS_CPY ,KC_LSFT,KC_A   ,KC_S   ,KC_D   ,KC_F   ,KC_G   ,KC_HOME,KC_MPRV,KC_MPRV,KC_END ,KC_H   ,KC_J   ,KC_K   ,KC_L   ,KC_SCLN,KC_QUOT,KC_ENT ,
        TS_PST ,KC_LCTL,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_B   ,KC_SLEP,KC_MUTE,KC_MUTE,KC_SLEP,KC_N   ,KC_M   ,KC_COMM,KC_DOT ,KC_SLSH,KC_UP  ,KC_RSFT,
        TS_LCK ,KC_CAPS,MO(FN) ,PDF(MC),KC_LGUI,KC_LALT,KC_SPC ,TS_JOYL,TS_MIC ,TS_MIC ,TS_JOYL,KC_BSPC,KC_PSCR,TG(FN), KC_BSLS,KC_LEFT,KC_DOWN,KC_RGHT,
        KC_F13 ,KC_F14 ,KC_F15 ,KC_F16 ,KC_F17 ,KC_F18 ,KC_F19 ,KC_F20 ,KC_F21 ,KC_F22
    ),
    [MC] = LAYOUT_split_5x8_7(
        KC_ESC ,KC_GRV ,KC_1   ,KC_2   ,KC_3   ,KC_4   ,KC_5   ,KC_6   ,KC_MPLY,KC_MPLY,KC_7   ,KC_8   ,KC_9   ,KC_0   ,KC_MINS,KC_EQL ,KC_BSPC,KC_DEL ,
        TSF_CUT,KC_TAB ,KC_Q   ,KC_W   ,KC_E   ,KC_R   ,KC_T   ,KC_Y   ,KC_MNXT,KC_MPRV,KC_PGUP,KC_PGDN,KC_U   ,KC_I   ,KC_O   ,KC_P   ,KC_LBRC,KC_RBRC,
        TSF_CPY,KC_LSFT,KC_A   ,KC_S   ,KC_D   ,KC_F   ,KC_G   ,KC_HOME,KC_MPRV,KC_MPRV,KC_END ,KC_H   ,KC_J   ,KC_K   ,KC_L   ,KC_SCLN,KC_QUOT,KC_ENT ,
        TSF_PST,KC_LCTL,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_B   ,KC_SLEP,KC_MUTE,KC_MUTE,KC_SLEP,KC_N   ,KC_M   ,KC_COMM,KC_DOT ,KC_SLSH,KC_UP  ,KC_RSFT,
        TSF_LCK,KC_CAPS,MO(FN) ,PDF(DF),KC_LGUI,KC_LALT,KC_SPC ,TS_JOYL,TSF_MIC,TSF_MIC,TS_JOYR,KC_BSPC,KC_PSCR,TG(FN), KC_BSLS,KC_LEFT,KC_DOWN,KC_RGHT,
        KC_F13 ,KC_F14 ,KC_F15 ,KC_F16 ,KC_F17 ,KC_F18 ,KC_F19 ,KC_F20 ,KC_F21 ,KC_F22
    ),
    [FN] = LAYOUT_split_5x8_7(
        RM_TOGG,_______,KC_F1  ,KC_F2  ,KC_F3  ,KC_F4  ,KC_F5  ,KC_F6  ,QK_BOOT,QK_BOOT,KC_F7  ,KC_F8  ,KC_F9  ,KC_F10 ,KC_F11 ,KC_F12 ,KC_PMNS,KC_PSLS,
        _______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,KC_P7  ,KC_P8  ,KC_P9  ,KC_PPLS,KC_PSLS,
        RM_NEXT,RM_HUEU,RM_SATU,RM_VALU,RM_SPDU,_______,_______,_______,_______,_______,_______,_______,_______,KC_P4  ,KC_P5  ,KC_P6  ,KC_PPLS,KC_PAST,
        RM_PREV,RM_HUED,RM_SATD,RM_VALD,RM_SPDD,_______,_______,_______,_______,_______,_______,_______,_______,KC_P1  ,KC_P2  ,KC_P3  ,KC_PENT,KC_PAST,
        _______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,KC_P0  ,KC_COMM,KC_PENT,KC_NUM ,
        _______,_______,_______,_______,_______,_______,_______,_______,_______,_______
    )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
            case TS_JOYL:
                joystick_digital_left = !joystick_digital_left;
                eeprom_update_byte((uint8_t*)EEPROM_ADDR_JOY_LEFT, joystick_digital_left);
                return false;

            case TS_JOYR:
                joystick_digital_right = !joystick_digital_right;
                eeprom_update_byte((uint8_t*)EEPROM_ADDR_JOY_RIGHT, joystick_digital_right);
                return false;
        }
    }
    return true;
}

bool rgb_matrix_indicators_user(void) {
    if (joystick_digital_left) {
        rgb_matrix_set_color(LED_INDEX_JOY_LEFT, 0x0,  0xFF, 0x00);
    } else {
        rgb_matrix_set_color(LED_INDEX_JOY_LEFT, 0xFF, 0x0,  0x00);
    }

    if (joystick_digital_right) {
        rgb_matrix_set_color(LED_INDEX_JOY_RIGHT, 0x0,  0xFF, 0x00);
    } else {
        rgb_matrix_set_color(LED_INDEX_JOY_RIGHT, 0xFF, 0x0,  0x00);
    }

    return false;
}

void process_digital_joystick(int x, int y, bool btn, joystick_state_t* state,
                              keypos_t up, keypos_t down, keypos_t left, keypos_t right, keypos_t btn_pos) {
    int layer = get_highest_layer(layer_state | default_layer_state);

    // UP
    if (y > JOYSTICK_DEADZONE && !state->up) {
        register_code16(keymap_key_to_keycode(layer,  up));
        state->up = true;
    } else if (y <= JOYSTICK_DEADZONE && state->up) {
        unregister_code16(keymap_key_to_keycode(layer,  up));
        state->up = false;
    }

    // DOWN
    if (y < -JOYSTICK_DEADZONE && !state->down) {
        register_code16(keymap_key_to_keycode(layer,  down));
        state->down = true;
    } else if (y >= -JOYSTICK_DEADZONE && state->down) {
        unregister_code16(keymap_key_to_keycode(layer,  down));
        state->down = false;
    }

    // RIGHT
    if (x > JOYSTICK_DEADZONE && !state->right) {
        register_code16(keymap_key_to_keycode(layer,  right));
        state->right = true;
    } else if (x <= JOYSTICK_DEADZONE && state->right) {
        unregister_code16(keymap_key_to_keycode(layer,  right));
        state->right = false;
    }

    // LEFT
    if (x < -JOYSTICK_DEADZONE && !state->left) {
        register_code16(keymap_key_to_keycode(layer,  left));
        state->left = true;
    } else if (x >= -JOYSTICK_DEADZONE && state->left) {
        unregister_code16(keymap_key_to_keycode(layer,  left));
        state->left = false;
    }

    // BUTTON
    if (btn && !state->btn) {
        register_code16(keymap_key_to_keycode(layer,  btn_pos));
        state->btn = true;
    } else if (!btn && state->btn) {
        unregister_code16(keymap_key_to_keycode(layer,  btn_pos));
        state->btn = false;
    }
}

void matrix_scan_user(void) {
    // this function handles both sides (master / slave) using the same pins
    // only the relevant sections are getting called
    // slave sends axis data to the master, which forwards it
    // analog and digital mode are also handled at the same time

    // read pins
    local_joystick.x = analogReadPin(JOY_PIN_X) - JOY_CENTER;
    local_joystick.y = analogReadPin(JOY_PIN_Y) - JOY_CENTER;
    bool btn = !readPin(JOY_PIN_B); // Active low

    // cache status
    bool is_left = is_keyboard_left();
    bool is_master = is_keyboard_master();

    // process left side
    if (is_left) {
        // digital mode
        if (joystick_digital_left) {
            process_digital_joystick(local_joystick.x, local_joystick.y, btn, &left_digital_state,
                (keypos_t){5, 0}, (keypos_t){5, 1}, (keypos_t){5, 2},
                (keypos_t){5, 3}, (keypos_t){5, 4});
        }
        // analog mode 
        else {
            if (btn) {
                register_code16(JS_0);
            } else {
                unregister_code16(JS_0);
            }
        }
    }
    // process right side
    else {
        // digital mode
        if (joystick_digital_right) {
            process_digital_joystick(local_joystick.x, local_joystick.y, btn, &right_digital_state,
                (keypos_t){5, 5}, (keypos_t){5, 6}, (keypos_t){5, 7},
                (keypos_t){5, 8}, (keypos_t){5, 9});
        }
        // analog mode 
        else {
            if (btn) {
                register_code16(JS_1);
            } else {
                unregister_code16(JS_1);
            }
        }
    }

    // handle received data
    if (is_master) {
        joystick_set_axis(0, joystick_digital_left  ? 0 : remote_joystick.x); // X
        joystick_set_axis(1, joystick_digital_left  ? 0 : remote_joystick.y); // Y
        joystick_set_axis(2, joystick_digital_right ? 0 : local_joystick.x);  // Z
        joystick_set_axis(3, joystick_digital_right ? 0 : local_joystick.y);  // Rz
    }
    // send data to master
    else {
        transaction_rpc_send(RPC_JOYSTICK_AXES, sizeof(local_joystick), &local_joystick);
    }
}

void receive_joystick_axes(uint8_t type, const void *data, uint8_t size, void *context) {
    if (size == sizeof(joystick_axes_t)) {
        memcpy(&remote_joystick, data, size);
    }
}

void keyboard_post_init_user(void) {
    joystick_digital_left  = eeprom_read_byte((uint8_t*)EEPROM_ADDR_JOY_LEFT);
    joystick_digital_right = eeprom_read_byte((uint8_t*)EEPROM_ADDR_JOY_RIGHT);
    transaction_register_rpc(RPC_JOYSTICK_AXES, receive_joystick_axes);
}
