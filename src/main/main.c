/* 
 * The standard main()
 */
#include <main/main.h>
#include <control/control.h>

#include <stdlib.h>


int
main(int argc, char *argv[])
{
	control_init(argc, argv);                //init program control
	control_loop();                          //event loop
	control_del();                           //clear memories, etc
	return EXIT_SUCCESS;
} /*main()*/
