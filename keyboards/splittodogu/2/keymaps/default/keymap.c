// Copyright 2025 Your Name (@tstrube)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "analog.h"
#include "color.h"
#include "eeprom.h"
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
#define LED_INDEX_JOY_LEFT    6
#define LED_INDEX_JOY_RIGHT   6
#define JOY_CENTER            512
#define JOYSTICK_DEADZONE     200
#define JOYSTICK_RELEASE      150
#define RPC_JOYSTICK_AXES     44

#define LJOY_UP_POS    (keypos_t){0, 5}
#define LJOY_DOWN_POS  (keypos_t){1, 5}
#define LJOY_LEFT_POS  (keypos_t){2, 5}
#define LJOY_RIGHT_POS (keypos_t){3, 5}
#define LJOY_BTN_POS   (keypos_t){4, 5}

#define RJOY_UP_POS    (keypos_t){5, 5}
#define RJOY_DOWN_POS  (keypos_t){6, 5}
#define RJOY_LEFT_POS  (keypos_t){7, 5}
#define RJOY_RIGHT_POS (keypos_t){8, 5}
#define RJOY_BTN_POS   (keypos_t){0, 11}

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

static joystick_axes_t remote_joystick = { JOY_CENTER, JOY_CENTER };
static joystick_state_t left_digital_state  = {0};
static joystick_state_t right_digital_state = {0};

bool joystick_digital_left  = true;
bool joystick_digital_right = true;

enum joy_keycodes {
    TS_JOYL = SAFE_RANGE,
    TS_JOYR
};

