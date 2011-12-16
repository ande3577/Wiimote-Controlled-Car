/****************
* Helloworld.cpp
*****************/
#include <stdio.h>
#include <unistd.h>

#include "ControlTasks.h"
#include "program_info.h"

//#define PORT_NAME "/dev/ttyUSB0"
#define PORT_NAME "/dev/ttyS0"
//#define PORT_NAME "/dev/rfcomm0"

int main(int argc, char **argv)
{
	char *dev_name;

#if _DEBUG
	printf("\nWiimote Controlled Car (CWiid)\n");
#if _DIAGNOSTIC_MODE
	printf("Diagnostic Mode\n");
#endif
	printf("Version: %s\n",__DEVICE_VERSION__);
	printf("Built with: GCC, version; %s\n", __VERSION__);
	printf("On: %s, %s\n",__DATE__,__TIME__);
	printf("\n");
#endif

	if (optind < argc)
		dev_name = argv[optind];
	else
		dev_name = PORT_NAME;

	control_tasks(dev_name);

	return 0;
}

