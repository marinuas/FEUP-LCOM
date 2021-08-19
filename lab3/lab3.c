#include <lcom/lcf.h>
#include <lcom/timer.h>
#include <lcom/lab3.h>
#include <stdbool.h>
#include <stdint.h>
#include "keyboard.h"
#include "i8042.h"


extern uint8_t data;
extern uint32_t cnt;
extern uint8_t arr[2];
extern uint32_t counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {
    uint8_t bit_n;
    int r;
    int ipc_status;
    uint32_t irq_set;
    message msg;
    bool makecode;
    uint8_t sc_size;


    if (kbd_subscribe_int(&bit_n) != 0)
      return 1;

    irq_set = BIT(bit_n);

    while(data != ESC_BREAKCODE) { /* You may want to use a different condition */
      /* Get a request message. */
      if( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
        printf("driver_receive failed with: %d", r);
        continue;
      }

      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */
            if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
              kbc_ih();
              get_data();
              sc_size = size(data);
              makecode = isMake(data);
              kbd_print_scancode(makecode, sc_size, arr);
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

  if (kbd_unsubscribe_int() != 0)
    return 1;

  kbd_print_no_sysinb(cnt);
  return 0;
}

int(kbd_test_poll)() {
  bool makecode;
  uint8_t sc_size;

  while(data != ESC_BREAKCODE) {
    read_kbc_poll();
    get_data();
    sc_size = size(data);
    makecode = isMake(data);
    kbd_print_scancode(makecode, sc_size, arr);
  }
  write_command();
  kbd_print_no_sysinb(cnt);
  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
    uint8_t bit_n;
    uint8_t timer0_bit;
    int r;
    int ipc_status;
    uint32_t irq_set;
    uint32_t timer_irq_set;
    message msg;
    bool makecode;
    uint8_t sc_size;

    if (timer_subscribe_int(&timer0_bit) != 0)
      return 1;

    if (kbd_subscribe_int(&bit_n) != 0)
      return 1;

    irq_set = BIT(bit_n);
    timer_irq_set = BIT(timer0_bit);

    while((data != ESC_BREAKCODE) && ((counter/60) < n)) { /* You may want to use a different condition */
      /* Get a request message. */
      if( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
        printf("driver_receive failed with: %d", r);
        continue;
      }

      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */
            if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
              kbc_ih();
              counter  = 0;
              get_data();
              sc_size = size(data);
              makecode = isMake(data);
              kbd_print_scancode(makecode, sc_size, arr);
            }

            if (msg.m_notify.interrupts & timer_irq_set) {
              timer_int_handler();
              if(counter%60 == 0)
                timer_print_elapsed_time();
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

  if (timer_unsubscribe_int() != 0)
    return 1;

  if (kbd_unsubscribe_int() != 0)
    return 1;

  kbd_print_no_sysinb(cnt);
  
  return 0;
}
