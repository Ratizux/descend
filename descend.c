#include "descend.h"
#include "lcd.h"

#define TILESET_SIZE 16
#define OBJ_DATA_LIST_SIZE 5
#define OBJ_SIZE 5

#ifdef SDCC
__pdata struct obj player;//5 Bytes
__pdata struct obj objects[OBJ_SIZE];//5 Bytes each
__pdata unsigned char framebuffer[40];//40 Bytes
//memory usage= 45 + 5*OBJ_SIZE
__code struct tile tileset[TILESET_SIZE]={{0,{0,0,0,0,0}},{120,{174,170,170,186,128}},{120,{190,162,162,162,128}},{120,{190,160,160,160,128}},{120,{190,138,138,190,128}},{120,{190,170,174,184,128}},{120,{136,136,170,156,136}},{120,{190,170,170,170,128}},{4,{15,9,9,9,13}},{1,{1,1,1,1,1}},{120,{190,138,138,142,128}},{69,{16,16,16,16,16}},{120,{128,128,128,128,128}},{8,{255,1,1,1,255}},{0,{126,5,9,17,126}}};
__code struct obj_data obj_data_list[OBJ_DATA_LIST_SIZE]={{68,{15,9,9,15,0}},{0,{0,0,0,0,0}},{0,{0,0,0,0,0}},{0,{0,0,0,0,0}},{0,{0,0,0,0,0}}};
__code unsigned char map[16][16]={{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{13},{0},{0},{0},{0},{0}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{153},{153},{153},{153},{11},{208}},{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{11},{217}},{{0},{0},{0},{0},{0},{208},{0},{0},{14},{0},{23},{66},{122},{6},{11},{217}},{{204},{204},{60},{84},{6},{152},{204},{155},{153},{153},{0},{0},{0},{144},{153},{153}}};
#else
struct tile tileset[TILESET_SIZE];
struct obj player;
struct obj_data obj_data_list[OBJ_DATA_LIST_SIZE]={{68,{15,9,9,15,0}},{0,{0,0,0,0,0}},{0,{0,0,0,0,0}},{0,{0,0,0,0,0}},{0,{0,0,0,0,0}}};
struct obj objects[OBJ_SIZE];
unsigned char map[16][16];
//bit compressed
unsigned char framebuffer[40];//byte -> 8 vertical pixels
#endif

#define PLAYER_WIDTH 4
#define PLAYER_HEIGHT 4

#define MAP_HEIGHT 128
#define MAP_WIDTH 160
#define MAP_BLK_HEIGHT 16
#define MAP_BLK_WIDTH 32

unsigned char offset_x,offset_y;
//screen box offset
#define SCREEN_HEIGHT 16
#define SCREEN_WIDTH 80
unsigned char offset_bitmap;
//bitmap box offset
//range 0-12
#define BITMAP_HEIGHT 16
#define BITMAP_WIDTH 20

unsigned char input;
//--------
//up down left right attack jump select start

unsigned char objects_size;

unsigned char get_tileset(unsigned char x,unsigned char y)//return tile id of specified block
{
	if(x%10<5) return map[y/8][x/10]&0b00001111;
	return (map[y/8][x/10]&0b11110000)>>4;
}

unsigned char collision_test(unsigned char x,unsigned char y)
{
	//check collision of given pixel
	unsigned char floor=tileset[get_tileset(x,y)].block&0b00001111;
	unsigned char ceiling=tileset[get_tileset(x,y)].block&0b11110000;
	ceiling>>=4;
	//printf("Collision test on %d, %d\n",x,y);
	//printf("Floor %d\nCeiling %d\n",floor,ceiling);
	if((ceiling<=y%8)&&(floor>y%8)) return 1;
	return 0;
}

unsigned char target_height,target_width;

//definition of functions
void obj_info(struct obj *target)
{
	target_height=(obj_data_list[target->type].size&0b11110000)>>4;
	target_width=obj_data_list[target->type].size&0b00001111;
}

