/*
 * KeyEventContext.h
 *
 *  Created on: Feb 16, 2020
 *      Author: guoxs
 */

#ifndef _KEY_EVENT_CONTEXT_H_
#define _KEY_EVENT_CONTEXT_H_

#define UI_KEY_PRESS    (1) // kernel report to APP
#define UI_KEY_RELEASE  (0) // kernel report to APP
#define UI_KEY_LPRESS   (2) // kernel report to APP
#define UI_KEY_LRELEASE (3) // kernel NOT report to APP, maybe need to check input driver.
#define KEY_MENU_2      (127)

#define KEYCODE_TO_STRING(CODE) ( \
     CODE == KEY_ENTER  ? "Enter" : \
     CODE == KEY_RIGHT  ? "Right" : \
     CODE == KEY_LEFT   ? "Left"  : \
     CODE == KEY_UP     ? "Up"    : \
     CODE == KEY_DOWN   ? "Down"  : \
     CODE == KEY_MENU   ? "Menu"  : \
     CODE == KEY_MENU_2 ? "Menu"  : \
     CODE == KEY_POWER  ? "Power" : \
                          "Unknow") \

#define KEYSTATUS_TO_STRING(STATUS) (      \
    STATUS == UI_KEY_PRESS    ? "Press"  : \
    STATUS == UI_KEY_LPRESS   ? "LPress" : \
    STATUS == UI_KEY_RELEASE  ? "Free"   : \
    STATUS == UI_KEY_LRELEASE ? "LFree"  : \
                                "Unknow")  \

typedef void (*on_key_event_callback)(int keyCode, int keyStatus);

int  start_key_event_ctx();
int  stop_key_event_ctx();
void set_key_event_callback(on_key_event_callback cb);

#endif /* _KEY_EVENT_CONTEXT_H_ */
