// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>
#include "mouse.h"

extern int packet_byte_cnt;
uint32_t counter_packets=0;
extern uint32_t counter;
extern uint8_t status;
extern uint8_t packet[3];
extern bool done;
// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {
    uint8_t bit_n;
    int r;
    int ipc_status;
    uint32_t irq_set;
    message msg;
    struct packet pp;


    if (mouse_subscribe_int(&bit_n) != 0)
      return 1;
    irq_set = BIT(bit_n);
    if(enable()!=0) return 1;
    while(cnt > 0) { /* You may want to use a different condition */
      /* Get a request message. */
      if( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
        printf("driver_receive failed with: %d", r);
        continue;
      }

      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */
            if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
              mouse_ih();
              get_mouse_status();
              if(packet_byte_cnt==3){
                if(isSyncronized()){
                  packet_assemble(&pp);
                  mouse_print_packet(&pp);
                  packet_byte_cnt=0;
                  cnt--;
                }
              }
            }
            break;
          default:
            break; /* no other notifications expected: do nothing */
        }
      } 
      
      else { /* received a standard message, not a notification */
      /* no standard messages expected: do nothing */
      }
    }

  if(disable()!=0) return 1;
  if (mouse_unsubscribe_int() != 0)
    return 1;
 
  return 0;
}

int (mouse_test_async)(uint8_t idle_time) {
    struct packet pp;
   uint8_t bit_n;
    uint8_t timer0_bit;
    int r;
    int ipc_status;
    uint32_t irq_set;
    uint32_t timer_irq_set;
    message msg;

    if (timer_subscribe_int(&timer0_bit) != 0)
      return 1;

    if (mouse_subscribe_int(&bit_n) != 0)
      return 1;

    timer_irq_set = BIT(timer0_bit);
    irq_set = BIT(bit_n);

    if(enable()!=0) return 1;

    while((counter/60) < idle_time) { /* You may want to use a different condition */
      /* Get a request message. */
      if( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
        printf("driver_receive failed with: %d", r);
        continue;
      }

      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */
            if (msg.m_notify.interrupts & timer_irq_set) {
                timer_int_handler();
            }
            if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
              mouse_ih();
              get_mouse_status();
              if(packet_byte_cnt==3){
                if(isSyncronized()){
                  packet_assemble(&pp);
                  mouse_print_packet(&pp);
                  packet_byte_cnt=0;
                  counter = 0;
                }
              }
            }
            break;
          default:
            break; /* no other notifications expected: do nothing */
        }
      } 
      
      else { /* received a standard message, not a notification */
      /* no standard messages expected: do nothing */
      }
    }

  if(disable()!=0) 
    return 1;

  if (mouse_unsubscribe_int() != 0)
    return 1;

  if (mouse_subscribe_int(&bit_n) != 0)
    return 1;
 
  return 0;
}

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
    uint8_t bit_n;
    int r;
    int ipc_status;
    uint32_t irq_set;
    message msg;
    struct packet pp;
    struct mouse_ev* mouse_ev;

    if (mouse_subscribe_int(&bit_n) != 0)
      return 1;

    irq_set = BIT(bit_n);

    if(enable()!=0) return 1;

    while(!done) { /* You may want to use a different condition */
      /* Get a request message. */
      if( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
        printf("driver_receive failed with: %d", r);
        continue;
      }

      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */
            if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
              mouse_ih();
              get_mouse_status();
              if(packet_byte_cnt==3){
                if(isSyncronized()){
                  packet_assemble(&pp);
                  packet_byte_cnt=0;
                  mouse_print_packet(&pp);
                  mouse_ev=mouse_event(&pp, mouse_ev);
                  
                    
                 mouse_gesture(tolerance, mouse_ev, x_len);   
                              
                }
              }
            }
            break;
          default:
            break; /* no other notifications expected: do nothing */
        }
      } 
      
      else { /* received a standard message, not a notification */
      /* no standard messages expected: do nothing */
      }
    }

  if(disable()!=0) return 1;

  if (mouse_unsubscribe_int() != 0)
    return 1;
 
  return 0;
}
/*
int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
    printf("%s(%u, %u): under construction\n", __func__, period, cnt);
    return 1;
}
*/
