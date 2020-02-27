#ifndef LEDGER_APP_IOST_UI_H
#define LEDGER_APP_IOST_UI_H

#include "glyphs.h"
//#include <bagl.h>
//#include "globals.h"

#if defined(TARGET_NANOS)
#define UI_BAGLE_ELEMENT(text) (const char* const)(text)
#elif defined(TARGET_NANOX)
#include "ux.h"
#define UI_BAGLE_ELEMENT(text) (const char* const)(text)
#elif defined(TARGET_BLUE)
#include "ux.h"
#define UI_BAGLE_ELEMENT(text) (const char* const)(text),0,0,0,NULL,NULL,NULL
#endif // TARGET_NANOX

// Common UI element definitions for Ledger Nano S, Ledger Nano X and Ledger Blue
#define UI_BACKGROUND() {{BAGL_RECTANGLE,0,0,0,128,32,0,0,BAGL_FILL,0,0xFFFFFF,0,0},UI_BAGLE_ELEMENT(NULL)}
#define UI_ICON_LEFT(userid, glyph) {{BAGL_ICON,userid,3,12,7,7,0,0,0,0xFFFFFF,0,0,glyph},UI_BAGLE_ELEMENT(NULL)}
#define UI_ICON_RIGHT(userid, glyph) {{BAGL_ICON,userid,117,13,8,6,0,0,0,0xFFFFFF,0,0,glyph},UI_BAGLE_ELEMENT(NULL)}
#define UI_TEXT(userid, x, y, w, text) {{BAGL_LABELINE,userid,x,y,w,12,0,0,0,0xFFFFFF,0,BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER,0},UI_BAGLE_ELEMENT(text)}

//extern bagl_element_t ui_background();
//extern bagl_element_t ui_icon_left(unsigned char user_id, unsigned char icon_id);
//extern bagl_element_t ui_icon_right(unsigned char user_id, unsigned char icon_id);
//extern bagl_element_t ui_text(unsigned char userid, short x, short y, unsigned short width, const char* const text);


extern void ui_idle(void);

#endif // LEDGER_APP_IOST_UI_H
