
/// ----------------------------------------------------------------------------
/// File Name          : bsp.c
/// Description        : �弶֧�ְ�
/// ----------------------------------------------------------------------------
/// History:

/// y11/m01/d19     v0.09   chenyong    modify
/// ��Ϊ9511Ӳ����V0����ΪV1
/// ������������ HARDWARE_VER

/// y11/m01/d18     v0.08   chenyong    modify
/// ����� PC924 ���ж� �� ��������
/// ��BSP_NVIC_Cfg()��, �����ϸ�, �����ж�����
/// ����һ�㺯��, ���ƿ���, ֻ�� MOTC(�ջ����) ��������, ������������

/// y10/m12/d24     v0.07   chenyong    modify
/// ���Ӻ��� BSP_MOTO_ClkGet() �� BSP_MOTC_ClkGet()

/// y10/m12/d23     v0.06   chenyong    modify
/// ����� PC924 ���ж� �� ��������

/// y10/m12/d03     v0.05   chenyong    modify
/// ���ı������������

/// y10/m11/d30     v0.04   chenyong    modify
/// ���ĺ��� BSP_POT_GetOne(), ԭ���̶�ȡ��Ԫ0��ֵ, ��Ϊ����ȡֵ

/// y10/m09/d10     v0.03   chenyong    modify
/// ����ΪPC924��������ֻ��BSP_Judge924()��������Ч
/// �������ͨ��ȫ�ֱ����жϰ�����
/// ����

/// y10/m07/d12     v0.02   chenyong    modify
/// ʹ��ioģ��. io �� MPU �������; bsp �빦���������

/// y10/m07/d05     v0.01   chenyong    setup





#define  BSP_MODULE     /// ģ�鶨��, ���������ⲿ����


/// Includes -------------------------------------------------------------------
#include  "bsp.h"
#include  "io.h"
#include "../3ds/crypto_des3.h"
#include "../3ds/bitstream.h"
#include <includes.h>
/// Private define --------------------------------------------------------------------------------------
    
/// --------    gpio    --------
/// ��io.h

/// -------- peripheral --------

/// ----    Timer   ----

/// ----   encoder  ----
    
///// ----   8MTimer  ----
//    #define     BSP_TMR_8M              TIM1                        /// ��ʱ��, ��Ҫ����125nSΪ��λ�Ķ�ʱ
//    #define     BSP_TMR_8M_CLK          8000000                     /// ��ʱ��ʱ��Ϊ8MHz
//    
////    #define     BSP_CC_TIME_OUT         2                           /// ʹ��cc2, ����һ�㶨ʱ��ʱ�䵽��
//    #define     BSP_CC_CLK_ODD          3                           /// ʹ��cc3, ����������ʱ����
//    #define     BSP_CC_CLK_EVEN         4                           /// ʹ��cc4, ����ż����ʱ����
//    
////#if     (BSP_CC_TIME_OUT == BSP_CC_CLK_ODD)
////    #error  "BSP_CC_TIME_OUT and BSP_CC_CLK_ODD use the same cc !"
////#endif
////#if     (BSP_CC_TIME_OUT == BSP_CC_CLK_EVEN)
////    #error  "BSP_CC_TIME_OUT and BSP_CC_CLK_EVEN use the same cc !"
////#endif
//#if     (BSP_CC_CLK_ODD == BSP_CC_CLK_EVEN)
//    #error  "BSP_CC_CLK_ODD and BSP_CC_CLK_EVEN use the same cc !"
//#endif
//
////#if     BSP_CC_TIME_OUT == 1
////    #define     BSP_FLAG_CC_TIME_OUT          TIM_FLAG_CC1
////    #define     BSP_OC_TIME_OUT_INIT_FUNC          TIM_OC1Init
////#elif   BSP_CC_TIME_OUT == 2
////    #define     BSP_FLAG_CC_TIME_OUT          TIM_FLAG_CC2
////    #define     BSP_OC_TIME_OUT_INIT_FUNC          TIM_OC2Init
////#elif   BSP_CC_TIME_OUT == 3
////    #define     BSP_FLAG_CC_TIME_OUT          TIM_FLAG_CC3
////    #define     BSP_OC_TIME_OUT_INIT_FUNC          TIM_OC3Init
////#elif   BSP_CC_TIME_OUT == 4
////    #define     BSP_FLAG_CC_TIME_OUT          TIM_FLAG_CC4
////    #define     BSP_OC_TIME_OUT_INIT_FUNC          TIM_OC4Init
////#endif
//
//#if     BSP_CC_CLK_ODD == 1
//    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC1
//    #define     BSP_OC_CLK_ODD_INIT     TIM_OC1Init
//#elif   BSP_CC_CLK_ODD == 2
//    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC2
//    #define     BSP_OC_CLK_ODD_INIT     TIM_OC2Init
//#elif   BSP_CC_CLK_ODD == 3
//    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC3
//    #define     BSP_OC_CLK_ODD_INIT     TIM_OC3Init
//#elif   BSP_CC_CLK_ODD == 4
//    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC4
//    #define     BSP_OC_CLK_ODD_INIT     TIM_OC4Init
//#endif
//
//#if     BSP_CC_CLK_EVEN == 1
//    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC1
//    #define     BSP_OC_CLK_EVEN_INIT    TIM_OC1Init
//#elif   BSP_CC_CLK_EVEN == 2
//    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC2
//    #define     BSP_OC_CLK_EVEN_INIT    TIM_OC2Init
//#elif   BSP_CC_CLK_EVEN == 3
//    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC3
//    #define     BSP_OC_CLK_EVEN_INIT    TIM_OC3Init
//#elif   BSP_CC_CLK_EVEN == 4
//    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC4
//    #define     BSP_OC_CLK_EVEN_INIT    TIM_OC4Init
//#endif
    
    
/// ----     ADC    ----
    /// ���λ��ֵ�ڵ�16λ, ż��λ��ֵ�ڸ�16λ
    #define     BSP_ADC_VAL_ODD_L_EVEN_H    1
    
#if     BSP_ADC_VAL_ODD_L_EVEN_H
    #define     BSP_ADC_POT_ODD         ADC1
    #define     BSP_ADC_POT_EVEN        ADC2
    #define     BSP_ADC_CH_POT_ODD      ADC_Channel_8
    #define     BSP_ADC_CH_POT_EVEN     ADC_Channel_9
#else
    #define     BSP_ADC_POT_ODD         ADC2
    #define     BSP_ADC_POT_EVEN        ADC1
    #define     BSP_ADC_CH_POT_ODD      ADC_Channel_9
    #define     BSP_ADC_CH_POT_EVEN     ADC_Channel_8
#endif
    
    #define     BSP_ADC_CLK             RCC_PCLK2_Div8              /// ����
//    #define     BSP_ADC_SAMPLETIME      ADC_SampleTime_239Cycles5   /// ���� Լ35.7K@72MHz
    /// 72M/8/(71.5+12.5) = 107.14K, 
    #define     BSP_ADC_SAMPLETIME      ADC_SampleTime_71Cycles5    /// ������ Լ107.1K@72MHz
    /// ͬʱ����ģʽ  DMAʹ��ADC1�ļĴ�������ADC1��2������
    #define     BSP_ADC_MODE            ADC_Mode_RegSimult          /// ͬʱ����ģʽ

/// ----   DMA_ADC  ----      
    #define     BSP_DMA_CHN_POT         DMA1_Channel1               /// ��ӦADC1
    #define     BSP_DMA_CPA_POT         (&(ADC1->DR))

    /// (ADC����Ƶ�� >> BSP_ADC_VAL_N_POWER) Ӧ������ߵ������Ƶ�� 6.6KHz
    #define     BSP_ADC_VAL_N_POWER     3
    #define     BSP_ADC_VAL_NUM         (1 << BSP_ADC_VAL_N_POWER)
    
/// ----     491    ----
    #define     BSP_491_BAUD_RATE       100000
    #define     BSP_491_STOP_BITS       USART_StopBits_2
    
///// ----   IRQ_491  ----
//    #define     BSP_491_IRQn            USART1_IRQn
//    #define     BSP_491_IRQSubPriority  8
    
///// ----   DMA_491  ----
//    #define     BSP_DMA_CHN_491RX       DMA1_Channel5
//    #define     BSP_DMA_CPA_491RX       (&(BSP_USART_491->DR))
//    
///// ----   DAT_491  ----
//    #define     BSP_491_RXBUF_N_POWER   8
//    #define     BSP_491_RXBUF_N         (1 << BSP_491_RXBUF_N_POWER)
//    #define     BSP_491_RXBUF_N_MSK     (BSP_491_RXBUF_N - 1)
//    #define     BSP_491_RXBUF_FREE      ((u16)(-1))                         /// bit9~15 ����Ϊ0����
    
    
/// Private macro ---------------------------------------------------------------------------------------

/// Private typedef -------------------------------------------------------------------------------------

/// ��λ��ֵ
typedef struct {
#if     BSP_ADC_VAL_ODD_L_EVEN_H
    u16     odd;
    u16     even;
#else
    u16     even;
    u16     odd;
#endif
} BSP_POT_t;


/// Private variables -----------------------------------------------------------------------------------

static  BSP_POT_t               BSP_POTValue[BSP_ADC_VAL_NUM];

static  u16                     BSP_TMR_CNT_PRESCALER[BSP_SPD_NUM];

//static  u32                     BSP_DIP_Value;
//static  BSP_MOT_TQ_TypeDef      BSP_DIP_TQ;
//static  BSP_DIP_BD_FUNC_TypeDef BSP_DIP_BDFunc;
//static  u8                      BSP_DIP_9511Addr;

//static  u16         BSP_491_RxBuf[1 << BSP_491_RXBUF_N_POWER];
//
//static  u32         BSP_491_RxBufOutIx;


/// Private consts --------------------------------------------------------------------------------------

static  const   IO_ID_TypeDef   io_id_pro_o[BSP_DEVICE_ID_N]    =   {PE13_PRO_O_ODD, PE05_PRO_O_EVEN};
static  const   IO_ID_TypeDef   io_id_clk_o[BSP_DEVICE_ID_N]    =   {PE03_CLK_O_EVEN,PE15_CLK_O_ODD};
static  const   IO_ID_TypeDef   io_id_dir_o[BSP_DEVICE_ID_N]    =   {PE04_DIR_O_EVEN,PE12_DIR_O_ODD};
static  const   IO_ID_TypeDef   io_id_tq1_o[BSP_DEVICE_ID_N]    =   {PE01_TQ1_O_EVEN,PB12_TQ1_O_ODD};
static  const   IO_ID_TypeDef   io_id_m1_o[BSP_DEVICE_ID_N]    =    {PD12_DIR_C_EVEN,PD08_RESET_O};
#ifdef  PC924
static  const   IO_ID_TypeDef   io_id_pro_c[BSP_DEVICE_ID_N]    =   {PB15_PRO_C_ODD, PD13_PRO_C_EVEN};
static  const   IO_ID_TypeDef   io_id_clk_c[BSP_DEVICE_ID_N]    =   {PD09_CLK_C_ODD, PD15_CLK_C_EVEN};
static  const   IO_ID_TypeDef   io_id_dir_c[BSP_DEVICE_ID_N]    =   {PB14_DIR_C_ODD, PD12_DIR_C_EVEN};
static  const   IO_ID_TypeDef   io_id_tq1_c[BSP_DEVICE_ID_N]    =   {PD10_TQ1_C_ODD, PC06_TQ1_C_EVEN};
static  const   IO_ID_TypeDef   io_id_m1_c[BSP_DEVICE_ID_N]     =   {PE14_M1_C_ODD,  PE00_M1_C_EVEN};
static  const   IO_ID_TypeDef   io_id_rst_c[BSP_DEVICE_ID_N]    =   {PD08_RESET_C_ODD, PD14_RESET_C_EVEN};
#endif