unsigned char obj_torches_floor(struct obj *target)
{
	if(target->y+target_height == MAP_HEIGHT) return 1;
	return collision_test(target->x,target->y+target_height)||collision_test(target->x+target_width-1,target->y+target_height);
}

unsigned char obj_torches_ceiling(struct obj *target)
{
	if(target->y == 0) return 1;
	return collision_test(target->x,target->y-1)||collision_test(target->x+target_width-1,target->y-1);
}

unsigned char obj_torches_left_wall(struct obj *target)
{
	if(target->x == 0) return 1;
	for(unsigned char i=0;i<target_height;i++)
	{
		if(collision_test(target->x-1,target->y+i)) return 1;
	}
	return 0;
}

unsigned char obj_torches_right_wall(struct obj *target)
{
	if(target->x+target_width == MAP_WIDTH) return 1;
	for(unsigned char i=0;i<target_height;i++)
	{
		if(collision_test(target->x+target_width,target->y+i)) return 1;
	}
	return 0;
}

unsigned char obj_spd_y(struct obj *target)
{
	return target->spd&0b00000111;
}

unsigned char obj_spd_x(struct obj *target)
{
	return (target->spd&0b01110000)>>4;
}

unsigned char spd_test(unsigned char idle,unsigned char spd)
{
	switch(spd)
	{
		case 1:return idle==0;
		case 2:return idle==0;
		case 3:return idle==1;
		case 4:return idle==1;
		case 5:return idle==2;
		case 6:return idle==2;
		case 7:return idle==3;
	}
	return 0;
}

void simulate(struct obj *target)
{
	obj_info(target);
	unsigned char xy_spd,xy_idle,direction;
	{
		xy_spd=obj_spd_y(target);
		if(xy_spd>0)
		{
			//move vertically
			xy_idle=target->idletime&0b00001111;
			//printf("idle=%d, spd=%d\n",y_idle,y_spd);
			if(spd_test(xy_idle,xy_spd))
			{
				direction=target->spd&0b00001000;
				if(direction&&obj_torches_floor(target))//move down, speed up
				{
					xy_spd=0;
				}
				else if((!direction)&&obj_torches_ceiling(target))
				{
					xy_spd=0;
				}
				else
				{
					if(direction)//move down, speed up
					{	
						target->y++;
						if(xy_spd>1) xy_spd--;
					}
					else//move up, speed down
					{
						target->y--;
						if(xy_spd<7) xy_spd++;
						else xy_spd=0;
					}
					target->idletime&=0b11110000;//reset y idle to 0
				}
				target->spd&=0b11111000;//clear low 3 bit
				target->spd|=xy_spd;//set new y spd
			}
			else
			{
				if(xy_idle<15)
				{
					xy_idle++;
					target->idletime&=0b11110000;//clear low 4 bit
					target->idletime|=xy_idle;
				}
			}
		}
		else
		{
			//printf("free-fall motion try\n");
			//free-fall motion try
			if(obj_torches_floor(target)==0)
			{
				//printf("started\n");
				target->spd&=0b11110000;//clear low 4 bit
				target->spd|=0b00001111;//set y spd=7, direction down
				target->idletime&=0b11110000;//reset y idle to 0
			}
		}
	}
	{
		xy_spd=obj_spd_x(target);
		if(xy_spd>0)//move horizontally
		{
			xy_idle=target->idletime&0b11110000;
			xy_idle>>=4;
			if(spd_test(xy_idle,xy_spd))
			{
				direction=target->spd&0b10000000;
				if(direction&&obj_torches_left_wall(target))//move from right to left
				{
					xy_spd=0;
				}
				else if((!direction)&&obj_torches_right_wall(target))//move from left to right
				{
					xy_spd=0;
				}
				else
				{
					if(direction) target->x--;
					else target->x++;
					target->idletime&=0b00001111;//reset idletime
					if(obj_torches_floor(target))
					{
						if(xy_spd<7) xy_spd++;//speed down
						else xy_spd=0;
					}
				}
				target->spd&=0b10001111;//clear x spd
				xy_spd<<=4;
				target->spd|=xy_spd;//set new x spd
			}
			else
			{
				xy_idle++;
				target->idletime&=0b00001111;//clear high 4 bit
				xy_idle<<=4;
				target->idletime|=xy_idle;
			}
		}
	}
}

