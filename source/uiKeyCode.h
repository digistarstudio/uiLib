

#pragma once


#ifdef _WINDOWS


#include <Winuser.h>


enum VUI_KEY_CODE
{
	uiK_LBUTTON = VK_LBUTTON,
	uiK_RBUTTON = VK_RBUTTON,
	uiK_CANCEL  = VK_CANCEL,
	uiK_MBUTTON = VK_MBUTTON,

	uiK_XBUTTON1 = VK_XBUTTON1,
	uiK_XBUTTON2 = VK_XBUTTON2,

	uiK_BACK = VK_BACK,
	uiK_TAB  = VK_TAB,

	uiK_CLEAR = VK_CLEAR,
	uiK_ENTER = VK_RETURN,

	uiK_SHIFT   = VK_SHIFT,
	uiK_CONTROL = VK_CONTROL,
	uiK_ALT     = VK_MENU,
	uiK_PAUSE   = VK_PAUSE,
	uiK_CAPS    = VK_CAPITAL,

	uiK_KANA    = VK_KANA,
	//uiK_HANGUEL = VK_HANGUEL,
	uiK_HANGUL  = VK_HANGUL,
	uiK_JUNJA   = VK_JUNJA,
	uiK_FINAL   = VK_FINAL,
	uiK_HANJA   = VK_HANJA,
	uiK_KANJI   = VK_KANJI,

	uiK_ESC        = VK_ESCAPE,
	uiK_CONVERT    = VK_CONVERT,
	uiK_NONCONVERT = VK_NONCONVERT,
	uiK_ACCEPT     = VK_ACCEPT,
	uiK_MODECHANGE = VK_MODECHANGE,
	uiK_SPACE      = VK_SPACE,
	uiK_PAGEUP     = VK_PRIOR,
	uiK_PAGEDOWN   = VK_NEXT,

	uiK_END      = VK_END,
	uiK_HOME     = VK_HOME,
	uiK_LEFT     = VK_LEFT,
	uiK_UP       = VK_UP,
	uiK_RIGHT    = VK_RIGHT,
	uiK_DOWN     = VK_DOWN,
	uiK_SELECT   = VK_SELECT,
	uiK_PRINT    = VK_PRINT,
	uiK_EXEVUTE  = VK_EXECUTE,
	uiK_SNAPSHOT = VK_SNAPSHOT,
	uiK_INSERT   = VK_INSERT,
	uiK_DELETE   = VK_DELETE,
	uiK_HELP     = VK_HELP,

	uiK_NUM0 = 0x30, // '0'
	uiK_NUM1 = 0x31, // '1'
	uiK_NUM2 = 0x32, // '2'
	uiK_NUM3 = 0x33, // '3'
	uiK_NUM4 = 0x34, // '4'
	uiK_NUM5 = 0x35, // '5'
	uiK_NUM6 = 0x36, // '6'
	uiK_NUM7 = 0x37, // '7'
	uiK_NUM8 = 0x38, // '8'
	uiK_NUM9 = 0x39, // '9'

	uiK_A = 0x41, // 'A'
	uiK_B = 0x42, // 'B'
	uiK_C = 0x43, // 'C'
	uiK_D = 0x44, // 'D'
	uiK_E = 0x45, // 'E'
	uiK_F = 0x46, // 'F'
	uiK_G = 0x47, // 'G'
	uiK_H = 0x48, // 'H'
	uiK_I = 0x49, // 'I'
	uiK_J = 0x4A, // 'J'
	uiK_K = 0x4B, // 'K'
	uiK_L = 0x4C, // 'L'
	uiK_M = 0x4D, // 'M'
	uiK_N = 0x4E, // 'N'
	uiK_O = 0x4F, // 'O'
	uiK_P = 0x50, // 'P'
	uiK_Q = 0x51, // 'Q'
	uiK_R = 0x52, // 'R'
	uiK_S = 0x53, // 'S'
	uiK_T = 0x54, // 'T'
	uiK_U = 0x55, // 'U'
	uiK_V = 0x56, // 'V'
	uiK_W = 0x57, // 'W'
	uiK_X = 0x58, // 'X'
	uiK_Y = 0x59, // 'Y'
	uiK_Z = 0x5A, // 'Z'

	uiK_LWIN  = VK_LWIN,
	uiK_RWIN  = VK_RWIN,
	uiK_APPS  = VK_APPS,
	uiK_SLEEP = VK_SLEEP,

	uiK_NUMPAD0 = VK_NUMPAD0,
	uiK_NUMPAD1 = VK_NUMPAD1,
	uiK_NUMPAD2 = VK_NUMPAD2,
	uiK_NUMPAD3 = VK_NUMPAD3,
	uiK_NUMPAD4 = VK_NUMPAD4,
	uiK_NUMPAD5 = VK_NUMPAD5,
	uiK_NUMPAD6 = VK_NUMPAD6,
	uiK_NUMPAD7 = VK_NUMPAD7,
	uiK_NUMPAD8 = VK_NUMPAD8,
	uiK_NUMPAD9 = VK_NUMPAD9,