enum layers {
    DF,
    MC,
    FN
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [DF] = LAYOUT(
        KC_ESC ,KC_GRV ,KC_1   ,KC_2   ,KC_3   ,KC_4   ,KC_5   ,KC_6   ,KC_MPLY,KC_MPLY,KC_7   ,KC_8   ,KC_9   ,KC_0   ,KC_MINS,KC_EQL ,KC_BSPC,KC_DEL ,
        TS_CUT ,KC_TAB ,KC_Q   ,KC_W   ,KC_E   ,KC_R   ,KC_T   ,KC_HOME,KC_MNXT,KC_MNXT,KC_PGUP,KC_Y   ,KC_U   ,KC_I   ,KC_O   ,KC_P   ,KC_LBRC,KC_RBRC,
        TS_CPY ,KC_CAPS,KC_A   ,KC_S   ,KC_D   ,KC_F   ,KC_G   ,KC_END ,KC_MPRV,KC_MPRV,KC_PGDN,KC_H   ,KC_J   ,KC_K   ,KC_L   ,KC_SCLN,KC_QUOT,KC_ENT ,
        TS_PST ,KC_LSFT,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_B   ,KC_SLEP,KC_MUTE,KC_MUTE,KC_SLEP,KC_N   ,KC_M   ,KC_COMM,KC_DOT ,KC_SLSH,KC_UP  ,KC_RSFT,
        TS_LCK ,KC_LCTL,MO(FN) ,PDF(MC),KC_LGUI,KC_LALT,KC_SPC ,TS_JOYL,TS_MIC ,TS_MIC ,TS_JOYR,KC_BSPC,KC_PSCR,TG(FN) ,KC_BSLS,KC_LEFT,KC_DOWN,KC_RGHT,
        KC_F14 ,KC_F15 ,KC_F16 ,KC_F17 ,KC_F18 ,KC_F19 ,KC_F20 ,KC_F21 ,KC_F22 ,KC_F23
    ),
    [MC] = LAYOUT(
        KC_ESC ,KC_GRV ,KC_1   ,KC_2   ,KC_3   ,KC_4   ,KC_5   ,KC_6   ,KC_MPLY,KC_MPLY,KC_7   ,KC_8   ,KC_9   ,KC_0   ,KC_MINS,KC_EQL ,KC_BSPC,KC_DEL ,
        TSF_CUT,KC_TAB ,KC_Q   ,KC_W   ,KC_E   ,KC_R   ,KC_T   ,KC_HOME,KC_MNXT,KC_MNXT,KC_PGUP,KC_Y   ,KC_U   ,KC_I   ,KC_O   ,KC_P   ,KC_LBRC,KC_RBRC,
        TSF_CPY,KC_CAPS,KC_A   ,KC_S   ,KC_D   ,KC_F   ,KC_G   ,KC_END ,KC_MPRV,KC_MPRV,KC_PGDN,KC_H   ,KC_J   ,KC_K   ,KC_L   ,KC_SCLN,KC_QUOT,KC_ENT ,
        TSF_PST,KC_LSFT,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_B   ,KC_SLEP,KC_MUTE,KC_MUTE,KC_SLEP,KC_N   ,KC_M   ,KC_COMM,KC_DOT ,KC_SLSH,KC_UP  ,KC_RSFT,
        TSF_LCK,KC_LCTL,MO(FN) ,PDF(DF),KC_LGUI,KC_LALT,KC_SPC ,TS_JOYL,TSF_MIC,TSF_MIC,TS_JOYR,KC_BSPC,KC_PSCR,TG(FN) ,KC_BSLS,KC_LEFT,KC_DOWN,KC_RGHT,
        KC_F14 ,KC_F15 ,KC_F16 ,KC_F17 ,KC_F18 ,KC_F19 ,KC_F20 ,KC_F21 ,KC_F22 ,KC_F23
    ),
    [FN] = LAYOUT(
        RM_TOGG,_______,KC_F1  ,KC_F2  ,KC_F3  ,KC_F4  ,KC_F5  ,KC_F6  ,QK_BOOT,QK_BOOT,KC_F7  ,KC_F8  ,KC_F9  ,KC_F10 ,KC_F11 ,KC_F12 ,KC_PMNS,KC_PSLS,
        _______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,KC_P7  ,KC_P8  ,KC_P9  ,KC_PPLS,KC_PSLS,
        RM_NEXT,RM_SPDU,RM_HUEU,RM_SATU,RM_VALU,_______,_______,_______,_______,_______,_______,_______,_______,KC_P4  ,KC_P5  ,KC_P6  ,KC_PPLS,KC_PAST,
        RM_PREV,RM_SPDD,RM_HUED,RM_SATD,RM_VALD,_______,_______,_______,_______,_______,_______,_______,_______,KC_P1  ,KC_P2  ,KC_P3  ,KC_PENT,KC_PAST,
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

void set_status_led(uint8_t i, bool digital) {
    hsv_t hsv = digital ? (hsv_t){HSV_BLUE} : (hsv_t){HSV_YELLOW};
    int brightness = rgb_matrix_get_val();
    if (hsv.v > brightness) hsv.v = brightness;
    rgb_t rgb = hsv_to_rgb(hsv);
    rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (is_keyboard_left()) {
        set_status_led(LED_INDEX_JOY_LEFT, joystick_digital_left);
    } else {
        set_status_led(LED_INDEX_JOY_RIGHT, joystick_digital_right);
    }
    return false;
}

static void handle_joystick(bool digital_mode, joystick_axes_t raw, bool btn_pressed,
                            joystick_state_t* state, uint8_t analog_index, int js_btn,
                            keypos_t up, keypos_t down, keypos_t left, keypos_t right, keypos_t btn) {
    int16_t x = raw.x - JOY_CENTER;
    int16_t y = raw.y - JOY_CENTER;

    layer_state_t layer = get_highest_layer(layer_state | default_layer_state);

    if (digital_mode) { // DIGITAL mode
        // UP
        if (y > JOYSTICK_DEADZONE && !state->up) {
            register_code16(keymap_key_to_keycode(layer, up));
            state->up = true;
        } else if (y < JOYSTICK_RELEASE && state->up) {
            unregister_code16(keymap_key_to_keycode(layer, up));
            state->up = false;
        }

        // DOWN
        if (y < -JOYSTICK_DEADZONE && !state->down) {
            register_code16(keymap_key_to_keycode(layer, down));
            state->down = true;
        } else if (y > -JOYSTICK_RELEASE && state->down) {
            unregister_code16(keymap_key_to_keycode(layer, down));
            state->down = false;
        }

        // LEFT
        if (x > JOYSTICK_DEADZONE && !state->left) {
            register_code16(keymap_key_to_keycode(layer, left));
            state->left = true;
        } else if (x < JOYSTICK_RELEASE && state->left) {
            unregister_code16(keymap_key_to_keycode(layer, left));
            state->left = false;
        }

        // RIGHT
        if (x < -JOYSTICK_DEADZONE && !state->right) {
            register_code16(keymap_key_to_keycode(layer, right));
            state->right = true;
        } else if (x > -JOYSTICK_RELEASE && state->right) {
            unregister_code16(keymap_key_to_keycode(layer, right));
            state->right = false;
        }

        // BUTTON
        if (btn_pressed && !state->btn) {
            register_code16(keymap_key_to_keycode(layer, btn));
            state->btn = true;
        } else if (!btn_pressed && state->btn) {
            unregister_code16(keymap_key_to_keycode(layer, btn));
            state->btn = false;
        }

    } else { // ANALOG mode
        // clear any lingering digital keycodes
        if (state->up)    { unregister_code16(keymap_key_to_keycode(layer, up));    state->up = false; }
        if (state->down)  { unregister_code16(keymap_key_to_keycode(layer, down));  state->down = false; }
        if (state->left)  { unregister_code16(keymap_key_to_keycode(layer, left));  state->left = false; }
        if (state->right) { unregister_code16(keymap_key_to_keycode(layer, right)); state->right = false; }
        if (state->btn)   { unregister_code16(keymap_key_to_keycode(layer, btn));   state->btn = false; }

        if (btn_pressed && !state->btn) {
            register_code16(js_btn);
            state->btn = true;
        } else if (!btn_pressed && state->btn) {
            unregister_code16(js_btn);
            state->btn = false;
        }
    }
}

void matrix_scan_user(void) {
    joystick_axes_t raw = {
        .x = analogReadPin(JOY_PIN_X),
        .y = analogReadPin(JOY_PIN_Y)
    };
    bool btn = !readPin(JOY_PIN_B); // Active low

    if (is_keyboard_left()) {
        handle_joystick(joystick_digital_left , raw, btn, &left_digital_state , 0, JS_0,
                        LJOY_UP_POS, LJOY_DOWN_POS, LJOY_LEFT_POS, LJOY_RIGHT_POS, LJOY_BTN_POS);
    } else {
        handle_joystick(joystick_digital_right, raw, btn, &right_digital_state, 2, JS_1,
                        RJOY_UP_POS, RJOY_DOWN_POS, RJOY_LEFT_POS, RJOY_RIGHT_POS, RJOY_BTN_POS);
    }

    if (is_keyboard_master()) {
        if (is_keyboard_left()) {
            joystick_set_axis(0, joystick_digital_left  ? 0 : raw.x);
            joystick_set_axis(1, joystick_digital_left  ? 0 : raw.y);
            joystick_set_axis(2, joystick_digital_right ? 0 : remote_joystick.x);
            joystick_set_axis(3, joystick_digital_right ? 0 : remote_joystick.y);
        } else {
            joystick_set_axis(0, joystick_digital_left  ? 0 : remote_joystick.x);
            joystick_set_axis(1, joystick_digital_left  ? 0 : remote_joystick.y);
            joystick_set_axis(2, joystick_digital_right ? 0 : raw.x);
            joystick_set_axis(3, joystick_digital_right ? 0 : raw.y);
        }
    } else {
        transaction_rpc_send(RPC_JOYSTICK_AXES, sizeof(raw), &raw);
    }
}

void receive_joystick_axes(uint8_t type, const void *data, uint8_t size, void *context) {
    if (size == sizeof(joystick_axes_t)) {
        memcpy(&remote_joystick, data, size);
    }
}

void keyboard_post_init_user(void) {
    setPinInputHigh(JOY_PIN_B);

    if (is_keyboard_left()) {
        joystick_digital_left  = eeprom_read_byte((uint8_t*)EEPROM_ADDR_JOY_LEFT);
    } else {
        joystick_digital_right = eeprom_read_byte((uint8_t*)EEPROM_ADDR_JOY_RIGHT);
    }

    if (is_keyboard_master()) {
        transaction_register_rpc(RPC_JOYSTICK_AXES, receive_joystick_axes);
    }
}