#ifndef SDCC
void display()
{
	unsigned char mask=0b00000001;
	move(0,0+10*offset_bitmap);printw("....");
	move(0,40+10*offset_bitmap);printw("....");
	for(unsigned char i=1;i<=16;i++)
	{
		move(i,0);
		for(unsigned char k=0;k<offset_bitmap;k++) printw("          ");
		printw("..");
		for(unsigned char j=0;j<20;j++)
		{
			if(i<9)
			{
				if(framebuffer[j]&mask) attron(A_REVERSE);
				printw("  ");
				attroff(A_REVERSE);
			}
			else
			{
				if(framebuffer[j+20]&mask) attron(A_REVERSE);
				printw("  ");
				attroff(A_REVERSE);
			}
		}
		mask<<=1;
		if(mask==0) mask=0b00000001;
		printw("..");
	}
	move(17,0+10*offset_bitmap);printw("....");
	move(17,40+10*offset_bitmap);printw("....");
	move(20,0);
	printw("X=%3d Y=%3d",player.x,player.y);
	move(21,0);
	printw("X Speed=%3d X Idle=%d",obj_spd_x(&player),(player.idletime&0b11110000)>>4);
	move(22,0);
	printw("Y Speed=%3d Y Idle=%d",obj_spd_y(&player),player.idletime&0b00001111);
	move(23,0);
	printw("Current tile ID=%d\n",get_tileset(player.x,player.y));
	move(24,0);
	printw("Is on the floor=%d\n",obj_torches_floor(&player));
	move(25,0);
	printw("Is under the ceiling=%d\n",obj_torches_ceiling(&player));
	move(26,0);
	printw("Touches left wall=%d\n",obj_torches_left_wall(&player));
	move(27,0);
	printw("Touches right wall=%d\n",obj_torches_right_wall(&player));
}
#endif

void render_texture(const unsigned char* texture,unsigned char x,unsigned char y)
{
	//texture size is always 5x8
	//check if object is in the bitmap range
	if(x >= offset_x+5*offset_bitmap+BITMAP_WIDTH) return;
	if(x+4 < offset_x+5*offset_bitmap) return;
	if(y+7 < offset_y) return;
	if(y >= offset_y+BITMAP_HEIGHT) return;
	for(unsigned char i=0;i<5;i++)
	{
		if(x+i < offset_x+5*offset_bitmap) continue;
		if(x+i >= offset_x+5*offset_bitmap+BITMAP_WIDTH) continue;
		unsigned char relative_x=x - offset_x - 5*offset_bitmap + i;
		unsigned char relative_y;
		if(y >= offset_y)
		{
			relative_y=y-offset_y;
			if(relative_y>8)//lower part of texture trimmed
			{
				framebuffer[relative_x+20]|=texture[i]<<(relative_y-8);
			}
			else//full texture rendered
			{
				framebuffer[relative_x]|=texture[i]<<relative_y;
				framebuffer[relative_x+20]|=texture[i]>>(8-relative_y);
				
			}
		}
		else//upper part of texture trimmed
		{
			relative_y=offset_y-y;
			framebuffer[relative_x]|=texture[i]>>relative_y;
			//framebuffer[relative_x+20]|=texture[i]<<(8-relative_y);
		}
	}
}

void render_object(struct obj *target)
{
	render_texture(obj_data_list[target->type].texture,target->x,target->y);
}

void render_tiles()
{
	unsigned char blk_x=offset_x-offset_x%5+offset_bitmap*5;
	unsigned char blk_y=offset_y-offset_y%8;
	for(unsigned char i=0;i<=16;i+=8)
	{
		if(blk_y+i >= MAP_HEIGHT) continue;//boundary check
		for(unsigned char j=0;j<=25;j+=5)
		{
			if(blk_x+j >= MAP_WIDTH) continue;//boundary check
			render_texture(tileset[get_tileset(blk_x+j,blk_y+i)].texture,blk_x+j,blk_y+i);
		}
	}
}

