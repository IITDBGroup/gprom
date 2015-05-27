/*-----------------------------------------------------------------------------
 *
 * termcolor.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_LOG_TERMCOLOR_H_
#define INCLUDE_LOG_TERMCOLOR_H_

#define TERM_ESCAPE     "\033["

#define TERM_TEXT_NORMAL    "0"
#define TERM_TEXT_BOLD      "1"
#define TERM_TEXT_UNDERSC   "4"
#define TERM_TEXT_BLINK     "5"

#define TERM_FG_BLACK   "30"
#define TERM_FG_RED     "31"
#define TERM_FG_GREEN   "32"
#define TERM_FG_YELLOW  "33"
#define TERM_FG_BLUE    "34"
#define TERM_FG_MAGENTA "35"
#define TERM_FG_CYAN    "36"
#define TERM_FG_WHITE   "37"

#define TERM_BG_BLACK   "40"
#define TERM_BG_RED     "41"
#define TERM_BG_GREEN   "42"
#define TERM_BG_YELLOW  "43"
#define TERM_BG_BLUE    "44"
#define TERM_BG_MAGENTA "45"
#define TERM_BG_CYAN    "46"
#define TERM_BG_WHITE   "47"

#define TERM_COMB(_code_) TERM_ESCAPE _code_ "m"

#define TERM_COLOR_ON(_color_) TERM_COMB(TERM_FG_ ## _color_)
#define TERM_BOLD_COLOR_ON(_color_) TERM_COMB(TERM_TEXT_BOLD) TERM_COMB(TERM_FG_ ## _color_)
#define TERM_BOLD TERM_BOLD_COLOR_ON(BLACK)
#define TERM_ALL_ON(_text_attrs_,_fg_color_,_bg_color_) TERM_COMB(TERM_TEXT_ ## _text_attrs_) TERM_COMB(TERM_FG_ ## _fg_color_) TERM_COMB(TERM_BG_ ## _bg_color_)

#define TERM_RESET "\033[0m"

#define TCOL(_color_,_text_) TERM_COLOR_ON(_color_) _text_ TERM_RESET
#define TBCOL(_color_,_text_) TERM_BOLD_COLOR_ON(_color_) _text_ TERM_RESET
#define TB(_text_) TERM_BOLD _text_ TERM_RESET
#define T_FG_BG(_fg_,_bg_,_text_) TERM_ALL_ON(NORMAL,_fg_,_bg_) _text_ TERM_RESET
#define TB_FG_BG(_fg_,_bg_,_text_) TERM_ALL_ON(BOLD,_fg_,_bg_) _text_ TERM_RESET

#endif /* INCLUDE_LOG_TERMCOLOR_H_ */
