#include <lcom/lcf.h>
#include <lcom/lab5.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

static uint8_t *video_mem;		/* Process (virtual) address to which VRAM is mapped */
static mmap_t map_mem;
//static void *temp_video_mem;
static unsigned h_res;	        /* Horizontal resolution in pixels */
static unsigned v_res;	        /* Vertical resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */
static unsigned bytes_per_pixel;
vbe_mode_info_t vmi_p;
uint8_t RedMaskSize;		/* size of direct color red mask in bits */
uint8_t GreenMaskSize;	/* size of direct color green mask in bits */
uint8_t BlueMaskSize; 	/* size of direct color blue mask in bits */
uint8_t RedFieldPosition;
uint8_t GreenFieldPosition;	/* size of direct color green mask in bits */
uint8_t BlueFieldPosition; 
static unsigned redScreenMask;
static unsigned greenScreenMask;
static unsigned blueScreenMask;
int r;
struct minix_mem_range mr; /* physical memory range */
unsigned int vram_base; /* VRAM’s physical addresss */
unsigned int vram_size; /* VRAM’s size, but you can use the frame-buffer size, instead */
extern uint32_t counter;

int vbe_get_info_mode(uint16_t mode, vbe_mode_info_t * vmi_p);
int VBE_inteface(uint16_t mode);
void* (vg_init)(uint16_t mode);
int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int (vg_draw_rectangle)(uint16_t x, uint16_t y,uint16_t width,uint16_t height,uint32_t color);
int mapping_vram();
void getPixel(uint16_t x, uint16_t y, uint32_t color);
int vg_draw_matrix(uint8_t no_rectangles, uint32_t first,uint8_t step);
int vg_draw_xpm(xpm_map_t xpm,uint16_t x, uint16_t y);
int vg_move_xpm_positive(xpm_map_t xpm, int16_t *x, int16_t *y,uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate );
int vg_move_xpm_negative(xpm_map_t xpm, int16_t *x, int16_t *y,uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate );
void change_buffer();

