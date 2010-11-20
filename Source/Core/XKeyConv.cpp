#include "HAGE.h"

#ifdef TARGET_LINUX

/*******************************************************/
/* STOLEN FROM QEMU, ORIGINAL COPYRIGHT NOTICE FOLLOWS */
/*******************************************************/

/*
 * QEMU SDL display driver
 *
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

 #include <X11/XKBlib.h>
 #include <strings.h>

static const uint16_t x_keycode_to_pc_keycode[115] = {
   0x147,      /*  97  Home   */
   0x148,      /*  98  Up     */
   0x149,      /*  99  PgUp   */
   0x14b,      /* 100  Left   */
   0x4c,        /* 101  KP-5   */
   0x14d,      /* 102  Right  */
   0x14f,      /* 103  End    */
   0x150,      /* 104  Down   */
   0x151,      /* 105  PgDn   */
   0x152,      /* 106  Ins    */
   0x153,      /* 107  Del    */
   0x11c,      /* 108  Enter  */
   0x11d,      /* 109  Ctrl-R */
   0x0,       /* 110  Pause  */
   0x137,      /* 111  Print  */
   0x135,      /* 112  Divide */
   0x138,      /* 113  Alt-R  */
   0x00,      /* 114  Break  */
   0x0,         /* 115 */
   0x0,         /* 116 */
   0x0,         /* 117 */
   0x0,         /* 118 */
   0x0,         /* 119 */
   0x0,         /* 120 */
   0x0,         /* 121 */
   0x0,         /* 122 */
   0x0,         /* 123 */
   0x0,         /* 124 */
   0x0,         /* 125 */
   0x0,         /* 126 */
   0x0,         /* 127 */
   0x0,         /* 128 */
   0x79,         /* 129 Henkan */
   0x0,         /* 130 */
   0x7b,         /* 131 Muhenkan */
   0x0,         /* 132 */
   0x7d,         /* 133 Yen */
   0x0,         /* 134 */
   0x0,         /* 135 */
   0x47,         /* 136 KP_7 */
   0x48,         /* 137 KP_8 */
   0x49,         /* 138 KP_9 */
   0x4b,         /* 139 KP_4 */
   0x4c,         /* 140 KP_5 */
   0x4d,         /* 141 KP_6 */
   0x4f,         /* 142 KP_1 */
   0x50,         /* 143 KP_2 */
   0x51,         /* 144 KP_3 */
   0x52,         /* 145 KP_0 */
   0x53,         /* 146 KP_. */
   0x47,         /* 147 KP_HOME */
   0x48,         /* 148 KP_UP */
   0x49,         /* 149 KP_PgUp */
   0x4b,         /* 150 KP_Left */
   0x4c,         /* 151 KP_ */
   0x4d,         /* 152 KP_Right */
   0x4f,         /* 153 KP_End */
   0x50,         /* 154 KP_Down */
   0x51,         /* 155 KP_PgDn */
   0x52,         /* 156 KP_Ins */
   0x53,         /* 157 KP_Del */
};

/* This table is generated based off the xfree86 -> scancode mapping above
 * and the keycode mappings in /usr/share/X11/xkb/keycodes/evdev
 * and  /usr/share/X11/xkb/keycodes/xfree86
 */

