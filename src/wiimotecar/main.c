/**
 * @author David S Anderson
 *
 *
 * Copyright (C) 2011 David S Anderson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <config.h>

#include "ControlTasks.h"

#define PORT_NAME "/dev/ttyUSB0"
//#define PORT_NAME "/dev/ttyS0"
//#define PORT_NAME "/dev/rfcomm0"

int main(int argc, char **argv)
{
	char *dev_name;

#if _DEBUG
	printf("\n%s\n", PACKAGE_STRING);
	printf("Built with: GCC, version; %s\n", __VERSION__);
	printf("On: %s, %s\n", __DATE__, __TIME__);
	printf("%s\n", PACKAGE_BUGREPORT);
	printf("\n");
#endif

	if (optind < argc)
		dev_name = argv[optind];
	else
		dev_name = PORT_NAME;

	control_tasks(dev_name);

	return 0;
}

