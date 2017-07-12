#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#include <termios.h>
#include <fcntl.h>
#include <errno.h>

#include <math.h>
#include <string.h>

#define SCREENSIZEX 800
#define SCREENSIZEY 600
#define SHIPACC 1200

#define PRTEXTQTD 14
#define PRIMGQTD 9

#define BTNQTD 7

#define SMENU 0
#define SGAME 1
#define SPAUSE 2
#define SFAIL 3

// timings
int i,now, old, start, gametimer, lDraw, isrec=0, quit=0;
int frames = 0;
int tempo_inicial, tempo_partida, pause_start;
char* str_tempo;

// function declarations
void init(int now);
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
    int     tblink;
    short   cblink;
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

typedef struct {
    SDL_Texture* renderedMessage;
    SDL_Rect drawRect;
} FixedLabel;

SDL_Event evt;

SDL_Window* win;
SDL_Renderer* ren;
SDL_Surface* surfLoader;
SDL_Texture* img[PRIMGQTD];
SDL_Texture* menuTexT[3];
SDL_Rect menuTexRect[3];

SDL_Texture* failTexT;
SDL_Rect failTexRect;

TTF_Font* FTitle;
TTF_Font* FText;
SDL_Color FontC = {70, 70, 70};

FixedLabel prText[PRTEXTQTD];

char* files[] = {
    "img/plane.bmp",
    "img/Tower.bmp",
    "img/heli.bmp",
    "img/tank.bmp",
    "img/back.bmp",
    "img/GameOver.bmp",
    "img/Heart.bmp",
    "img/hud.bmp",
    "img/menu.bmp"
};

char* texts[] = {
    "Beach Raid",
    "Game Over",
    "Pause",
    "Recordes:",
    "Nivel: -  Facil  +",
    "Nivel: -  Medio  +",
    "Nivel: - Dificil +",
    "Novo Recorde!",
    "Enter: Volta ao jogo  Esc: Volta ao menu principal",
    "Instrucoes:",
    "Setas movem o aviao",
    "Espaco atira, Enter",
    "e Esc controlam os",
    "Menus."
};

SDL_Rect black = {0, 0, SCREENSIZEX, SCREENSIZEY};
SDL_Rect bgsrc = {0,0,200,150};

// up,down,left,right,shoot,enter
short keyb[BTNQTD] = {0,0,0,0,0,0,0};
short keyst[BTNQTD] = {0,0,0,0,0,0,0};
short keyt[BTNQTD] = {0,0,0,0,0,0,0};
short keya[BTNQTD] = {0,0,0,0,0,0,0};
int record[] = {0,0,0};

// entities
Player nave;
Inimigo inimigo[50];
Tiro tiros[50];
SDL_Color tiroC;
Cloud nuvem[20];
BG bg[2];

// tiro iterator/ enemy iterator
short iT=0;
short iE=0;

short gamestate=SMENU;
short _gamestate=SMENU;
short ispause=0;

short difficulty = 0;
short lives=0;

void DuracaoJogo(int now){ //called in colisao_update()
    FILE *f;
    int tempo;
    tempo = now-tempo_inicial;
    //record
    f = fopen("record.txt","r+");
    if(f==(FILE*)0||f==(FILE*)-1){// in case file does not exist
        f = fopen("record.txt","w+");
        fprintf(f,"0\n0\n0");
        rewind(f);
    }
    printf("%d\n",f);
    printf("antes: %d/%d/%d\n",record[0],record[1], record[2]);
    fscanf(f,"%d\n%d\n%d", record,(record+1),(record+2));
    printf("lido: %d/%d/%d\n",record[0],record[1], record[2]);
    if(tempo>record[difficulty]){
        isrec=1;
        record[difficulty] = tempo;
        rewind(f);
        fprintf(f,"%d\n%d\n%d", record[0], record[1], record[2]);
    }
    fclose(f);
}

void damage(int now){
    lives--;
    if(lives==0){
        gamestate=SFAIL;
        DuracaoJogo(now);
    }
}

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
void hud_init(){
}
void hud_render(){
    surfLoader = TTF_RenderText_Blended(FText, str_tempo, FontC);
    SDL_Texture* RM = SDL_CreateTextureFromSurface(ren, surfLoader);
    SDL_Rect DR = {15,90,0,0};
    DR.w = surfLoader->w;
    DR.h = surfLoader->h;
    SDL_FreeSurface(surfLoader);
    SDL_Rect hud = {0,0,180,124};
    SDL_RenderCopy(ren,img[7], NULL, &hud);
    SDL_RenderCopy(ren, RM, NULL, &DR);
    SDL_Rect hDR = {60,20,30,30};
    for(i=0;i<lives;i++){
        SDL_RenderCopy(ren,img[6], NULL, &hDR);
        hDR.x+=35;
    }
}
void hud_update(int dt,int now){
    int tempo = now-tempo_inicial;
    sprintf(str_tempo,"%02d:%02d.%03d",tempo/60000,(tempo/1000)%60,tempo%1000);
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
                break;
            }
        }
    }
}

