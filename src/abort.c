#include <stdio.h>
#include "abort.h"

void frc_abort()
{
	printf("in dummy func frc_abort()\n");
}

void set_abort(int x)
{
	printf("in dummy func set_abort(%d)\n", x);
}

int was_abort()
{
	int ret = 0;
	printf("in dummy func was_abort() returning %d\n");
	return ret;
}