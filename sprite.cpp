

#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"

#include "sprite.h"


SDL_Surface* screen;
SDL_Event event;


/**HELPER FUNCS******/
SDL_Surface* CreateSurface(Uint32 flags,int width,int height,const SDL_Surface* display = screen)
{
  // 'display' is the surface whose format you want to match
  //  if this is really the display format, then use the surface returned from SDL_SetVideoMode

  const SDL_PixelFormat& fmt = *(display->format);
  return SDL_CreateRGBSurface(flags,width,height,
                  fmt.BitsPerPixel,
                  fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask );
}

/*****FUNCTION DEFS********/

int spr_create(sprite* ps, int x, int y, int w, int h, int num_frames, int attr = 0)
{
	int i;
	
	
	
	
	
	/*set sprite members to params*/
	ps->x = x;
	ps->y = y;
	ps->w = w;
	ps->h = h;
	ps->xv = 0;
	ps->yv = 0;
	ps->attr = attr;
	/*default state of sprite (alive)*/
	ps->state = SPR_STATE_ALIVE;
	ps->anim_state = 0;
	ps->curframe = 0;
	ps->numframes = num_frames;
	ps->attr = attr;
	
	ps->counter_1 = 0;
	ps->counter_2 = 0;
	ps->max_count_1 = 0;
	ps->max_count_2 = 0;
	ps->cur_anim = 0;
	ps->anim_counter = 0;
	ps->anim_counter_max = 0;
	ps->anim_index = 0;
	

	/*set frames and animation array to NULL*/
	for(i = 0; i < SPR_MAX_FRAMES; i++){
		ps->frames[i] = NULL;
	}
	
	for(i = 0; i < SPR_MAX_ANIMATIONS; i++){
		ps->animations[i] = NULL;
	}
	
	return 1;
}
int spr_free(sprite* s){
	int i;
	
	/*clear animations*/
	for(i = 0; i < SPR_MAX_ANIMATIONS; i++){
		if(s->animations[i]){
			free(s->animations[i]);
			s->animations[i] = NULL;
		}
	}
	
	for(i = 0; i < s->numframes; i++){
		if(s->frames[i]){
			SDL_FreeSurface(s->frames[i]);
			s->frames[i] = NULL;
		}
	}
	
	return 1;
}
int spr_draw(sprite* s, SDL_Surface* surf)
{
	SDL_Rect r_sp;
	SDL_Rect r_surf;
	
	if(s == NULL || surf == NULL){
		return 0;
	}
	
	/*define sprite and destination surface rects*/
	r_sp.x = 0;
	r_sp.y = 0;
	r_sp.w = s->w;
	r_sp.h = s->h;
	
	r_surf.x = s->x;
	r_surf.y = s->y;
	r_surf.w = s->w;
	r_surf.h = s->h;
	
	fprintf(stderr, "Outputting frame[%i] of animation[%i]\n", s->curframe, s->cur_anim);
	
	if(SDL_BlitSurface(s->frames[s->curframe], &r_sp, surf, &r_surf)<0){
		fprintf(stderr,"Sprite blit failed: %s\n", SDL_GetError());
		return 0;
	}
	
	return 1;
}

