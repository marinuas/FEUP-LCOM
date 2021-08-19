// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>
#include "i8042.h"
#include "video_gr.h"
#include "keyboard.c"


extern uint8_t data;

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
    VBE_inteface(mode);
    sleep(delay);
    vg_exit();
    return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    uint8_t bit_n;
    int r;
    int ipc_status;
    uint32_t irq_set;
    message msg;


    if (kbd_subscribe_int(&bit_n) != 0)
      return 1;

    irq_set = BIT(bit_n);
    vg_init(mode);
    vg_draw_rectangle(x,y,width,height,color);          


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
              (kbc_ih)();
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
    vg_exit();

   
return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
    uint8_t bit_n;
    int r;
    int ipc_status;
    uint32_t irq_set;
    message msg;


    if (kbd_subscribe_int(&bit_n) != 0)
      return 1;

    irq_set = BIT(bit_n);

    vg_init(mode);
    vg_draw_matrix(no_rectangles, first, step);          

    while(data != ESC_BREAKCODE){
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
                    }       
                break;
            default:
                break; /* no other notifications expected: do nothing */
        }
        } else { /* received a standard message, not a notification */
        /* no standard messages expected: do nothing */
        }
        
    }
    if (kbd_unsubscribe_int() != 0)
        return 1;
   
    vg_exit();

    
    return 0;
}
int (video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y){
    uint8_t bit_n;
    int r;
    int ipc_status;
    uint32_t irq_set;
    message msg;


    if (kbd_subscribe_int(&bit_n) != 0)
      return 1;

    irq_set = BIT(bit_n);

    vg_init(0x105);       
    vg_draw_xpm(xpm, x, y);
    while(data != ESC_BREAKCODE){
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
                    }       
                break;
            default:
                break; /* no other notifications expected: do nothing */
        }
        } else { /* received a standard message, not a notification */
        /* no standard messages expected: do nothing */
        }
        
    }
    if (kbd_unsubscribe_int() != 0)
        return 1;
    vg_exit();
   
   
    return 0;
}
int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  uint8_t bit_n;
    uint8_t timer0_bit;
    int r;
    int ipc_status;
    uint32_t irq_set;
    uint32_t timer_irq_set;
    message msg;
    int res;
     int16_t x= xi, y=yi;

    

    if (timer_subscribe_int(&timer0_bit) != 0)
      return 1;

    if (kbd_subscribe_int(&bit_n) != 0)
      return 1;

    vg_init(0x105);
    irq_set = BIT(bit_n);
    timer_irq_set = BIT(timer0_bit);
    timer_set_frequency(0, fr_rate);

    vg_draw_xpm(xpm, xi, yi);
    while(data != ESC_BREAKCODE)  { /* You may want to use a different condition */
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
            }

            else if (msg.m_notify.interrupts & timer_irq_set) {
                if (((xi % h_res) == xf && (yi % v_res) == yf) || xi > h_res || yi > v_res){
                    if(speed>0){
                       timer_int_handler();
                        res = vg_move_xpm_positive(xpm,&x,&y, xi, yi, xf, yf, speed, fr_rate);
                    }
                    else if(speed<0){
                        res = vg_move_xpm_negative(xpm,&x,&y,xi,yi,xf,yf,speed,fr_rate);
                    }

                   if(res==1) 
                      break; /*movement complete*/
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

  if (timer_unsubscribe_int() != 0)
    return 1;

  if (kbd_unsubscribe_int() != 0)
    return 1;

  vg_exit();
  return 0;
}

int(video_test_controller)() {
  /* To be completed */
  printf("%s(): under construction\n", __func__);

  return 1;
}

