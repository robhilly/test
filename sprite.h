/*basic sprite engine*/
#ifndef SPRITE_H
#define SPRITE_H

#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"

/*CONSTANTS*****/
/*	-SCREEN    */
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP 32

/*	-BITMAP		*/
#define BITMAP_STATE_DEAD 0
#define BITMAP_STATE_ALIVE 1
#define BITMAP_STATE_DYING 2
#define BITMAP_ATTR_LOADED 128
#define BITMAP_EXTRACT_MODE_CELL 0
#define BITMAP_EXTRACT_MODE_ABS 1

/*	-SPRITE		*/
#define SPR_STATE_DEAD			0
#define SPR_STATE_ALIVE			1
#define SPR_STATE_DYING			2
#define SPR_STATE_ANIM_DONE		1
#define SPR_MAX_FRAMES			64 
#define SPR_MAX_ANIMATIONS		16
#define SPR_ATTR_SINGLE_FRAME	1 
#define SPR_ATTR_MULTI_FRAME	2 
#define SPR_ATTR_MULTI_ANIM		4 
#define SPR_ATTR_ANIM_ONE_SHOT	8
#define SPR_ATTR_VISIBLE		16
#define SPR_ATTR_BOUNCE			32
#define SPR_ATTR_WRAPAROUND		64
#define SPR_ATTR_LOADED			128

/*	-TIME		*/
#define TICK_INTERVAL 25

#define RGB(r, g, b) SDL_MapRGB(screen->format, r, g, b)

typedef struct spriteObj{
	int state, anim_state, attr;
	int x, y;
	int xv, yv;
	int bpp;
	int w,h;
	int counter_1, counter_2;
	int max_count_1, max_count_2;
	int curframe, numframes;
	int cur_anim, anim_counter;
	int anim_index, anim_counter_max;
	int *animations[SPR_MAX_ANIMATIONS];
	SDL_Surface* frames[SPR_MAX_FRAMES];
}sprite;

/***function prototypes***/
/*		return 0 on error, otherwise 1 or positive_sign*/

/*		-SPRITE MANAGEMENT FUNCTIONS			*/

int spr_create(sprite* s, int x, int y, int w, int h, int num_frames, int attr);
int spr_free(sprite* s);
int spr_draw(sprite* s, SDL_Surface* surf);
int spr_draw_scaled(sprite* s, int sw, int sh, SDL_Surface* surf);
int spr_load_frames(sprite* s, SDL_Surface* surf, int frame, int cx, int cy,int bor=0, int mode=0);
int spr_animate(sprite* s);
int spr_move(sprite* s);
int spr_load_animation(sprite* s, int anim_index, int num_frames, int *sequence);
int spr_set_pos(sprite* s, int x, int y);
int spr_set_vel(sprite* s, int xv, int yv);
int spr_set_anim_rate(sprite* s, int rate);
int spr_set_animation(sprite* s, int index);
int spr_hide(sprite* s);
int spr_show(sprite* s);

/*returns: true if collision occured, false otherwise*/
int spr_collision(sprite* s1, sprite* s2);

extern SDL_Surface* screen;
extern SDL_Event event;
extern int scr_width, scr_height, scr_bpp;


#endif