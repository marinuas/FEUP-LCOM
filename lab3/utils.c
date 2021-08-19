#include <lcom/lcf.h>



#include <stdbool.h>
#include <stdint.h>
uint32_t cnt;


int (util_sys_inb)(int port, uint8_t *value) {
  uint32_t val;
  
  if(sys_inb(port, &val)!=0) return 1;
  #ifdef LAB3
  cnt++;
  #endif
  *value=(uint8_t) val;
  return OK;
}
