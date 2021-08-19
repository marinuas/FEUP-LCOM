// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include "video_gr.h"
#include "timer.c"


static uint8_t *video_mem;		/* Process (virtual) address to which VRAM is mapped */
static uint8_t *double_buff;
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


int VBE_inteface(uint16_t mode){
    reg86_t r;
    //INICIALIZA AO R O VALOR DE 0
    memset(&r, 0, sizeof(reg86_t));

    r.ax = 0x4F02; // VBE call, function 02 -- set VBE mode
    r.bx = 1<<14|mode; // set bit 14: linear framebuffer
    r.intno = 0x10;
    if( sys_int86(&r) != OK ) {
        printf("set_vbe_mode: sys_int86() failed \n");
        return 1;
    }
    return 0;
}

void* (vg_init)(uint16_t mode){
    vbe_get_mode_info(mode, &vmi_p);

    h_res = vmi_p.XResolution;
  	v_res = vmi_p.YResolution;
    bits_per_pixel = vmi_p.BitsPerPixel;
    bytes_per_pixel = ceil(bits_per_pixel / 8.0);
    RedMaskSize = vmi_p.RedMaskSize;
    redScreenMask = vmi_p.RedMaskSize;
    RedFieldPosition = vmi_p.RedFieldPosition;
	GreenMaskSize = vmi_p.GreenMaskSize;
    GreenFieldPosition = vmi_p.GreenFieldPosition;
    greenScreenMask = vmi_p.GreenMaskSize;
	BlueMaskSize = vmi_p.BlueMaskSize;
	BlueFieldPosition = vmi_p.BlueFieldPosition;
    blueScreenMask = vmi_p.BlueMaskSize;
    
    mapping_vram();

    VBE_inteface(mode);
    
    
    return video_mem;
}

void pixel_color(uint16_t x, uint16_t y, uint32_t color){
    uint8_t *pixel = video_mem;
    pixel += (h_res * y * bytes_per_pixel) + (x * bytes_per_pixel);

    for (unsigned i = 0; i < bytes_per_pixel; i++, pixel++) {
        *pixel = (color >> (i * 8));
    }
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color){
    for(int i=0; i<len; i++){
        pixel_color(x+i,y,color);
    }
    return 0;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y,uint16_t width,uint16_t height,uint32_t color){
   for (int i = 0; i < height; i++)
  { 
    vg_draw_hline(x, y + i, width, color);
  }
  return 0;
}	


int mapping_vram(){
    vram_base = vmi_p.PhysBasePtr;
    vram_size = bytes_per_pixel * h_res * v_res;
 
    double_buff = malloc(vram_size);

    /* Allow memory mapping */
    mr.mr_base = (phys_bytes) vram_base;
    mr.mr_limit = mr.mr_base + vram_size;

    if( 0 != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);
        
    /* Map memory */
    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);
    if(video_mem == MAP_FAILED)
        panic("couldn’t map video memory");

    return 0;
}
uint32_t getR(uint32_t color){
    uint32_t colorpos = color ;

    uint32_t mask = pow(2, RedMaskSize) - 1;
    mask=mask<<RedFieldPosition;
    return (uint8_t)((colorpos & mask)>>RedFieldPosition);

}
uint32_t getG(uint32_t color){
    uint32_t colorpos = color;

    uint32_t mask = pow(2, GreenMaskSize) - 1;
    mask=mask<<GreenFieldPosition;
    return (uint8_t)((colorpos & mask)>>GreenFieldPosition);
    
}
uint32_t getB(uint32_t color){
    uint32_t colorpos = color;

    uint32_t mask = pow(2, BlueMaskSize) - 1;
    mask=mask<<BlueFieldPosition;
    return (uint8_t)((colorpos & mask)>>BlueFieldPosition);
    
}

int vg_draw_matrix(uint8_t no_rectangles, uint32_t first,uint8_t step){
    uint16_t w=h_res/no_rectangles;
    uint16_t h=v_res/no_rectangles;
    uint32_t color=0, r, g ,b;
   for (uint16_t  i = 0; i < no_rectangles; i++)
  { 
      for(uint16_t j=0; j<no_rectangles;j++){
          if(h_res%no_rectangles==0 && v_res%no_rectangles==0){
            color= (first + (i * no_rectangles + j) * step) % (1 << bits_per_pixel);
            vg_draw_rectangle(i*w,j*h, w,h,color);
            
          }else{
            r = (getR(first) + j * step) % (1 << RedMaskSize)<<RedFieldPosition;
	        g = (getG(first) + i * step) % (1 << GreenMaskSize)<<GreenFieldPosition;
	        b = (getB(first) + (j + i) * step) % (1 << BlueMaskSize)<<BlueFieldPosition;
            color=r|g|b;
            vg_draw_rectangle(i*w,j*h, w,h,color);
          }
      }
  }
  return 0;
}

int vg_draw_xpm(xpm_map_t xpm,uint16_t x, uint16_t y){
    uint8_t* pixmap;
    xpm_image_t image;
    pixmap=(xpm_load)(xpm, XPM_INDEXED, &image);
    for(int i=0; i<image.height;i++){
        for(int j=0; j<image.width; j++){
            pixel_color( j+x, i+y, pixmap[j + i*image.width]);
        }
    }
    return 0;
}



int vg_move_xpm_positive(xpm_map_t xpm, int16_t *x, int16_t *y, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate ){
   
    speed=abs(speed);
    memset(video_mem, 0, bytes_per_pixel * h_res * v_res);
    if(yi==yf){
        if(xi>xf){
            *x-=speed;
        }else{
            *x+=speed;
        }
        
    }
    else if(xi==xf){
            if(yi>yf){
            *y-=speed;
        }else{
            *y+=speed;
        }
    }

    if((*x>=xf && yi==yf && xi<=xf) ||( *y>=yf && xi==xf && yi<=yf)||(*y<=yf && xi==xf && yi>yf)||(*x<=xf && yi==yf && xi>xf)){/* if we are in the final position*/
        vg_draw_xpm(xpm, xf, yf);
        return 1;
    }
    else{
        vg_draw_xpm(xpm, *x, *y);
    }
    
    return 0;
    
} 
    
int vg_move_xpm_negative(xpm_map_t xpm, int16_t *x, int16_t *y, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate ){
    speed=abs(speed);
    if(counter%speed==0){
        memset(video_mem, 0, bytes_per_pixel * h_res * v_res);
            if(yi==yf){
            if(xi>xf){
                *x-=1;
            }else{
                *x+=1;
            }
            
        }
        else if(xi==xf){
                if(yi>yf){
                *y-=1;
            }else{
                *y+=1;
            }
            
        }
        vg_draw_xpm(xpm, *x, *y);
    }
    return 0;                    
}
void change_buffer(){
    memcpy(video_mem, double_buff, vram_size);
}
