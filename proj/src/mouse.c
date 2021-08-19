#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "mouse.h"
#include "i8042.h"
#include <lcom/utils.h>
#include <math.h>

int packet_byte_cnt=0;
int hook_id_mouse=12;
uint8_t status;
uint8_t packet[3];
bool done = false;
int mouse_x = 400;
int mouse_y = 220;


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


int mouse_pos(struct packet *pp, enum game_state *state){
  if(mouse_x < 0) mouse_x = 0;

	if(mouse_x > 800) mouse_x = 800;

	if(mouse_y < 0) mouse_y = 0;

	if(mouse_y > 600) mouse_y = 600;
  
  mouse_x += pp->delta_x;
  mouse_y -= pp->delta_y;

  if(pp->lb && *state == MENU && mouse_x>250 && mouse_x<540 && mouse_y>205 && mouse_y<295)
		*state = PLAY;
  if(pp->lb && *state == MENU && mouse_x>250 && mouse_x<540 && mouse_y>325 && mouse_y<415)
		*state = INSTRUCTIONS;
  if(pp->lb && *state == MENU && mouse_x>250 && mouse_x<540 && mouse_y>440 && mouse_y<530)
		*state = EXIT;  

    return 0;
}