void colisao_update(int now){
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
                    damage(now);
                }
            }
        }
    }
    for(it=0;it<50;it++){// tiros-player colision
        if(tiros[it].on && tiros[it].type==1){
            SDL_bool col = SDL_HasIntersection(&tiros[it].r,&nave.r);
            if(col){// Bang *dead*!
                tiros[it].on=0;
                damage(now);
            }
        }
    }
}
void game_update(int now){
    if(now>gametimer){// random enemy generation
        gametimer=(((rand() % 7) + 2)*(difficulty==0?225:125))+now;
        short t1, q1, aux;
        t1=(rand()%5)/2;
        q1=((rand()%4)+1)/2;


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
    if(keyb[6]){
        gamestate = SPAUSE;
    }
}
void game_init(){
    isrec=0;
    gametimer=now;
    create_still(200);
    lives=difficulty==2?1:3;
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

void pause_init(){
    prText[2].drawRect.x=350;
    prText[2].drawRect.y=240;
}
void pause_update(){
    if(keyb[5]==1 && keyst[5]==0){
        gamestate=SGAME;
    }
    if(keyb[6]==1 && keyst[6]==0){
        gamestate=SMENU;
    }
}
void pause_render(){
    SDL_RenderCopy(ren,img[8],NULL,NULL);
    SDL_RenderCopy(ren, prText[0].renderedMessage, NULL, &prText[0].drawRect);
    SDL_RenderCopy(ren, prText[2].renderedMessage, NULL, &prText[2].drawRect);
}

void menu_init(){
    char str[] = "F: 00:00.000";
    char types[] = "FMD";

    FILE* f;
    f = fopen("record.txt","r+");
    if(f==(FILE*)0||f==(FILE*)-1){// in case file does not exist
        f = fopen("record.txt","w+");
        fprintf(f,"0\n0\n0");
        rewind(f);
    }
    fscanf(f,"%d\n%d\n%d", record,(record+1),(record+2));
    fclose(f);

    for(i=0;i<3;i++){
        sprintf(str,"%c: %02d:%02d.%03d", types[i], record[i]/60000, (record[i]/1000)%60, record[i]%1000);
        surfLoader = TTF_RenderText_Blended(FText, str, FontC);
        menuTexT[i] = SDL_CreateTextureFromSurface(ren, surfLoader);
        menuTexRect[i].x = 480;
        menuTexRect[i].y = 300+i*25;
        menuTexRect[i].w =  surfLoader->w;
        menuTexRect[i].h = surfLoader->h;
        SDL_FreeSurface(surfLoader);
    }

    for(i=4;i<7;i++){// dificuldades
        prText[i].drawRect.y=240;
        prText[i].drawRect.x=150;
    }
    for(i=9;i<PRTEXTQTD;i++){// Instrucoes
        prText[i].drawRect.y=110+i*20;
        prText[i].drawRect.x=150;
    }
    prText[0].drawRect.x=300;
    prText[0].drawRect.y=170;

    prText[3].drawRect.x=500;
    prText[3].drawRect.y=240;
}
void menu_update(){
    if(keyb[3]==1 && keyst[3]==0){
        difficulty = difficulty<2?difficulty+1:0;
    }
    if(keyb[2]==1 && keyst[2]==0){
        difficulty = difficulty>0?difficulty-1:2;
    }
    if(keyb[5]==1 && keyst[5]==0){
        gamestate=SGAME;
    }
    if(keyb[6]==1 && keyst[6]==0){
        quit=1;
    }
}
void menu_render(){
    SDL_RenderCopy(ren,img[8],NULL,NULL);
    SDL_RenderCopy(ren, prText[0].renderedMessage, NULL, &prText[0].drawRect);
    SDL_RenderCopy(ren, prText[difficulty+4].renderedMessage, NULL, &prText[difficulty+4].drawRect);
    for(i=9;i<PRTEXTQTD;i++){// Instrucoes
        SDL_RenderCopy(ren, prText[i].renderedMessage, NULL, &prText[i].drawRect);
    }
    SDL_RenderCopy(ren, prText[3].renderedMessage, NULL, &prText[3].drawRect);

    for(i=0;i<3;i++){
        SDL_RenderCopy(ren, menuTexT[i], NULL,&menuTexRect[i]);
    }
}

void fail_init(){
    prText[1].drawRect.x=310;
    prText[1].drawRect.y=240;

    prText[7].drawRect.x=130;
    prText[7].drawRect.y=340;

    char *diff[3]={
        "Facil",
        "Medio",
        "Dificil"
    };
    char str[50];

    sprintf(str,"Voce durou %s no modo %s", str_tempo, diff[difficulty]);
    surfLoader = TTF_RenderText_Blended(FText, str, FontC);
    failTexT = SDL_CreateTextureFromSurface(ren, surfLoader);
    failTexRect.x = 130;
    failTexRect.y = 320;
    failTexRect.w =  surfLoader->w;
    failTexRect.h = surfLoader->h;
    SDL_FreeSurface(surfLoader);
}
void fail_update(){
    if(keyb[5]==1 && keyst[5]==0){
        gamestate=SMENU;
    }
    if(keyb[6]==1 && keyst[6]==0){
        quit=1;
    }
}
void fail_render(){
    SDL_RenderCopy(ren,img[8],NULL,NULL);
    SDL_RenderCopy(ren,prText[0].renderedMessage,NULL,&prText[0].drawRect);
    SDL_RenderCopy(ren,prText[1].renderedMessage,NULL,&prText[1].drawRect);
    SDL_RenderCopy(ren,failTexT,NULL,&failTexRect);

    if(isrec)SDL_RenderCopy(ren,prText[7].renderedMessage,NULL,&prText[7].drawRect);
}

void init(int now){
    switch(_gamestate){
        case SMENU:
        ispause=0;
        menu_init();
        break;
        case SFAIL:
        fail_init();
        break;
        case SPAUSE:
        ispause=1;
        pause_init();
        pause_start = now;
        break;
        case SGAME:
        if(!ispause){
            tempo_inicial = now;
            nave_init();
            tiros_init();
            inimigos_init();
            game_init();
            hud_init();
            fundo_init();
        }else{
            tempo_inicial+=now-pause_start;
        }
        ispause=0;
        break;
    }
    //jogo começou
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
    SDL_SetRenderDrawColor(ren,255,255,255,0xFF);
    SDL_RenderFillRect(ren, &black);

    // render entities
    switch(_gamestate){
        case SMENU:
        menu_render();
        break;
        case SFAIL:
        fail_render();
        break;
        case SPAUSE:
        pause_render();
        break;
        case SGAME:
        fundo_render();
        tiros_render();
        inimigos_render();
        nave_render();
        hud_render();
        break;
    }
    // update screen
    SDL_RenderPresent(ren);
}
// game logic
void update(int dt,int now){
    switch(_gamestate){
        case SMENU:
        menu_update();
        break;
        case SFAIL:
        fail_update();
        break;
        case SPAUSE:
        pause_update();
        break;
        case SGAME: //jogando
        nave_update(dt,now);
        tiros_update(dt,now);
        inimigos_update(dt,now);
        colisao_update(now);
        game_update(now);
        fundo_update(dt);
        hud_update(dt,now);
    }
    if(gamestate!=_gamestate){
        _gamestate=gamestate;
        init(now);
    }
    for(i=0;i<BTNQTD;i++){
        keyst[i] = keyb[i];
    }
}

void imgLoad(){
    for(i=0;i<PRIMGQTD;i++){
        surfLoader = SDL_LoadBMP(files[i]);
        SDL_SetColorKey(surfLoader, SDL_TRUE, SDL_MapRGB(surfLoader->format, 255,255,255));
        img[i] = SDL_CreateTextureFromSurface(ren, surfLoader);
        SDL_FreeSurface(surfLoader);
    }
}
void txtLoad(){
    for(i=0;i<PRTEXTQTD;i++){
        surfLoader = TTF_RenderText_Blended(i<3?FTitle:FText, texts[i], FontC);
        prText[i].renderedMessage = SDL_CreateTextureFromSurface(ren, surfLoader);
        prText[i].drawRect.x = 0;
        prText[i].drawRect.y = 0;
        prText[i].drawRect.w = surfLoader->w;
        prText[i].drawRect.h = surfLoader->h;
        SDL_FreeSurface(surfLoader);
    }
}

// Projeto arduino-serial no github
int serialport_init(const char* serialport, int baud){
    struct termios toptions;
    int fd;

    //fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open(serialport, O_RDWR | O_NONBLOCK );

    if (fd == -1)  {
        perror("serialport_init: Unable to open port ");
        return -1;
    }

    //int iflags = TIOCM_DTR;
    //ioctl(fd, TIOCMBIS, &iflags);     // turn on DTR
    //ioctl(fd, TIOCMBIC, &iflags);    // turn off DTR

    if (tcgetattr(fd, &toptions) < 0) {
        perror("serialport_init: Couldn't get term attributes");
        return -1;
    }
    speed_t brate = baud; // let you override switch below if needed
    switch(baud) {
        case 9600:   brate=B9600;   break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    //toptions.c_cflag &= ~HUPCL; // disable hang-up-on-close to avoid reset

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 0;
    //toptions.c_cc[VTIME] = 20;

    tcsetattr(fd, TCSANOW, &toptions);
    if( tcsetattr(fd, TCSAFLUSH, &toptions) < 0) {
        perror("init_serialport: Couldn't set term attributes");
        return -1;
    }
    return fd;
}
char readCharSerial(int fd){
    char b[1];
    int n = read(fd, b, 1);  // read a char at a time
    if( n==-1) return -1;    // couldn't read
    if( n==0 ) return -2;
    return b[0];
}

short EventHandler(){
    int has = SDL_WaitEventTimeout(&evt, 0);
    if (has) {
        if (evt.type == SDL_QUIT || quit==1) {
            return 1;
        }
        if((evt.type == SDL_KEYDOWN) || (evt.type == SDL_KEYUP)){
            switch(evt.key.keysym.sym){
                case SDLK_UP:
                case SDLK_w:
                keyt[0] = evt.key.state==SDL_PRESSED?1:0;
                break;
                case SDLK_DOWN:
                case SDLK_s:
                keyt[1] = evt.key.state==SDL_PRESSED?1:0;
                break;
                case SDLK_LEFT:
                case SDLK_a:
                keyt[2] = evt.key.state==SDL_PRESSED?1:0;
                break;
                case SDLK_RIGHT:
                case SDLK_d:
                keyt[3] = evt.key.state==SDL_PRESSED?1:0;
                break;
                case SDLK_SPACE:
                keyt[4] = evt.key.state==SDL_PRESSED?1:0;
                break;
                case SDLK_RETURN:
                case SDLK_RETURN2:
                case SDLK_KP_ENTER:
                keyt[5] = evt.key.state==SDL_PRESSED?1:0;
                break;
                case SDLK_ESCAPE:
                keyt[6] = evt.key.state==SDL_PRESSED?1:0;
                break;
            }
        }
    }
    return 0;
}

int main (int argc, char *argv[]){
    time_t t;
    char serial[50];
    srand((unsigned) time(&t));

    int fd;

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    FILE* c = fopen("config.cfg","r");
    fgets(serial,50,c);
    fclose(c);
    for(i=0;i<strlen(serial);i++){
        if(serial[i]=='\n'){
            serial[i]=0;
        }
    }
    fd = serialport_init(serial, 9600);
    printf("%s\n", serial);

    str_tempo = malloc(10*sizeof(char));
    win = SDL_CreateWindow("Beach Raid", 200, 200, SCREENSIZEX, SCREENSIZEY, SDL_WINDOW_SHOWN);//SDL_WINDOW_BORDERLESS
    ren = SDL_CreateRenderer(win, -1, 0);

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    FText = TTF_OpenFont("fonts/Erika Ormig.ttf", 18);
    FTitle = TTF_OpenFont("fonts/AFCS.ttf", 32);

    // image & text pre rendering
    imgLoad();
    txtLoad();
    // init entities
    now = SDL_GetTicks();
    init(now);
    char ard;
    for (;;){
        if(EventHandler())break;
        if(fd!=-1){
            ard = readCharSerial(fd);
            if(ard>0){
                ard -='0';
                printf("ard:%d\n",ard);
                for(i=0;i<BTNQTD;i++){
                    keya[i] = (ard>>i)%2;
                }
            }
        }
        for(i=0;i<BTNQTD;i++){
            keyb[i] = keya[i]||keyt[i];
        }
        // calculate DT
        now = SDL_GetTicks();
        int dt = now - old;
        old = now;

        // update & render entities
        update(dt, now);
        if(now>lDraw+14){
            render();
            lDraw=now;
        }
    }
    return 0;
}
