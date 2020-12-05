
#include <hw_config.h>
#include <usb_lib.h>
#include <usb_desc.h>
#include <usb_pwr.h>


int main()
{
	Set_System();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();

	while (1)
	{
		__NOP();
	}

	return 0;
}
