#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>

#include "moves.h"
#include "video_gr.h"

extern int dog_init_pos;
int fogo_x[50];
int fogo_y[50];
int et_y[50];
int et_x[50];
int coin_x[100];
int coin_y[100];
int asteroide_y[50];
int asteroide_x[50];

int asteroides_pos_arr(){
  int i=0;
  while(i<50){
    asteroide_y[i]=rand()%401+50;
    asteroide_x[i]=1100;
    i++;
  }
  return 0;
}


int fogos_pos_arr(){
  int i=0;
  while(i<50){
    fogo_x[i]=50;
    fogo_y[i]=dog_init_pos+40;
    i++;
  }
  return 0;
}

int ets_pos_arr(){
  int i=0;
  while(i<50){
    et_y[i]=rand()%401+50;
    et_x[i]=1100;
    i++;
  }
  return 0;
}

int coins_pos_arr(){
  int i=0;
  while(i<100){
    coin_y[i]=rand()%401+90;
    coin_x[i]=1100;
    i++;
  }
  return 0;
}

int up(xpm_image_t image, uint8_t* xpm_loaded){
  if (dog_init_pos <= 10)dog_init_pos -=0;
  else{
    dog_init_pos-=40;
  }
  vg_draw_xpm_proj(image,  40,  dog_init_pos, xpm_loaded);
  return 0;
}

int down(xpm_image_t image, uint8_t* xpm_loaded){
  if (dog_init_pos >= 500) dog_init_pos +=0;
  else 
    dog_init_pos+=40;
  vg_draw_xpm_proj(image,  40,  dog_init_pos, xpm_loaded);
  return 0;
}




