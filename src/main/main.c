#include <main/main.h>
#include <control/control.h>

#include <stdlib.h>


int
main(int argc, char *argv[])
{
	control_init(argc, argv);
	control_loop();
	control_del();
	return EXIT_SUCCESS;
} /*main()*/
