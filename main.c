#include "descend.h"
#include "lcd.h"

#ifdef SDCC
void main()
{
	init();
	lcd1602_init();
	while(1)
	{
		frame();
	}
}
#else
int main()
{
	init();
	//write_def();
	initscr();
	halfdelay(1);
	while(1)
	{
		frame();
		//printf("%d %d, camera offset %d\n",player.x,player.y,offset_y);
	}
	endwin();
}
#endif