#ifdef SDCC
/*void get_input()//vim keymap
{
	input=0;
	if(P3_1==0) input|=0b00100000;//left
	if(P3_0==0) input|=0b01000000;//down
	if(P3_2==0) input|=0b00000100;//jump
	if(P3_3==0)	input|=0b00010000;//right
}*/
void get_input()
{
	input=0;
	P1=0b01111111;//first line
	if(P1_2==0) input|=0b00000100;//jump
	P1=0b10111111;
	if(P1_3==0) input|=0b00100000;//left
	if(P1_1==0) input|=0b00010000;//right
	P1=0b11011111;
	if(P1_2==0) input|=0b01000000;//down
}
#else
void get_input()
{
	input=0;
	unsigned char resp=getch();
	//up down left right attack jump select start
	if(resp=='w') input|=0b00000100;
	if(resp=='a') input|=0b00100000;
	if(resp=='d') input|=0b00010000;
	if(resp=='s') input|=0b01000000;
	if(resp=='c') editor();
}
#endif

void handle_input()
{
	if(input&4)//jump
	{
		//make sure player is on the ground
		if(obj_torches_floor(&player))
		{
			//printf("Player is on the ground\n");
			player.spd&=0b11110000;
			player.spd|=0b00000001;//set to 1
			player.idletime&=0b11110000;//reset y idle time
		}
	}
	if(input&32)//left
	{
		unsigned char spd=obj_spd_x(&player);
		if(spd==0 || player.spd&0b10000000)
		{
			player.spd&=0b00001111;
			player.spd|=0b10010000;
			player.idletime&=0b00001111;//reset x idle time
		}
		else
		{
			spd++;//slow down
			player.spd&=0b00001111;
			player.spd|=spd<<4;
			player.idletime&=0b00001111;//reset x idle time
		}
	}
	else if(input&16)//right
	{
		unsigned char spd=obj_spd_x(&player);
		if(spd==0 || !(player.spd&0b10000000))
		{
			player.spd&=0b00001111;
			player.spd|=0b00010000;
			player.idletime&=0b00001111;//reset x idle time
		}
		else
		{
			spd++;//slow down
			player.spd&=0b10001111;
			player.spd|=spd<<4;
			player.idletime&=0b00001111;//reset x idle time
		}
	}
	if(input&64)//down
	{
		player.spd=0b00001001;
		player.idletime=0b00000000;//reset x-y idle time
	}
}

void frame()
{
	simulate(&player);
	for(unsigned char i=0;i<objects_size;i++)
	{
		simulate(objects+i);
	}
	//clear framebuffer
	for(unsigned char i=0;i<40;i++) framebuffer[i]=0;
	//adjust camera
	if(player.y-offset_y<4 && offset_y>0) offset_y--;
	else if(offset_y+SCREEN_HEIGHT-player.y-PLAYER_HEIGHT<4 && offset_y<MAP_HEIGHT-SCREEN_HEIGHT) offset_y++;
	if(player.x-offset_x-5*offset_bitmap<4)//move the camera left
	{
		if(offset_bitmap>0)
		{
			offset_bitmap--;
		}
		else if(offset_x>0)
		{
			offset_x--;
		}
	}
	else if(offset_x+5*offset_bitmap+BITMAP_WIDTH-player.x-PLAYER_WIDTH<4)//move the camera right
	{
		if(offset_bitmap<(SCREEN_WIDTH-BITMAP_WIDTH)/5)
		{
			offset_bitmap++;
		}
		else if(offset_x<SCREEN_WIDTH)
		{
			offset_x++;
		}
	}
	//offset_y=107;
	//render tiles
	render_tiles();
	//render obj
	render_object(&player);
	for(unsigned char i=0;i<objects_size;i++)
	{
		render_object(objects+i);
	}
	//display
	display();
	//handle user input
	handle_input();
	//get input for next frame
	get_input();
}

