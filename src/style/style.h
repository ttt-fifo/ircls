/*
 * All ncurses styles are defined here (static styles, for dynamic style
 * assignments look at stylerot.h)
 * The styles are then exported in a global variable (see global.{h,c})
 * and used by all objects in the current program
 */
#ifndef STYLE_H
#define STYLE_H


enum StyleEnum {STYLE_TIME = 10,                 //the time in chat window
	        STYLE_PRIV,                      //all msg from / to me
		STYLE_ERROR,                     //error in chat window
                STYLE_BAR,                       //the generic status bar style
		STYLE_BAR_TXTBOX,                //the textbox into status bar
		STYLE_COUNT,                     //privmsg count into st.bar
		STYLE_COUNT_HIGHLIGHT,           //highlighted privmsg cnt
		STYLE_INPUT,                     //user input style
                STYLE_MOD_NICK};                 //modifier for every sending
                                                 //nick into chat window

/*
 * Initialize all ncurses styles
 */
void
style_init(void);


#endif /*STYLE_H*/
