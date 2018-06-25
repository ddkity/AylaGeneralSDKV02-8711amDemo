#include "device.h"
#include "main.h"
#include "product.h"

int main(void)
{
	UartInit();
	while(1)
	{
		userhandler();
	}
}