#ifdef SDCC
void init()
{
	player.x=0;
	player.y=111;
	player.spd=0;
	player.idletime=0;
	player.type=0;
	offset_x=0;
	offset_y=123;
	offset_bitmap=0;
	objects_size=0;
}
#else
void init()
{
	offset_x=0;
	offset_y=111;
	offset_bitmap=0;
	player.x=0;
	player.y=123;
	tileset[1].block=0x78;
	tileset[1].texture[0]=0b10101110;
	tileset[1].texture[1]=0b10101010;
	tileset[1].texture[2]=0b10101010;
	tileset[1].texture[3]=0b10111010;
	tileset[1].texture[4]=0b10000000;
	tileset[2].block=0x78;
	tileset[2].texture[0]=0b10111110;
	tileset[2].texture[1]=0b10100010;
	tileset[2].texture[2]=0b10100010;
	tileset[2].texture[3]=0b10100010;
	tileset[2].texture[4]=0b10000000;
	tileset[3].block=0x78;
	tileset[3].texture[0]=0b10111110;
	tileset[3].texture[1]=0b10100000;
	tileset[3].texture[2]=0b10100000;
	tileset[3].texture[3]=0b10100000;
	tileset[3].texture[4]=0b10000000;
	tileset[4].block=0x78;
	tileset[4].texture[0]=0b10111110;
	tileset[4].texture[1]=0b10001010;
	tileset[4].texture[2]=0b10001010;
	tileset[4].texture[3]=0b10111110;
	tileset[4].texture[4]=0b10000000;
	tileset[5].block=0x78;
	tileset[5].texture[0]=0b10111110;
	tileset[5].texture[1]=0b10101010;
	tileset[5].texture[2]=0b10101110;
	tileset[5].texture[3]=0b10111000;
	tileset[5].texture[4]=0b10000000;
	tileset[6].block=0x78;
	tileset[6].texture[0]=0b10001000;
	tileset[6].texture[1]=0b10001000;
	tileset[6].texture[2]=0b10101010;
	tileset[6].texture[3]=0b10011100;
	tileset[6].texture[4]=0b10001000;
	tileset[7].block=0x78;
	tileset[7].texture[0]=0b10111110;
	tileset[7].texture[1]=0b10101010;
	tileset[7].texture[2]=0b10101010;
	tileset[7].texture[3]=0b10101010;
	tileset[7].texture[4]=0b10000000;
	tileset[8].block=0x04;
	tileset[8].texture[0]=0b00001111;
	tileset[8].texture[1]=0b00001001;
	tileset[8].texture[2]=0b00001001;
	tileset[8].texture[3]=0b00001001;
	tileset[8].texture[4]=0b00001101;
	tileset[9].block=0x01;
	tileset[9].texture[0]=0b00000001;
	tileset[9].texture[1]=0b00000001;
	tileset[9].texture[2]=0b00000001;
	tileset[9].texture[3]=0b00000001;
	tileset[9].texture[4]=0b00000001;
	tileset[10].block=0x78;
	tileset[10].texture[0]=0b10111110;
	tileset[10].texture[1]=0b10001010;
	tileset[10].texture[2]=0b10001010;
	tileset[10].texture[3]=0b10001110;
	tileset[10].texture[4]=0b10000000;
	tileset[11].block=0x45;
	tileset[11].texture[0]=0b00010000;
	tileset[11].texture[1]=0b00010000;
	tileset[11].texture[2]=0b00010000;
	tileset[11].texture[3]=0b00010000;
	tileset[11].texture[4]=0b00010000;
	tileset[12].block=0x78;
	tileset[12].texture[0]=0b10000000;
	tileset[12].texture[1]=0b10000000;
	tileset[12].texture[2]=0b10000000;
	tileset[12].texture[3]=0b10000000;
	tileset[12].texture[4]=0b10000000;
	tileset[13].block=0x08;
	tileset[13].texture[0]=0b11111111;
	tileset[13].texture[1]=0b00000001;
	tileset[13].texture[2]=0b00000001;
	tileset[13].texture[3]=0b00000001;
	tileset[13].texture[4]=0b11111111;
	tileset[14].block=0x00;
	tileset[14].texture[0]=0b01111110;
	tileset[14].texture[1]=0b00000101;
	tileset[14].texture[2]=0b00001001;
	tileset[14].texture[3]=0b00010001;
	tileset[14].texture[4]=0b01111110;
	map[15][0]=0xcc;
	map[15][1]=0xcc;
	map[15][2]=0x3c;
	map[15][3]=0x54;
	map[15][4]=0x06;
	map[15][5]=0x98;
	map[15][6]=0xcc;
	map[15][7]=0x9b;
	map[15][8]=0x99;
	map[15][9]=0x99;
	map[15][10]=0x00;
	map[15][11]=0x00;
	map[15][12]=0x00;
	map[15][13]=0x90;
	map[15][14]=0x99;
	map[15][14]=0x99;
	map[15][15]=0x99;
	map[14][5]=0xd0;
	map[14][8]=0x0e;
	map[14][10]=0x17;
	map[14][11]=0x42;
	map[14][12]=0x7a;
	map[14][13]=0x06;
	map[14][14]=0x0b;
	map[14][15]=0xd9;
	map[13][14]=0x0b;
	map[13][15]=0xd9;
	map[12][14]=0x0b;
	map[12][15]=0xd0;
	map[12][13]=0x99;
	map[12][12]=0x99;
	map[12][11]=0x99;
	map[12][10]=0x99;
	map[11][10]=0x0d;
}
void write_def()
{
	printf("struct tile tileset[TILESET_SIZE]={");
	for(int i=0;i<TILESET_SIZE;i++)
	{
		printf("{%u,{%u,%u,%u,%u,%u}}",tileset[i].block,tileset[i].texture[0],tileset[i].texture[1],tileset[i].texture[2],tileset[i].texture[3],tileset[i].texture[4]);
		if(i!=TILESET_SIZE-1) printf(",");
	}
	printf("};\n");
	
	printf("struct obj_data obj_data_list[OBJ_DATA_LIST_SIZE]={");
	for(int i=0;i<OBJ_DATA_LIST_SIZE;i++)
	{
		printf("{%u,{%u,%u,%u,%u,%u}}",obj_data_list[i].size,obj_data_list[i].texture[0],obj_data_list[i].texture[1],obj_data_list[i].texture[2],obj_data_list[i].texture[3],obj_data_list[i].texture[4]);
		if(i!=OBJ_DATA_LIST_SIZE-1) printf(",");
	}
	printf("};\n");
	printf("unsigned char map[16][16]={");
	for(int i=0;i<16;i++)
	{
		printf("{");
		for(int j=0;j<16;j++)
		{
			printf("{%u}",map[i][j]);
			if(j!=15) printf(",");
		}
		printf("}");
		if(i!=15) printf(",");
	}
	printf("};\n");
	exit(0);
}