static          TIM_TypeDef*    tim_id_encoder[BSP_DEVICE_ID_N] =   {BSP_TMR_E_ODD, BSP_TMR_E_EVEN};


/// Private function prototypes ------------------------------------------------


static  void  BSP_EnPeriph              (void);
static  void  BSP_NVIC_Cfg              (void);

static  void  BSP_Judge924              (void);
static  void  BSP_DIP_Init              (void);

static  void  BSP_TM8M_Init             (void);
static  void  BSP_TM_CNT_Init           (void);
static  void  BSP_ENC_Init              (void);
static  void  BSP_ENC_IndexEn           (void);

static  void  BSP_LED_Init              (void);
static  void  BSP_HD_Init               (void);
static  void  BSP_MOTO_Init             (void);
#ifdef  PC924
static  void  BSP_MOTC_Init             (void);
#endif
static  void  BSP_SWI_Init              (void);
static  void  BSP_POT_Init              (void);
static  void  BSP_SQN_Init              (void);
static  void  BSP_SOL_Init              (void);
static  void  BSP_MOTO_Sitch_Init              (void);
static  void  BSP_MOTO_Step_Init (void);
static  u16                     BSP_TimerSol[BSP_DEVICE_ID_N];      /// ??????

/// Private consts --------------------------------------------------------------------------------------

static  const   IO_ID_TypeDef   io_id_sol[BSP_DEVICE_ID_N]      =   {PC08_SOL_ODD, PC09_SOL_EVEN};
static  const   IO_ID_TypeDef   io_id_qf[1]      =   {PA08_SOL0};






///// -----------------------------------------------------------------------------------------------------
///// @brief wait for a time (usually less than 1ms, otherwise by os)
///// @param time : 1000 ��Ӧ 125us @72Hz
///// @retval : None
///// -----------------------------------------------------------------------------------------------------
//static  void  BSP_Delay (u32  time)
//{
//    volatile  u32  i;
//    
//    
//    for (i = time; i != 0; i--) {
//        ;
//    }
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief  Enable the Peripherals
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_EnPeriph (void)
{
    /// Enable peripheral clocks
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1
                         | RCC_AHBPeriph_DMA2                   /// Ŀǰû��ʹ��
                         | RCC_AHBPeriph_CRC,
                         ENABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2
                         | RCC_APB1Periph_TIM3
                         | RCC_APB1Periph_TIM4
                         | RCC_APB1Periph_USART3                /// Ŀǰû��ʹ��
                         | RCC_APB1Periph_CAN1,                 /// Ŀǰû��ʹ��
                         ENABLE);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO
                         | RCC_APB2Periph_GPIOA
                         | RCC_APB2Periph_GPIOB
                         | RCC_APB2Periph_GPIOC
                         | RCC_APB2Periph_GPIOD
                         | RCC_APB2Periph_GPIOE
                         | RCC_APB2Periph_ADC1
                         | RCC_APB2Periph_ADC2
                         | RCC_APB2Periph_TIM1
                         | RCC_APB2Periph_USART1
                         | RCC_APB2Periph_ADC3,                 /// V8 �� VB �� ADC3
                         ENABLE);
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  Configures the nested vectored interrupt controller.
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_NVIC_Cfg (void)
{
    NVIC_InitTypeDef    sNVICInit;
    
    
    /// Configure the NVIC Preemption Priority Bits. ��ʱ���ò���ռ��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    
    /// Enable the BSP_USART_491 Interrupt
    sNVICInit.NVIC_IRQChannel               =   USART1_IRQn;
    sNVICInit.NVIC_IRQChannelSubPriority    =   8;
    sNVICInit.NVIC_IRQChannelCmd            =   ENABLE;
    NVIC_Init(&sNVICInit);
    
    /// ��ʱ
    sNVICInit.NVIC_IRQChannel               =   TIM1_CC_IRQn;
    sNVICInit.NVIC_IRQChannelSubPriority    =   4;
    sNVICInit.NVIC_IRQChannelCmd            =   ENABLE;
    NVIC_Init(&sNVICInit);
    
/// �������924, ���ش������ж�, ��������
#ifdef  PC924                               
    /// ģ�����
    sNVICInit.NVIC_IRQChannel               =   TIM3_IRQn;
    sNVICInit.NVIC_IRQChannelSubPriority    =   2;
    sNVICInit.NVIC_IRQChannelCmd            =   ENABLE;
    NVIC_Init(&sNVICInit);
    
    /// the irq of encoder index gpio
    sNVICInit.NVIC_IRQChannel               =   EXTI2_IRQn;
    sNVICInit.NVIC_IRQChannelSubPriority    =   1;
    sNVICInit.NVIC_IRQChannelCmd            =   ENABLE;
    NVIC_Init(&sNVICInit);
    sNVICInit.NVIC_IRQChannel               =   EXTI9_5_IRQn;
    sNVICInit.NVIC_IRQChannelSubPriority    =   1;
    sNVICInit.NVIC_IRQChannelCmd            =   ENABLE;
    NVIC_Init(&sNVICInit);
    
//    /// the timer of encoder
//    sNVICInit.NVIC_IRQChannel               =   TIM2_IRQn;
//    sNVICInit.NVIC_IRQChannelSubPriority    =   0;
//    sNVICInit.NVIC_IRQChannelCmd            =   ENABLE;
//    NVIC_Init(&sNVICInit);
//    sNVICInit.NVIC_IRQChannel               =   TIM4_IRQn;
//    sNVICInit.NVIC_IRQChannelSubPriority    =   0;
//    sNVICInit.NVIC_IRQChannelCmd            =   ENABLE;
//    NVIC_Init(&sNVICInit);
#endif

}


//static  void  BSP_US_Init (void)
static  void  BSP_TM8M_Init (void)
{
    TIM_TimeBaseInitTypeDef     sTimeBaseInit;
    
    
    /// Time Base configuration
    sTimeBaseInit.TIM_Prescaler         =   SystemFrequency / BSP_TMR_8M_CLK - 1;   /// clock = 8MHz
    sTimeBaseInit.TIM_CounterMode       =   TIM_CounterMode_Up;
    sTimeBaseInit.TIM_Period            =   (u16)(-1);                              /// MAX_U16
    sTimeBaseInit.TIM_ClockDivision     =   TIM_CKD_DIV1;
    sTimeBaseInit.TIM_RepetitionCounter =   0;
    TIM_TimeBaseInit(BSP_TMR_8M, &sTimeBaseInit);

    /// TIM1 counter enable
    TIM_Cmd(BSP_TMR_8M, ENABLE);
}

//u16  BSP_US_Get (void)
u16  BSP_TM8M_Get (void)
{
    return  BSP_TMR_8M->CNT;
}

///// !!! ��������
////void  BSP_US_WaitUntil (u16 time)
//void  BSP_TM8M_WaitUntil (u16 time)
//{
//    TIM_OCInitTypeDef   sOCInit;
//    
//    
//    TIM_OCStructInit(&sOCInit);
//    sOCInit.TIM_Pulse   =   time;
//    TIM_ClearFlag(BSP_TMR_8M, BSP_FLAG_CC_TIME_OUT);
//    BSP_OC_TIME_OUT_INIT_FUNC(BSP_TMR_8M, &sOCInit);
//    while (TIM_GetFlagStatus(BSP_TMR_8M, BSP_FLAG_CC_TIME_OUT) == RESET) {
//        ;
//    }
//    /// ����־�������־
//}
//
///// !!! ��������
////void  BSP_US_Delay (u16 us)
//void  BSP_TM8M_Delay (u16 time)
//{
//    if (time < 2) {
//        time = 2;
//    }
//    BSP_TM8M_WaitUntil((BSP_TMR_8M->CNT) + time);
//}

void  BSP_TM8M_Delay (u16 time)
{
    u16     start;
    u16     aim;
    u16     cnt;
    
    
    start   =   BSP_TMR_8M->CNT;
    if (time < 2) {
        time = 2;
    }
    aim     =   start + time;
    
    if (aim > start) {
        do {
            cnt =   BSP_TMR_8M->CNT;
        } while ((cnt >= start) && (cnt < aim));
    } else {                                        /// ���������, ֻ���� aim < start
        do {
            cnt =   BSP_TMR_8M->CNT;
        } while ((cnt < aim) || (cnt >= start));
    }
}


static  void  BSP_TM_CNT_Init (void)
{
    u32                         i;
    u32                         m_d10;
    u16                         spd_d10;
    TIM_TimeBaseInitTypeDef     sTimeBaseInit;
    
    
    for (i = 0; i < BSP_SPD_NUM; i++) {
        m_d10                       =   SystemFrequency / BSP_COUNTER_PR * 60 / 10;
        spd_d10                     =   i + BSP_SPD_D10_MIN;
        BSP_TMR_CNT_PRESCALER[i]    =   (m_d10 * 2 + spd_d10) / (spd_d10 * 2) - 1;
    }
    
    /// Time Base configuration
    sTimeBaseInit.TIM_Prescaler         =   BSP_TMR_CNT_PRESCALER[0];               /// clock = SystemFrequency
    sTimeBaseInit.TIM_CounterMode       =   TIM_CounterMode_Up;
    sTimeBaseInit.TIM_Period            =   (u16)(-1);                              /// MAX_U16
    sTimeBaseInit.TIM_ClockDivision     =   TIM_CKD_DIV1;
    sTimeBaseInit.TIM_RepetitionCounter =   0;
    TIM_TimeBaseInit(BSP_TMR_CNT, &sTimeBaseInit);
    
    /// TIM3 counter enable
    TIM_Cmd(BSP_TMR_CNT, ENABLE);
}

void  BSP_TM_CNT_SpdSet (u16 spd_d10)
{
    if ((BSP_SPD_D10_MIN <= spd_d10) && (spd_d10 <= BSP_SPD_D10_MAX)) {
        TIM_PrescalerConfig(BSP_TMR_CNT,
                            BSP_TMR_CNT_PRESCALER[spd_d10 - BSP_SPD_D10_MIN],
                            TIM_PSCReloadMode_Immediate);
    }
}

u16  BSP_TM_CNT_Get (void)
{
    return  BSP_TMR_CNT->CNT;
}

//void  BSP_TM_MC_Set (u16 cnt)
//{
//    BSP_TMR_8M->CNT =   cnt;
//}

