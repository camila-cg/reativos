#include "SDL2/SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREENSIZEX 800
#define SCREENSIZEY 600
#define SHIPACC 1200
// timings
int i,now, old, start, gametimer, diff;
int frames = 0;

void init();
void update(int dt,int now);
void render();
typedef struct {
  SDL_Rect  r;
  float   x,y;
  float   vx,vy;
  float   acc;
  float   ax,ay;
  double  ang;
  int     timer;
} Player;

typedef struct {
  SDL_Rect  r;
  float   x,y;
  float   vx,vy;
  short   on;
  short   type;
} Tiro;

typedef struct {
  SDL_Rect  r;
  float   x,y;
  float   vx,vy;
  float   acc;
  float   ax,ay;
  short   on;
  short   type;
  double  ang;
  int     timer;
  short   state;
} Inimigo;

typedef struct {
  SDL_Rect  r;
  float   y;
  short type;
} BG;

typedef struct {
  SDL_Rect  r;
  float   x,y;
  float   vx,vy;
} Cloud;

SDL_Window* win;
SDL_Renderer* ren;
SDL_Surface* surfLoader;
SDL_Texture* img[6];

char* files[] = {
  "img.bmp",
  "Tower.bmp",
  "heli.bmp",
  "tank.bmp",
  "back.bmp",
  "GameOver.bmp"
};

SDL_Rect black = {0, 0, SCREENSIZEX, SCREENSIZEY};
SDL_Rect bgsrc = {0,0,200,150};
// up,down,left,right,shoot,enter
short keyb[6] = {0,0,0,0,0,0};
// entities
Player nave;
Inimigo inimigo[50];
Tiro tiros[50];
Cloud nuvem[20];
BG bg[2];
// tiro iterator/ enemy iterator
short iT=0;
short iE=0;
SDL_Color tiroC;
// 2 playing, 0 failscreen
short gamestate=2;

void shoot(float x, float y, float spd, double ang, short type){
  tiros[iT].on = 1;
  tiros[iT].x = x;
  tiros[iT].y = y;
  tiros[iT].vx = sin(ang/180.0*M_PI)*abs(spd);
  tiros[iT].vy = cos(ang/180.0*M_PI)*spd;
  tiros[iT].type = type;
  iT++;
  if(iT==50)iT=0;
}

void nave_init(){
  nave.r.y=525;
  nave.x = 375;
  nave.r.w=50;
  nave.r.h=50;
  nave.timer=now;
  nave.vx =0;
  nave.vy =0;
  nave.acc=1200;
}
void nave_render(){
  nave.r.x = (int)nave.x;
  SDL_RenderCopyEx(ren,img[0],NULL,&nave.r,nave.ang,NULL, SDL_FLIP_NONE);
}
void nave_update(int dt,int now){
  // keyboard -> acc's
  nave.ax=0;
  if(nave.vx!=0)
    nave.ax=(nave.vx>0?-1:1)*nave.acc*(nave.x<75||nave.x>675?2.5:0.7);
  if(keyb[2])
    nave.ax=nave.acc*(nave.vx>0 ? -1.5: -1);
  else if(keyb[3])
    nave.ax=nave.acc*(nave.vx>0 ? 1: 1.5);

  // speed and position updates
  nave.vx+=nave.ax*(dt/1000.0);
  nave.x+=nave.vx*(dt/1000.0);
  if(nave.vx>1000)nave.vx=1000;
  if(nave.vx<-1000)nave.vx=-1000;
  // rotation angle
  nave.ang = (atan(nave.vx/3000.0))*180.0/M_PI;
  // wraparound
  if(nave.x<-50)nave.x+=SCREENSIZEX+50;
  if(nave.x>SCREENSIZEX)nave.x-=SCREENSIZEX+50;

  // shoot every 250 ms
  if(keyb[4] && nave.timer<now){
    nave.timer = now+250;
    shoot(nave.x+23, nave.r.y+10, -1200, nave.ang, 0);
  }
}