void set_tileset(unsigned char x,unsigned char y,unsigned char target)
{
	if(x%2==0)
	{
		map[y][x/2]&=0b11110000;
		map[y][x/2]|=target;
		return;
	}
	map[y][x/2]&=0b00001111;
	map[y][x/2]|=target<<4;
}

void editor_display(int x,int y,int highlight_x,int highlight_y,unsigned char map_edit_mode,int tile_x,int tile_y)
{
	move(0,0);
	for(int i=0;i<LINES-8-8;i++)
	{
		move(i,0);
		if(y+i >= MAP_HEIGHT) break;
		for(int j=0;j<(COLS-5)/2;j++)
		{
			if(x+j >= MAP_WIDTH) break;
			int target=get_tileset(x+j,y+i);
			int k=(x+j)%5;
			int z=(y+i)%8;
			if(tileset[target].texture[k]&(0b00000001<<z)) attron(A_REVERSE);
			if((x+j)/5==highlight_x && (y+i)/8==highlight_y && map_edit_mode) printw("..");
			else printw("  ");
			attroff(A_REVERSE);
		}
	}
	for(int i=LINES-8;i<LINES;i++)
	{
		move(i,0);
		for(int j=0;j<80;j++)
		{
			unsigned char floor=tileset[j/5].block&0b00001111;
			unsigned char ceiling=tileset[j/5].block&0b11110000;
			ceiling>>=4;
			int k=j%5;
			int z=i-(LINES-8);
			if(tileset[j/5].texture[k]&(0b00000001<<z)) attron(A_REVERSE);
			if(map_edit_mode)
			{
				if(i-(LINES-8)<floor && i-(LINES-8)>=ceiling) printw("..");
				else printw("  ");
			}
			else
			{
				if(i-(LINES-8)==tile_y && j==tile_x) printw("[]");
				else printw("  ");
			}
			attroff(A_REVERSE);
		}
	}
}

