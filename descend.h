#ifndef DESCEND_H
#define DESCEND_H

#ifdef SDCC
#include <8052.h>
#else
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#endif

struct obj
{
	unsigned char x,y;
	unsigned char spd;
	//sxxxsyyy
	//x-sign x-speed y-sign y-speed
	//not a conventional speed! value = extra frame required to move 1 pixel
	unsigned char idletime;
	//xxxxyyyy
	//frames elapsed after last move
	//size of object
	unsigned char type;
	//0 player
	//1 bullet
};

struct obj_data
{
	unsigned char size;
	//xxxxyyyy
	unsigned char texture[5];
};//store properties of objects with same type

struct tile
{
	unsigned char block;
	//ccccffff
	//ceiling, can be 0-8
	//floor, can be 0-8
	//celing must less than floor to work
	//otherwise tile is an air block
	unsigned char texture[5];
};

#ifdef SDCC
extern __pdata unsigned char framebuffer[];
#else
extern unsigned char framebuffer[40];
#endif
extern unsigned char offset_bitmap;

void obj_info(struct obj *target);

unsigned char obj_torches_floor(struct obj *target);

unsigned char obj_torches_ceiling(struct obj *target);

unsigned char obj_torches_left_wall(struct obj *target);

unsigned char obj_torches_right_wall(struct obj *target);

unsigned char obj_spd_y(struct obj *target);

unsigned char obj_spd_x(struct obj *target);

void simulate(struct obj *target);

void display();

void render_texture(const unsigned char* texture,unsigned char x,unsigned char y);

void render_object(struct obj *target);

void render_tiles();

void get_input();

void handle_input();

void frame();

unsigned char spd_test(unsigned char idle,unsigned char spd);

#ifdef SDCC
void init();
#else
void init();
void write_def();
void editor();
#endif

#endif
