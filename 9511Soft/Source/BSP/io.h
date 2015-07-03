
/// ----------------------------------------------------------------------------
/// File Name          : io.h
/// Description        : 通用IO相关
/// ----------------------------------------------------------------------------
/// History:

/// y11/m01/d18     v0.02   chenyong    modify
/// 因为9511硬件由V0升级为V1
/// 加入条件编译 HARDWARE_VER

/// y10/m07/d10     v0.01   chenyong    setup
/// IO模块用于建立应用引脚ID与端口(port)和引脚号(pin)之间的对应关系
/// 此模块方便对IO资源的分配, 在io.h中改写, 不用定义端口.
/// 同时此模块的函数可以替代stm32f10x_gpio.c中部分函数的功能。


/// Define to prevent recursive inclusion --------------------------------------
#ifndef  IO_H
#define  IO_H


/// Includes -------------------------------------------------------------------


/// Exported constants ---------------------------------------------------------

/// IO_ID的低4位表示PIN号
#define  IO_ID_BITS_PIN     4


/// Exported macro -------------------------------------------------------------


/// Exported types -------------------------------------------------------------

typedef enum
{
    /// GPIOA
    PA00_ENC_A_ODD  =   0,
    PA01_ENC_B_ODD,
    PA02_ENC_I_ODD,
    PA03_V_DETECT,
    PA04_DIP5,
    PA05_DIP6,
    PA06_DIP7,
    PA07_DIP8,
    PA08_SOL0,
    PA09_491_TX,
    PA10_491_RX,
    PA11_CAN_RX,
    PA12_CAN_TX,
    PA13_JTMS,
    PA14_JTCK,
    PA15_JTDI,
    
    /// GPIOB
    PB00_POT_ODD,
    PB01_POT_EVEN,
    PB02_BOOT1,
    PB03_JTDO,
    PB04_JTRST,
    PB05_LED1,
    PB06_ENC_A_EVEN,
    PB07_ENC_B_EVEN,
    PB08_ENC_I_EVEN,
    PB09_LED2,
    PB10_485_TX,
    PB11_485_RX,
    PB12_TQ1_O_ODD,
    PB13_TQ2_O_ODD,
    PB14_DIR_C_ODD,
    PB15_PRO_C_ODD,
    
    /// GPIOC
    PC00_DIP1,
    PC01_DIP2,
    PC02_DIP3,
    PC03_DIP4,
    PC04_DIP9,
    PC05_DIP10,
    PC06_TQ1_C_EVEN,
    PC07_TQ2_C_EVEN,
   // PC08_SOL1,
    //PC09_SOL2,
	PC08_SOL_ODD,
    PC09_SOL_EVEN,
    PC10_HD_UP_EVEN,
    PC11_HD_R_EVEN,
    PC12_HD_G_EVEN,
    PC13_491_DE,
    PC14,
    PC15_485_DE,
    
    /// GPIOD
    PD00_HD_DOWN_EVEN,
    PD01_DJ_CUT_EVEN,
    PD02_SQN_G_ODD,
    PD03_SQN_R_ODD,
    PD04_SQN_G_EVEN,
    PD05_SQN_R_EVEN,
    PD06_XDJ_ODD,
    PD07_XDJ_EVEN,
//    PD08_RESET_C_ODD,
#if     HARDWARE_VER == 0
    PD08_RESET_C_ODD,
#elif   HARDWARE_VER == 1
    PD08_RESET_O,           /// 开环电机复位, 奇偶电机都使用此信号 (9511_V1新加)
#endif
    PD09_CLK_C_ODD,
    PD10_TQ1_C_ODD,
    PD11_TQ2_C_ODD,
    PD12_DIR_C_EVEN,
    PD13_PRO_C_EVEN,
    PD14_RESET_C_EVEN,
    PD15_CLK_C_EVEN,
    
    /// GPIOE
    PE00_M1_C_EVEN,
    PE01_TQ1_O_EVEN,
    PE02_TQ2_O_EVEN,
    PE03_CLK_O_EVEN,
    PE04_DIR_O_EVEN,
    PE05_PRO_O_EVEN,
    PE06,
    PE07_DJ_CUT_ODD,
    PE08_HD_UP_ODD,
    PE09_HD_R_ODD,
    PE10_HD_DOWN_ODD,
    PE11_HD_G_ODD,
    PE12_DIR_O_ODD,
    PE13_PRO_O_ODD,
//    PE14_M1_C_ODD,
#if     HARDWARE_VER == 0
    PE14_M1_C_ODD,
#elif   HARDWARE_VER == 1
    PE14_ENABLE_O,          /// 开环电机使能, 奇偶电机都使用此信号 (9511_V1新加)
#endif
    PE15_CLK_O_ODD,
    
    /// GPIOF
    PF00,
    PF01,
    PF02,
    PF03,
    PF04,
    PF05,
    PF06,
    PF07,
    PF08,
    PF09,
    PF10,
    PF11,
    PF12,
    PF13,
    PF14,
    PF15,
    
    /// GPIOG
    PG00,
    PG01,
    PG02,
    PG03,
    PG04,
    PG05,
    PG06,
    PG07,
    PG08,
    PG09,
    PG10,
    PG11,
    PG12,
    PG13,
    PG14,
    PG15
    
} IO_ID_TypeDef;


/// Exported functions ---------------------------------------------------------

void  IO_Init           (IO_ID_TypeDef id,           GPIOMode_TypeDef mode);
void  IO_MultiInit      (IO_ID_TypeDef id, u32 nbit, GPIOMode_TypeDef mode);

u32   IO_InGet          (IO_ID_TypeDef id);
u32   IO_MultiInGet     (IO_ID_TypeDef id, u32 nbit);

u32   IO_OutGet         (IO_ID_TypeDef id);
u32   IO_MultiOutGet    (IO_ID_TypeDef id, u32 nbit);

u32   IO_OutSet         (IO_ID_TypeDef id,           u32 value);
u32   IO_MultiOutSet    (IO_ID_TypeDef id, u32 nbit, u32 value);

void  IO_OutToggle      (IO_ID_TypeDef id);
void  IO_MultiOutToggle (IO_ID_TypeDef id, u32 nbit);



#endif
/// End of file -----------------------------------------------------------------------------------------