void tiros_init(){
  for(i=0;i<50;i++){
    tiros[i].r.x=-5;
    tiros[i].r.y=-20;
    tiros[i].r.w=5;
    tiros[i].r.h=5;
    tiros[i].on = 0;
  }
  tiroC.r=255;
  tiroC.g=0;
  tiroC.b=0;
}
void tiros_render(){
  SDL_SetRenderDrawColor(ren, tiroC.r, tiroC.g, tiroC.b, 0xFF);
  for(i=0;i<50;i++){
    tiros[i].r.x=(int)tiros[i].x;
    tiros[i].r.y=(int)tiros[i].y;
    if(tiros[i].on){
      SDL_RenderFillRect(ren, &tiros[i].r);
    }
  }
}
void tiros_update(int dt,int now){
  for(i=0;i<50;i++){
    if(tiros[i].on){
      tiros[i].x+=tiros[i].vx*(dt/1000.0);
      tiros[i].y+=tiros[i].vy*(dt/1000.0);
    }
    if(tiros[i].y<-50)tiros[i].on = 0;
  }
}
// helicopter
void create_weaver(short rtl, float xoff){
  inimigo[iE].on=1;
  inimigo[iE].y=-50;
  inimigo[iE].r.w=50;
  inimigo[iE].r.h=50;
  inimigo[iE].vy=200;
  inimigo[iE].timer=now;
  inimigo[iE].type=1;
  if(rtl){
    inimigo[iE].x=775-xoff;
    inimigo[iE].state=1;
  }else{
    inimigo[iE].x=25+xoff;
    inimigo[iE].state=0;
  }
  iE++;
  if(iE>=50)iE=0;
}
// tower
void create_still(float x){
  inimigo[iE].on=1;
  inimigo[iE].x=x;
  inimigo[iE].y=-50;
  inimigo[iE].r.w=50;
  inimigo[iE].r.h=50;
  inimigo[iE].vx=0;
  inimigo[iE].vy=300;
  inimigo[iE].ax=0;
  inimigo[iE].ang=0;
  inimigo[iE].type=0;
  iE++;
  if(iE>=50)iE=0;
}
// tank
void create_driver(float x, float vx, float ax){
  inimigo[iE].on=1;
  inimigo[iE].x=x;
  inimigo[iE].y=-50;
  inimigo[iE].r.w=50;
  inimigo[iE].r.h=50;
  inimigo[iE].vx=vx;
  inimigo[iE].ax=ax;
  inimigo[iE].vy=75;
  inimigo[iE].type=2;
  iE++;
  if(iE>=50)iE=0;
}

void inimigos_init(){
  for(i=0;i<50;i++){
    inimigo[i].x=-50;
    inimigo[i].y=-50;
    inimigo[i].on=0;
  }
}
void inimigos_render(){
  // tower heli tank  on img
  // 1     2    3
  for(i=0;i<50;i++){
    inimigo[i].r.x=(int)inimigo[i].x;
    inimigo[i].r.y=(int)inimigo[i].y;
    if(inimigo[i].on){
      SDL_RenderCopyEx(ren,img[inimigo[i].type+1],NULL,&inimigo[i].r,inimigo[i].ang,NULL, SDL_FLIP_NONE);
    }
  }
}
void inimigos_update(int dt,int now){
  for(i=0;i<50;i++){
    if(inimigo[i].on){
      // upd pos and spd
      inimigo[i].vx+=inimigo[i].ax*(dt/1000.0);
      inimigo[i].x+=inimigo[i].vx*(dt/1000.0);
      inimigo[i].vy+=inimigo[i].ay*(dt/1000.0);
      inimigo[i].y+=inimigo[i].vy*(dt/1000.0);

      if(inimigo[i].y<-50)inimigo[i].on = 0;
      switch(inimigo[i].type){
        case 0:// tower
          if(now>inimigo[i].timer){
            inimigo[i].timer = now + 1000;
            shoot(inimigo[i].x+23, inimigo[i].y+25, 600,0, 1);
          }
          break;
        case 1:// helicopter
          inimigo[i].ax=(inimigo[i].state==1?1:-1)*400;
          if(inimigo[i].timer<=now){
            inimigo[i].timer=now+600;
            inimigo[i].state=!inimigo[i].state;
          }
          inimigo[i].ang = (atan(inimigo[i].vx/200.0))*180.0/M_PI;

          if(inimigo[i].vx>250)inimigo[i].vx=250;
          if(inimigo[i].vx<-250)inimigo[i].vx=-250;
          break;
        case 2:// driver
          inimigo[i].ax = (inimigo[i].x>nave.x)?-75:75;
          if(now>inimigo[i].timer){
            inimigo[i].timer = now + 2000;
            shoot(inimigo[i].x+23, inimigo[i].y+25, 600, 0, 1);
          }

          inimigo[i].ang = (atan(inimigo[i].vx/200.0))*180.0/M_PI;
          if(inimigo[i].vx>100)inimigo[i].vx=100;
          if(inimigo[i].vx<-100)inimigo[i].vx=-100;
      }
    }
  }
}

void colisao_update(){
  short ie, it;
  for(ie=0;ie<50;ie++){
    if(inimigo[ie].on){
      for(it=0;it<50;it++){// enemy-tiros colision
        if(tiros[it].on && tiros[it].type==0){
          SDL_bool col = SDL_HasIntersection(&inimigo[ie].r,&tiros[it].r);
          if(col){// Bang! You killed someone
            tiros[it].on=0;
            inimigo[ie].on=0;
          }
        }
      }
      if(inimigo[ie].type<2){// enemy-player colision
        SDL_bool col = SDL_HasIntersection(&inimigo[ie].r,&nave.r);
        if(col){// Bang *dead*!
          inimigo[ie].on=0;
          gamestate=0;
        }
      }
    }
  }
  for(it=0;it<50;it++){// tiros-player colision
    if(tiros[it].on && tiros[it].type==1){
      SDL_bool col = SDL_HasIntersection(&tiros[it].r,&nave.r);
      if(col){// Bang *dead*!
        tiros[it].on=0;
        gamestate=0;
      }
    }
  }
}
void game_update(int now){
  if(now>gametimer){// random enemy generation
    gametimer=(((rand() % 7) + 2)*125)+now;
    short t1, q1, t2, q2, aux;
    t1=(rand()%5)/2;
    q1=((rand()%4)+1)/2;
    t2=(rand()%5)-2;
    q2=((rand()%3)+1)/2;
    t2=t2<0?0:t2;

    while(q1--){
      switch(t1){
        case 0:
          create_still((rand()%550) +100);
          break;
        case 1:
          create_weaver(rand()%2,(rand()%120) -60);
          break;
        case 2:
          aux=(rand()%550) +100;
          if(aux>375){
            create_driver(aux, (rand()%100)-150,(rand()%10)+5);
          }else{
            create_driver(aux, (rand()%100)+50,(rand()%10)-15);
          }
          break;
      }
    }
  }
}
void game_init(){
  gametimer=now;
  create_still(200);
}

