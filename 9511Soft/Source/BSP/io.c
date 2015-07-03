
/// ----------------------------------------------------------------------------
/// File Name          : io.c
/// Description        : 通用IO相关
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07/d10     v0.01   chenyong    setup
/// IO模块用于建立应用引脚ID与端口(port)和引脚号(pin)之间的对应关系
/// 此模块方便对IO资源的分配, 在io.h中改写, 不用定义端口.
/// 同时此模块的函数可以替代stm32f10x_gpio.c中部分函数的功能。




#define  IO_MODULE          /// 模块定义, 用区分内外部定义


/// Includes --------------------------------------------------------------------------------------------

#include  <stm32f10x.h>
#include  "io.h"


/// Private typedef -------------------------------------------------------------------------------------


/// Private define --------------------------------------------------------------------------------------


/// Private macro ---------------------------------------------------------------------------------------


/// Private function prototypes -------------------------------------------------------------------------

//static  __INLINE  void  IO_IdToPortPin (IO_ID_TypeDef id, u32* port, u32* pin);

static  __INLINE  void  IO_IdToPortPin     (IO_ID_TypeDef   id,
                                            GPIO_TypeDef**  port,
                                            u32*            pin);
                                            
static  __INLINE  void  IO_IdsToPortPinMsk (IO_ID_TypeDef   id,
                                            u32             nbit,
                                            GPIO_TypeDef**  port,
                                            u32*            pin,
                                            u32*            mask);


/// Private variables -----------------------------------------------------------------------------------


/// Private consts --------------------------------------------------------------------------------------


/// Private functions -----------------------------------------------------------------------------------

/// -----------------------------------------------------------------------------------------------------
/// @brief  由应用引脚id号得到端口号和端口内引脚号
/// @param  - id    : 应用引脚id号
///         - port  : 端口号(输出)
///         - pin   : 端口内引脚号(输出)
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
//static  __INLINE  void  IO_IdToPortPin (IO_ID_TypeDef id, u32* port, u32* pin)
//{
//    *port   =   id >> IO_ID_BITS_PIN;
//    *pin    =   id & ((1 << IO_ID_BITS_PIN) - 1);
//}

