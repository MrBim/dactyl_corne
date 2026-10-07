/* Translated from https://github.com/MrBim/zmk-corne (ZMK, Corne 3x6+3)
 * to QMK for the Dactyl ManuForm 5x6 (4 full 6-key finger rows + a
 * partial 5th row of 2 keys per hand + a 6-key thumb cluster per hand
 * = 64 keys total).
 *
 * The dactyl has more keys than the corne (64 vs. 42):
 *   - The extra finger row is used as a number row (tap dance: tap the
 *     digit, hold for its shifted symbol). Its top-left key toggles
 *     _MOUSE, its top-right is Delete.
 *   - The partial 5th row (4 keys) is unused - KC_NO on every layer.
 *   - 3 of the 6 thumb-cluster slots per hand carry a key; the rest are
 *     KC_NO.
 *
 * Only _BASE uses KC_NO for dead keys. Every other layer uses _______
 * (transparent), so unassigned keys fall through to _BASE.
 *
 * Thumb cluster: 3 rows of 2 keys per hand, cascading inward and down.
 * The hands mirror each other, so in each pair the LAYOUT_5x6 argument
 * order runs outer->inner on the left hand and inner->outer on the
 * right. Laid out by physical position rather than argument order:
 *
 *                outer       inner
 *   row 1   L:   Enter       Sym/Dir
 *           R:   Space       Base/Num
 *   row 2   L:   Base/Num    GUI
 *           R:   Sym/Dir     Alt
 *   row 3   L:   blank       MO(_MOUSE)
 *           R:   blank       MO(_MOUSE)
 *
 * Sym/Dir  = TD_LTOG_L: tap -> _SYM,  hold -> _DIR.
 * Base/Num = TD_LTOG_R: tap -> _BASE, hold -> _NUM.
 */

#include QMK_KEYBOARD_H

enum layers {
    _BASE,
    _SYM,
    _DIR,
    _NUM,
    _MOUSE,
};

enum custom_keycodes {
    U_COLLAPSE = SAFE_RANGE,
    U_EXPAND,
};

enum tap_dances {
    TD_SCLN,   // ; tap, : hold
    TD_QUOT,   // ' tap, " hold
    TD_SLSH,   // / tap, ? hold
    TD_N1,     // 1 tap, ! hold
    TD_N2,     // 2 tap, @ hold
    TD_N3,     // 3 tap, # hold
    TD_N4,     // 4 tap, $ hold
    TD_N5,     // 5 tap, % hold
    TD_N6,     // 6 tap, ^ hold
    TD_N7,     // 7 tap, & hold
    TD_N8,     // 8 tap, * hold
    TD_N9,     // 9 tap, ( hold
    TD_N0,     // 0 tap, ) hold
    TD_LTGT,   // < tap, > hold
    TD_BRACE,  // { tap, } hold
    TD_PAREN,  // ( tap, ) hold
    TD_BRKT,   // [ tap, ] hold
    TD_GRV,    // ` tap, ~ hold
    TD_EQPLUS, // = tap, + hold
    TD_LTOG_L, // tap: to _SYM, hold: momentary _DIR
    TD_LTOG_R, // tap: to _BASE, hold: momentary _NUM
};

// --- Generic tap(keycode)/hold(keycode) tap dance ---
// Pattern per https://docs.qmk.fm/features/tap_dance "Example 3".

typedef struct {
    uint16_t tap;
    uint16_t hold;
    uint16_t held;
} tap_dance_tap_hold_t;

void tap_dance_tap_hold_finished(tap_dance_state_t *state, void *user_data) {
    tap_dance_tap_hold_t *tap_hold = (tap_dance_tap_hold_t *)user_data;

    if (state->pressed) {
        if (state->count == 1
#ifndef PERMISSIVE_HOLD
            && !state->interrupted
#endif
        ) {
            register_code16(tap_hold->hold);
            tap_hold->held = tap_hold->hold;
        } else {
            register_code16(tap_hold->tap);
            tap_hold->held = tap_hold->tap;
        }
    }
}

void tap_dance_tap_hold_reset(tap_dance_state_t *state, void *user_data) {
    tap_dance_tap_hold_t *tap_hold = (tap_dance_tap_hold_t *)user_data;

    if (tap_hold->held) {
        unregister_code16(tap_hold->held);
        tap_hold->held = 0;
    }
}

#define ACTION_TAP_DANCE_TAP_HOLD(tap, hold)                                        \
    {                                                                               \
        .fn        = {NULL, tap_dance_tap_hold_finished, tap_dance_tap_hold_reset}, \
        .user_data = (void *)&((tap_dance_tap_hold_t){tap, hold, 0}),               \
    }

