/* 
 * Control controls all objects:
 *  - Getting irc communication from irclsd
 *  - Drawing on screen
 *  - Getting input from user ans sending to named pipe for irclsd
 */
#ifndef CONTROL_H
#define CONTROL_H


/*
 * Initializes the control
 * Gets arguments as input from main
 */
void
control_init(int argc, char *argv[]);

/*
 * Frees the memory, closes all file handles
 */
void
control_del(void);

/*
 * Control event loop
 */
void
control_loop(void);


#endif /*CONTROL_H*/