///// -----------------------------------------------------------------------------------------------------
///// @brief  Reset the RCC clock configuration to the default reset state
/////         (for debug purpose)
///// @param  None
///// @retval : None
///// -----------------------------------------------------------------------------------------------------
//static  void  BSP_RCC_Reset (void)
//{
//    /// Set HSION bit
//    RCC->CR    |=   (uint32_t)0x00000001;
//    
//    /// Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits
//#ifndef STM32F10X_CL
//    RCC->CFGR  &=   (uint32_t)0xF8FF0000;
//#else
//    RCC->CFGR  &=   (uint32_t)0xF0FF0000;
//#endif
//    
//    /// Reset HSEON, CSSON and PLLON bits
//    RCC->CR    &=   (uint32_t)0xFEF6FFFF;
//    
//    /// Reset HSEBYP bit
//    RCC->CR    &=   (uint32_t)0xFFFBFFFF;
//    
//    /// Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits
//    RCC->CFGR  &=   (uint32_t)0xFF80FFFF;
//    
//#ifdef STM32F10X_CL
//    /// Reset PLL2ON and PLL3ON bits
//    RCC->CR &= (uint32_t)0xEBFFFFFF;
//    
//    /// Disable all interrupts and clear pending bits
//    RCC->CIR = 0x00FF0000;
//    
//    /// Reset CFGR2 register
//    RCC->CFGR2 = 0x00000000;
//#elif defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL)
//    /// Disable all interrupts and clear pending bits
//    RCC->CIR = 0x009F0000;
//    
//    /// Reset CFGR2 register
//    RCC->CFGR2 = 0x00000000;
//#else
//    /// Disable all interrupts and clear pending bits
//    RCC->CIR = 0x009F0000;
//#endif
//    
//#if defined (STM32F10X_HD) || (defined STM32F10X_XL)
//    #ifdef DATA_IN_ExtSRAM
//    SystemInit_ExtMemCtl(); 
//    #endif
//#endif 
//}
//
///// -----------------------------------------------------------------------------------------------------
///// @brief  Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers
/////         Configure the Flash Latency cycles and enable prefetch buffer
///// @param  None
///// @retval : None
///// -----------------------------------------------------------------------------------------------------
//static  void  SetSysClock (void)
//{
//    
//}


/// -----------------------------------------------------------------------------------------------------
/// @brief  �ж��Ƿ�Ϊ924��
///         ͨ�� motor �� pro �����ж�. ��¼������ BSP_Is924 ��.
///         ������Ϊ��������. �������һ���ջ����pro����Ϊ��, ����Ϊ��924. Ȼ��ָ�Ϊ����.
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_Judge924 (void)
{
#ifdef  PC924
    u32     i;
#endif
    
    
    BSP_Is924   =   FALSE;
    
#ifdef  PC924
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        IO_Init(io_id_pro_c[i], GPIO_Mode_IPD);
        if (IO_InGet(io_id_pro_c[i]) != 0) {
            BSP_Is924   =   TRUE;
        }
        IO_Init(io_id_pro_c[i], GPIO_Mode_IPU);
    }
#endif
}

///// -----------------------------------------------------------------------------------------------------
///// @brief  �����Ƿ�Ϊ924��
///// @param  None
///// @retval : �Ƿ�
///// -----------------------------------------------------------------------------------------------------
//bool  BSP_BoardIs924 (void)
//{
//    return  BSP_Is924;
//}


/// -----------------------------------------------------------------------------------------------------
/// @brief Initialize the Board Support Package (BSP).
///        This function SHOULD be called before any other BSP function is called.
///        ��ʼ���弶֧�ְ�
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_Init (void)
{
    /// !!! ����˳��Ҫ����ı� !!!
    BSP_EnPeriph();
    BSP_NVIC_Cfg();
    
    BSP_TM8M_Init();
    BSP_Judge924();
    BSP_DIP_Init();
    /// !!! ����˳��Ҫ����ı� !!!
    
    if (BSP_Is924) {
        BSP_TM_CNT_Init();
        BSP_ENC_Init();
        BSP_ENC_IndexEn();
#ifdef  PC924
        BSP_MOTC_Init();
#endif
    }
    
    BSP_LED_Init();
    BSP_HD_Init();
	BSP_SOL_Init();
    BSP_MOTO_Init();

    BSP_SWI_Init();
    BSP_POT_Init();
//    BSP_491_Init();
    BSP_SQN_Init();
}


/// -------- HMI --------

/// -------- led --------

static  const  IO_ID_TypeDef  io_id_led[BSP_LED_ID_N] =   {PB05_LED1, PB09_LED2};