// --- Generic tap(switch to layer)/hold(momentary layer) tap dance ---
// Same shape as above, but drives layer_move()/layer_on()/layer_off()
// instead of register_code16(), to replicate the ZMK mo_tog behaviour.

typedef struct {
    uint8_t tap_layer;
    uint8_t hold_layer;
    bool    held;
} tap_dance_layer_tap_hold_t;

void tap_dance_layer_tap_hold_finished(tap_dance_state_t *state, void *user_data) {
    tap_dance_layer_tap_hold_t *l = (tap_dance_layer_tap_hold_t *)user_data;

    if (state->pressed) {
        layer_on(l->hold_layer);
        l->held = true;
    } else {
        layer_move(l->tap_layer);
    }
}

void tap_dance_layer_tap_hold_reset(tap_dance_state_t *state, void *user_data) {
    tap_dance_layer_tap_hold_t *l = (tap_dance_layer_tap_hold_t *)user_data;

    if (l->held) {
        layer_off(l->hold_layer);
        l->held = false;
    }
}

#define ACTION_TAP_DANCE_LAYER_TAP_HOLD(tap_layer, hold_layer)                                  \
    {                                                                                           \
        .fn        = {NULL, tap_dance_layer_tap_hold_finished, tap_dance_layer_tap_hold_reset}, \
        .user_data = (void *)&((tap_dance_layer_tap_hold_t){tap_layer, hold_layer, false}),      \
    }

tap_dance_action_t tap_dance_actions[] = {
    [TD_SCLN]   = ACTION_TAP_DANCE_TAP_HOLD(KC_SCLN, S(KC_SCLN)),
    [TD_QUOT]   = ACTION_TAP_DANCE_TAP_HOLD(KC_QUOT, S(KC_QUOT)),
    [TD_SLSH]   = ACTION_TAP_DANCE_TAP_HOLD(KC_SLSH, S(KC_SLSH)),
    [TD_N1]     = ACTION_TAP_DANCE_TAP_HOLD(KC_1, S(KC_1)),
    [TD_N2]     = ACTION_TAP_DANCE_TAP_HOLD(KC_2, S(KC_2)),
    [TD_N3]     = ACTION_TAP_DANCE_TAP_HOLD(KC_3, S(KC_3)),
    [TD_N4]     = ACTION_TAP_DANCE_TAP_HOLD(KC_4, S(KC_4)),
    [TD_N5]     = ACTION_TAP_DANCE_TAP_HOLD(KC_5, S(KC_5)),
    [TD_N6]     = ACTION_TAP_DANCE_TAP_HOLD(KC_6, S(KC_6)),
    [TD_N7]     = ACTION_TAP_DANCE_TAP_HOLD(KC_7, S(KC_7)),
    [TD_N8]     = ACTION_TAP_DANCE_TAP_HOLD(KC_8, S(KC_8)),
    [TD_N9]     = ACTION_TAP_DANCE_TAP_HOLD(KC_9, S(KC_9)),
    [TD_N0]     = ACTION_TAP_DANCE_TAP_HOLD(KC_0, S(KC_0)),
    [TD_LTGT]   = ACTION_TAP_DANCE_TAP_HOLD(S(KC_COMM), S(KC_DOT)),
    [TD_BRACE]  = ACTION_TAP_DANCE_TAP_HOLD(S(KC_LBRC), S(KC_RBRC)),
    [TD_PAREN]  = ACTION_TAP_DANCE_TAP_HOLD(S(KC_9), S(KC_0)),
    [TD_BRKT]   = ACTION_TAP_DANCE_TAP_HOLD(KC_LBRC, KC_RBRC),
    [TD_GRV]    = ACTION_TAP_DANCE_TAP_HOLD(KC_GRV, S(KC_GRV)),
    [TD_EQPLUS] = ACTION_TAP_DANCE_TAP_HOLD(KC_EQL, S(KC_EQL)),
    [TD_LTOG_L] = ACTION_TAP_DANCE_LAYER_TAP_HOLD(_SYM, _DIR),
    [TD_LTOG_R] = ACTION_TAP_DANCE_LAYER_TAP_HOLD(_BASE, _NUM),
};

// --- Combos ---

enum combo_events {
    COMBO_CAPS_WORD,
    COMBO_ESC,
    COMBO_TAB,
    COMBO_LCTL_L,
    COMBO_LCTL_R,
    COMBO_BSPC,
    COMBO_QUOTES,
    COMBO_LSFT_L,
    COMBO_LSFT_R,
    COMBO_CAPS_LOCK,
    COMBO_LALT,
    COMBO_DEL,
};