/*Load into sprite frame array from a surface holding the bitmap, given frame position cell wise (cx, cy) if mode = 0, else absolute pixel coords are used */
int spr_load_frame(sprite* s, SDL_Surface* surf, int frame, int cx, int cy, int bor = 0, int mode = 0)
{
	SDL_Rect sRec;
	SDL_PixelFormat* sf;
	
	if(frame >= s->numframes){
		fprintf(stderr, "frame index exceeds sprite's max frames: i=%i,mfr=%i\n", frame, s->numframes);
		return 0;
	}
	
	if(mode == BITMAP_EXTRACT_MODE_CELL){
		/*calculate cell coords given width and height and border*/
		cx = cx * (s->w + bor) + bor;
		cy = cy * (s->h + bor) + bor;
	}
	
	sRec.x = cx;
	sRec.y = cy;
	sRec.w = s->w;
	sRec.h = s->h;
	
	sf = screen->format;

	/*convert surface  to frame (screen) format*/
	surf = SDL_ConvertSurface(surf, sf, 0);
	
	if(surf == NULL){
		fprintf(stderr, "\nUnable to convert surface bitmap to screen format: %s", SDL_GetError());
		return 0;
	}
	
/***
SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int
depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);


typedef struct{
SDL_Palette *palette;
Uint8 BitsPerPixel;
Uint8 BytesPerPixel;
Uint32 Rmask, Gmask, Bmask, Amask;
Uint8 Rshift, Gshift, Bshift, Ashift;
Uint8 Rloss, Gloss, Bloss, Aloss;
Uint32 colorkey;
Uint8 alpha;
} SDL_PixelFormat;
**/	
	
	
	if(s->frames[frame] == NULL){
		fprintf(stderr,"Creating blank surface for frame:%i\n", frame);
		s->frames[frame] = SDL_CreateRGBSurface(0, s->w, s->h, sf->BitsPerPixel,
			sf->Rmask, sf->Gmask, sf->Bmask, sf->Amask);
		if(s->frames[frame] == NULL){
			fprintf(stderr,"\nError creating surface for frame[%i]: %s",frame, SDL_GetError());
			//return 0;
		}

	}
	else{
		fprintf(stderr,"Frame:%i is not NULL, can't load frame:%s", frame, SDL_GetError());
		return 0;
	}
	fprintf(stderr, "Surface for frame created successfully\n");
	
	/*copy frame to sprite array*/
	if(SDL_BlitSurface(surf, &sRec, s->frames[frame], NULL)<0){
		fprintf(stderr,"Sprite frame copy failed: %s\n", SDL_GetError());
		return 0;
	}
	
	
	
	if(!s->frames[frame]){
		fprintf(stderr,"Error converting frame surface:%s\n", SDL_GetError());
		return 0;
	}
	
	s->attr |= SPR_ATTR_LOADED;
	
	return 1;
}

/*load animation sequence with an extra element with end signal (-1)*/
int spr_load_animation(sprite* s, int anim_index, int num_frames, int *sequence)
{
	int index;
	
	if(anim_index >= SPR_MAX_ANIMATIONS){
		fprintf(stderr, "Index exceeds max animations allowed for sprite");
	}
	
	/*allocate memory for animation sequence*/
	s->animations[anim_index] = (int*)malloc((num_frames+1) * sizeof(int));
	if(!s->animations[anim_index]){
		fprintf(stderr, "Error allocating animation sequence");
		return 0;
	}
	
	/*iterate through sequence copying to sprite animation array*/
	for(index = 0; index < num_frames; index++){
		s->animations[anim_index][index] = sequence[index];
	}
	/*set end signal for sequence*/
	s->animations[anim_index][index] = -1;
	
	return 1;
}

int spr_animate(sprite* s)
{
	if(!s){
		fprintf(stderr, "Invalid sprite object to animate\n");
		return 0;
	}
	
	if(s->attr & SPR_ATTR_SINGLE_FRAME){
		s->curframe = 0;
		return 1;
	}
	else if(s->attr & SPR_ATTR_MULTI_FRAME){
		if(++s->anim_counter >= s->anim_counter_max){
			/*update to next frame in animation*/
			s->anim_counter = 0;
			if(++s->curframe >= s->numframes){
				/*reset animation to first frame*/
				s->curframe = 0;
			}
		}
	}
	else if(s->attr & SPR_ATTR_MULTI_ANIM){
		if(++s->anim_counter >= s->anim_counter_max){
			s->anim_counter = 0;
			/*increment animation index (for multiple animations)*/
			s->anim_index++;
			/*grab next frame and see if its end of anim (-1)*/
			s->curframe = s->animations[s->cur_anim][s->anim_index];
			if(s->curframe == -1){
				/*check if one time animation, if so set frame to last in sequence*/
				if(s->attr & SPR_ATTR_ANIM_ONE_SHOT){
					s->anim_state = SPR_STATE_ANIM_DONE;
					s->anim_index--;
					s->curframe = s->animations[s->cur_anim][s->anim_index];
				}else{
					/*reset animation to first frame in sequence*/
					s->anim_index = 0;
					s->curframe = s->animations[s->cur_anim][s->anim_index];
				}
			}
		}
	}
	return 1;
}

Uint32 TimeLeft(void)
{
	static Uint32 next_tick = 0;
	Uint32 cur_tick;
		
	cur_tick = SDL_GetTicks();
	if(next_tick <= cur_tick){
		next_tick = cur_tick + TICK_INTERVAL;
		return 0;
	}else{
		return (next_tick - cur_tick);
	}
}


