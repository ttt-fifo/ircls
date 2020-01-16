#include <style/style.h>
#include <global/global.h>

#include <ncursesw/curses.h>


extern Style style;


void
style_init(void)
{
	use_default_colors();
	start_color();

	init_pair(STYLE_ROW_A, COLOR_CYAN, -1);
	init_pair(STYLE_ROW_B, COLOR_MAGENTA, -1);
	init_pair(STYLE_ROW_A_HL, COLOR_CYAN, -1);
	init_pair(STYLE_ROW_B_HL, COLOR_WHITE, -1);
	init_pair(STYLE_ROW_TOME, COLOR_YELLOW, -1);
	init_pair(STYLE_ERROR, COLOR_RED, -1);
	init_pair(STYLE_BAR, COLOR_CYAN, COLOR_MAGENTA);
	init_pair(STYLE_BAR_TXTBOX, COLOR_BLACK, COLOR_GREEN);
	init_pair(STYLE_COUNT, COLOR_CYAN, COLOR_MAGENTA);
	init_pair(STYLE_COUNT_HIGHLIGHT, COLOR_YELLOW, COLOR_RED);
	init_pair(STYLE_BORDER, COLOR_MAGENTA, -1);
	init_pair(STYLE_INPUT, COLOR_GREEN, -1);

	style.row_a = COLOR_PAIR(STYLE_ROW_A);
	style.row_b = COLOR_PAIR(STYLE_ROW_B);
	style.row_a_hl = COLOR_PAIR(STYLE_ROW_A_HL) | A_BOLD;
	style.row_b_hl = COLOR_PAIR(STYLE_ROW_B_HL) | A_BOLD;
	style.row_tome = COLOR_PAIR(STYLE_ROW_TOME) | A_BOLD;
	style.error = COLOR_PAIR(STYLE_ERROR) | A_BOLD;
	style.bar = COLOR_PAIR(STYLE_BAR);
	style.bar_txtbox = COLOR_PAIR(STYLE_BAR_TXTBOX);
	style.count = COLOR_PAIR(STYLE_COUNT);
	style.count_highlight = COLOR_PAIR(STYLE_COUNT_HIGHLIGHT) | A_BOLD;
	style.border = COLOR_PAIR(STYLE_BORDER);
	style.input = COLOR_PAIR(STYLE_INPUT);
} /*style_init()*/