/// -----------------------------------------------------------------------------------------------------
/// @brief Initialize the I/O for the leds. ָʾ�Ƴ�ʼ��Ϊ����(50Hz)�������
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_LED_Init (void)
{
    u32     i;
    
    
    for (i = 0; i < BSP_LED_ID_N; i++) {
        IO_Init(io_id_led[i], GPIO_Mode_Out_OD);
        BSP_LED_Set(i, BSP_LED_OFF);
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set any or all the LEDs on the board. ʹָ����ָʾ��������, ���Ե���
/// @param  led : The ID of the LED to control
///              0    LED 1
///              1    LED 2
///         st  : ָʾ��״̬
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_LED_Set (u8 led, BSP_LED_TypeDef st)
{
//    switch (led) {
//    case 0:
//        BSP_LED_Set(1, st);
//        BSP_LED_Set(2, st);
//        break;
//        
//    case 1:
//        IO_OutSet(PB05_LED1, (u32)st);
//        break;
//        
//    case 2:
//        IO_OutSet(PB09_LED2, (u32)st);
//        break;
//        
//    default:
//        break;
//    }
    IO_OutSet(io_id_led[led % BSP_LED_ID_N],(u32)st);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief TOGGLE any or all the LEDs on the board. ʹָ����ָʾ�Ʒ�ת, ���Ե���
/// @param led : The ID of the LED to control
///              0    LED 1
///              1    LED 2
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_LED_Toggle (u8 led)
{
//    switch (led) {
//    case 0:
//        BSP_LED_Toggle(1);
//        BSP_LED_Toggle(2);
//        break;
//        
//    case 1:
//        IO_OutToggle(PB05_LED1);
//        break;
//        
//    case 2:
//        IO_OutToggle(PB09_LED2);
//        break;
//        
//    default:
//        break;
//    }
    IO_OutToggle(io_id_led[led % BSP_LED_ID_N]);
}


/// -------- Head Board --------

static  const  IO_ID_TypeDef  io_id_hd_up[BSP_DEVICE_ID_N]  =   {PE08_HD_UP_ODD  , PC10_HD_UP_EVEN};
static  const  IO_ID_TypeDef  io_id_hd_dn[BSP_DEVICE_ID_N]  =   {PE10_HD_DOWN_ODD, PD00_HD_DOWN_EVEN};
static  const  IO_ID_TypeDef  io_id_hd_r[BSP_DEVICE_ID_N]   =   {PE09_HD_R_ODD   , PC11_HD_R_EVEN};
static  const  IO_ID_TypeDef  io_id_hd_g[BSP_DEVICE_ID_N]   =   {PE11_HD_G_ODD   , PC12_HD_G_EVEN};

/// -----------------------------------------------------------------------------------------------------
/// @brief Initialize the I/O for the head board.
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_HD_Init (void)
{    
//    /// ��żͷ����IO��ʼ��
//    IO_Init(PE08_HD_UP_ODD   , GPIO_Mode_IPU);
//    IO_Init(PE10_HD_DOWN_ODD , GPIO_Mode_IPU);
//    IO_Init(PC10_HD_UP_EVEN  , GPIO_Mode_IPU);
//    IO_Init(PD00_HD_DOWN_EVEN, GPIO_Mode_IPU);
//    
//    /// ��żͷ���̵�IO��ʼ��
//    IO_Init(PE09_HD_R_ODD , GPIO_Mode_Out_PP);
//    IO_Init(PE11_HD_G_ODD , GPIO_Mode_Out_PP);
//    IO_Init(PC11_HD_R_EVEN, GPIO_Mode_Out_PP);
//    IO_Init(PC12_HD_G_EVEN, GPIO_Mode_Out_PP);
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        /// ����IO��ʼ��
        IO_Init(io_id_hd_up[i], GPIO_Mode_IPU);
        IO_Init(io_id_hd_dn[i], GPIO_Mode_IPU);
        /// ���̵�IO��ʼ��
        IO_Init(io_id_hd_r[i], GPIO_Mode_Out_PP);
        IO_Init(io_id_hd_g[i], GPIO_Mode_Out_PP);
        BSP_HD_LedSet((BSP_ID_TypeDef)i, BSP_LED_OFF, BSP_LED_OFF);
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Get the Inputs for the keys (switch on head board).
/// @param  id   : The ID of key (the odd or even)
///         up   : save the status of the up   switch (output)
///         down : save the status of the down switch (output)
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_HD_KeyGet (BSP_ID_TypeDef id, BSP_HD_KEY_TypeDef *up, BSP_HD_KEY_TypeDef *down)
{
//    switch (id) {
//    case BSP_ID_ODD:
//        *up     =   (BSP_HD_KEY_TypeDef)IO_InGet(PE08_HD_UP_ODD  );
//        *down   =   (BSP_HD_KEY_TypeDef)IO_InGet(PE10_HD_DOWN_ODD);
//        break;
//        
//    case BSP_ID_EVEN:
//    default:                /// continue
//        *up     =   (BSP_HD_KEY_TypeDef)IO_InGet(PC10_HD_UP_EVEN  );
//        *down   =   (BSP_HD_KEY_TypeDef)IO_InGet(PD00_HD_DOWN_EVEN);
//        break;
//    }
    *up     =   (BSP_HD_KEY_TypeDef)IO_InGet(io_id_hd_up[id]);
    *down   =   (BSP_HD_KEY_TypeDef)IO_InGet(io_id_hd_dn[id]);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set the Outputs for the lens (on head board).
/// @param  id    : The ID of led (the odd or even)
///         red   : the status of the red   led
///         green : the status of the green led
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_HD_LedSet (BSP_ID_TypeDef id, BSP_LED_TypeDef red, BSP_LED_TypeDef green)
{
//    switch (id) {
////    case BSP_ID_ALL:
////        BSP_HD_LedSet(BSP_ID_ODD,  red, green);
////        BSP_HD_LedSet(BSP_ID_EVEN, red, green);
////        break;
//        
//    case BSP_ID_ODD:
//        IO_OutSet(PE09_HD_R_ODD, (u32)red);
//        IO_OutSet(PE11_HD_G_ODD, (u32)green);
//        break;
//        
//    case BSP_ID_EVEN:
//        IO_OutSet(PC11_HD_R_EVEN, (u32)red);
//        IO_OutSet(PC12_HD_G_EVEN, (u32)green);
//        break;
//        
//    default:
//        break;
//    }
    IO_OutSet(io_id_hd_r[id], (u32)red);
    IO_OutSet(io_id_hd_g[id], (u32)green);
}


/// -------- dip --------

/// -----------------------------------------------------------------------------------------------------
/// @brief Initialize the Inputs for the DIPs.
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_DIP_Init (void)
{
    u32     i;
    u32     dip;
    u8     Addr_Tmp,Addr_Tmp2;
    
    IO_MultiInit(PC00_DIP1, 6, GPIO_Mode_IPU);          /// DIP1~4, DIP9~10
    IO_MultiInit(PA04_DIP5, 4, GPIO_Mode_IPU);          /// DIP5~8
    
    for (i = 100; i != 0; i--) {                        /// max scan times is 100
        dip                 =   BSP_DIP_Get();
        if ((dip >> 3) & 0x01) 
		{
            BSP_DIP_RedIsSpkl   =   FALSE;
        } 
		else 
		{
            BSP_DIP_RedIsSpkl   =   TRUE;
        } 
        BSP_DIP_TQ          =   (BSP_MOT_TQ_TypeDef)(dip & 0x03);
        BSP_DIP_BDFunc      =   (BSP_DIP_BD_FUNC_TypeDef)((dip >> 4) & 0x01);
        Addr_Tmp			=   (dip >> 5) & 0x1;//����6��7��8��9��10������6�����λ��Ӧ�ñ�����λ
		if(Addr_Tmp==1)
		{
			Addr_Tmp2=Addr_Tmp<<4;
			Addr_Tmp=Addr_Tmp2+((dip >> 6) & 0x0F);
		switch(Addr_Tmp)
		{
		/*	case 0x00:
				 BSP_DIP_9511Addr =  0x4F;
			break; */
			case 0x10:
				BSP_DIP_9511Addr =   0x71;
			break;
			case 0x11:
				 BSP_DIP_9511Addr =   0x72;
			break;
			case 0x12:
				BSP_DIP_9511Addr =   0x73;
			break;
			case 0x13:
				BSP_DIP_9511Addr =   0x74;
			break;
			case 0x14:
				 BSP_DIP_9511Addr =   0x75;
			break;
			case 0x15:
				BSP_DIP_9511Addr =   0x76;
			break;
			case 0x16:
				BSP_DIP_9511Addr =   0x77;
			break;
			case 0x17:
				 BSP_DIP_9511Addr =   0x78;
			break;
			case 0x18:
				BSP_DIP_9511Addr =   0x79;
			break;
			case 0x19:
				BSP_DIP_9511Addr =   0x7A;
			break;
			case 0x1A:
				 BSP_DIP_9511Addr =   0x7B;
			break;
			case 0x1B:
				BSP_DIP_9511Addr =   0x7C;
			break;
			case 0x1C:
				BSP_DIP_9511Addr =   0x7D;
			break;
			case 0x1D:
				 BSP_DIP_9511Addr =   0x7E;
			break;
			case 0x1E:
				BSP_DIP_9511Addr =   0x7F;
			break;
		/*	case 0x1F:
			case 0:
				while(1);
			break; */
           	default:
		       BSP_DIP_9511Addr	 =0x2F;//	 Addr_Tmp;
			break;
		}
		}
		else
		{
			BSP_DIP_9511Addr    =   (dip >> 6) & 0x0F;
		}

        BSP_TM8M_Delay(BSP_TMR_8M_CLK / 1000);          /// delay 1/1000S
        if (dip == BSP_DIP_Get()) {
            break;
        }
    }
}

///// -----------------------------------------------------------------------------------------------------
///// @brief Get the Inputs for the DIP0~10.
///// @param None
///// @retval : DIP0~10, 10bits, 0 - ON, 1 - OFF
///// -----------------------------------------------------------------------------------------------------
//u32  BSP_DIP_Get (void)
//{
//    u32     tmp;
//    
//    
//    tmp     =   IO_MultiInGet(PC00_DIP1, 4);
//    tmp    |=   IO_MultiInGet(PA04_DIP5, 4) << 4;
//    tmp    |=   IO_MultiInGet(PC04_DIP9, 2) << 8;
//    
//    return  tmp;
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief Get the Inputs for the DIP0~10.
/// @param None
/// @retval : DIP0~10, 10bits, 0 - ON, 1 - OFF
/// -----------------------------------------------------------------------------------------------------
u32  BSP_DIP_Get (void)
{
    u32     tmp;
    
    
    tmp     =   IO_MultiInGet(PC00_DIP1, 4);
    tmp    |=   IO_MultiInGet(PA04_DIP5, 4) << 4;
    tmp    |=   IO_MultiInGet(PC04_DIP9, 2) << 8;
    
    return  tmp & ((1 << BSP_DIP_N_BITS) - 1);
}

///// -----------------------------------------------------------------------------------------------------
///// @brief   get the open loop motors' TQ (current) value.
/////          �����ϵ�ʱDIP0,1��״̬,�õ�����������������趨ֵ.
///// @param  None
///// @retval : �趨�ĵ���
/////         3 -  20%    DIP1 OFF, DIP2 OFF
/////         2 -  50%    DIP1 ON , DIP2 OFF
/////         1 -  75%    DIP1 OFF, DIP2 ON 
/////         0 - 100%    DIP1 ON , DIP2 ON 
///// -----------------------------------------------------------------------------------------------------
//BSP_MOT_TQ_TypeDef  BSP_DIP_GetTQ (void)
//{
//    return  BSP_DIP_TQ;
//}
//
///// -----------------------------------------------------------------------------------------------------
///// @brief  get the board's function.
/////         �����ϵ�ʱDIP5��״̬,�õ��幦��.
///// @param  None
///// @retval : �幦��
/////         0 - BSP_DIP_BD_FUNC_UP �ϻ�ͷ��
/////         1 - BSP_DIP_BD_FUNC_DN �»�ͷ��
///// -----------------------------------------------------------------------------------------------------
//BSP_DIP_BD_FUNC_TypeDef  BSP_DIP_GetBDFunc (void)
//{
//    return  BSP_DIP_BDFunc;
//}
//
///// -----------------------------------------------------------------------------------------------------
///// @brief  Get the 9511 board address.
/////         �����ϵ�ʱDIP7,8,9,10��״̬,�õ����ַ. 0 ��Ϊ 16. ������9511�ı���ϰ��.
///// @param  None
///// @retval : from 1 to 16
///// -----------------------------------------------------------------------------------------------------
//u8  BSP_DIP_Get9511Addr (void)
//{
//    return  BSP_DIP_9511Addr;
//}


/// -------- EXECUTOR --------

/// -------- solenoid --------

/// ����


/// -------- motor (controled by open loop)  --------

/// -----------------------------------------------------------------------------------------------------
/// @brief Initialize the I/O for the motors controled by open loop.
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_MOTO_Init (void)
{
//    /// ����
//    IO_Init(PE13_PRO_O_ODD , GPIO_Mode_IPU);
//    IO_Init(PE05_PRO_O_EVEN, GPIO_Mode_IPU);
//    /// ����
//    IO_Init(PE15_CLK_O_ODD , GPIO_Mode_Out_PP);
//    IO_Init(PE03_CLK_O_EVEN, GPIO_Mode_Out_PP);
//    /// ����
//    IO_Init(PE12_DIR_O_ODD , GPIO_Mode_Out_PP);
//    IO_Init(PE04_DIR_O_EVEN, GPIO_Mode_Out_PP);
//    /// ����
//    IO_MultiInit(PB12_TQ1_O_ODD , 2, GPIO_Mode_Out_PP);
//    IO_MultiInit(PE01_TQ1_O_EVEN, 2, GPIO_Mode_Out_PP);
    u32     i;
    
    
#if HARDWARE_VER == 1
   // IO_Init(PD08_RESET_O , GPIO_Mode_Out_PP);
    IO_Init(PE14_ENABLE_O, GPIO_Mode_Out_PP);
  //  IO_OutSet(PD08_RESET_O, 1);
#endif
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        IO_Init(io_id_pro_o[i], GPIO_Mode_IPU);
        IO_Init(io_id_clk_o[i], GPIO_Mode_Out_PP);
        BSP_MOTO_ClkSet((BSP_ID_TypeDef)i, Bit_RESET);
        IO_Init(io_id_dir_o[i], GPIO_Mode_Out_PP);

	    IO_MultiInit(io_id_tq1_o[i], 2, GPIO_Mode_Out_PP);
		BSP_MOTO_TQSet((BSP_ID_TypeDef)i, BSP_MOT_TQ_50);
		IO_MultiInit(io_id_m1_o[i], 3, GPIO_Mode_Out_PP);
		BSP_MOTO_MSet((BSP_ID_TypeDef)i, BSP_MOT_M_5);



    }
    
#if HARDWARE_VER == 1
    BSP_TM8M_Delay(BSP_TMR_8M_CLK * 1 / 1000000);       /// delay 2/1000000S (2us)
    IO_OutSet(PE14_ENABLE_O, 1);
#endif
}
static  void  BSP_MOTO_Sitch_Init (void)
{
   u32     i;
   for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        IO_MultiInit(io_id_tq1_o[i], 2, GPIO_Mode_Out_PP);
       // BSP_MOTO_TQSet((BSP_ID_TypeDef)i, BSP_MOT_TQ_20);//100
		BSP_MOTO_TQSet((BSP_ID_TypeDef)i, BSP_MOT_TQ_50);//120
	//	BSP_MOTO_TQSet((BSP_ID_TypeDef)i, BSP_MOT_TQ_75);//330
	//	BSP_MOTO_TQSet((BSP_ID_TypeDef)i, BSP_MOT_TQ_100); //3300
    }
}

static  void  BSP_MOTO_Step_Init (void)
{
   u32     i;
   for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        IO_MultiInit(io_id_m1_o[i], 3, GPIO_Mode_Out_PP);
     //  BSP_MOTO_MSet((BSP_ID_TypeDef)i, BSP_MOT_M_0);
	//	BSP_MOTO_MSet((BSP_ID_TypeDef)i, BSP_MOT_M_1);
	//	BSP_MOTO_MSet((BSP_ID_TypeDef)i, BSP_MOT_M_2);
	//	BSP_MOTO_MSet((BSP_ID_TypeDef)i, BSP_MOT_M_3);
	//	BSP_MOTO_MSet((BSP_ID_TypeDef)i, BSP_MOT_M_4);
		BSP_MOTO_MSet((BSP_ID_TypeDef)i, BSP_MOT_M_5);
	//	BSP_MOTO_MSet((BSP_ID_TypeDef)i, BSP_MOT_M_6);
	//	BSP_MOTO_MSet((BSP_ID_TypeDef)i, BSP_MOT_M_7);	
    }
}
/// -----------------------------------------------------------------------------------------------------
/// @brief Get the status of the open_loop motor.
/// @param  id   : The ID of motor (the odd or even)
/// @retval : Status
/// -----------------------------------------------------------------------------------------------------
BSP_MOT_ST_TypeDef  BSP_MOTO_StatusGet (BSP_ID_TypeDef id)
{
//    switch (id) {
//    case BSP_ID_ODD:
//        return  (BSP_MOT_ST_TypeDef)IO_InGet(PE13_PRO_O_ODD);
//        
//    case BSP_ID_EVEN:
//    default:                /// continue
//        return  (BSP_MOT_ST_TypeDef)IO_InGet(PE05_PRO_O_EVEN);
//    }
    return  (BSP_MOT_ST_TypeDef)IO_InGet(io_id_pro_o[id]);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Get the Outputs for the clock signal of the open_loop motor.
/// @param  id    : The ID of motor (the odd or even)
/// @retval : the status of the clock signal
///         0 - Bit_RESET; 1 - Bit_SET.
/// -----------------------------------------------------------------------------------------------------
BitAction  BSP_MOTO_ClkGet (BSP_ID_TypeDef id)
{
    return  (BitAction)IO_OutGet(io_id_clk_o[id]);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set the Outputs for the clock signal of the open_loop motor.
/// @param  id    : The ID of motor (the odd or even)
///         bit   : the status of the clock signal
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_MOTO_ClkSet (BSP_ID_TypeDef id, BitAction bit)
{
//    switch (id) {
////    case BSP_ID_ALL:
////        BSP_MOTO_ClkSet(BSP_ID_ODD,  bit);
////        BSP_MOTO_ClkSet(BSP_ID_EVEN, bit);
////        break;
//        
//    case BSP_ID_ODD:
//        IO_OutSet(PE15_CLK_O_ODD , (u32)bit);
//        break;
//        
//    case BSP_ID_EVEN:
//        IO_OutSet(PE03_CLK_O_EVEN, (u32)bit);
//        break;
//        
//    default:
//        break;
//    }
    IO_OutSet(io_id_clk_o[id], (u32)bit);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set the Outputs for the direction signal of the open_loop motor.
/// @param  id    : The ID of motor (the odd or even)
///         dir   : the status of the direction signal
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_MOTO_DirSet (BSP_ID_TypeDef id, BSP_MOT_DIR_TypeDef dir)
{
//    switch (id) {        
////    case BSP_ID_ALL:
////        BSP_MOTO_DirSet(BSP_ID_ODD,  dir);
////        BSP_MOTO_DirSet(BSP_ID_EVEN, dir);
////        break;
//        
//    case BSP_ID_ODD:
//        IO_OutSet(PE12_DIR_O_ODD , (u32)dir);
//        break;
//        
//    case BSP_ID_EVEN:
//        IO_OutSet(PE04_DIR_O_EVEN, (u32)dir);
//        break;
//
//    default:
//        break;
//    }
    IO_OutSet(io_id_dir_o[id], (u32)dir);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set the Outputs for the current signals of the open_loop motor.
/// @param  id    : The ID of motor (the odd or even)
///         tq    : the status of the current signal
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_MOTO_TQSet (BSP_ID_TypeDef id, BSP_MOT_TQ_TypeDef tq)
{
//    switch (id) {        
////    case BSP_ID_ALL:
////        BSP_MOTO_TQSet(BSP_ID_ODD,  tq);
////        BSP_MOTO_TQSet(BSP_ID_EVEN, tq);
////        break;
//        
//    case BSP_ID_ODD:
//        IO_MultiOutSet(PB12_TQ1_O_ODD , 2, (u32)tq);
//        break;
//        
//    case BSP_ID_EVEN:
//        IO_MultiOutSet(PE01_TQ1_O_EVEN, 2, (u32)tq);
//        break;
//
//    default:
//        break;
//    }
    IO_MultiOutSet(io_id_tq1_o[id], 2, (u32)tq);
}
void  BSP_MOTO_MSet (BSP_ID_TypeDef id, BSP_MOT_M123_TypeDef tq)
{

    IO_MultiOutSet(io_id_m1_o[id], 3, (u32)tq);
}

#ifdef  PC924

/// -------- motor (controled by closed loop) --------

/// -----------------------------------------------------------------------------------------------------
/// @brief Initialize the I/O for the motors controled by closed loop.
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_MOTC_Init (void)
{
//    /// ����
//    IO_Init(PB15_PRO_C_ODD , GPIO_Mode_IPU);
//    IO_Init(PD13_PRO_C_EVEN, GPIO_Mode_IPU);
//    /// ����
//    IO_Init(PD09_CLK_C_ODD , GPIO_Mode_Out_PP);
//    IO_Init(PD15_CLK_C_EVEN, GPIO_Mode_Out_PP);
//    /// ����
//    IO_Init(PB14_DIR_C_ODD , GPIO_Mode_Out_PP);
//    IO_Init(PD12_DIR_C_EVEN, GPIO_Mode_Out_PP);
//    /// ����
//    IO_MultiInit(PD10_TQ1_C_ODD , 2, GPIO_Mode_Out_PP);
//    IO_MultiInit(PC06_TQ1_C_EVEN, 2, GPIO_Mode_Out_PP);
//    /// ����
//    IO_Init(PE14_M1_C_ODD  , GPIO_Mode_Out_PP);
//    IO_Init(PE00_M1_C_EVEN , GPIO_Mode_Out_PP);
//    /// ��λ
//    IO_Init(PD08_RESET_C_ODD , GPIO_Mode_Out_PP);
//    IO_Init(PD14_RESET_C_EVEN, GPIO_Mode_Out_PP);
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        IO_Init(io_id_pro_c[i], GPIO_Mode_IPU);
        IO_Init(io_id_clk_c[i], GPIO_Mode_Out_PP);
        BSP_MOTC_ClkSet((BSP_ID_TypeDef)i, Bit_RESET);
        IO_Init(io_id_dir_c[i], GPIO_Mode_Out_PP);
        IO_MultiInit(io_id_tq1_c[i], 2, GPIO_Mode_Out_PP);
        BSP_MOTC_TQSet((BSP_ID_TypeDef)i, BSP_MOT_TQ_20);
        IO_Init(io_id_m1_c[i] , GPIO_Mode_Out_PP);
        BSP_MOTC_MSet((BSP_ID_TypeDef)i, BSP_MOT_M_P225);
        IO_Init(io_id_rst_c[i], GPIO_Mode_Out_PP);
        BSP_MOTC_STSet((BSP_ID_TypeDef)i, BSP_MOT_IST_NORMAL);
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Get the status of the closed_loop motor.
/// @param  id   : The ID of motor (the odd or even)
/// @retval : Status
/// -----------------------------------------------------------------------------------------------------
BSP_MOT_ST_TypeDef  BSP_MOTC_StatusGet (BSP_ID_TypeDef id)
{
//    switch (id) {
//    case BSP_ID_ODD:
//        return  (BSP_MOT_ST_TypeDef)IO_InGet(PB15_PRO_C_ODD);
//        
//    case BSP_ID_EVEN:
//    default:                /// continue
//        return  (BSP_MOT_ST_TypeDef)IO_InGet(PD13_PRO_C_EVEN);
//    }
    return  (BSP_MOT_ST_TypeDef)IO_InGet(io_id_pro_c[id]);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Get the Outputs for the clock signal of the closed_loop motor.
/// @param  id    : The ID of motor (the odd or even)
/// @retval : the status of the clock signal
///         0 - Bit_RESET; 1 - Bit_SET.
/// -----------------------------------------------------------------------------------------------------
BitAction  BSP_MOTC_ClkGet (BSP_ID_TypeDef id)
{
    return  (BitAction)IO_OutGet(io_id_clk_c[id]);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set the Outputs for the clock signal of the closed_loop motor.
/// @param  id    : The ID of motor (the odd or even)
///         bit   : the status of the clock signal
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_MOTC_ClkSet (BSP_ID_TypeDef id, BitAction bit)
{
//    switch (id) {        
////    case BSP_ID_ALL:
////        BSP_MOTC_ClkSet(BSP_ID_ODD,  bit);
////        BSP_MOTC_ClkSet(BSP_ID_EVEN, bit);
////        break;
//        
//    case BSP_ID_ODD:
//        IO_OutSet(PD09_CLK_C_ODD , (u32)bit);
//        break;
//        
//    case BSP_ID_EVEN:
//        IO_OutSet(PD15_CLK_C_EVEN, (u32)bit);
//        break;
//
//    default:
//        break;
//    }
    IO_OutSet(io_id_clk_c[id], (u32)bit);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Get the Outputs for the direction signal of the closed_loop motor.
/// @param  id    : The ID of motor (the odd or even)
/// @retval : the status of the direction signal
/// -----------------------------------------------------------------------------------------------------
BSP_MOT_DIR_TypeDef  BSP_MOTC_DirGet (BSP_ID_TypeDef id)
{
    return  (BSP_MOT_DIR_TypeDef)IO_OutGet(io_id_dir_c[id]);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set the Outputs for the direction signal of the closed_loop motor.
/// @param  id    : The ID of motor (the odd or even)
///         dir   : the status of the direction signal
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_MOTC_DirSet (BSP_ID_TypeDef id, BSP_MOT_DIR_TypeDef dir)
{
//    switch (id) {        
////    case BSP_ID_ALL:
////        BSP_MOTC_DirSet(BSP_ID_ODD,  dir);
////        BSP_MOTC_DirSet(BSP_ID_EVEN, dir);
////        break;
//        
//    case BSP_ID_ODD:
//        IO_OutSet(PB14_DIR_C_ODD , (u32)dir);
//        break;
//        
//    case BSP_ID_EVEN:
//        IO_OutSet(PD12_DIR_C_EVEN, (u32)dir);
//        break;
//
//    default:
//        break;
//    }
    IO_OutSet(io_id_dir_c[id], (u32)dir);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set the Outputs for the current signals of the closed_loop motor.
/// @param  id    : The ID of motor (the odd or even)
///         tq    : the status of the current signal
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_MOTC_TQSet (BSP_ID_TypeDef id, BSP_MOT_TQ_TypeDef tq)
{
//    switch (id) {        
////    case BSP_ID_ALL:
////        BSP_MOTC_TQSet(BSP_ID_ODD,  tq);
////        BSP_MOTC_TQSet(BSP_ID_EVEN, tq);
////        break;
//        
//    case BSP_ID_ODD:
//        IO_MultiOutSet(PD10_TQ1_C_ODD , 2, (u32)tq);
//        break;
//        
//    case BSP_ID_EVEN:
//        IO_MultiOutSet(PC06_TQ1_C_EVEN, 2, (u32)tq);
//        break;
//
//    default:
//        break;
//    }
    IO_MultiOutSet(io_id_tq1_c[id], 2, (u32)tq);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set the mode of the closed_loop motor.
/// @param  id    : The ID of motor (the odd or even)
///         m     : The StepAccuracy of the closed_loop motor
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_MOTC_MSet (BSP_ID_TypeDef id, BSP_MOT_M_TypeDef m)
{
    IO_OutSet(io_id_m1_c[id], (u32)m);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set the status of the closed_loop motor.
/// @param  id    : The ID of motor (the odd or even)
///         st    : The status of the closed_loop motor (initialization or normal)
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_MOTC_STSet (BSP_ID_TypeDef id, BSP_MOT_IST_TypeDef ist)
{
    IO_OutSet(io_id_rst_c[id], (u32)ist);
}

#endif


/// -------- SENSOR --------

/// -------- encoder --------

/// -----------------------------------------------------------------------------------------------------
/// @brief Initialize the I/O and Timer for the encoder, 4X mode
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_ENC_Init (void)
{
    TIM_TimeBaseInitTypeDef     sTimeBaseInit;
    TIM_ICInitTypeDef           sTimeICInit;
    u32                         i;
    
    static  const  IO_ID_TypeDef  io_id_enc_a[BSP_DEVICE_ID_N]  =   {PA00_ENC_A_ODD, PB06_ENC_A_EVEN};
    static  const  IO_ID_TypeDef  io_id_enc_i[BSP_DEVICE_ID_N]  =   {PA02_ENC_I_ODD, PB08_ENC_I_EVEN};
    
    
    /// Timer configuration in Encoder mode
    /// Time Base configuration
    sTimeBaseInit.TIM_Prescaler     =   0;                      /// No prescaling
    sTimeBaseInit.TIM_CounterMode   =   TIM_CounterMode_Up;
    sTimeBaseInit.TIM_Period        =   BSP_ENC_N_T_PR - 1;
    sTimeBaseInit.TIM_ClockDivision =   TIM_CKD_DIV1;
//    TIM_TimeBaseInit(BSP_TMR_E_ODD , &sTimeBaseInit);
//    TIM_TimeBaseInit(BSP_TMR_E_EVEN, &sTimeBaseInit);
    
//    TIM_EncoderInterfaceConfig(BSP_TMR_E_ODD , TIM_EncoderMode_TI12,
//                               TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
//    TIM_EncoderInterfaceConfig(BSP_TMR_E_EVEN, TIM_EncoderMode_TI12,
//                               TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    
    TIM_ICStructInit(&sTimeICInit);
    /// ������������ٶ�Ϊ1200rpm, ��4��Ƶ���巭תʱ��Ϊ60*1000*1000/1200/1600=31.25us
    /// 6 ��ʾ: 4 * 6 / 72  =   0.33us�˲� @72M & TIM_CKD_DIV1
    /// 9 ��ʾ: 8 * 8 / 72  =   0.88us�˲� @72M & TIM_CKD_DIV1
    sTimeICInit.TIM_ICFilter        =   6;
//    TIM_ICInit(BSP_TMR_E_ODD , &sTimeICInit);
//    TIM_ICInit(BSP_TMR_E_EVEN, &sTimeICInit);
    
    /// ��ʱ��IO��ʼ�� (������ӳ��)
//    IO_MultiInit(PA00_ENC_A_ODD , 2, GPIO_Mode_IN_FLOATING);
//    IO_MultiInit(PB06_ENC_A_EVEN, 2, GPIO_Mode_IN_FLOATING);
//    
//    IO_Init(PA02_ENC_I_ODD , GPIO_Mode_IN_FLOATING);
//    IO_Init(PB08_ENC_I_EVEN, GPIO_Mode_IN_FLOATING);
    
//    TIM_Cmd(BSP_TMR_E_ODD , ENABLE);
//    TIM_Cmd(BSP_TMR_E_EVEN, ENABLE);
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        TIM_TimeBaseInit(tim_id_encoder[i], &sTimeBaseInit);
        
/// ʹ���������������ݷ�����һ��. �����빳��˳ʱ��ת��(�������¿�)Ϊ�������ӷ���, 
/// ���ڳ��ִ���, �������ʱ��ת��(����˿�)Ϊ�������ӷ���, �˷�����תӦʹ��������ֵ����
/// ����������߷�ʽ, ��·��Ӳ��������. �������, �ɵ���һ���źŵļ���.
        TIM_EncoderInterfaceConfig(tim_id_encoder[i], TIM_EncoderMode_TI12,
/// һ���, AB����һ���ƽ����, ���ɸı��������.
/// ʵ����, A����(��������3�޸�), ��������û�б仯, ��B(����4)����ɸı��������.
/// !!! оƬ������bug, ���ĵ�û�и��� !!!
//                                   TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
                                   TIM_ICPolarity_Rising, TIM_ICPolarity_Falling);      /// ��Ŀǰ�������
        TIM_ICInit(tim_id_encoder[i], &sTimeICInit);
        IO_MultiInit(io_id_enc_a[i], 2, GPIO_Mode_IN_FLOATING);
        IO_Init(io_id_enc_i[i], GPIO_Mode_IN_FLOATING);
        
        TIM_ITConfig(tim_id_encoder[i], TIM_IT_Update, ENABLE);         /// �򿪸����ж�
        
        TIM_Cmd(tim_id_encoder[i], ENABLE);
    }
}

static  void  BSP_ENC_IndexEn (void)
{
    EXTI_InitTypeDef    sEXTIInit;
    
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);
    
    sEXTIInit.EXTI_Line     =   BSP_EXIT_LINE_I_E_ODD;
    sEXTIInit.EXTI_Mode     =   EXTI_Mode_Interrupt;
    sEXTIInit.EXTI_Trigger  =   EXTI_Trigger_Falling;
    sEXTIInit.EXTI_LineCmd  =   ENABLE;
    EXTI_Init(&sEXTIInit);
    sEXTIInit.EXTI_Line     =   BSP_EXIT_LINE_I_E_EVEN;
    sEXTIInit.EXTI_Mode     =   EXTI_Mode_Interrupt;
    sEXTIInit.EXTI_Trigger  =   EXTI_Trigger_Falling;
    sEXTIInit.EXTI_LineCmd  =   ENABLE;
    EXTI_Init(&sEXTIInit);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Set the counter of the encoder
/// @param  id    : The ID of led (the odd or even)
///         cnt   : the value of the counter
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_ENC_CntSet (BSP_ID_TypeDef id, u16 cnt)
{
//    switch (id) {
////    case BSP_ID_ALL:
//////        BSP_TMR_E_ODD->CNT  =   cnt;
//////        BSP_TMR_E_EVEN->CNT =   cnt;
////        BSP_ENC_CntSet(BSP_ID_ODD , cnt);
////        BSP_ENC_CntSet(BSP_ID_EVEN, cnt);
////        break;
//        
//    case BSP_ID_ODD:
//        BSP_TMR_E_ODD->CNT  =   cnt;
//        break;
//        
//    case BSP_ID_EVEN:
//        BSP_TMR_E_EVEN->CNT =   cnt;
//        break;
//
//    default:
//        break;
//    }
    tim_id_encoder[id]->CNT =   cnt;
}

u16  BSP_ENC_CntGet (BSP_ID_TypeDef id)
{
    return (tim_id_encoder[id]->CNT);
}



/// -------- 24V detect --------

/// ����


/// -------- approach switch --------

static  const  IO_ID_TypeDef  io_id_swi_fi[BSP_DEVICE_ID_N] =   {PE07_DJ_CUT_ODD, PD01_DJ_CUT_EVEN};
static  const  IO_ID_TypeDef  io_id_swi_se[BSP_DEVICE_ID_N] =   {PD06_XDJ_ODD, PD07_XDJ_EVEN};

/// -----------------------------------------------------------------------------------------------------
/// @brief Initialize the I/O for the approach switch.
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_SWI_Init (void)
{
//    /// ���߼�����ߵ�λ(��һ���)
//    IO_Init(PE07_DJ_CUT_ODD , GPIO_Mode_IPD);
//    IO_Init(PD01_DJ_CUT_EVEN, GPIO_Mode_IPD);
//    /// �¶ϼ�(�ڶ����)
//    IO_Init(PD06_XDJ_ODD , GPIO_Mode_IPD);
//    IO_Init(PD07_XDJ_EVEN, GPIO_Mode_IPD);
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        IO_Init(io_id_swi_fi[i], GPIO_Mode_IPD);        /// ���߼�����ߵ�λ(��һ���)
        IO_Init(io_id_swi_se[i], GPIO_Mode_IPD);        /// �¶ϼ�(�ڶ����)
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Get the Inputs for detector of thread_breaking (first detector).
/// @param  id   : The ID of key (the odd or even)
/// @retval : the signal of detector
/// -----------------------------------------------------------------------------------------------------
BSP_SWITCH_TypeDef  BSP_SWI_FiGet (BSP_ID_TypeDef id)
{
//    switch (id) {
//    case BSP_ID_ODD:
//        return  (BSP_SWITCH_TypeDef)IO_InGet(PE07_DJ_CUT_ODD);
//        
//    case BSP_ID_EVEN:
//    default:                /// continue
//        return  (BSP_SWITCH_TypeDef)IO_InGet(PD01_DJ_CUT_EVEN);
//    }
    return  (BSP_SWITCH_TypeDef)IO_InGet(io_id_swi_fi[id]);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Get the Inputs for detector of the second thread_breaking
/// @param  id   : The ID of key (the odd or even)
/// @retval : the signal of detector
/// -----------------------------------------------------------------------------------------------------
BSP_SWITCH_TypeDef  BSP_SWI_SeGet (BSP_ID_TypeDef id)
{
//    switch (id) {
//    case BSP_ID_ODD:
//        return  (BSP_SWITCH_TypeDef)IO_InGet(PD06_XDJ_ODD);
//        
//    case BSP_ID_EVEN:
//    default:                /// continue
//        return  (BSP_SWITCH_TypeDef)IO_InGet(PD07_XDJ_EVEN);
//    }
    return  (BSP_SWITCH_TypeDef)IO_InGet(io_id_swi_se[id]);
}


/// -------- potentiometer --------

/// -----------------------------------------------------------------------------------------------------
/// @brief  Initialize the I/O, ADC and DMA for the potentiometer
///         û��ʹ���ⲿ��ʱ����, ��Ϊ���岻��, ��ռ����һ����ʱ��
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_POT_Init (void)
{
    ADC_InitTypeDef             sADCInit;
    DMA_InitTypeDef             sDMAInit;
    
    
//      /* DMA1 channel1 configuration ----------------------------------------------*/
//  DMA_DeInit(DMA1_Channel1);
//  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
//  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_DualConvertedValueTab;
//  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//  DMA_InitStructure.DMA_BufferSize = 16;
//  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
//  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
//  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
//  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
//  /* Enable DMA1 Channel1 */
//  DMA_Cmd(DMA1_Channel1, ENABLE);
    
    /// DMA configuration
    sDMAInit.DMA_PeripheralBaseAddr =   (u32)BSP_DMA_CPA_POT;
    sDMAInit.DMA_MemoryBaseAddr     =   (u32)BSP_POTValue;
    sDMAInit.DMA_DIR                =   DMA_DIR_PeripheralSRC;
    sDMAInit.DMA_BufferSize         =   BSP_ADC_VAL_NUM;
    sDMAInit.DMA_PeripheralInc      =   DMA_PeripheralInc_Disable;
    sDMAInit.DMA_MemoryInc          =   DMA_MemoryInc_Enable;
    sDMAInit.DMA_PeripheralDataSize =   DMA_PeripheralDataSize_Word;
    sDMAInit.DMA_MemoryDataSize     =   DMA_MemoryDataSize_Word;
    sDMAInit.DMA_Mode               =   DMA_Mode_Circular;
    sDMAInit.DMA_Priority           =   DMA_Priority_Low;
    sDMAInit.DMA_M2M                =   DMA_M2M_Disable;
    DMA_Init(BSP_DMA_CHN_POT, &sDMAInit);
    
    DMA_Cmd(BSP_DMA_CHN_POT, ENABLE);
    
    
//  /* ADC1 configuration ------------------------------------------------------*/
//  ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
//  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
//  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
//  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
//  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//  ADC_InitStructure.ADC_NbrOfChannel = 2;
//  ADC_Init(ADC1, &ADC_InitStructure);
//  /* ADC1 regular channels configuration */ 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_239Cycles5);    
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 2, ADC_SampleTime_239Cycles5);
//  /* Enable ADC1 DMA */
//  ADC_DMACmd(ADC1, ENABLE);
//
//  /* ADC2 configuration ------------------------------------------------------*/
//  ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
//  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
//  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
//  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
//  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//  ADC_InitStructure.ADC_NbrOfChannel = 2;
//  ADC_Init(ADC2, &ADC_InitStructure);
//  /* ADC2 regular channels configuration */ 
//  ADC_RegularChannelConfig(ADC2, ADC_Channel_11, 1, ADC_SampleTime_239Cycles5);
//  ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 2, ADC_SampleTime_239Cycles5);
//  /* Enable ADC2 external trigger conversion */
//  ADC_ExternalTrigConvCmd(ADC2, ENABLE);

    /// ADCCLK = PCLK2 / 8, ����
    RCC_ADCCLKConfig(BSP_ADC_CLK);

    /// ADC configuration
    sADCInit.ADC_Mode               =   BSP_ADC_MODE;
    sADCInit.ADC_ScanConvMode       =   DISABLE;
    sADCInit.ADC_ContinuousConvMode =   ENABLE;
    sADCInit.ADC_ExternalTrigConv   =   ADC_ExternalTrigConv_None;
    sADCInit.ADC_DataAlign          =   ADC_DataAlign_Right;
    sADCInit.ADC_NbrOfChannel       =   1;
    ADC_Init(BSP_ADC_POT_ODD , &sADCInit);
    ADC_Init(BSP_ADC_POT_EVEN, &sADCInit);
    
    /// ADC regular channels configuration
    ADC_RegularChannelConfig(BSP_ADC_POT_ODD , BSP_ADC_CH_POT_ODD , 1, BSP_ADC_SAMPLETIME);
    ADC_RegularChannelConfig(BSP_ADC_POT_EVEN, BSP_ADC_CH_POT_EVEN, 1, BSP_ADC_SAMPLETIME);
    
    /// Enable ADC1 DMA
    ADC_DMACmd(ADC1, ENABLE);
    /// Enable ADC2 external trigger conversion
    ADC_ExternalTrigConvCmd(ADC2, ENABLE);
    
    
    /// Enable ADC
    ADC_Cmd(BSP_ADC_POT_ODD , ENABLE);
    ADC_Cmd(BSP_ADC_POT_EVEN, ENABLE);
    
    /// Enable ADC reset calibaration register
    ADC_ResetCalibration(BSP_ADC_POT_ODD);
    ADC_ResetCalibration(BSP_ADC_POT_EVEN);
    /// Check the end of ADC reset calibration register
    while(ADC_GetResetCalibrationStatus(BSP_ADC_POT_ODD));
    while(ADC_GetResetCalibrationStatus(BSP_ADC_POT_EVEN));
    /// Start ADC calibaration */
    ADC_StartCalibration(BSP_ADC_POT_ODD);
    ADC_StartCalibration(BSP_ADC_POT_EVEN);
    /// Check the end of ADC calibration */
    while(ADC_GetCalibrationStatus(BSP_ADC_POT_ODD));
    while(ADC_GetCalibrationStatus(BSP_ADC_POT_EVEN));
    
    IO_Init(PB00_POT_ODD , GPIO_Mode_AIN);
    IO_Init(PB01_POT_EVEN, GPIO_Mode_AIN);
    
    /// Start ADC1 Software Conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

///// -----------------------------------------------------------------------------------------------------
///// @brief Get the Inputs for the keys (switch on head board).
///// @param  id   : The ID of key (the odd or even)
/////         up   : save the status of the up   switch (output)
/////         down : save the status of the down switch (output)
///// @retval : None
///// -----------------------------------------------------------------------------------------------------
//u32  BSP_POT_Get (BSP_ID_TypeDef id)
//{
//    u32     i;
//    u32     tmp =   0;
//    
//    
//    switch (id) {
//    case BSP_ID_ODD:
//        for (i = 0; i < BSP_ADC_VAL_NUM; i++) {
//            tmp    +=   BSP_POTValue[i].odd; 
//        }
//        break;
//        
//    case BSP_ID_EVEN:
//    default:                /// continue
//        for (i = 0; i < BSP_ADC_VAL_NUM; i++) {
//            tmp    +=   BSP_POTValue[i].even; 
//        }
//        break;
//    }
//    return  tmp;
//}

u16  BSP_POT_GetOne (BSP_ID_TypeDef id)
{
    static  u8      i_odd   =   0;
    static  u8      i_even  =   0;
            u16     tmp;
    
    
    switch (id) {
    case BSP_ID_ODD:
        tmp     =   BSP_POTValue[i_odd++].odd;
        i_odd  %=   BSP_ADC_VAL_NUM;
        break;
        
    case BSP_ID_EVEN:
        tmp     =   BSP_POTValue[i_even++].even;
        i_even %=   BSP_ADC_VAL_NUM;
        break;
    
    default:
        tmp     =   0;
        break;
    }
    
    return  (tmp << (16 - 12));
}

u16  BSP_POT_GetAve (BSP_ID_TypeDef id)
{
    u32     i;
    u16     tmp =   0;
    
    
    switch (id) {
    case BSP_ID_ODD:
        for (i = 0; i < BSP_ADC_VAL_NUM; i++) {
            tmp    +=   BSP_POTValue[i].odd; 
        }
        break;
        
    case BSP_ID_EVEN:
        for (i = 0; i < BSP_ADC_VAL_NUM; i++) {
            tmp    +=   BSP_POTValue[i].even; 
        }
        break;
        
    default:
        break;
    }
    
#if ((16 - 12) >= BSP_ADC_VAL_N_POWER)
    return  (tmp << ((16 - 12) - BSP_ADC_VAL_N_POWER));
#else
    return  (tmp >> (BSP_ADC_VAL_N_POWER - (16 - 12)));
#endif

}


/// -------- COMMUNICATION --------

/// -------- 491 --------

///// -----------------------------------------------------------------------------------------------------
///// @brief  �ͷ�491_TxBuf��һ�����ݿռ�, filled by 0xFFFF.
///// @param  ���ݿռ���������
///// @retval : None
///// -----------------------------------------------------------------------------------------------------
//static  void  BSP_491_RxBufFreeOne (u32 i)
//{
//    BSP_491_RxBuf[i]    =   (u16)BSP_491_RXBUF_FREE;
//}

///// -----------------------------------------------------------------------------------------------------
///// @brief  Initialize the USART_491 TxBuf, filled by 0xFFFF. and BSP_491_RxBufOutIx.
///// @param None
///// @retval : None
///// -----------------------------------------------------------------------------------------------------
//static  void  BSP_491_RxBufInit (void)
//{
//    u32     i;
//    
//    
//    for (i = 0; i < BSP_491_RXBUF_N; i++) {
//        BSP_491_RxBuf[i]    =   BSP_491_RXBUF_FREE;
//    }
//    BSP_491_RxBufOutIx  =   0;
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief  Initialize the I/O, USART and DMA for the 491
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_491_Init (void)
{
//    DMA_InitTypeDef             sDMAInit;
    USART_InitTypeDef           sUSARTInit;
    
    
//    BSP_491_RxBufInit();
//    
//    /// DMA configuration (triggered by USART_491 Rx event)
//    sDMAInit.DMA_PeripheralBaseAddr =   (u32)BSP_DMA_CPA_491RX;
//    sDMAInit.DMA_MemoryBaseAddr     =   (u32)BSP_491_RxBuf;
//    sDMAInit.DMA_DIR                =   DMA_DIR_PeripheralSRC;
//    sDMAInit.DMA_BufferSize         =   1 << BSP_491_RXBUF_N_POWER;
//    sDMAInit.DMA_PeripheralInc      =   DMA_PeripheralInc_Disable;
//    sDMAInit.DMA_MemoryInc          =   DMA_MemoryInc_Enable;
//    sDMAInit.DMA_PeripheralDataSize =   DMA_PeripheralDataSize_HalfWord;
//    sDMAInit.DMA_MemoryDataSize     =   DMA_MemoryDataSize_HalfWord;
//    sDMAInit.DMA_Mode               =   DMA_Mode_Circular;
//    sDMAInit.DMA_Priority           =   DMA_Priority_High;
//    sDMAInit.DMA_M2M                =   DMA_M2M_Disable;
//    DMA_Init(BSP_DMA_CHN_491RX, &sDMAInit);
//    
//    DMA_Cmd(BSP_DMA_CHN_491RX, ENABLE);
    
    
    /// Configure USART_491
    sUSARTInit.USART_BaudRate               =   BSP_491_BAUD_RATE;
    if (BSP_Is924) {
        sUSARTInit.USART_WordLength         =   USART_WordLength_9b;
    } else {
        sUSARTInit.USART_WordLength         =   USART_WordLength_8b;
    }
    sUSARTInit.USART_StopBits               =   BSP_491_STOP_BITS;
    sUSARTInit.USART_Parity                 =   USART_Parity_No;
    sUSARTInit.USART_HardwareFlowControl    =   USART_HardwareFlowControl_None;     /// no RTS and CTS
    sUSARTInit.USART_Mode                   =   USART_Mode_Rx | USART_Mode_Tx;      /// Rx and Tx enabled
    
    USART_Init(BSP_USART_491, &sUSARTInit);
    
//    /// Enable USART_491 DMA Rx request
//    USART_DMACmd(BSP_USART_491, USART_DMAReq_Rx, ENABLE);
    
    /// Enable BSP_USART_491 Receive and Transmit interrupts
    USART_ITConfig(BSP_USART_491, USART_IT_RXNE, ENABLE);
    USART_ITConfig(BSP_USART_491, USART_IT_TC  , DISABLE);
//    USART_ITConfig(BSP_USART_491, USART_IT_TXE , ENABLE);
//    USART_ITConfig(BSP_USART_491, USART_IT_TC  , ENABLE);
    
    /// Configure the GPIO ports
    IO_Init(PC13_491_DE, GPIO_Mode_Out_PP);
    IO_Init(PA09_491_TX, GPIO_Mode_AF_PP);
    IO_Init(PA10_491_RX, GPIO_Mode_IN_FLOATING);
    BSP_491_DeSet(DISABLE);                         /// �ͷŷ�������
    
    /// Enable the USART_491
    USART_Cmd(BSP_USART_491, ENABLE);
}

///// -----------------------------------------------------------------------------------------------------
///// @brief  ����һ��491���յ�����(8 or 9 bits), �ӽ���buffer�ж�ȡ
///// @param  data : ��ȡ������(���)
///// @retval : ������״̬
/////         - BSP_491_RXBUF_NORMAL      ������ȡ���յ�������
/////         - BSP_491_RXBUF_FULL        ����buffer��, ����
/////         - BSP_491_RXBUF_EMPTY       ����buffer��, ������
///// -----------------------------------------------------------------------------------------------------
//BSP_491_RxBuf_TypeDef  BSP_491_RxGet (u16* data)
//{
//    if (BSP_491_RxBuf[(BSP_491_RxBufOutIx - 1) & BSP_491_RXBUF_N_MSK] != BSP_491_RXBUF_FREE) {
//        if (BSP_491_RxBuf[BSP_491_RxBufOutIx & BSP_491_RXBUF_N_MSK] != BSP_491_RXBUF_FREE) {
//            *data               =   BSP_491_RxBuf[BSP_491_RxBufOutIx++];
//            BSP_491_RxBufOutIx &=   BSP_491_RXBUF_N_MSK; 
//            return  BSP_491_RXBUF_NORMAL;
//        } else {
//            return  BSP_491_RXBUF_EMPTY;
//        }
//    } else {
//        return  BSP_491_RXBUF_FULL;
//    }
//}
//
///// -----------------------------------------------------------------------------------------------------
///// @brief  д��һ��491���͵�����(8 or 9 bits)
///// @param  data : Ҫ���͵�����
///// @retval : ������״̬
/////         - BSP_491_TX_NORMAL     ����д��Ҫ��������
/////         - BSP_491_TX_FULL       ���ݼĴ�����, ��Ҫ�ȴ�
///// -----------------------------------------------------------------------------------------------------
//BSP_491_Tx_TypeDef  BSP_491_TxSet (u16 data)
//{
//    if (USART_GetFlagStatus(BSP_USART_491, USART_FLAG_TXE) != RESET) {
//        USART_SendData(BSP_USART_491, data);
//        return  BSP_491_TX_NORMAL;
//    } else {
//        return  BSP_491_TX_FULL;
//    }
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief  ʹ�ܻ��ֹ491����
/// @param  NewState : ʹ�ܻ��ֹ
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  BSP_491_DeSet (FunctionalState NewState)
{
    if (NewState != DISABLE) {
        IO_OutSet(PC13_491_DE, 1);
    } else {
        IO_OutSet(PC13_491_DE, 0);
    }
}


/// -------- sequin --------

static  const  IO_ID_TypeDef  io_id_sqn[BSP_DEVICE_ID_N]    =   {PD02_SQN_G_ODD, PD04_SQN_G_EVEN};

/// -----------------------------------------------------------------------------------------------------
/// @brief Initialize the I/O to sequin board
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  BSP_SQN_Init (void)
{
//    IO_Init(PD02_SQN_G_ODD , GPIO_Mode_Out_PP);
//    IO_Init(PD03_SQN_R_ODD , GPIO_Mode_Out_PP);
//    IO_Init(PD04_SQN_G_EVEN, GPIO_Mode_Out_PP);
//    IO_Init(PD05_SQN_R_EVEN, GPIO_Mode_Out_PP);
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        IO_MultiInit(io_id_sqn[i], 2, GPIO_Mode_Out_PP);
        BSP_SQN_EmbStSend((BSP_ID_TypeDef)i, BSP_EMB_ST_NORMAL);
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  Send embroidery status to sequin board
/// @param  id  : The ID of sequin board (the odd or even)
///         st  : the status of embroidery
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
//void  BSP_SQN_EmbStSend (BSP_ID_TypeDef id, BSP_EMB_ST_TypeDef st)
//{
//    switch (id) {
//    case BSP_ID_ODD:
//        IO_MultiOutSet(PD02_SQN_G_ODD , 2, (u32)st);
//        break;
//        
//    case BSP_ID_EVEN:
//    default:
//        IO_MultiOutSet(PD04_SQN_G_EVEN, 2, (u32)st);
//        break;
//    }
//}

void  BSP_SQN_EmbStSend (BSP_ID_TypeDef id, BSP_EMB_ST_TypeDef st)
{
    IO_MultiOutSet(io_id_sqn[id], 2, (u32)st);
}

/********************************************************************
�������ܣ����STM32��ΨһID��Ȼ��ת���ɺ���¼��ͬ����˳��
��ڲ�����p Ϊ����ָ�룬len Ϊ���ݳ���
�� �أ�8 λ���
�� ע��
********************************************************************/
void UID_Change2_D(u8 * dst,u8 * shunxu)
{
	u8 i;
	for(i=0;i<12;i++)
	{
		dst[i]=*(u8 *)(CONFIRM_STM32_ID_ADDR+shunxu[i]);
	} 
}
void  	Get_9511_ID(void)//�����¼��д���4�ֽ�����
{
	u8 i;
	for(i=0;i<4;i++)
	 {
	 	PRO_9511_ID[i]=*(u8 *)(CONFIRM_9511_ID_ADDR+i);
	 }
}
extern void PRO_MainSetPssW(u16 buf[], u8 len,u32 Addr);
extern crypto_des3_ctx_t ds3_crypto_des3_ctx_t;
extern unsigned char ds3_src[8];
extern unsigned char ds3_dst[8];

static u8 Get_Main_ID_Status(u32 Addr,u8 len)
{
	u8 status=1,i;
	u8 tmp_read_buf[20],com_tmp,len_tmp;
	for(i=0;i<len;i++)
	{
		tmp_read_buf[i]=*(u8*)(Addr+i);
	}
	len_tmp=len>>1;
	for(i=0;i<len_tmp;i++)
	{
		com_tmp=(~tmp_read_buf[i+len_tmp]);
		if(tmp_read_buf[i]!=com_tmp)
		{
			status=0;	//���������쳣
			break;
		}
	}
	return status;	
}
extern u8 Confirm_status;
extern u8 ds3_src_tmp[8];
extern u16 Sec_Number;
extern u8 thir_status;
void  	Get_Main_ID(void)//������ؼ���ʱ��д���4�ֽ�����
{
	u8 i,first_status=0,second_status=0,status=0,tmp_u8;
	u16 tmp1=0,tmp2=0;
	u32 Addr1,Addr2;
	Addr1=CONFIRM_MAINCONTRAL_ADDR1;
	Addr2=CONFIRM_MAINCONTRAL_ADDR2;
	tmp1=*(u16*)(Addr1+16);
	tmp2=*(u16*)(Addr2+16);
	if((tmp1==0xFFFF)&&(tmp2==0xFFFF))//9511������
		thir_status=0;
	else if((tmp1!=0xFFFF)&&(tmp2==0xFFFF))//��һ���������
	{
		status=Get_Main_ID_Status(Addr1,16);//����1����ʾ��һ�������Ч������0����Ч��������
		if(status==1)
			thir_status=1;
		else
			thir_status=3;//��һ�������Ч���ڶ��鲻����	
	}
	else if((tmp1==0xFFFF)&&(tmp2!=0xFFFF))//�ڶ����������
	{
		status=Get_Main_ID_Status(Addr2,16);//����1����ʾ�ڶ��������Ч������0����Ч��������
		if(status==1)
			thir_status=2;
		else
			thir_status=4;//�ڶ��������Ч����һ�鲻����	
	}
	else //���������������
	{
	
		status=Get_Main_ID_Status(Addr1,16);//����1����ʾ��һ�������Ч������0����Ч
		if(status==1)
			first_status=1;	//��Ч
		else
			first_status=0;	//��Ч
		status=Get_Main_ID_Status(Addr2,16);//����1����ʾ�ڶ��������Ч������0����Ч
		if(status==1)
			second_status=1;	//��Ч
		else
			second_status=0;	//��Ч
		if((tmp1>=tmp2)&&(first_status==1))//��һ�����£�������Ч
			thir_status=1;
		else if((tmp1>=tmp2)&&(first_status==0))//��һ�����£�������Ч
		{
			if(second_status==1)//�ڶ�����Ч���Եڶ���Ϊ׼
				thir_status=2;
			else
				thir_status=5;//��������Ч��������	
		}
		else if((tmp2>tmp1)&&(second_status==1))//��er�����£�������Ч
			  thir_status=2;
		else if((tmp2>tmp1)&&(second_status==1))//��er�����£�������Ч
		{
			if(first_status==1)//��һ����Ч���Ե�һ��Ϊ׼
				thir_status=1;
			else
				thir_status=5;//��������Ч��������	
		}
	}	
	switch(thir_status)
	{	
		case 0://������������
			for(i=0;i<4;i++)//���ϴ���������Ϊ0xFF��
			{
				MAIN_Sec_ID[i]=0xFF;
			}
			Sec_Number=0;
			SECURITY_9511_STAT=FALSE;
		 break;
		 case 1: //��һ������Ч����
		 	for(i=0;i<8;i++)
				 ds3_src[i]=*(u8*)(Addr1+i);
			crypto_des3_decrypt(&ds3_crypto_des3_ctx_t,
                    ds3_dst,
                    ds3_src);//����
			Sec_Number=tmp1;
			for(i=0;i<4;i++)//�����뱣�棬ΪͨѶ���ϴ���
			{
				MAIN_Sec_ID[i]=ds3_dst[i+4];
			}
			if(Confirm_status==0)//���û����֤��������ǰ����˳��
				//for(i=0;i<4;i++)//�����뱣�棬ΪͨѶ���ϴ���
				{
					tmp1=~tmp1;
					tmp_u8=Sec_Number&0xFF;
					ds3_src_tmp[0]=tmp_u8;
					tmp_u8=(Sec_Number&0xFF00)>>8;
					ds3_src_tmp[1]=tmp_u8;
					tmp_u8=tmp1&0xFF;
					ds3_src_tmp[2]=tmp_u8;
					tmp_u8=(tmp1&0xFF00)>>8;
					ds3_src_tmp[3]=tmp_u8;
				}
			if((MAIN_Sec_ID[0]!=0xff)||(MAIN_Sec_ID[1]!=0xff)||(MAIN_Sec_ID[2]!=0xff)||(MAIN_Sec_ID[3]!=0xff))//�Ǽ�������
		 	{
			 	//�ӵ����ڶ���ҳȡ���ݣ����9511�Ѿ������ˣ��ж������Ƿ�һ��,��һ�����ý�ֹ����
				//9511û�м��ܣ�ֱ���˳�
				SECURITY_9511_STAT=TRUE;
		 	}

		 break;
		 case 2: //�ڶ�������Ч����
		 	for(i=0;i<8;i++)
				 ds3_src[i]=*(u8*)(Addr2+i);
			crypto_des3_decrypt(&ds3_crypto_des3_ctx_t,
                    ds3_dst,
                    ds3_src);//����
			Sec_Number=tmp2;
			for(i=0;i<4;i++)//�����뱣�棬ΪͨѶ���ϴ���
			{
				MAIN_Sec_ID[i]=ds3_dst[i+4];
			}
			if(Confirm_status==0)
			//for(i=0;i<4;i++)//�����뱣�棬ΪͨѶ���ϴ���
				{
					tmp2=~tmp2;
					tmp_u8=Sec_Number&0xFF;
					ds3_src_tmp[0]=tmp_u8;
					tmp_u8=(Sec_Number&0xFF00)>>8;
					ds3_src_tmp[1]=tmp_u8;
					tmp_u8=tmp2&0xFF;
					ds3_src_tmp[2]=tmp_u8;
					tmp_u8=(tmp2&0xFF00)>>8;
					ds3_src_tmp[3]=tmp_u8;
				}
			if((MAIN_Sec_ID[0]!=0xff)||(MAIN_Sec_ID[1]!=0xff)||(MAIN_Sec_ID[2]!=0xff)||(MAIN_Sec_ID[3]!=0xff))//�Ǽ�������
		 	{
			 	//�ӵ����ڶ���ҳȡ���ݣ����9511�Ѿ������ˣ��ж������Ƿ�һ��,��һ�����ý�ֹ����
				//9511û�м��ܣ�ֱ���˳�
				SECURITY_9511_STAT=TRUE;
		 	}
		 break;
		 case 3://��һ�������Ч���ڶ��鲻����
		  Sec_Number=0;
		 for(i=0;i<4;i++)//����������Ϊ0xFF��
		{
			MAIN_Sec_ID[i]=0xFF;
		}
		SECURITY_9511_STAT=FALSE;
		FLASH_Unlock();
		FLASH_ErasePage(Addr1);
		FLASH_Lock();
		 break;
		 case 4://�ڶ��������Ч����һ�鲻����
		 Sec_Number=0;
		 for(i=0;i<4;i++)//����������Ϊ0xFF��
		{
			MAIN_Sec_ID[i]=0xFF;
		}
		SECURITY_9511_STAT=FALSE;
		FLASH_Unlock();
		FLASH_ErasePage(Addr2);
		FLASH_Lock();
		 break;
		 case 5://����������Ч����
		 Sec_Number=0;
		 for(i=0;i<4;i++)//����������Ϊ0xFF��
		{
			MAIN_Sec_ID[i]=0xFF;
		}
		SECURITY_9511_STAT=FALSE;
		FLASH_Unlock();
		FLASH_ErasePage(Addr1);
		FLASH_ErasePage(Addr2);
		FLASH_Lock();
		 break;
		 default :
		 break;
	}
		
}

static  void  BSP_SOL_Init (void)
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        IO_Init(io_id_sol[i], GPIO_Mode_Out_PP);
        BSP_SOL_Set((BSP_ID_TypeDef)i, BSP_SOL_OFF, 0);
    }
	IO_Init(io_id_qf[0], GPIO_Mode_Out_PP);
    BSP_SOL_Set((BSP_ID_TypeDef)0, BSP_SOL_OFF, 0);
}
 extern u32  Tick_cnt;
/// tmr ?????????, 0 ???????    ???18.2??
void  BSP_SOL_Set (BSP_ID_TypeDef id, BSP_SOL_TypeDef st, u16 tmr)
{
	Tick_cnt=OS_TICKS_PER_SEC/2;
    IO_OutSet(io_id_sol[id], (u32)st);
    BSP_TimerSol[id]    =   tmr;
}

/// ??????, ???????????
/// ???????

void  BSP_SOL_TmrProc (void)
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if (BSP_TimerSol[i] != 0) {
            BSP_TimerSol[i]--;
            if (BSP_TimerSol[i] == 0) {
                BSP_SOL_Set((BSP_ID_TypeDef)i, BSP_SOL_OFF, 0);
            }
        }
    }
}




/// End of file -----------------------------------------------------------------------------------------