int main(int argc, char* argv[])
{
	//SDL_Surface* screen;
	//SDL_Event event;
	Uint8 *p;
	Uint32 pix;
	SDL_Surface* pl_bmp;
	SDL_Surface* exp_bmp;
	
	sprite plane;
	
	int pl_anim[] = {0,1,2,3,4,5,6,7};
	int exp_anim[] = {8,9,10,11,12,13};
	
	/*line variables*/
	int x0, y0, x1, y1;
	int dx, dy, x_inc, y_inc;
	int error;
	int index;
	int i2 = 0;
	int a_st = 0;
	
	int loop = 1;
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		fprintf(stderr, "\nUnable to initialize SDL: %s", SDL_GetError());
		exit(1);
	}
	
	fprintf(stderr, "Video initialized\n");
	
	atexit(SDL_Quit);
	
	/*initialize video surface*/
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0);
	if(screen==NULL){
		fprintf(stderr, "\nUnable to set Video Mode: %s", SDL_GetError());
		SDL_Quit();
		exit(1);
	}
	fprintf(stderr, "Screen set up\n");
	if((pl_bmp = SDL_LoadBMP("pl1.bmp")) == NULL){
		fprintf(stderr,"\nUnable to load bitmap");
		SDL_Quit();
		exit(1);
	}

	if((exp_bmp = SDL_LoadBMP("ex1.bmp")) == NULL){
		fprintf(stderr,"\nUnable to load bitmap");
		SDL_Quit();
		exit(1);
	}	
	
	;
	if((spr_create(&plane, 100, 100, 32, 32, 14, SPR_ATTR_MULTI_ANIM)) == 0){
		fprintf(stderr, "Unable to initialize sprite object\n");
		SDL_Quit();
		exit(1);
	}
	
	fprintf(stderr,"Sprite initialized\n");
	
	/**
	int spr_load_frame(sprite* s, SDL_Surface* surf, int frame, int cx, int cy, int bor = 0, int mode = 0)
	**/
	for(index = 0; index < 8; index++){
		if(spr_load_frame(&plane, pl_bmp, index, index, 0, 1, 0) <= 0){
			fprintf(stderr, "Frame loading of plane failed\n");
		}
		i2++;
	}
	
	SDL_FreeSurface(pl_bmp);
	
	for(index = 0; index < 6; index++){
		if(spr_load_frame(&plane, exp_bmp, i2++, index, 0, 1, 0) <= 0){
			fprintf(stderr, "Frame loading of explosion for plane sprite failed\n");
		}
	}
	
	SDL_FreeSurface(exp_bmp);
	
	if(spr_load_animation(&plane, 0, 8, pl_anim) == 0){
		fprintf(stderr,"plane anim failed to load\n");
	}
	if(spr_load_animation(&plane, 1, 6, exp_anim) == 0){
		fprintf(stderr,"exp anim failed to load\n");
	}
	
	fprintf(stderr,"Anim[0]:");
	for(index = 0; plane.animations[0][index]!=-1;index++){
		fprintf(stderr," %i", plane.animations[0][index]);
	}
	fprintf(stderr,"\n");
	
	fprintf(stderr,"Anim[1]:");
	for(index = 0; plane.animations[1][index]!=-1;index++){
		fprintf(stderr," %i", plane.animations[1][index]);
	}
	fprintf(stderr,"\n");
	
	
	plane.anim_counter_max = 3;
	
	fprintf(stderr, "Entering loop\n");;
	while(loop){
		while( SDL_PollEvent( &event ) ){
			/* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
			switch( event.type ){
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym){
						case SDLK_LEFT:
							a_st = 0;
							break;
						
						case SDLK_RIGHT:
							a_st = 1;
							break;
							
						case SDLK_UP:
							loop = 0;
							break;
							
						default: break;
						
					}
					fprintf(stderr, "Key press detected\n" );
					break;
				case SDL_KEYUP:
					fprintf(stderr,  "Key release detected\n" );
					break;
				default:
					break;
			}
		}
		
		plane.cur_anim = a_st;
		
		spr_animate(&plane);
		spr_draw(&plane, screen);
		
		
		/*update screen*/
		SDL_UpdateRect(screen, 0, 0, 0, 0);
		SDL_Delay(TimeLeft());
		
	}
	
	spr_free(&plane);
	
	SDL_Quit();
	
	return 0;
}