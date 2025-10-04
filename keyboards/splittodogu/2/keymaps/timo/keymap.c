// Copyright 2025 Timo Strube (@tstrube)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "eeprom.h"
#include "split_util.h"

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
#define EEPROM_ADDR_JOY_LEFT  0x00
#define EEPROM_ADDR_JOY_RIGHT 0x01
#define LED_INDEX_JOY_LEFT    44
#define LED_INDEX_JOY_RIGHT   89

bool joystick_digital_left = false;
bool joystick_digital_right = false;
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
    [DF] = LAYOUT_split_5x8_7(
        KC_ESC ,KC_GRV ,KC_1   ,KC_2   ,KC_3   ,KC_4   ,KC_5   ,KC_6   ,KC_MPLY,KC_MPLY,KC_7   ,KC_8   ,KC_9   ,KC_0   ,KC_MINS,KC_EQL ,KC_BSPC,KC_DEL ,
        TS_CUT ,KC_TAB ,KC_Q   ,KC_W   ,KC_E   ,KC_R   ,KC_T   ,KC_Y   ,KC_MNXT,KC_MNXT,KC_PGUP,KC_PGDN,KC_U   ,KC_I   ,KC_O   ,KC_P   ,KC_LBRC,KC_RBRC,
        TS_CPY ,KC_LSFT,KC_A   ,KC_S   ,KC_D   ,KC_F   ,KC_G   ,KC_HOME,KC_MPRV,KC_MPRV,KC_END ,KC_H   ,KC_J   ,KC_K   ,KC_L   ,KC_SCLN,KC_QUOT,KC_ENT ,
        TS_PST ,KC_LCTL,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_B   ,KC_SLEP,KC_MUTE,KC_MUTE,KC_SLEP,KC_N   ,KC_M   ,KC_COMM,KC_DOT ,KC_SLSH,KC_UP  ,KC_RSFT,
        TS_LCK ,KC_CAPS,MO(FN) ,PDF(MC),KC_LGUI,KC_LALT,KC_SPC ,TS_JOYL,TS_MIC ,TSF_MIC,TS_JOYR,KC_BSPC,KC_PSCR,TG(_FN),KC_BSLS,KC_LEFT,KC_DOWN,KC_RGHT,
        KC_F13 ,KC_F14 ,KC_F15 ,KC_F16 ,KC_F17 ,KC_F18 ,KC_F19 ,KC_F20 ,KC_F21 ,KC_F22
    ),
    [MC] = LAYOUT_split_5x8_7(
        KC_ESC ,KC_GRV ,KC_1   ,KC_2   ,KC_3   ,KC_4   ,KC_5   ,KC_6   ,KC_MPLY,KC_MPLY,KC_7   ,KC_8   ,KC_9   ,KC_0   ,KC_MINS,KC_EQL ,KC_BSPC,KC_DEL ,
        TSF_CUT,KC_TAB ,KC_Q   ,KC_W   ,KC_E   ,KC_R   ,KC_T   ,KC_Y   ,KC_MNXT,KC_MPRV,KC_PGUP,KC_PGDN,KC_U   ,KC_I   ,KC_O   ,KC_P   ,KC_LBRC,KC_RBRC,
        TSF_CPY,KC_LSFT,KC_A   ,KC_S   ,KC_D   ,KC_F   ,KC_G   ,KC_HOME,KC_MPRV,KC_MPRV,KC_END ,KC_H   ,KC_J   ,KC_K   ,KC_L   ,KC_SCLN,KC_QUOT,KC_ENT ,
        TSF_PST,KC_LCTL,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_B   ,KC_SLEP,KC_MUTE,KC_MUTE,KC_SLEP,KC_N   ,KC_M   ,KC_COMM,KC_DOT ,KC_SLSH,KC_UP  ,KC_RSFT,
        TSF_LCK,KC_CAPS,MO(FN) ,PDF(DF),KC_LGUI,KC_LALT,KC_SPC ,TS_JOYL,TSF_MIC,TSF_MIC,TS_JOYR,KC_BSPC,KC_PSCR,TG(_FN),KC_BSLS,KC_LEFT,KC_DOWN,KC_RGHT,
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
                rgblight_setrgb_at(
                    joystick_digital_left ? 0x00 : 0xFF,
                    joystick_digital_left ? 0xFF : 0x00,
                    0x00,
                    LED_INDEX_JOY_LEFT
                );
                return false;

            case TS_JOYR:
                joystick_digital_right = !joystick_digital_right;
                eeprom_update_byte((uint8_t*)EEPROM_ADDR_JOY_RIGHT, joystick_digital_right);
                rgblight_setrgb_at(
                    joystick_digital_right ? 0x00 : 0xFF,
                    joystick_digital_right ? 0xFF : 0x00,
                    0x00,
                    LED_INDEX_JOY_RIGHT
                );
                return false;
        }
    }
    return true;
}

void keyboard_post_init_user(void) {
    joystick_digital_left  = eeprom_read_byte((uint8_t*)EEPROM_ADDR_JOY_LEFT);
    joystick_digital_right = eeprom_read_byte((uint8_t*)EEPROM_ADDR_JOY_RIGHT);

    rgblight_setrgb_at(
        joystick_digital_left ? 0x00 : 0xFF,
        joystick_digital_left ? 0xFF : 0x00,
        0x00,
        LED_INDEX_JOY_LEFT
    );
    rgblight_setrgb_at(
        joystick_digital_right ? 0x00 : 0xFF,
        joystick_digital_right ? 0xFF : 0x00,
        0x00,
        LED_INDEX_JOY_RIGHT
    );
}