static const uint16_t evdev_keycode_to_pc_keycode[61] = {
    0x131,     /*  97 EVDEV - KEYPAD SLASH */
    0,         /*  98 EVDEV - KATA (Katakana) */
    0,         /*  99 EVDEV - HIRA (Hiragana) */
    0x79,      /* 100 EVDEV - HENK (Henkan) */
    0x70,      /* 101 EVDEV - HKTG (Hiragana/Katakana toggle) */
    0x7b,      /* 102 EVDEV - MUHE (Muhenkan) */
    0,         /* 103 EVDEV - JPCM (KPJPComma) */
    0x11c,     /* 104 KPEN */
    0x11d,     /* 105 RCTL */
    0x135,      /* 106 KPDV */
    0x137,      /* 107 PRSC */
    0x138,      /* 108 RALT */
    0,         /* 109 EVDEV - LNFD ("Internet" Keyboards) */
    0x147,      /* 110 HOME */
    0x148,      /* 111 UP */
    0x149,      /* 112 PGUP */
    0x14b,      /* 113 LEFT */
    0x14d,      /* 114 RGHT */
    0x14f,      /* 115 END */
    0x150,      /* 116 DOWN */
    0x151,      /* 117 PGDN */
    0x152,      /* 118 INS */
    0x153,      /* 119 DELE */
    0,         /* 120 EVDEV - I120 ("Internet" Keyboards) */
    0,         /* 121 EVDEV - MUTE */
    0,         /* 122 EVDEV - VOL- */
    0,         /* 123 EVDEV - VOL+ */
    0,         /* 124 EVDEV - POWR */
    0,         /* 125 EVDEV - KPEQ */
    0,         /* 126 EVDEV - I126 ("Internet" Keyboards) */
    0,         /* 127 EVDEV - PAUS */
    0,         /* 128 EVDEV - ???? */
    0,         /* 129 EVDEV - I129 ("Internet" Keyboards) */
    0xf1,      /* 130 EVDEV - HNGL (Korean Hangul Latin toggle) */
    0xf2,      /* 131 EVDEV - HJCV (Korean Hangul Hanja toggle) */
    0x7d,      /* 132 AE13 (Yen)*/
    0x15b,      /* 133 EVDEV - LWIN */
    0x15c,      /* 134 EVDEV - RWIN */
    0x15d,      /* 135 EVDEV - MENU */
    0,         /* 136 EVDEV - STOP */
    0,         /* 137 EVDEV - AGAI */
    0,         /* 138 EVDEV - PROP */
    0,         /* 139 EVDEV - UNDO */
    0,         /* 140 EVDEV - FRNT */
    0,         /* 141 EVDEV - COPY */
    0,         /* 142 EVDEV - OPEN */
    0,         /* 143 EVDEV - PAST */
    0,         /* 144 EVDEV - FIND */
    0,         /* 145 EVDEV - CUT  */
    0,         /* 146 EVDEV - HELP */
    0,         /* 147 EVDEV - I147 */
    0,         /* 148 EVDEV - I148 */
    0,         /* 149 EVDEV - I149 */
    0,         /* 150 EVDEV - I150 */
    0,         /* 151 EVDEV - I151 */
    0,         /* 152 EVDEV - I152 */
    0,         /* 153 EVDEV - I153 */
    0,         /* 154 EVDEV - I154 */
    0,         /* 155 EVDEV - I156 */
    0,         /* 156 EVDEV - I157 */
    0,         /* 157 EVDEV - I158 */
};

uint16_t translate_xfree86_keycode(const int key)
{
    return x_keycode_to_pc_keycode[key];
}

uint16_t translate_evdev_keycode(const int key)
{
    return evdev_keycode_to_pc_keycode[key];
}

static int check_for_evdev(Display* display)
{
    XkbDescPtr desc = NULL;
    int has_evdev = 0;
    char *keycodes = NULL;

    // apparently xfree switched to evdev table
    return 1;

    desc = XkbGetKeyboard(display,
                          XkbGBN_AllComponentsMask,
                          XkbUseCoreKbd);
    if (desc && desc->names) {
        keycodes = XGetAtomName(display, desc->names->keycodes);
        if (keycodes == NULL) {
            printf( "could not lookup keycode name\n");
        } else if (strncmp(keycodes, "evdev", strlen("evdev"))) {
            printf("using evdev\n");
            has_evdev = 1;
        } else if (!strncmp(keycodes, "xfree86", strlen("xfree86"))) {
            printf( "unknown keycodes `%s', please report to "
                    "qemu-devel@nongnu.org\n", keycodes);
        }
        else
        {
            printf("using xfree86\n");
        }
    }

    if (desc) {
        XkbFreeKeyboard(desc, XkbGBN_AllComponentsMask, True);
    }
    if (keycodes) {
        XFree(keycodes);
    }
    return has_evdev;
}

extern HAGE::u32 x_keycode_to_scancode(unsigned int keycode,Display* XDisplay)
{
    static int has_evdev = -1;

    if (has_evdev == -1)
        has_evdev = check_for_evdev(XDisplay);

    if (keycode < 9) {
        keycode = 0;
    } else if (keycode < 97) {
        keycode -= 8; /* just an offset */
    } else if (keycode < 158) {
        /* use conversion table */
        if (has_evdev)
            keycode = translate_evdev_keycode(keycode - 97);
        else
            keycode = translate_xfree86_keycode(keycode - 97);
    } else if (keycode == 208) { /* Hiragana_Katakana */
        keycode = 0x70;
    } else if (keycode == 211) { /* backslash */
        keycode = 0x73;
    } else {
        keycode = 0;
    }
    return keycode;
}

#endif