void editor()
{
	static int x=0,y=0;
	static int highlight_x=0,highlight_y=0;
	static int tile_x=0,tile_y=0;
	unsigned char map_edit=1;
	while(true)
	{
		editor_display(x,y,highlight_x,highlight_y,map_edit,tile_x,tile_y);
		int resp=getch();
		switch(resp)
		{
			case 'd':x++;break;
			case 'a':if(x>0) x--;break;
			case 's':y++;break;
			case 'w':if(y>0) y--;break;
			case 'l':
				if(map_edit)
				{
					if(highlight_x<MAP_BLK_WIDTH-1) highlight_x++;
				}
				else
				{
					if(tile_x<80-1) tile_x++;
				}
				break;
			case 'j':
				if(map_edit)
				{
					if(highlight_x>0) highlight_x--;
				}
				else
				{
					if(tile_x>0) tile_x--;
				}
				break;
			case 'k':
				if(map_edit)
				{
					if(highlight_y<MAP_BLK_HEIGHT-1)highlight_y++;
				}
				else
				{
					if(tile_y<8-1) tile_y++;
				}
				break;
			case 'i':
				if(map_edit)
				{
					if(highlight_y>0) highlight_y--;
				}
				else
				{
					if(tile_y>0) tile_y--;
				}
				break;
			case '0':set_tileset(highlight_x,highlight_y,0);break;
			case '1':set_tileset(highlight_x,highlight_y,1);break;
			case '2':set_tileset(highlight_x,highlight_y,2);break;
			case '3':set_tileset(highlight_x,highlight_y,3);break;
			case '4':set_tileset(highlight_x,highlight_y,4);break;
			case '5':set_tileset(highlight_x,highlight_y,5);break;
			case '6':set_tileset(highlight_x,highlight_y,6);break;
			case '7':set_tileset(highlight_x,highlight_y,7);break;
			case '8':set_tileset(highlight_x,highlight_y,8);break;
			case '9':set_tileset(highlight_x,highlight_y,9);break;
			case '-':set_tileset(highlight_x,highlight_y,10);break;
			case '=':set_tileset(highlight_x,highlight_y,11);break;
			case 'o':set_tileset(highlight_x,highlight_y,12);break;
			case 'p':set_tileset(highlight_x,highlight_y,13);break;
			case '[':set_tileset(highlight_x,highlight_y,14);break;
			case ']':set_tileset(highlight_x,highlight_y,15);break;
			case 'z':
			{
				unsigned char mask=0b00000001<<tile_y;
				unsigned char target=tile_x/5;
				tileset[target].texture[tile_x%5]|=mask;
			}
			break;
			case 'x':
			{
				unsigned char mask=0b00000001<<tile_y;
				mask=~mask;
				unsigned char target=tile_x/5;
				tileset[target].texture[tile_x%5]&=mask;
			}
			break;
			case 'f':map_edit=!map_edit;break;
		}
		if(resp=='c') break;
	}
}
#endif
