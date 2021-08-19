#include <lcom/lcf.h>


#include <stdbool.h>
#include <stdint.h>


//KBC commands
#define IRQ1                      1
#define STATUS_REG                0x64
#define OUT_BUF                   0x60
#define IN_BUF                    0X60
#define DELAY_US                  2000


#define ESC_BREAKCODE             0x81

#define FIRST_BYTE                0xE0

//STATUS REGISTER
#define PARITY_ERROR              BIT(7)
#define TIMEOUT_ERROR             BIT(6)
#define INH                       BIT(4)
#define AUX                       BIT(3)  
#define IBF                       BIT(1) //IF BIT 1=1 CANNOT WRITE
#define OBF                       BIT(0)

//KEYBOARD-RELATED KBC COMMANDS
#define READ_COMMAND_BYTE         0X20
#define WRITE_COMMAND_BYTE        0X60
#define CHECK_KBC                 0XAA
#define CHECK_KEYBOARD_INTERFACE  0XAB
#define DISABLE_KBD_INTERFACE     0XAD
#define ENABLE_KBD_INTERFACE      0XAE


//KBC COMMAND BYTE
#define DIS2                      BIT(5)
#define DIS                       BIT(4)
#define INIT2                     BIT(1)
#define INIT                      BIT(0)
