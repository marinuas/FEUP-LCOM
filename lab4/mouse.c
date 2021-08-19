#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "mouse.h"
#include <lcom/utils.h>
#include <math.h>

int packet_byte_cnt=0;
int hook_id_mouse=12;
uint8_t status;
uint8_t packet[3];
bool done = false;




int (mouse_subscribe_int)(uint8_t *bit_no) {
    *bit_no = hook_id_mouse;
    if(sys_irqsetpolicy(IRQ12, IRQ_REENABLE|IRQ_EXCLUSIVE, &hook_id_mouse) != 0)
      return 1;

  return 0;
}
int enable(){
  hook_id_disable();
  if(mouse_write_command(SET_STREAM_MODE) != 0) return 1;
  if(mouse_write_command(ENABLE_DATA_REP) != 0) return 1;
  hook_id_enable();
  return 0;
}

void hook_id_enable(){
  if(sys_irqenable(&hook_id_mouse)!=0) return;
}

int disable(){
  hook_id_disable();
  if(mouse_write_command(DISABLE_DATA_REP) != 0) return 1;
  hook_id_enable();
  return 0;
}

void hook_id_disable(){
  if(sys_irqdisable(&hook_id_mouse)!=0) return;
}

int (mouse_unsubscribe_int)() {
  if(sys_irqrmpolicy(&hook_id_mouse) != 0) return 1;

  return 0;
}
void (mouse_ih)() {
  status=read_kbc();
  //printf("%x\n", status);
  packet_byte_cnt++;
}

void get_mouse_status(){
  if(packet_byte_cnt==1){
    packet[0] = status;
  }

  else if(packet_byte_cnt==2){
    packet[1]=status;
  }

  else if(packet_byte_cnt==3){
    packet[2] = status;
  }
}

void packet_assemble(struct packet *pp){
  int msb_x, msb_y;
  pp->bytes[0]=packet[0];
  pp->bytes[1]=packet[1];
  pp->bytes[2]=packet[2];

  if(packet[0] & RB)
    pp->rb=true;
  else pp->rb=false;

  if(packet[0] & MB)
    pp->mb=true;
  else pp->mb=false;

  if(packet[0] & LB)
    pp->lb=true;
  else pp->lb=false;

  if(packet[0] & X_OVFL)
    pp->x_ov=true;
  else pp->x_ov=false;

  if(packet[0] & Y_OVFL)
    pp->y_ov=true;
  else pp->y_ov=false;

  if(packet[0] & MSB_Y_DELTA)
      msb_y=1;
  else msb_y=0;

  if(packet[0] & MSB_X_DELTA)
      msb_x=1;
  else msb_x=0;

  pp->delta_x=complementTo2(packet[1], msb_x);
  pp->delta_y=complementTo2(packet[2], msb_y);
}

int complementTo2(uint8_t byte, int msb){
  if(msb==0)
    return byte;
  
  return byte - 256;
}

bool isSyncronized(){
  if(packet[0] & MIDDLE_BIT)
    return true;
  return false;
}

int mouse_write_command(uint32_t command ){
  uint8_t st, ackn;
  while (1)
  {
    if(util_sys_inb(STATUS_REG, &st)!=0) return 1;
    

    if(st & (AUX | IBF))
      if(sys_outb(STATUS_REG, WRITE_BYTE_TO_MOUSE)!=0) return 1;
    
    if(util_sys_inb(STATUS_REG, &st)!=0) return 1;

    if(st & (AUX | IBF))
      if(sys_outb(OUT_BUF, command)!=0) return 1;
    
    if(util_sys_inb(OUT_BUF, &ackn)!=0) return 1;
    if(ackn==ACK) break;
    else if(ackn==ERROR) return 1;
  }

  return 0;
}
 bool l_pressed=false;
  bool l_released=false;
  bool r_pressed=false;
struct mouse_ev* mouse_event(struct packet *pp, struct mouse_ev* mouse_ev){
 
  
  mouse_ev->delta_x=pp->delta_x;
  mouse_ev->delta_y=pp->delta_y;
  
  if(pp->lb && !pp->mb && !pp->rb && !l_pressed){
   if(!l_pressed){
      mouse_ev->type=LB_PRESSED;
      l_pressed=true;
   }
  }
  else if(!pp->lb && !pp->mb && pp->rb && !r_pressed){
    
    if(l_released) {
      r_pressed=true;
     mouse_ev->type=RB_PRESSED;
    }
    
  }
  else if(!pp->lb && !pp->mb && !pp->rb){
    if(l_pressed) {
      l_pressed=false;
      l_released=true;
     mouse_ev->type=LB_RELEASED;
    }
    else if(r_pressed){
      r_pressed=false;
     mouse_ev->type=RB_RELEASED;
    }
    
  }
  else if((l_pressed && pp->delta_y >0) || (r_pressed && 0 > pp->delta_y)){
    mouse_ev->type=MOUSE_MOV;
  }
  else{
    mouse_ev->type=BUTTON_EV;
  }
  return mouse_ev;
  
}

int mouse_gesture(uint8_t tolerance, struct mouse_ev* mouse_ev, uint8_t x_len){  
  int16_t x=0, y=0;
  static enum state_t st = START; 
  switch (st)
  {
    //INITT
  case START:
    x = 0, y = 0;
    if( mouse_ev->type == LB_PRESSED){
        st = DRAWL;
    } 
    break;

  //DRAWL
  case DRAWL:
    if(mouse_ev->type == LB_RELEASED){
      if(x<x_len)
        st = START;

      st = MID;
    }
    else if(mouse_ev->type == MOUSE_MOV) {
      if((mouse_ev->delta_x < 0 && abs(mouse_ev->delta_x)>tolerance) || (mouse_ev->delta_y<0 && abs(mouse_ev->delta_y)>tolerance)){
        st=START;
      }
      x += mouse_ev->delta_x;
      y += mouse_ev->delta_y;
    }
    break;
  
  //MID
  case MID:
    if(mouse_ev->type==RB_PRESSED) {
      x=0, y=0;
      st=DRAWR;
    }
    else if(mouse_ev->type==LB_PRESSED){
      if((abs(mouse_ev->delta_x)>tolerance) || (abs(mouse_ev->delta_y)>tolerance))
        st=START;
    }
    break;

  //DRAWR
  case DRAWR:
    if( mouse_ev->type == RB_RELEASED){
        if(x<x_len)
          st=START;

        done= true;
        break;
      }

    else if( mouse_ev->type == MOUSE_MOV) {
      if((mouse_ev->delta_x < 0 && abs(mouse_ev->delta_x)>tolerance) || (mouse_ev->delta_y<0 && abs(mouse_ev->delta_y)>tolerance)){
        st=START;
      }
      x += mouse_ev->delta_x;
      y += mouse_ev->delta_y;
    }
      
    break;
  default:
    break;
  }

  return 0;
}