const uint16_t PROGMEM combo_caps_word[] = {KC_F, KC_G, KC_H, KC_J, COMBO_END};
const uint16_t PROGMEM combo_esc[]       = {KC_Q, KC_W, COMBO_END};
const uint16_t PROGMEM combo_tab[]       = {KC_S, KC_D, COMBO_END};
const uint16_t PROGMEM combo_lctl_l[]    = {KC_X, KC_V, COMBO_END};
const uint16_t PROGMEM combo_lctl_r[]    = {KC_M, KC_DOT, COMBO_END};
const uint16_t PROGMEM combo_bspc[]      = {KC_I, KC_O, COMBO_END};
const uint16_t PROGMEM combo_quotes[]    = {KC_K, KC_L, COMBO_END};
const uint16_t PROGMEM combo_lsft_l[]    = {KC_D, KC_F, COMBO_END};
const uint16_t PROGMEM combo_lsft_r[]    = {KC_J, KC_K, COMBO_END};
const uint16_t PROGMEM combo_caps_lock[] = {KC_D, KC_F, KC_J, KC_K, COMBO_END};
const uint16_t PROGMEM combo_lalt[]      = {KC_V, KC_B, COMBO_END};
const uint16_t PROGMEM combo_del[]       = {KC_Y, KC_U, COMBO_END};

combo_t key_combos[COMBO_COUNT] = {
    [COMBO_CAPS_WORD] = COMBO(combo_caps_word, CW_TOGG),
    [COMBO_ESC]        = COMBO(combo_esc, KC_ESC),
    [COMBO_TAB]        = COMBO(combo_tab, KC_TAB),
    [COMBO_LCTL_L]     = COMBO(combo_lctl_l, KC_LCTL),
    [COMBO_LCTL_R]     = COMBO(combo_lctl_r, KC_LCTL),
    [COMBO_BSPC]       = COMBO(combo_bspc, KC_BSPC),
    [COMBO_QUOTES]     = COMBO(combo_quotes, TD(TD_QUOT)),
    [COMBO_LSFT_L]     = COMBO(combo_lsft_l, KC_LSFT),
    [COMBO_LSFT_R]     = COMBO(combo_lsft_r, KC_LSFT),
    [COMBO_CAPS_LOCK]  = COMBO(combo_caps_lock, KC_CAPS),
    [COMBO_LALT]       = COMBO(combo_lalt, KC_LALT),
    [COMBO_DEL]        = COMBO(combo_del, KC_DEL),
};