	uiK_MULTIPLY  = VK_MULTIPLY,
	uiK_ADD       = VK_ADD,
	uiK_SEPARATOR = VK_SEPARATOR,
	uiK_SUBTRACT  = VK_SUBTRACT,
	uiK_DECIMAL   = VK_DECIMAL,
	uiK_DIVIDE    = VK_DIVIDE,

	uiK_F1  = VK_F1,
	uiK_F2  = VK_F2,
	uiK_F3  = VK_F3,
	uiK_F4  = VK_F4,
	uiK_F5  = VK_F5,
	uiK_F6  = VK_F6,
	uiK_F7  = VK_F7,
	uiK_F8  = VK_F8,
	uiK_F9  = VK_F9,
	uiK_F10 = VK_F10,
	uiK_F11 = VK_F11,
	uiK_F12 = VK_F12,

	uiK_F13 = VK_F13,
	uiK_F14 = VK_F14,
	uiK_F15 = VK_F15,
	uiK_F16 = VK_F16,
	uiK_F17 = VK_F17,
	uiK_F18 = VK_F18,
	uiK_F19 = VK_F19,
	uiK_F20 = VK_F20,
	uiK_F21 = VK_F21,
	uiK_F22 = VK_F22,
	uiK_F23 = VK_F23,
	uiK_F24 = VK_F24,

	uiK_NUMLOCK = VK_NUMLOCK,
	uiK_SCROLL  = VK_SCROLL,
	uiK_LSHIFT  = VK_LSHIFT,
	uiK_RSHIFT  = VK_RSHIFT,
	uiK_LCTRL   = VK_LCONTROL,
	uiK_RCTRL   = VK_RCONTROL,
	uiK_LALT    = VK_LMENU,
	uiK_RALT    = VK_RMENU,

	uiK_BROWSER_BACK      = VK_BROWSER_BACK,
	uiK_BROWSER_FORWARD   = VK_BROWSER_FORWARD,
	uiK_BROWSER_REFRESH   = VK_BROWSER_REFRESH,
	uiK_BROWSER_STOP      = VK_BROWSER_STOP,
	uiK_BROWSER_SEARCH    = VK_BROWSER_SEARCH,
	uiK_BROWSER_FAVORITES = VK_BROWSER_FAVORITES,
	uiK_BROWSER_HOME      = VK_BROWSER_HOME,

	uiK_VOLUME_MUTE = VK_VOLUME_MUTE,
	uiK_VOLUME_DOWN = VK_VOLUME_DOWN,
	uiK_VOLUME_UP   = VK_VOLUME_UP,

	uiK_MEDIA_NEXT_TRACK = VK_MEDIA_NEXT_TRACK,
	uiK_MEDIA_PREV_TRACK = VK_MEDIA_PREV_TRACK,
	uiK_MEDIA_STOP       = VK_MEDIA_STOP,
	uiK_MEDIA_PLAY_PAUSE = VK_MEDIA_PLAY_PAUSE,

	uiK_LAUNCH_MAIL         = VK_LAUNCH_MAIL,
	uiK_LAUNCH_MEDIA_SELECT = VK_LAUNCH_MEDIA_SELECT,
	uiK_LAUNCH_APP1         = VK_LAUNCH_APP1,
	uiK_LAUNCH_APP2         = VK_LAUNCH_APP2,

	uiK_OEM_1      = VK_OEM_1,
	uiK_OEM_PLUS   = VK_OEM_PLUS,
	uiK_OEM_COMMA  = VK_OEM_COMMA,
	uiK_OEM_MINUS  = VK_OEM_MINUS,
	uiK_OEM_PERIOD = VK_OEM_PERIOD,

	uiK_OEM_2   = VK_OEM_2,
	uiK_OEM_3   = VK_OEM_3,
	uiK_OEM_4   = VK_OEM_4,
	uiK_OEM_5   = VK_OEM_5,
	uiK_OEM_6   = VK_OEM_6,
	uiK_OEM_7   = VK_OEM_7,
	uiK_OEM_8   = VK_OEM_8,
	uiK_OEM_102 = VK_OEM_102,

	uiK_PROCESSKEY = VK_PROCESSKEY,
	uiK_PACKET     = VK_PACKET,
	uiK_ATTN       = VK_ATTN,

	uiK_CRSEL     = VK_CRSEL,
	uiK_EXSEL     = VK_EXSEL,
	uiK_EREOF     = VK_EREOF,
	uiK_PLAY      = VK_PLAY,
	uiK_ZOOM      = VK_ZOOM,
	uiK_NONAME    = VK_NONAME,
	uiK_PA1       = VK_PA1,
	uiK_OEM_CLEAR = VK_OEM_CLEAR,

};


#endif