void fundo_init(){
  for(i=0;i<2;i++){
    bg[i].r.x=0;
    bg[i].r.h=600;
    bg[i].r.w=800;
    bg[i].y=i*600;
    bg[i].type = i;
  }
}
void fundo_update(int dt){
  for(i=0;i<2;i++){
    bg[i].y+=300*(dt/1000.0);
    if(bg[i].y>600){
      bg[i].type = rand()%3;
      bg[i].y -= 1200;
    }
  }
}
void fundo_render(){
  for(i=0;i<2;i++){
    bg[i].r.y = (int)bg[i].y;
    bgsrc.y=bg[i].type*150;
    SDL_RenderCopy(ren,img[4],&bgsrc,&bg[i].r);
  }
}

void menu_init(){

}
void menu_update(){
  if(keyb[5]){
    init();
    gamestate=2;
  }
}
void menu_render(){
  SDL_RenderCopy(ren,img[5],NULL,NULL);
}
void init(){
  old   = SDL_GetTicks();
  start = SDL_GetTicks();

  nave_init();
  tiros_init();
  inimigos_init();
  game_init();
  menu_init();
  fundo_init();
}

void render(){
  // fps calculation
  if ((now-start) > 1000) {
    printf("FPS = %d\n", frames);
    frames = 0;
    start = now;
  }
  frames++;

  // clear all
  SDL_SetRenderDrawColor(ren,255,255,0,0xFF);
  SDL_RenderFillRect(ren, &black);

  // render entities
  switch(gamestate){
    case 0:
      menu_render();
      break;
    case 2:
      fundo_render();
      tiros_render();
      inimigos_render();
      nave_render();
      break;
  }
  // update screen
  SDL_RenderPresent(ren);
}
// game logic
void update(int dt,int now){
  switch(gamestate){
    case 0:
      menu_update();
      break;
    case 2:
      nave_update(dt,now);
      tiros_update(dt,now);
      inimigos_update(dt,now);
      colisao_update();
      game_update(now);
      fundo_update(dt);
  }
}

int main (int argc, char *argv[]){
  time_t t;
  srand((unsigned) time(&t));

  SDL_Init(SDL_INIT_EVERYTHING);

  win = SDL_CreateWindow("Beach Raid", 200, 200, SCREENSIZEX, SCREENSIZEY, SDL_WINDOW_SHOWN);//SDL_WINDOW_BORDERLESS
  ren = SDL_CreateRenderer(win, -1, 0);

  SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

  // image loading
  for(i=0;i<6;i++){
    surfLoader = SDL_LoadBMP(files[i]);
    SDL_SetColorKey(surfLoader, SDL_TRUE, SDL_MapRGB(surfLoader->format, 255,255,255));
    img[i] = SDL_CreateTextureFromSurface(ren, surfLoader);
    SDL_FreeSurface(surfLoader);
  }
  init();

  SDL_Event evt;
  for (;;){
    int has = SDL_WaitEventTimeout(&evt, 0);
    if (has) {
      if (evt.type == SDL_QUIT) {
        break;
      }
      if((evt.type == SDL_KEYDOWN) || (evt.type == SDL_KEYUP)){
        switch(evt.key.keysym.sym){
          case SDLK_UP:
          case SDLK_w:
            keyb[0] = evt.key.state==SDL_PRESSED?1:0;
            break;
          case SDLK_DOWN:
          case SDLK_s:
            keyb[1] = evt.key.state==SDL_PRESSED?1:0;
            break;
          case SDLK_LEFT:
          case SDLK_a:
            keyb[2] = evt.key.state==SDL_PRESSED?1:0;
            break;
          case SDLK_RIGHT:
          case SDLK_d:
            keyb[3] = evt.key.state==SDL_PRESSED?1:0;
            break;
          case SDLK_SPACE:
            keyb[4] = evt.key.state==SDL_PRESSED?1:0;
            break;
          case SDLK_RETURN:
          case SDLK_RETURN2:
          case SDLK_KP_ENTER:
            keyb[5] = evt.key.state==SDL_PRESSED?1:0;
            break;
        }
      }
    }
    // calculate DT
    now = SDL_GetTicks();
    int dt = now - old;
    old = now;
    // update & render
    update(dt, now);
    render();
  }
  return 0;
}