// --- Macros ---

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Emit the tap keycode when a tap-hold dance is released before it
    // finishes. Without this a quick tap produces nothing, because
    // tap_dance_tap_hold_finished() only registers while state->pressed.
    // Required by the QMK tap-hold pattern these dances are built on.
    if (IS_QK_TAP_DANCE(keycode) && QK_TAP_DANCE_GET_INDEX(keycode) < TD_LTOG_L) {
        tap_dance_action_t *action = &tap_dance_actions[QK_TAP_DANCE_GET_INDEX(keycode)];
        tap_dance_state_t  *state  = tap_dance_get_state(QK_TAP_DANCE_GET_INDEX(keycode));

        if (!record->event.pressed && state != NULL && state->count && !state->finished) {
            tap_dance_tap_hold_t *tap_hold = (tap_dance_tap_hold_t *)action->user_data;
            tap_code16(tap_hold->tap);
        }
    }

    if (record->event.pressed) {
        switch (keycode) {
            case U_COLLAPSE:
                tap_code16(G(KC_K));
                tap_code16(G(KC_LBRC));
                break;
            case U_EXPAND:
                tap_code16(G(KC_K));
                tap_code16(G(KC_RBRC));
                break;
        }
    }
    return true;
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BASE] = LAYOUT_5x6(
        // ------------------------------------------------------- QWERTY --------------------------------------------------------
        TG(_MOUSE), TD(TD_N1)   , TD(TD_N2) , TD(TD_N3)       , TD(TD_N4)       , TD(TD_N5)    ,                TD(TD_N6)    , TD(TD_N7)    , TD(TD_N8)   , TD(TD_N9)     , TD(TD_N0)    , KC_DEL     ,
        KC_ESC    , KC_Q        , KC_W      , KC_E            , KC_R            , KC_T         ,                KC_Y         , KC_U         , KC_I        , KC_O          , KC_P         , KC_BSPC    ,
        KC_TAB    , KC_A        , KC_S      , KC_D            , KC_F            , KC_G         ,                KC_H         , KC_J         , KC_K        , KC_L          , TD(TD_SCLN)  , TD(TD_QUOT),
        KC_LCTL   , KC_Z        , KC_X      , KC_C            , KC_V            , KC_B         ,                KC_N         , KC_M         , KC_COMM     , KC_DOT        , TD(TD_SLSH)  , KC_LSFT    ,
                                  KC_NO     , KC_NO           ,                                                                               KC_NO       , KC_NO        ,
                                                                KC_ENT          , TD(TD_LTOG_L),                TD(TD_LTOG_R), KC_SPC       ,
                                                                TD(TD_LTOG_R)   , KC_LGUI      ,                KC_LALT      , TD(TD_LTOG_L),
                                                                KC_NO           , MO(_MOUSE)   ,                MO(_MOUSE)   , KC_NO        
    ),

    [_SYM] = LAYOUT_5x6(
        _______,    _______,      _______,    _______,          _______,          _______,                      _______,       _______,       _______,      _______,        _______,       _______,
        _______,    TD(TD_N1),    TD(TD_N2),  TD(TD_N3),        TD(TD_N4),        TD(TD_N5),                    TD(TD_N6),     TD(TD_N7),     TD(TD_N8),    TD(TD_N9),      TD(TD_N0),     _______,
        _______,    KC_BSLS,      S(KC_7),    S(KC_EQL),        KC_MINS,          KC_EQL,                       TD(TD_LTGT),   TD(TD_BRACE),  TD(TD_PAREN), TD(TD_BRKT),    _______,       _______,
        _______,    TD(TD_GRV),   S(KC_MINS), S(KC_3),          S(KC_5),          S(KC_BSLS),                   S(KC_DOT),     KC_F12,        _______,      _______,        _______,       _______,
                                  _______,    _______,                                                                                        _______,      _______,
                                                                _______,          _______,                      _______,       _______,
                                                                _______,          _______,                      _______,       _______,
                                                                _______,          _______,                      _______,       _______
    ),

    [_DIR] = LAYOUT_5x6(
        _______,    _______,      _______,    _______,          _______,          _______,                      _______,       _______,       _______,      _______,        _______,       _______,
        _______,    _______,      _______,    _______,          _______,          _______,                      U_COLLAPSE,    KC_HOME,       KC_UP,        KC_END,         U_EXPAND,      _______,
        _______,    _______,      KC_MPRV,    KC_MPLY,          KC_MNXT,          _______,                      RGUI(KC_LBRC), KC_LEFT,       KC_DOWN,      KC_RIGHT,       LGUI(KC_RBRC), _______,
        _______,    _______,      KC_MUTE,    KC_VOLD,          KC_VOLU,          _______,                      _______,       RALT(KC_LEFT), _______,      RALT(KC_RIGHT), _______,       _______,
                                  _______,    _______,                                                                                        _______,      _______,
                                                                _______,          _______,                      _______,       _______,
                                                                _______,          _______,                      _______,       _______,
                                                                _______,          _______,                      _______,       _______
    ),

    [_NUM] = LAYOUT_5x6(
        _______,    _______,      _______,    _______,          _______,          _______,                      _______,       _______,       _______,      _______,        _______,       _______,
        _______,    _______,      _______,    _______,          _______,          _______,                      TD(TD_EQPLUS), KC_KP_1,       KC_KP_2,      KC_KP_3,        S(KC_5),       _______,
        _______,    _______,      _______,    KC_DEL,           RSFT(RALT(KC_F)), _______,                      KC_KP_0,       KC_KP_4,       KC_KP_5,      KC_KP_6,        _______,       _______,
        _______,    _______,      _______,    LSFT(LGUI(KC_C)), _______,          _______,                      KC_MINS,       KC_KP_7,       KC_KP_8,      KC_KP_9,        KC_DOT,        _______,
                                  _______,    _______,                                                                                        _______,      _______,
                                                                _______,          _______,                      _______,       _______,
                                                                _______,          _______,                      _______,       _______,
                                                                _______,          _______,                      _______,       _______
    ),

    [_MOUSE] = LAYOUT_5x6(
        _______,    _______,      _______,    _______,          _______,          _______,                      _______,       _______,       _______,      _______,        _______,       _______,
        _______,    _______,      _______,    _______,          _______,          _______,                      _______,       MS_BTN1,       MS_UP,        MS_BTN2,        _______,       _______,
        _______,    _______,      MS_ACL0,    MS_ACL1,          MS_ACL2,          _______,                      MS_WHLU,       MS_LEFT,       MS_DOWN,      MS_RGHT,        _______,       _______,
        _______,    _______,      _______,    _______,          _______,          _______,                      MS_WHLD,       _______,       _______,      _______,        _______,       _______,
                                  _______,    _______,                                                                                        _______,      _______,
                                                                _______,          _______,                      _______,       _______,
                                                                _______,          _______,                      _______,       _______,
                                                                _______,          _______,                      _______,       _______
    ),
};
