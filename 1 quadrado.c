//renato e camila
#include <SDL2/SDL.h>
#include <assert.h>

int main (int argc, char* args[]){
	/* INITIALIZATION */

	int err = SDL_Init(SDL_INIT_EVERYTHING);
	assert(err == 0);
	
	SDL_Window* window = SDL_CreateWindow("Input",
							SDL_WINDOWPOS_UNDEFINED,
							SDL_WINDOWPOS_UNDEFINED,
							640, 480, SDL_WINDOW_SHOWN);
	assert(window != NULL);

	SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,0);
	assert(renderer != NULL);

	/* EXECUTION */

	SDL_Rect r = { 200,200, 50, 50 };
	int screen[] = {640,480};
	double rect[] = {200.0,200.0,50.0,0.0};
	SDL_Event e;
	int speed = 10;
	unsigned int t = SDL_GetTicks();
	unsigned int dt = 0;
	unsigned int oldt = t;
	unsigned int posSwitch=0;
	while (1){
		t = SDL_GetTicks();
		dt = t-oldt;
		if(SDL_PollEvent(&e) != 0){		
			if (e.type == SDL_QUIT) {
				break;
			} else if (e.type == SDL_MOUSEBUTTONDOWN) {
				SDL_MouseButtonEvent* me = (SDL_MouseButtonEvent*) &e;
				int x,y;
				x=me->x;
				y=me->y;
				//if((x>=r.x)&&(x<(r.x +50))||(y>=r.y)&&(y<(r.y +50)))
				//quadrado para
		
			}
		}
		
		rect[0] += ((dt/1000.0)*rect[2]);
		rect[1] += ((dt/1000.0)*rect[3]);

		r.x = (int)rect[0];
		r.y = (int)rect[1];

		printf("%f %f %f %f\n",rect[0],rect[1],rect[2],rect[3]);
		
		if(t>=posSwitch){
			double aux = rect[2];
			rect[2] = -1*rect[3];
			rect[3] = aux;
			posSwitch = t+1000;
		}
		
		SDL_SetRenderDrawColor(renderer,0xFF,0xFF,0xFF,0x00);
		SDL_RenderFillRect(renderer, NULL);

		SDL_SetRenderDrawColor(renderer,0x00,0x00,0xFF,0x00);
		SDL_RenderFillRect(renderer, &r);

		SDL_RenderPresent(renderer);
		oldt = t;
	}

	/* FINALIZATION */

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
