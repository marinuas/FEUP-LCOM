#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "i8042.h"
#include "keyboard.h"
#include <lcom/utils.h>

int (mouse_subscribe_int)(uint8_t *bit_no);
int (mouse_unsubscribe_int)();
int enable();
int disable();
void (mouse_ih)();
void get_mouse_status();
void packet_assemble(struct packet *pp);
int complementTo2(  uint8_t byte, int msb);
bool isSyncronized();
int mouse_write_command(uint32_t command );
void hook_id_enable();
void hook_id_disable();
struct mouse_ev* mouse_event(struct packet *pp, struct mouse_ev* mouse_ev );
int mouse_gesture(uint8_t tolerance, struct mouse_ev* mouse_ev, uint8_t x_len);

enum state_t {START, DRAWL, DRAWR, MID, COMP};