//static  __INLINE  u32  IO_GetMsk (u32 pin, u32 nbit)
//{
//    u32     n;
//    
//    
//    n   =   (1 << IO_ID_BITS_PIN) - pin;
//    n   =   n < nbit ? n : nbit;
//    return  (1 << n) - 1;
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief  由单个应用引脚id号得到端口号和端口内引脚号
/// @param  - id    : 应用引脚id号
///         - port  : 端口号(输出)
///         - pin   : 端口内引脚号(输出)
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  __INLINE  void  IO_IdToPortPin (IO_ID_TypeDef   id,
                                        GPIO_TypeDef**  port,
                                        u32*            pin)
{
    /// switch - case 结构汇编后, 情况类似于查表而非条件跳转, 实际不影响速度
    /// 使用数组后, 程序空间增加, 所以不使用数组
//    GPIO_TypeDef*  gpio[7]  =   {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG};
//    
//    
//    *port   =   gpio[id >> IO_ID_BITS_PIN];
    
    switch (id >> IO_ID_BITS_PIN) {
    case 0:
        *port   =   GPIOA;
        break;
        
    case 1:
        *port   =   GPIOB;
        break;
        
    case 2:
        *port   =   GPIOC;
        break;
        
    case 3:
        *port   =   GPIOD;
        break;
        
    case 4:
        *port   =   GPIOE;
        break;
        
    case 5:
        *port   =   GPIOF;
        break;
        
    case 6:
    default:                /// continue
        *port   =   GPIOG;
        break;
    }
    
    *pin    =   id & ((1 << IO_ID_BITS_PIN) - 1);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  由最低位应用引脚id号和位数得到端口号, 端口内最低引脚号, 掩码
/// @param  - id    : 最低位的应用引脚id号
///         - nbit  : 连续id号的个数
///         - port  : 端口号(输出)
///         - pin   : 端口内引脚号(输出)
///         - mask  : 掩码(输出)
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  __INLINE  void  IO_IdsToPortPinMsk (IO_ID_TypeDef   id,
                                            u32             nbit,
                                            GPIO_TypeDef**  port,
                                            u32*            pin,
                                            u32*            mask)
{
    u32     np;
    u32     nb;
    
    
    IO_IdToPortPin(id, port, &np);
    
    nb      =   (1 << IO_ID_BITS_PIN) - np;
    nb      =   (nbit <= nb) ? nbit : nb;
    
    *pin    =   np;
    *mask   =   (1 << nb) - 1;
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  读输入引脚状态
///         例如读PA03和PA04口, 则调用 IO_InGet(PA03, 2);
///         例如读取整个PD口,   则调用 IO_InGet(PD00, 1 << IO_ID_BITS_PIN);
/// @param  - id    : 应用引脚id号或最小id号
///         - nbit  : 读出的位数
/// @retval : 读出的结果
/// -----------------------------------------------------------------------------------------------------
//void  IO_Init (IO_ID_TypeDef id, u32 nbit, GPIOMode_TypeDef mode)
//{
//    GPIO_InitTypeDef    sGPIOInit;
//    u32                 port;
//    u32                 pin;
//    
//    
//    IO_IdToPortPin(id, &port, &pin);
//    
//    sGPIOInit.GPIO_Pin      =   (((1 << nbit) - 1) << pin) & ((1 << IO_ID_BITS_PIN) - 1);
//    sGPIOInit.GPIO_Mode     =   mode;
//    sGPIOInit.GPIO_Speed    =   GPIO_Speed_50MHz;
//    
//    switch (port) {
//    case 0:
//        GPIO_Init(GPIOA, &sGPIOInit);
//        break;
//        
//    case 1:
//        GPIO_Init(GPIOB, &sGPIOInit);
//        break;
//        
//    case 2:
//        GPIO_Init(GPIOC, &sGPIOInit);
//        break;
//        
//    case 3:
//        GPIO_Init(GPIOD, &sGPIOInit);
//        break;
//        
//    case 4:
//        GPIO_Init(GPIOE, &sGPIOInit);
//        break;
//        
//    case 5:
//        GPIO_Init(GPIOF, &sGPIOInit);
//        break;
//        
//    case 6:
//        GPIO_Init(GPIOG, &sGPIOInit);
//        break;
//        
//    default:
//        break;
//    }
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief  单个IO口初始化
///         该函数对于输出口按50MHz初始化
/// @param  - id    : 应用引脚id号
///         - mode  : 设置的模式
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  IO_Init (IO_ID_TypeDef id, GPIOMode_TypeDef mode)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    GPIO_InitTypeDef    sGPIOInit;
    
    
    IO_IdToPortPin(id, &port, &pin);
    
    sGPIOInit.GPIO_Pin      =   (uint16_t)(1 << pin);
    sGPIOInit.GPIO_Mode     =   mode;
    sGPIOInit.GPIO_Speed    =   GPIO_Speed_50MHz;
    GPIO_Init(port, &sGPIOInit);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  多个IO口初始化
///         该函数对于输出口按50MHz初始化
/// @param  - id    : 最低应用引脚id号
///         - nbit  : 连续引脚个数
///         - mode  : 设置的模式
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  IO_MultiInit (IO_ID_TypeDef id, u32 nbit, GPIOMode_TypeDef mode)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    u32                 mask;
    GPIO_InitTypeDef    sGPIOInit;
    
    
    IO_IdsToPortPinMsk(id, nbit, &port, &pin, &mask);
    
    sGPIOInit.GPIO_Pin      =   (uint16_t)(mask << pin);
    sGPIOInit.GPIO_Mode     =   mode;
    sGPIOInit.GPIO_Speed    =   GPIO_Speed_50MHz;
    GPIO_Init(port, &sGPIOInit);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  读输入引脚状态
///         例如读PA03和PA04口, 则调用 IO_InGet(PA03, 2);
///         例如读取整个PD口,   则调用 IO_InGet(PD00, 1 << IO_ID_BITS_PIN);
/// @param  - id    : 应用引脚id号或最小id号
///         - nbit  : 读出的位数
/// @retval : 读出的结果
/// -----------------------------------------------------------------------------------------------------
//u32  IO_InGet (IO_ID_TypeDef id, u32 nbit)
//{
//    u32     port;
//    u32     pin;
//    u32     msk;
//    
//    
////    port    =   id >> IO_ID_BITS_PIN;
////    pin     =   id & ((1 << IO_ID_BITS_PIN) - 1);
//    IO_IdToPortPin(id, &port, &pin);
//    
//    msk     =   (1 << nbit) - 1;
//    
//    switch (port) {
//    case 0:
//        return  ((GPIOA->IDR) >> pin) & msk;
//        
//    case 1:
//        return  ((GPIOB->IDR) >> pin) & msk;
//        
//    case 2:
//        return  ((GPIOC->IDR) >> pin) & msk;
//        
//    case 3:
//        return  ((GPIOD->IDR) >> pin) & msk;
//        
//    case 4:
//        return  ((GPIOE->IDR) >> pin) & msk;
//        
//    case 5:
//        return  ((GPIOF->IDR) >> pin) & msk;
//        
//    case 6:
//        return  ((GPIOG->IDR) >> pin) & msk;
//        
//    default:
//        return  0;
//    }
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief  读单个输入引脚状态
///         例如读PA04口, 则调用 IO_InGet(PA04);
/// @param  - id    : 应用引脚id号
/// @retval : 读出的结果, bit0有效, 其他位为零
/// -----------------------------------------------------------------------------------------------------
u32  IO_InGet (IO_ID_TypeDef id)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    
    
    IO_IdToPortPin(id, &port, &pin);
    return  (u32)(((port->IDR) >> pin) & 1);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  读多个输入引脚状态
///         例如读PA03和PA04口, 则调用 IO_MultiInGet(PA03, 2);
///         例如读取整个PD口,   则调用 IO_MultiInGet(PD00, 1 << IO_ID_BITS_PIN);
/// @param  - id    : 最低的应用引脚id号
///         - nbit  : 读出的位数, 即连续引脚的个数
/// @retval : 读出的结果, 有效位表示结果, 其他位为零
/// -----------------------------------------------------------------------------------------------------
u32  IO_MultiInGet (IO_ID_TypeDef id, u32 nbit)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    u32                 mask;
    
    
    IO_IdsToPortPinMsk(id, nbit, &port, &pin, &mask);
    return  (u32)(((port->IDR) >> pin) & mask);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  读输出引脚状态
///         例如读PA03和PA04口, 则调用 IO_OutGet(PA03, 2);
///         例如读取整个PD口,   则调用 IO_OutGet(PD00, 1 << IO_ID_BITS_PIN);
/// @param  - id    : 应用引脚id号或最小id号
///         - nbit  : 读出的位数
/// @retval : 读出的结果
/// -----------------------------------------------------------------------------------------------------
//u32  IO_OutGet (IO_ID_TypeDef id, u32 nbit)
//{
//    u32     port;
//    u32     pin;
//    u32     msk;
//    
//    
////    port    =   id >> IO_ID_BITS_PIN;
////    pin     =   id & ((1 << IO_ID_BITS_PIN) - 1);
//    IO_IdToPortPin(id, &port, &pin);
//    
//    msk     =   (1 << nbit) - 1;
//    
//    switch (port) {
//    case 0:
//        return  ((GPIOA->ODR) >> pin) & msk;
//        
//    case 1:
//        return  ((GPIOB->ODR) >> pin) & msk;
//        
//    case 2:
//        return  ((GPIOC->ODR) >> pin) & msk;
//        
//    case 3:
//        return  ((GPIOD->ODR) >> pin) & msk;
//        
//    case 4:
//        return  ((GPIOE->ODR) >> pin) & msk;
//        
//    case 5:
//        return  ((GPIOF->ODR) >> pin) & msk;
//        
//    case 6:
//        return  ((GPIOG->ODR) >> pin) & msk;
//        
//    default:
//        return  0;
//    }
//}
/// -----------------------------------------------------------------------------------------------------
/// @brief  读单个输出引脚的状态
///         例如读PA04口, 则调用 IO_OutGet(PA04);
/// @param  - id    : 应用引脚id号
/// @retval : 读出的结果, bit0有效, 其他位为零
/// -----------------------------------------------------------------------------------------------------
u32  IO_OutGet (IO_ID_TypeDef id)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    
    
    IO_IdToPortPin(id, &port, &pin);
    return  (u32)(((port->ODR) >> pin) & 1);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  读多个输出引脚的状态
///         例如读PA03和PA04口, 则调用 IO_MultiOutGet(PA03, 2);
///         例如读取整个PD口,   则调用 IO_MultiOutGet(PD00, 1 << IO_ID_BITS_PIN);
/// @param  - id    : 最低的应用引脚id号
///         - nbit  : 读出的位数, 即连续引脚的个数
/// @retval : 读出的结果, 有效位表示结果, 其他位为零
/// -----------------------------------------------------------------------------------------------------
u32  IO_MultiOutGet (IO_ID_TypeDef id, u32 nbit)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    u32                 mask;
    
    
    IO_IdsToPortPinMsk(id, nbit, &port, &pin, &mask);
    return  (u32)(((port->ODR) >> pin) & mask);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  设置输出引脚状态
///         例如设置PB03为High, PB04为Low, PB05为High, 则调用 IO_OutSet(PB03, 3, 0x05);
/// @param  - id    : 应用引脚id号或最小id号
///         - nbit  : 设置的位数
///         - value : 设置的值
/// @retval : 设置后端口的状态
/// -----------------------------------------------------------------------------------------------------
//u32  IO_OutSet (IO_ID_TypeDef id, u32 nbit, u32 value)
//{
//    u32     port;
//    u32     pin;
//    u32     msk;
//    u32     and;
//    u32     or;
//    u32     tmp;
//    
//    
////    port    =   id >> IO_ID_BITS_PIN;
////    pin     =   id & ((1 << IO_ID_BITS_PIN) - 1);
//    IO_IdToPortPin(id, &port, &pin);
//    
//    msk     =   (1 << nbit) - 1;
//    and     =  ~(msk << pin);
//    or      =   (value & msk) << pin;
//    
//    switch (port) {
//    case 0:
//        tmp         =   (GPIOA->ODR) & and | or;
//        GPIOA->ODR  =   tmp;
//        break;
//        
//    case 1:
//        tmp         =   (GPIOB->ODR) & and | or;
//        GPIOB->ODR  =   tmp;
//        break;
//        
//    case 2:
//        tmp         =   (GPIOC->ODR) & and | or;
//        GPIOC->ODR  =   tmp;
//        break;
//        
//    case 3:
//        tmp         =   (GPIOD->ODR) & and | or;
//        GPIOD->ODR  =   tmp;
//        break;
//        
//    case 4:
//        tmp         =   (GPIOE->ODR) & and | or;
//        GPIOE->ODR  =   tmp;
//        break;
//        
//    case 5:
//        tmp         =   (GPIOF->ODR) & and | or;
//        GPIOF->ODR  =   tmp;
//        break;
//        
//    case 6:
//        tmp         =   (GPIOG->ODR) & and | or;
//        GPIOG->ODR  =   tmp;
//        break;
//        
//    default:
//        tmp         =   0;
//        break;
//    }
//    
//    return  tmp;
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief  设置单个输出引脚状态
///         例如设置PB03为High, 则调用 IO_OutSet(PB03, 0x01);
/// @param  - id    : 应用引脚id号
///         - value : 设置的值, bit0有效, 其他位不关心
/// @retval : 设置后端口的状态, 16位有效
/// -----------------------------------------------------------------------------------------------------
u32  IO_OutSet (IO_ID_TypeDef id, u32 value)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    u32                 tmp;
    
    
    IO_IdToPortPin(id, &port, &pin);
    tmp         =   (port->ODR) & (~(1 << pin)) | ((value & 1) << pin);
    (port->ODR) =   tmp;
    return  tmp;
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  设置多个输出引脚状态
///         例如设置PB03为High, PB04为Low, PB05为High, 则调用 IO_MultiOutSet(PB03, 3, 0x05);
/// @param  - id    : 最低的应用引脚id号
///         - nbit  : 设置的位数, 即连续引脚的个数
///         - value : 设置的值, 有效位以上忽略
/// @retval : 设置后端口的状态, 16位有效
/// -----------------------------------------------------------------------------------------------------
u32  IO_MultiOutSet (IO_ID_TypeDef id, u32 nbit, u32 value)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    u32                 mask;
    u32                 tmp;
    
    
    IO_IdsToPortPinMsk(id, nbit, &port, &pin, &mask);
    tmp         =   (port->ODR) & (~(mask << pin)) | ((value & mask) << pin);
    (port->ODR) =   tmp;
    return  tmp;
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  翻转输出引脚状态
///         例如翻转PC13, PC14, PC15, 则调用 IO_OutToggle(PC13, 3);
/// @param  - id    : 应用引脚id号或最小id号
///         - nbit  : 操作的位数
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
//void  IO_OutToggle (IO_ID_TypeDef id, u32 nbit)
//{
//    u32     port;
//    u32     pin;
//    u32     xor;
//    
//    
////    port    =   id >> IO_ID_BITS_PIN;
////    pin     =   id & ((1 << IO_ID_BITS_PIN) - 1);
//    IO_IdToPortPin(id, &port, &pin);
//    
//    xor     =   ((1 << nbit) - 1) << pin;
//    
//    switch (port) {
//    case 0:
//        (GPIOA->ODR)   ^=   xor;
//        break;
//        
//    case 1:
//        (GPIOB->ODR)   ^=   xor;
//        break;
//        
//    case 2:
//        (GPIOC->ODR)   ^=   xor;
//        break;
//        
//    case 3:
//        (GPIOD->ODR)   ^=   xor;
//        break;
//        
//    case 4:
//        (GPIOE->ODR)   ^=   xor;
//        break;
//        
//    case 5:
//        (GPIOF->ODR)   ^=   xor;
//        break;
//        
//    case 6:
//        (GPIOG->ODR)   ^=   xor;
//        break;
//        
//    default:
//        break;
//    }
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief  翻转单个输出引脚状态
///         例如翻转PC13, 则调用 IO_OutToggle(PC13);
/// @param  - id    : 应用引脚id号
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  IO_OutToggle (IO_ID_TypeDef id)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    
    
    IO_IdToPortPin(id, &port, &pin);
    (port->ODR)    ^=   (1 << pin);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  翻转多个输出引脚状态
///         例如翻转PC13, PC14, 则调用 IO_OutToggle(PC13, 2);
/// @param  - id    : 最低的应用引脚id号
///         - nbit  : 设置的位数, 即连续引脚的个数
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  IO_MultiOutToggle (IO_ID_TypeDef id, u32 nbit)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    u32                 mask;
    
    
    IO_IdsToPortPinMsk(id, nbit, &port, &pin, &mask);
    (port->ODR)    ^=   (mask << pin);
}



/// End of file -----------------------------------------------------------------------------------------
