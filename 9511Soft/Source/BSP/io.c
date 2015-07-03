
/// ----------------------------------------------------------------------------
/// File Name          : io.c
/// Description        : ͨ��IO���
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07/d10     v0.01   chenyong    setup
/// IOģ�����ڽ���Ӧ������ID��˿�(port)�����ź�(pin)֮��Ķ�Ӧ��ϵ
/// ��ģ�鷽���IO��Դ�ķ���, ��io.h�и�д, ���ö���˿�.
/// ͬʱ��ģ��ĺ����������stm32f10x_gpio.c�в��ֺ����Ĺ��ܡ�




#define  IO_MODULE          /// ģ�鶨��, ���������ⲿ����


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
/// @brief  ��Ӧ������id�ŵõ��˿ںźͶ˿������ź�
/// @param  - id    : Ӧ������id��
///         - port  : �˿ں�(���)
///         - pin   : �˿������ź�(���)
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
/// @brief  �ɵ���Ӧ������id�ŵõ��˿ںźͶ˿������ź�
/// @param  - id    : Ӧ������id��
///         - port  : �˿ں�(���)
///         - pin   : �˿������ź�(���)
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  __INLINE  void  IO_IdToPortPin (IO_ID_TypeDef   id,
                                        GPIO_TypeDef**  port,
                                        u32*            pin)
{
    /// switch - case �ṹ����, ��������ڲ�����������ת, ʵ�ʲ�Ӱ���ٶ�
    /// ʹ�������, ����ռ�����, ���Բ�ʹ������
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
/// @brief  �����λӦ������id�ź�λ���õ��˿ں�, �˿���������ź�, ����
/// @param  - id    : ���λ��Ӧ������id��
///         - nbit  : ����id�ŵĸ���
///         - port  : �˿ں�(���)
///         - pin   : �˿������ź�(���)
///         - mask  : ����(���)
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
/// @brief  ����������״̬
///         �����PA03��PA04��, ����� IO_InGet(PA03, 2);
///         �����ȡ����PD��,   ����� IO_InGet(PD00, 1 << IO_ID_BITS_PIN);
/// @param  - id    : Ӧ������id�Ż���Сid��
///         - nbit  : ������λ��
/// @retval : �����Ľ��
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
/// @brief  ����IO�ڳ�ʼ��
///         �ú�����������ڰ�50MHz��ʼ��
/// @param  - id    : Ӧ������id��
///         - mode  : ���õ�ģʽ
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
/// @brief  ���IO�ڳ�ʼ��
///         �ú�����������ڰ�50MHz��ʼ��
/// @param  - id    : ���Ӧ������id��
///         - nbit  : �������Ÿ���
///         - mode  : ���õ�ģʽ
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
/// @brief  ����������״̬
///         �����PA03��PA04��, ����� IO_InGet(PA03, 2);
///         �����ȡ����PD��,   ����� IO_InGet(PD00, 1 << IO_ID_BITS_PIN);
/// @param  - id    : Ӧ������id�Ż���Сid��
///         - nbit  : ������λ��
/// @retval : �����Ľ��
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
/// @brief  ��������������״̬
///         �����PA04��, ����� IO_InGet(PA04);
/// @param  - id    : Ӧ������id��
/// @retval : �����Ľ��, bit0��Ч, ����λΪ��
/// -----------------------------------------------------------------------------------------------------
u32  IO_InGet (IO_ID_TypeDef id)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    
    
    IO_IdToPortPin(id, &port, &pin);
    return  (u32)(((port->IDR) >> pin) & 1);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  �������������״̬
///         �����PA03��PA04��, ����� IO_MultiInGet(PA03, 2);
///         �����ȡ����PD��,   ����� IO_MultiInGet(PD00, 1 << IO_ID_BITS_PIN);
/// @param  - id    : ��͵�Ӧ������id��
///         - nbit  : ������λ��, ���������ŵĸ���
/// @retval : �����Ľ��, ��Чλ��ʾ���, ����λΪ��
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
/// @brief  ���������״̬
///         �����PA03��PA04��, ����� IO_OutGet(PA03, 2);
///         �����ȡ����PD��,   ����� IO_OutGet(PD00, 1 << IO_ID_BITS_PIN);
/// @param  - id    : Ӧ������id�Ż���Сid��
///         - nbit  : ������λ��
/// @retval : �����Ľ��
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
/// @brief  ������������ŵ�״̬
///         �����PA04��, ����� IO_OutGet(PA04);
/// @param  - id    : Ӧ������id��
/// @retval : �����Ľ��, bit0��Ч, ����λΪ��
/// -----------------------------------------------------------------------------------------------------
u32  IO_OutGet (IO_ID_TypeDef id)
{
    GPIO_TypeDef*       port;
    u32                 pin;
    
    
    IO_IdToPortPin(id, &port, &pin);
    return  (u32)(((port->ODR) >> pin) & 1);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  �����������ŵ�״̬
///         �����PA03��PA04��, ����� IO_MultiOutGet(PA03, 2);
///         �����ȡ����PD��,   ����� IO_MultiOutGet(PD00, 1 << IO_ID_BITS_PIN);
/// @param  - id    : ��͵�Ӧ������id��
///         - nbit  : ������λ��, ���������ŵĸ���
/// @retval : �����Ľ��, ��Чλ��ʾ���, ����λΪ��
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
/// @brief  �����������״̬
///         ��������PB03ΪHigh, PB04ΪLow, PB05ΪHigh, ����� IO_OutSet(PB03, 3, 0x05);
/// @param  - id    : Ӧ������id�Ż���Сid��
///         - nbit  : ���õ�λ��
///         - value : ���õ�ֵ
/// @retval : ���ú�˿ڵ�״̬
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
/// @brief  ���õ����������״̬
///         ��������PB03ΪHigh, ����� IO_OutSet(PB03, 0x01);
/// @param  - id    : Ӧ������id��
///         - value : ���õ�ֵ, bit0��Ч, ����λ������
/// @retval : ���ú�˿ڵ�״̬, 16λ��Ч
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
/// @brief  ���ö���������״̬
///         ��������PB03ΪHigh, PB04ΪLow, PB05ΪHigh, ����� IO_MultiOutSet(PB03, 3, 0x05);
/// @param  - id    : ��͵�Ӧ������id��
///         - nbit  : ���õ�λ��, ���������ŵĸ���
///         - value : ���õ�ֵ, ��Чλ���Ϻ���
/// @retval : ���ú�˿ڵ�״̬, 16λ��Ч
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
/// @brief  ��ת�������״̬
///         ���緭תPC13, PC14, PC15, ����� IO_OutToggle(PC13, 3);
/// @param  - id    : Ӧ������id�Ż���Сid��
///         - nbit  : ������λ��
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
/// @brief  ��ת�����������״̬
///         ���緭תPC13, ����� IO_OutToggle(PC13);
/// @param  - id    : Ӧ������id��
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
/// @brief  ��ת����������״̬
///         ���緭תPC13, PC14, ����� IO_OutToggle(PC13, 2);
/// @param  - id    : ��͵�Ӧ������id��
///         - nbit  : ���õ�λ��, ���������ŵĸ���
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
