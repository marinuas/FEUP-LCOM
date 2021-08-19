#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>
#include "i8042.h"
#include "i8254.h"
#include "timer.h"


int (kbd_subscribe_int)(uint8_t *bit_no) ;
int (kbd_unsubscribe_int)();
void (kbc_ih)();
int read_kbc();
int size(uint8_t data);
bool isMake(uint8_t data);
void get_data();
int read_kbc_poll();
int write_command();

