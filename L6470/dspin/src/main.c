/**
  ******************************************************************************
  * @file    main.c 
  * @author  IPC Rennes
  * @version V2.0
  * @date    October 4, 2013
  * @brief   Demo program entry file.
  * @note    (C) COPYRIGHT 2013 STMicroelectronics
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
 * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup Firmware Demo Main loop
  * @{
  */
/* Global variables ----------------------------------------------------------*/
  dSPIN_RegsStruct_TypeDef dSPIN_RegsStructArray[NUMBER_OF_SLAVES];
  double   MAX_SPEED[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_MAX_SPEED;
  uint8_t daisy_chain = DAISY_CHAIN;
  uint8_t number_of_slaves = NUMBER_OF_SLAVES;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DELAY_COUNT    0x3FFFF

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
  RCC_ClocksTypeDef RCC_ClockFreq;
  uint32_t i;        
  uint8_t commandArray[NUMBER_OF_SLAVES];
  uint32_t argumentArray[NUMBER_OF_SLAVES];
  uint32_t responseArray[NUMBER_OF_SLAVES];
  double   ACC[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_ACC;
  double   DEC[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_DEC;
  double   FS_SPD[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_FS_SPD;
#if defined(L6470)
  double   KVAL_HOLD[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_KVAL_HOLD;
  double   KVAL_RUN[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_KVAL_RUN;
  double   KVAL_ACC[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_KVAL_ACC;
  double   KVAL_DEC[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_KVAL_DEC;
  double   INT_SPD[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_INT_SPD;
  double   ST_SLP[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_ST_SLP;
  double   FN_SLP_ACC[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_FN_SLP_ACC;
  double   FN_SLP_DEC[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_FN_SLP_DEC;
  double   K_THERM[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_K_THERM;
  double   STALL_TH[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_STALL_TH;
  uint8_t  OCD_TH[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_OCD_TH;
  uint8_t  ALARM_EN[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_ALARM_EN;
  /* OR-ed definitions */
  double   MIN_SPEED[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_MIN_SPEED;
  uint16_t LSPD_BIT[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_LSPD_BIT;
  uint8_t  STEP_MODE[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_STEP_MODE;
  uint8_t  SYNC_MODE[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_SYNC_MODE;
  uint16_t CONFIG_CLOCK_SETTING[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_CLOCK_SETTING;
  uint16_t CONFIG_SW_MODE[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_SW_MODE;
  uint16_t CONFIG_OC_SD[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_OC_SD;
  uint16_t CONFIG_SR[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_SR;
  uint16_t CONFIG_VS_COMP[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_VS_COMP;
  uint16_t CONFIG_PWM_DIV[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_PWM_DIV;
  uint16_t CONFIG_PWM_MUL[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_PWM_MUL;
  bool pOpenA[1];
  bool pOpenB[1];
#endif /* defined(L6470) */
#if defined(L6472)
  double   MIN_SPEED[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_MIN_SPEED;
  double   TVAL_HOLD[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_TVAL_HOLD;
  double   TVAL_RUN[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_TVAL_RUN;
  double   TVAL_ACC[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_TVAL_ACC;
  double   TVAL_DEC[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_TVAL_DEC;
  double   TON_MIN[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_TON_MIN;
  double   TOFF_MIN[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_TOFF_MIN;
  uint8_t  TOFF_FAST[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_TOFF_FAST;
  uint8_t  FAST_STEP[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_FAST_STEP;    
  uint8_t  OCD_TH[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_OCD_TH;
  uint8_t  ALARM_EN[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_ALARM_EN;
  /* OR-ed definitions */
  uint8_t  STEP_MODE[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_STEP_MODE;
  uint8_t  SYNC_MODE[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_SYNC_MODE;
  uint16_t CONFIG_CLOCK_SETTING[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_CLOCK_SETTING;
  uint16_t CONFIG_SW_MODE[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_SW_MODE;
  uint16_t CONFIG_TQ_REG[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_TQ_REG;
  uint16_t CONFIG_OC_SD[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_OC_SD;
  uint16_t CONFIG_SR[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_SR;
  uint16_t CONFIG_TSW[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_TSW;
  uint16_t CONFIG_PRED_EN[NUMBER_OF_SLAVES] = dSPIN_DC_CONF_PARAM_PRED;
#endif /* defined(L6472) */  
  uint32_t dSPIN_rx_data = 0;
  dSPIN_RegsStruct_TypeDef dSPIN_RegsStruct;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */

  
  int main(void) {
        
      /*!< At this stage the microcontroller clock setting is already configured, 
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f10x_xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f10x.c file
      */     

      /* Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers */
      SetSysClock();

#if defined(DEBUG)
      /* This function fills the RCC_ClockFreq structure with the current
         frequencies of different on chip clocks (for debug purpose) */
      RCC_GetClocksFreq(&RCC_ClockFreq);
#endif /* defined(DEBUG) */
      
      /* Initialize peripherals used by dSPIN */
      dSPIN_Peripherals_Init();
      
      /* Visual LED checking */
      dSPIN_Led_Check();
      
      /* Resets and puts dSPIN into standby mode */
      dSPIN_Reset_And_Standby();
      
      if (daisy_chain == 0)
      {
	/* Structure initialization by default values, in order to avoid blank records */
	dSPIN_Regs_Struct_Reset(&dSPIN_RegsStruct);
	
	/* Acceleration rate settings to dSPIN_CONF_PARAM_ACC in steps/s2, range 14.55 to 59590 steps/s2 */
	dSPIN_RegsStruct.ACC 		= AccDec_Steps_to_Par(dSPIN_CONF_PARAM_ACC);
	/* Deceleration rate settings to dSPIN_CONF_PARAM_DEC in steps/s2, range 14.55 to 59590 steps/s2 */
	dSPIN_RegsStruct.DEC 		= AccDec_Steps_to_Par(dSPIN_CONF_PARAM_DEC); 
	/* Maximum speed settings to dSPIN_CONF_PARAM_MAX_SPEED in steps/s, range 15.25 to 15610 steps/s */
	dSPIN_RegsStruct.MAX_SPEED 	= MaxSpd_Steps_to_Par(dSPIN_CONF_PARAM_MAX_SPEED);
	/* Full step speed settings dSPIN_CONF_PARAM_FS_SPD in steps/s, range 7.63 to 15625 steps/s */
	dSPIN_RegsStruct.FS_SPD 	= FSSpd_Steps_to_Par(dSPIN_CONF_PARAM_FS_SPD);
#if defined(L6470)
	/* Minimum speed settings to dSPIN_CONF_PARAM_MIN_SPEED in steps/s, range 0 to 976.3 steps/s */
	dSPIN_RegsStruct.MIN_SPEED	= dSPIN_CONF_PARAM_LSPD_BIT|MinSpd_Steps_to_Par(dSPIN_CONF_PARAM_MIN_SPEED);
        /* Acceleration duty cycle (torque) settings to dSPIN_CONF_PARAM_KVAL_ACC in %, range 0 to 99.6% */
	dSPIN_RegsStruct.KVAL_ACC 	= Kval_Perc_to_Par(dSPIN_CONF_PARAM_KVAL_ACC);
        /* Deceleration duty cycle (torque) settings to dSPIN_CONF_PARAM_KVAL_DEC in %, range 0 to 99.6% */
	dSPIN_RegsStruct.KVAL_DEC 	= Kval_Perc_to_Par(dSPIN_CONF_PARAM_KVAL_DEC);		
        /* Run duty cycle (torque) settings to dSPIN_CONF_PARAM_KVAL_RUN in %, range 0 to 99.6% */
	dSPIN_RegsStruct.KVAL_RUN 	= Kval_Perc_to_Par(dSPIN_CONF_PARAM_KVAL_RUN);
        /* Hold duty cycle (torque) settings to dSPIN_CONF_PARAM_KVAL_HOLD in %, range 0 to 99.6% */
	dSPIN_RegsStruct.KVAL_HOLD 	= Kval_Perc_to_Par(dSPIN_CONF_PARAM_KVAL_HOLD);
	        /* Thermal compensation param settings to dSPIN_CONF_PARAM_K_THERM, range 1 to 1.46875 */
	dSPIN_RegsStruct.K_THERM 	= KTherm_to_Par(dSPIN_CONF_PARAM_K_THERM);
	/* Intersect speed settings for BEMF compensation to dSPIN_CONF_PARAM_INT_SPD in steps/s, range 0 to 3906 steps/s */
	dSPIN_RegsStruct.INT_SPD 	= IntSpd_Steps_to_Par(dSPIN_CONF_PARAM_INT_SPD);
	/* BEMF start slope settings for BEMF compensation to dSPIN_CONF_PARAM_ST_SLP in % step/s, range 0 to 0.4% s/step */
	dSPIN_RegsStruct.ST_SLP 	= BEMF_Slope_Perc_to_Par(dSPIN_CONF_PARAM_ST_SLP);
	/* BEMF final acc slope settings for BEMF compensation to dSPIN_CONF_PARAM_FN_SLP_ACC in% step/s, range 0 to 0.4% s/step */
	dSPIN_RegsStruct.FN_SLP_ACC = BEMF_Slope_Perc_to_Par(dSPIN_CONF_PARAM_FN_SLP_ACC);
	/* BEMF final dec slope settings for BEMF compensation to dSPIN_CONF_PARAM_FN_SLP_DEC in% step/s, range 0 to 0.4% s/step */
	dSPIN_RegsStruct.FN_SLP_DEC = BEMF_Slope_Perc_to_Par(dSPIN_CONF_PARAM_FN_SLP_DEC);
	/* Stall threshold settings to dSPIN_CONF_PARAM_STALL_TH in mA, range 31.25 to 4000mA */
	dSPIN_RegsStruct.STALL_TH 	= StallTh_to_Par(dSPIN_CONF_PARAM_STALL_TH);
        /* Set Config register according to config parameters */
        /* clock setting, switch hard stop interrupt mode, */
        /*  supply voltage compensation, overcurrent shutdown */
        /* slew-rate , PWM frequency */
	dSPIN_RegsStruct.CONFIG 	= (uint16_t)dSPIN_CONF_PARAM_CLOCK_SETTING |
                                          (uint16_t)dSPIN_CONF_PARAM_SW_MODE	   | 
                                          (uint16_t)dSPIN_CONF_PARAM_VS_COMP       |    
                                          (uint16_t)dSPIN_CONF_PARAM_OC_SD         | 
                                          (uint16_t)dSPIN_CONF_PARAM_SR	           | 
                                          (uint16_t)dSPIN_CONF_PARAM_PWM_DIV       |
                                          (uint16_t)dSPIN_CONF_PARAM_PWM_MUL;
#endif /* defined(L6470) */
#if defined(L6472)
        /* Minimum speed settings to dSPIN_CONF_PARAM_MIN_SPEED in steps/s, range 0 to 976.3 steps/s */
	dSPIN_RegsStruct.MIN_SPEED	= MinSpd_Steps_to_Par(dSPIN_CONF_PARAM_MIN_SPEED);
        /* Torque regulation DAC current during motor acceleration, range 31.25mA to 4000mA */
	dSPIN_RegsStruct.TVAL_ACC 	= Tval_Current_to_Par(dSPIN_CONF_PARAM_TVAL_ACC);
        /* Torque regulation DAC current during motor deceleration, range 31.25mA to 4000mA */
	dSPIN_RegsStruct.TVAL_DEC 	= Tval_Current_to_Par(dSPIN_CONF_PARAM_TVAL_DEC);		
        /* Torque regulation DAC current when motor is running, range 31.25mA to 4000mA */
	dSPIN_RegsStruct.TVAL_RUN 	= Tval_Current_to_Par(dSPIN_CONF_PARAM_TVAL_RUN);
        /* Torque regulation DAC current when motor is stopped, range 31.25mA to 4000mA */
	dSPIN_RegsStruct.TVAL_HOLD 	= Tval_Current_to_Par(dSPIN_CONF_PARAM_TVAL_HOLD);
        /* Maximum fast decay and fall step times used by the current control system, range 2us to 32us */
        dSPIN_RegsStruct.T_FAST 	= (uint8_t)dSPIN_CONF_PARAM_TOFF_FAST | (uint8_t)dSPIN_CONF_PARAM_FAST_STEP;
        /* Minimum ON time value used by the current control system, range 0.5us to 64us */
        dSPIN_RegsStruct.TON_MIN 	= Tmin_Time_to_Par(dSPIN_CONF_PARAM_TON_MIN);
        /* Minimum OFF time value used by the current control system, range 0.5us to 64us */
        dSPIN_RegsStruct.TOFF_MIN	= Tmin_Time_to_Par(dSPIN_CONF_PARAM_TOFF_MIN);
        /* Set Config register according to config parameters */
        /* clock setting, switch hard stop interrupt mode, */
        /*  supply voltage compensation, overcurrent shutdown */
        /* slew-rate , target switching period, predictive current control */
	dSPIN_RegsStruct.CONFIG 	= (uint16_t)dSPIN_CONF_PARAM_CLOCK_SETTING |
                                          (uint16_t)dSPIN_CONF_PARAM_SW_MODE	   | 
                                          (uint16_t)dSPIN_CONF_PARAM_TQ_REG        |    
                                          (uint16_t)dSPIN_CONF_PARAM_OC_SD         | 
                                          (uint16_t)dSPIN_CONF_PARAM_SR	           | 
                                          (uint16_t)dSPIN_CONF_PARAM_TSW           |
                                          (uint16_t)dSPIN_CONF_PARAM_PRED;        
#endif /* defined(L6472) */
	/* Overcurrent threshold settings to dSPIN_CONF_PARAM_OCD_TH in mA */
	dSPIN_RegsStruct.OCD_TH 	= dSPIN_CONF_PARAM_OCD_TH;        
        /* Alarm settings to dSPIN_CONF_PARAM_ALARM_EN */
	dSPIN_RegsStruct.ALARM_EN 	= dSPIN_CONF_PARAM_ALARM_EN;
        /* Step mode and sycn mode settings via dSPIN_CONF_PARAM_SYNC_MODE and dSPIN_CONF_PARAM_STEP_MODE */
	dSPIN_RegsStruct.STEP_MODE 	= (uint8_t)dSPIN_CONF_PARAM_SYNC_MODE |
                                          (uint8_t)dSPIN_CONF_PARAM_STEP_MODE;

	/* Program all dSPIN registers */
	dSPIN_Registers_Set(&dSPIN_RegsStruct);

#if defined(DEBUG)
        /* check the values of all dSPIN registers */
        dSPIN_rx_data = dSPIN_Registers_Check(&dSPIN_RegsStruct);
        
        /* get the values of all dSPIN registers and print them to the terminal I/O */
        dSPIN_Registers_Get(&dSPIN_RegsStruct);
#endif /* defined(DEBUG) */           

        /**********************************************************************/
        /* Start example of FLAG interrupt management */
        /**********************************************************************/
        /* Clear Flag pin */
        dSPIN_rx_data = dSPIN_Get_Status();  
        /* Interrupt configuration for FLAG signal */
        dSPIN_Flag_Interrupt_GPIO_Config();
        /* Run constant speed of 400 steps/s forward direction */
        dSPIN_Run(FWD, Speed_Steps_to_Par(400));
        /* Tentative to write to the current motor absolute position register */
        /* while the motor is running */
        dSPIN_Set_Param(dSPIN_ABS_POS, 100);
        dSPIN_Delay(0x004FFFFF);
        /* Get Status to clear FLAG due to non-performable command */
        dSPIN_rx_data = dSPIN_Get_Status();
        dSPIN_Delay(0x004FFFFF);        
        /* Perform SoftStop commmand */
	dSPIN_Soft_Stop();
        /* Wait until not busy - busy pin test */
	while(dSPIN_Busy_HW());
        dSPIN_Delay(0x004FFFFF);
        /**********************************************************************/
        /* End example of FLAG interrupt management */
        /**********************************************************************/

        /**********************************************************************/
        /* Start example of BUSY interrupt management */
        /**********************************************************************/
        /* Interrupt configuration for BUSY signal */
        dSPIN_Busy_Interrupt_GPIO_Config();
	/* Move by 100,000 steps reverse, range 0 to 4,194,303 */
	dSPIN_Move(REV, (uint32_t)(100000));
        /* STEVAL_PCC009V2 : during busy time the POWER LED is switched OFF */
        /* ST_DSPIN_6470H_DISCOVERY : during busy time the LED_BUSY is switched ON */
        /* Wait until not busy - busy pin test */
	while(dSPIN_Busy_HW());
        /* Disable the power bridges */
	dSPIN_Soft_HiZ();
        dSPIN_Delay(0x004FFFFF);
        /**********************************************************************/
        /* End example of BUSY interrupt management */
        /**********************************************************************/          
        
	/* Move by 60,000 steps forward, range 0 to 4,194,303 */
	dSPIN_Move(FWD, (uint32_t)(60000));
	
	/* Wait until not busy - busy pin test */
	while(dSPIN_Busy_HW());

#if defined(L6470)
	/* Send dSPIN command change hold duty cycle to 0.5% */
	dSPIN_Set_Param(dSPIN_KVAL_HOLD, Kval_Perc_to_Par(0.5));
	
	/* Send dSPIN command change run duty cycle to 5% */
	dSPIN_Set_Param(dSPIN_KVAL_RUN, Kval_Perc_to_Par(5));
#endif /* defined(L6470) */
#if defined(L6472)
	/* Send dSPIN command change hold cuurent to 40mA */
	dSPIN_Set_Param(dSPIN_TVAL_HOLD, Tval_Current_to_Par(40));

	/* Send dSPIN command change run current to 200mA */
	dSPIN_Set_Param(dSPIN_TVAL_RUN, Tval_Current_to_Par(200));
#endif /* defined(L6472) */

	/* Run constant speed of 50 steps/s reverse direction */
	dSPIN_Run(REV, Speed_Steps_to_Par(50));

	/* Wait few seconds - motor turns */
	dSPIN_Delay(0x004FFFFF);
	
	/* Perform SoftStop commmand */
	dSPIN_Soft_Stop();

#if defined(L6470)
        /* RESET KVAL_HOLD to initial value */
	dSPIN_Set_Param(dSPIN_KVAL_HOLD, Kval_Perc_to_Par(dSPIN_CONF_PARAM_KVAL_HOLD));
	
	/* RESET KVAL_RUN to initial value */
	dSPIN_Set_Param(dSPIN_KVAL_RUN, Kval_Perc_to_Par(dSPIN_CONF_PARAM_KVAL_RUN));
#endif /* defined(L6470) */
#if defined(L6472)
        /* RESET TVAL_HOLD to initial value */
	dSPIN_Set_Param(dSPIN_TVAL_HOLD, Tval_Current_to_Par(dSPIN_CONF_PARAM_TVAL_HOLD));

	/* RESET TVAL_RUN to initial value */
	dSPIN_Set_Param(dSPIN_TVAL_RUN, Tval_Current_to_Par(dSPIN_CONF_PARAM_TVAL_RUN));
#endif /* defined(L6472) */
        
	/* Wait until not busy - busy status check in Status register */
	while(dSPIN_Busy_SW());
	
	/* Move by 100,000 steps forward, range 0 to 4,194,303 */
	dSPIN_Move(FWD, (uint32_t)(100000));
	
	/* Wait until not busy */
	while(dSPIN_Busy_SW());
        
        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);
	
	/* Test of the Flag pin by polling, wait in endless cycle if problem is detected */
	if(dSPIN_Flag()) while(1);
	
	/* Issue dSPIN Go Home command */
	dSPIN_Go_Home();
	/* Wait until not busy - busy pin test */
	while(dSPIN_Busy_HW());
        
        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);
	
	/* Issue dSPIN Go To command */
	dSPIN_Go_To(0x0000FFFF);
	/* Wait until not busy - busy pin test */
	while(dSPIN_Busy_HW());
	
        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);
        
	/* Issue dSPIN Go To command */
	dSPIN_Go_To_Dir(FWD, 0x0001FFFF);
	/* Wait until not busy - busy pin test */
	while(dSPIN_Busy_HW());
        
        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);

#if defined(L6470)
	/* Read run duty cycle (dSPIN_KVAL_RUN) parameter from dSPIN */
	dSPIN_rx_data = dSPIN_Get_Param(dSPIN_KVAL_RUN);
	
	/* Read intersect speed (dSPIN_INT_SPD) parameter from dSPIN */
	dSPIN_rx_data = dSPIN_Get_Param(dSPIN_INT_SPD);
#endif /* defined(L6470) */
#if defined(L6472)
	/* Read run current (dSPIN_TVAL_RUN) parameter from dSPIN */
	dSPIN_rx_data = dSPIN_Get_Param(dSPIN_TVAL_RUN);
#endif /* defined(L6472) */

	/* Read Status register content */
	dSPIN_rx_data = dSPIN_Get_Status();
	
	/* Read absolute position (dSPIN_ABS_POS) parameter from dSPIN */
	dSPIN_rx_data = dSPIN_Get_Param(dSPIN_ABS_POS);

	/* Reset position counter */
	dSPIN_Reset_Pos();

	/* Read absolute position (dSPIN_ABS_POS) parameter from dSPIN */
	dSPIN_rx_data = dSPIN_Get_Param(dSPIN_ABS_POS);

	/* Issue dSPIN Hard HiZ command - disable power stage (High Impedance) */
	dSPIN_Hard_HiZ();
 
        /**********************************************************************/
        /* Start example of GoUntil Command */
        /**********************************************************************/
        /* Configure Interrupt for switch motor in case the MCU is used to pilot the switch */
        dSPIN_Switch_Motor_Interrupt_Config();        
        /* Motion in FW direction at speed 400steps/s via GoUntil command*/
        /* When SW is closed:  */
        /*    As ACT is set to ACTION_COPY, ABS_POS is saved to MARK register */
        /*    then a soft stop is done          */
        dSPIN_Go_Until(ACTION_COPY, FWD, Speed_Steps_to_Par(400));
        
        /* Waiting for soft Stop after GoUntil command*/
        while(dSPIN_Busy_HW())
        /* User action needed to go further */
        /* User attention drawn by toggling a LED */
#if defined(STEVAL_PCC009V2)
        {
          dSPIN_Gpio_Toggle(POWER_LED_Port, POWER_LED_Pin);
          dSPIN_Delay(0x100000);
        }
        GPIO_SetBits(POWER_LED_Port, POWER_LED_Pin);
#endif     
#if defined(ST_DSPIN_6470H_DISCOVERY)
        {
          dSPIN_Gpio_Toggle(LED_SPARE_Port, LED_SPARE_Pin); 
          dSPIN_Delay(0x100000);
        }
        GPIO_ResetBits(LED_SPARE_Port, LED_SPARE_Pin);
#endif
        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);
        
        /* Move by 50,000 steps in reverse direction, range 0 to 4,194,303 */
        dSPIN_Move(REV, (uint32_t)(50000));
       
        /* Waiting for end of move command*/
	while(dSPIN_Busy_HW());
        
        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);
        
        /* Go to Mark saved with GoUntil command */
        dSPIN_Go_Mark();
        
        /* Wait until not busy - busy pin test */
	while(dSPIN_Busy_HW());

        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);
        
        /**********************************************************************/
        /* End example of GoUntil Command */
        /**********************************************************************/
        
        /**********************************************************************/
        /* Start example of ReleaseSw Command */
        /**********************************************************************/
        /* Motion via dSPIN_Release_SW command in REV direction at minimum speed*/
        /* (or  5 steps/s is minimum speed is < 5step/s)*/
        /* When SW is opened :  */
        /*    As ACT is set to ACTION_RESET, ABS_POS is reset i.e Home position is set */
        /*    then a soft stop is done          */
        dSPIN_Release_SW(ACTION_RESET, REV);
        
        /* Waiting for soft Stop after ReleaseSw command*/
        while(dSPIN_Busy_HW())
        /* User action needed to go further */
        /* User attention drawn by toggling a LED */
#if defined(STEVAL_PCC009V2)
        {
          dSPIN_Gpio_Toggle(POWER_LED_Port, POWER_LED_Pin);
          dSPIN_Delay(0x100000);
        }
        GPIO_SetBits(POWER_LED_Port, POWER_LED_Pin);
#endif        
#if defined(ST_DSPIN_6470H_DISCOVERY)
        {
          dSPIN_Gpio_Toggle(LED_SPARE_Port, LED_SPARE_Pin); 
          dSPIN_Delay(0x100000);
        }
        GPIO_ResetBits(LED_SPARE_Port, LED_SPARE_Pin);
#endif    
        
        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);
        
        /* Move by 100,000 steps forward, range 0 to 4,194,303 */
        dSPIN_Move(FWD, (uint32_t)(100000));
       
        /* Waiting for end of move command*/
	while(dSPIN_Busy_HW());
        
        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);        
        
        /* Go to Home set with ReleaseSW command */
        dSPIN_Go_Home();
        
       /* Wait until not busy - busy pin test */
	while(dSPIN_Busy_HW());
        
        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);
        /**********************************************************************/
        /* End example of ReleaseSw Command */
        /**********************************************************************/

         /* Get Status to clear FLAG due to switch turn-on event (falling edge on SW pin) */
        dSPIN_rx_data = dSPIN_Get_Status();       
        
        /**********************************************************************/
        /* Start example of StepClock Command */
        /**********************************************************************/
        dSPIN_Busy_Interrupt_GPIO_DeConfig();
        /* Enable Step Clock Mode */
        dSPIN_Step_Clock(FWD);
        dSPIN_Busy_Interrupt_GPIO_Config();
        /* Wait a few seconds, LED busy is off */
        dSPIN_Delay(0x004FFFFF);
        
        /* Set PWM period to 500 so PWM Frequency = 1MHz/ 500 = 2KHz */
        /* and so, motor moves at 2000 steps/s */
        dSPIN_PWM_Enable(500);
        dSPIN_Delay(0x00FFFFFF);
        dSPIN_PWM_DISABLE();
        /**********************************************************************/
        /* End example of StepClock Command */
        /**********************************************************************/
      }
      else /* (daisy_chain == 0) */
      {
        /**********************************************************************/
        /* Start example of DAISY CHAINING */
        /**********************************************************************/
        /* Structure initialization by default values, in order to avoid blank records */
        for (i=0;i<number_of_slaves;i++)
        {
	   dSPIN_Regs_Struct_Reset(&dSPIN_RegsStructArray[i]);
        }
        
        /* Setting of parameters for ALL DEVICES */
        for (i=0;i<number_of_slaves;i++)
        {
          dSPIN_RegsStructArray[i].ACC 		= AccDec_Steps_to_Par(ACC[i]);
          dSPIN_RegsStructArray[i].DEC 		= AccDec_Steps_to_Par(DEC[i]);
          dSPIN_RegsStructArray[i].MAX_SPEED 	= MaxSpd_Steps_to_Par(MAX_SPEED[i]);
          dSPIN_RegsStructArray[i].FS_SPD 	= FSSpd_Steps_to_Par(FS_SPD[i]);
#if defined(L6470)
          dSPIN_RegsStructArray[i].MIN_SPEED	= LSPD_BIT[i]|MinSpd_Steps_to_Par(MIN_SPEED[i]);
          dSPIN_RegsStructArray[i].KVAL_ACC 	= Kval_Perc_to_Par(KVAL_ACC[i]);
          dSPIN_RegsStructArray[i].KVAL_DEC 	= Kval_Perc_to_Par(KVAL_DEC[i]);		
          dSPIN_RegsStructArray[i].KVAL_RUN 	= Kval_Perc_to_Par(KVAL_RUN[i]);
          dSPIN_RegsStructArray[i].KVAL_HOLD 	= Kval_Perc_to_Par(KVAL_HOLD[i]);
          dSPIN_RegsStructArray[i].K_THERM 	= KTherm_to_Par(K_THERM[i]);
          dSPIN_RegsStructArray[i].INT_SPD 	= IntSpd_Steps_to_Par(INT_SPD[i]);
          dSPIN_RegsStructArray[i].ST_SLP 	= BEMF_Slope_Perc_to_Par(ST_SLP[i]);
          dSPIN_RegsStructArray[i].FN_SLP_ACC   = BEMF_Slope_Perc_to_Par(FN_SLP_ACC[i]);
          dSPIN_RegsStructArray[i].FN_SLP_DEC   = BEMF_Slope_Perc_to_Par(FN_SLP_DEC[i]);
          dSPIN_RegsStructArray[i].STALL_TH 	= StallTh_to_Par(STALL_TH[i]);
          dSPIN_RegsStructArray[i].CONFIG 	= (uint16_t)CONFIG_CLOCK_SETTING[i] |
                                            (uint16_t)CONFIG_SW_MODE[i]	   | 
                                            (uint16_t)CONFIG_VS_COMP[i]       |    
                                            (uint16_t)CONFIG_OC_SD[i]         | 
                                            (uint16_t)CONFIG_SR[i]	           | 
                                            (uint16_t)CONFIG_PWM_DIV[i]       |
                                            (uint16_t)CONFIG_PWM_MUL[i];
#endif /* defined(L6470) */
#if defined(L6472)          
          dSPIN_RegsStructArray[i].MIN_SPEED	= MinSpd_Steps_to_Par(MIN_SPEED[i]);
          dSPIN_RegsStructArray[i].TVAL_ACC 	= Tval_Current_to_Par(TVAL_ACC[i]);
          dSPIN_RegsStructArray[i].TVAL_DEC 	= Tval_Current_to_Par(TVAL_DEC[i]);		
          dSPIN_RegsStructArray[i].TVAL_RUN 	= Tval_Current_to_Par(TVAL_RUN[i]);
          dSPIN_RegsStructArray[i].TVAL_HOLD 	= Tval_Current_to_Par(TVAL_HOLD[i]);
          dSPIN_RegsStructArray[i].TON_MIN 	= Tmin_Time_to_Par(TON_MIN[i]);		
          dSPIN_RegsStructArray[i].TOFF_MIN 	= Tmin_Time_to_Par(TOFF_MIN[i]);
          dSPIN_RegsStructArray[i].T_FAST 	= (uint8_t)TOFF_FAST[i] | (uint8_t)FAST_STEP[i];          
          dSPIN_RegsStructArray[i].CONFIG 	= (uint16_t)CONFIG_CLOCK_SETTING[i] | \
                                                  (uint16_t)CONFIG_SW_MODE[i]	| \
                                                  (uint16_t)CONFIG_TQ_REG[i]    | \
                                                  (uint16_t)CONFIG_OC_SD[i]     | \
                                                  (uint16_t)CONFIG_SR[i]	| \
                                                  (uint16_t)CONFIG_TSW[i]       | \
                                                  (uint16_t)CONFIG_PRED_EN[i];
#endif /* defined(L6472) */          
          dSPIN_RegsStructArray[i].OCD_TH 	= OCD_TH[i];
          dSPIN_RegsStructArray[i].ALARM_EN 	= ALARM_EN[i];
          dSPIN_RegsStructArray[i].STEP_MODE 	= (uint8_t)SYNC_MODE[i] | (uint8_t)STEP_MODE[i];
        }

        /* Program all dSPIN registers of All Devices */
        dSPIN_All_Slaves_Registers_Set(number_of_slaves, &dSPIN_RegsStructArray[0]);
  
        /* Get status of all devices, clear FLAG pin */
        dSPIN_All_Slaves_Get_Status(number_of_slaves, responseArray);
        
        if (number_of_slaves > 1)
        {
          /* Initialization of command array with NOP instruction */
          for (i=0;i<number_of_slaves;i++)
          {
            commandArray[i] = dSPIN_NOP; 
          }

          /* Move DEVICE 1, keep other slaves stopped */
          dSPIN_One_Slave_Move(DEVICE_1, number_of_slaves, FWD, 150000);
         
          /* Wait until not busy - busy pin test */
          while(dSPIN_Busy_HW());

          /* Move DEVICE 1, keep other slaves stopped */
          dSPIN_One_Slave_Move(DEVICE_2, number_of_slaves, FWD, 60000);
         
          /* Wait until not busy - busy pin test */
          while(dSPIN_Busy_HW());
        
          /* Move DEVICE 1 by 60000 steps in reverse direction */
          /* Run DEVICE 2 at 400 steps/s in forward direction */
          /* No operation for other slaves */
          commandArray[DEVICE_1] = (uint8_t) dSPIN_MOVE |(uint8_t) REV;
          argumentArray[DEVICE_1] = 60000;
          commandArray[DEVICE_2] = (uint8_t) dSPIN_RUN |(uint8_t) FWD;
          argumentArray[DEVICE_2] = Speed_Steps_to_Par(400);
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, argumentArray);

          /* Wait until not busy - busy pin test */
          /* DEVICE 1 and DEVICE 2 turns in opposite directions */
          while(dSPIN_Busy_HW());

          /* Wait few seconds - DEVICE 1 is stopped, DEVICE 2 turns forward */
          dSPIN_Delay(0x00FFFFFF);

          /* Move DEVICE 1 to HOME position via the shortest path */
          /* Run DEVICE 2 at 150 steps/s in reverse direction */
          /* No operation for other slaves */
          commandArray[DEVICE_1] = (uint8_t) dSPIN_GO_HOME;
          commandArray[DEVICE_2] = (uint8_t) dSPIN_RUN |(uint8_t) REV;
          argumentArray[DEVICE_2] = Speed_Steps_to_Par(150);
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, argumentArray);          
          
          /* Wait until not busy - busy pin test */
          /* DEVICE 1 goes to zero position, turning reverse in this example */
          /* DEVICE 2 changes direction to turn reverse */
          while(dSPIN_Busy_HW());        
 
          /* Wait few seconds - DEVICE 1 is stopped, DEVICE 2 turns reverse */
          dSPIN_Delay(0x00FFFFFF);
        
          /* No change for DEVICE 1 */
          /* Stop DEVICE 2 */
          /* No operation for other slaves */
          commandArray[DEVICE_1] = dSPIN_NOP;
          commandArray[DEVICE_2] = dSPIN_SOFT_STOP;         
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, argumentArray);
        
          /* Wait until not busy - busy pin test */
          while(dSPIN_Busy_HW());
        }
        else /* (number_of_slaves > 1) */
        {
          /* Move by 60,000 steps rorward, range 0 to 4,194,303 */
          dSPIN_One_Slave_Move(number_of_slaves, DEVICE_1, FWD, 60000);

          /* Wait until not busy - busy pin test */
          while(dSPIN_Busy_HW());

#if defined(L6470)
          /* Send dSPIN command change hold duty cycle to 0.5% */
          commandArray[DEVICE_1] = dSPIN_KVAL_HOLD;
          argumentArray[DEVICE_1] = Kval_Perc_to_Par(0.5);
          dSPIN_All_Slaves_Set_Param(number_of_slaves, commandArray, argumentArray);

          /* Send dSPIN command change run duty cycle to 5% */
          commandArray[DEVICE_1] = dSPIN_KVAL_RUN;
          argumentArray[DEVICE_1] = Kval_Perc_to_Par(5);
          dSPIN_All_Slaves_Set_Param(number_of_slaves, commandArray, argumentArray);        
#endif /* defined(L6470) */
#if defined(L6472)
          /* Send dSPIN command change hold current to 40mA */
          commandArray[DEVICE_1] = dSPIN_TVAL_HOLD;
          argumentArray[DEVICE_1] = Tval_Current_to_Par(40);
          dSPIN_All_Slaves_Set_Param(number_of_slaves, commandArray, argumentArray);

          /* Send dSPIN command change run current to 200mA */
          commandArray[DEVICE_1] = dSPIN_TVAL_RUN;
          argumentArray[DEVICE_1] = Tval_Current_to_Par(200);
          dSPIN_All_Slaves_Set_Param(number_of_slaves, commandArray, argumentArray);        
#endif /* defined(L6472) */
          /* Run constant speed of 50 steps/s reverse direction */
          dSPIN_One_Slave_Run(DEVICE_1, number_of_slaves, REV, Speed_Steps_to_Par(50));

          /* Wait few seconds - motor turns */
          dSPIN_Delay(0x00FFFFFF);

          /* Perform SoftStop commmand */
          commandArray[DEVICE_1] = dSPIN_SOFT_STOP;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, 0);
#if defined(L6470)
          /* RESET KVAL_HOLD to initial value */
          commandArray[DEVICE_1] = dSPIN_KVAL_HOLD;
          argumentArray[DEVICE_1] = dSPIN_RegsStructArray[DEVICE_1].KVAL_HOLD;
          dSPIN_All_Slaves_Set_Param(number_of_slaves, commandArray, argumentArray);

          /* RESET KVAL_RUN to initial value */
          commandArray[DEVICE_1] = dSPIN_KVAL_RUN;
          argumentArray[DEVICE_1] = dSPIN_RegsStructArray[DEVICE_1].KVAL_RUN;
          dSPIN_All_Slaves_Set_Param(number_of_slaves, commandArray, argumentArray);
#endif /* defined(L6470) */
#if defined(L6472)
          /* RESET TVAL_HOLD to initial value */
          commandArray[DEVICE_1] = dSPIN_TVAL_HOLD;
          argumentArray[DEVICE_1] = dSPIN_RegsStructArray[DEVICE_1].TVAL_HOLD;
          dSPIN_All_Slaves_Set_Param(number_of_slaves, commandArray, argumentArray);

          /* RESET TVAL_RUN to initial value */
          commandArray[DEVICE_1] = dSPIN_TVAL_RUN;
          argumentArray[DEVICE_1] = dSPIN_RegsStructArray[DEVICE_1].TVAL_RUN;
          dSPIN_All_Slaves_Set_Param(number_of_slaves, commandArray, argumentArray);
#endif /* defined(L6472) */ 
          /* Wait until not busy - busy status check in Status register */
          while(dSPIN_Busy_SW());
	
          /* Move by 100,000 steps forward, range 0 to 4,194,303 */
          dSPIN_One_Slave_Move(DEVICE_1, number_of_slaves, FWD, (uint32_t)(100000));        
	
          /* Wait until not busy */
          while(dSPIN_One_Or_More_Slaves_Busy_SW(number_of_slaves));
	
          /* Test of the Flag pin by polling, wait in endless cycle if problem is detected */
          if(dSPIN_Flag()) while(1);
	
          /* Issue dSPIN Go Home command */
          commandArray[DEVICE_1] = (uint8_t) dSPIN_GO_HOME;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, 0);

          /* Wait until not busy - busy pin test */
          while(dSPIN_Busy_HW());
	
          /* Issue dSPIN Go To command */
          commandArray[DEVICE_1] = dSPIN_GO_TO;
          argumentArray[DEVICE_1] = 0x0000FFFF;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, argumentArray);
          /* Wait until not busy - busy pin test */
          while(dSPIN_Busy_HW());
	
          /* Issue dSPIN Go To command */
          commandArray[DEVICE_1] = (uint8_t)dSPIN_GO_TO_DIR | (uint8_t)FWD;
          argumentArray[DEVICE_1] = 0x0001FFFF;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, argumentArray);
          /* Wait until not busy - busy pin test */
          while(dSPIN_Busy_HW());
#if defined(L6470)
          /* Read run duty cycle (dSPIN_KVAL_RUN) parameter from dSPIN */
          commandArray[DEVICE_1] = dSPIN_KVAL_RUN;
          dSPIN_All_Slaves_Get_Param(number_of_slaves, commandArray, responseArray);
	
          /* Read intersect speed (dSPIN_INT_SPD) parameter from dSPIN */
          commandArray[DEVICE_1] = dSPIN_INT_SPD;
          dSPIN_All_Slaves_Get_Param(number_of_slaves, commandArray, responseArray);
#endif /* defined(L6470) */
#if defined(L6472)
          /* Read run current (dSPIN_TVAL_RUN) parameter from dSPIN */
          commandArray[DEVICE_1] = dSPIN_TVAL_RUN;
          dSPIN_All_Slaves_Get_Param(number_of_slaves, commandArray, responseArray);
#endif /* defined(L6472) */
          /* Read Status register content */
          dSPIN_All_Slaves_Get_Status(number_of_slaves, responseArray);
	
          /* Read absolute position (dSPIN_ABS_POS) parameter from dSPIN */
          commandArray[DEVICE_1] = dSPIN_ABS_POS;
          dSPIN_All_Slaves_Get_Param(number_of_slaves, commandArray, responseArray);

          /* Reset position counter */
          commandArray[DEVICE_1] = dSPIN_RESET_POS;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, 0);   

          /* Read absolute position (dSPIN_ABS_POS) parameter from dSPIN */
          commandArray[DEVICE_1] = dSPIN_ABS_POS;
          dSPIN_All_Slaves_Get_Param(number_of_slaves, commandArray, responseArray);

          /* Issue dSPIN Hard HiZ command - disable power stage (High Impedance) */
          commandArray[DEVICE_1] = dSPIN_HARD_HIZ;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, 0);
 
          /********************************************************************/
          /* Start example of GoUntil Command */
          /********************************************************************/
          /* Configure Interrupt for switch motor in case the MCU is used to pilot the switch */
          dSPIN_Switch_Motor_Interrupt_Config();
          /* Motion in FW direction at speed 400steps/s via GoUntil command*/
          /* When SW is closed:  */
          /*    As ACT is set ot ACTION_COPY, ABS_POS is saved to MARK register */
          /*    then a soft stop is done          */
          commandArray[DEVICE_1] = (uint8_t)dSPIN_GO_UNTIL | (uint8_t)ACTION_COPY | (uint8_t)FWD;
          argumentArray[DEVICE_1] = Speed_Steps_to_Par(400);
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, argumentArray);
        
          /* Waiting for soft Stop after GoUntil command*/
          while(dSPIN_Busy_HW())
          /* User action needed to go further */
          /* User attention drawn by toggling a LED */
#if defined(STEVAL_PCC009V2)
          {
            dSPIN_Gpio_Toggle(POWER_LED_Port, POWER_LED_Pin);
            dSPIN_Delay(0x00100000);
          }
          GPIO_SetBits(POWER_LED_Port, POWER_LED_Pin);
#endif        
#if defined(ST_DSPIN_6470H_DISCOVERY)
          {
            dSPIN_Gpio_Toggle(LED_SPARE_Port, LED_SPARE_Pin); 
            dSPIN_Delay(0x00100000);
          }
          GPIO_ResetBits(LED_SPARE_Port, LED_SPARE_Pin);
#endif           

          /* Move by 50,000 steps in reverse direction, range 0 to 4,194,303 */
          dSPIN_One_Slave_Move(DEVICE_1, number_of_slaves, REV, (uint32_t)(50000));
       
          /* Waiting for end of move command*/
          while(dSPIN_Busy_HW());
        
          /* Go to Mark saved with GoUntil command */
          commandArray[DEVICE_1] = dSPIN_GO_MARK;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, 0);
        
          /* Wait until not busy - busy pin test */
          while(dSPIN_Busy_HW());
        
          /********************************************************************/
          /* End example of GoUntil Command */
          /********************************************************************/
        
          dSPIN_Delay(0x00FFFFFF);
        
          /********************************************************************/
          /* Start example of ReleaseSw Command */
          /********************************************************************/
          /* Motion via dSPIN_Release_SW command in REV direction at minimum speed*/
          /* (or  5 steps/s is minimum speed is < 5step/s)*/
          /* When SW is closed:  */
          /*    As ACT is set ot ACTION_RESET, ABS_POS is reset i.e Home position is set */
          /*    then a soft stop is done          */
          commandArray[DEVICE_1] = (uint8_t)dSPIN_RELEASE_SW | (uint8_t)ACTION_RESET | (uint8_t)REV;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, 0);          
        
          /* Waiting for soft Stop after ReleaseSw command*/
          while(dSPIN_Busy_HW())
          /* User action needed to go further */
          /* User attention drawn by toggling a LED */
#if defined(STEVAL_PCC009V2)
          {
            dSPIN_Gpio_Toggle(POWER_LED_Port, POWER_LED_Pin);
            dSPIN_Delay(0x00100000);
          }
          GPIO_SetBits(POWER_LED_Port, POWER_LED_Pin);
#endif        
#if defined(ST_DSPIN_6470H_DISCOVERY)
          {
            dSPIN_Gpio_Toggle(LED_SPARE_Port, LED_SPARE_Pin); 
            dSPIN_Delay(0x00100000);
          }
          GPIO_ResetBits(LED_SPARE_Port, LED_SPARE_Pin);
#endif

          /* Move by 100,000 steps forward, range 0 to 4,194,303 */
          dSPIN_One_Slave_Move(DEVICE_1, number_of_slaves, FWD, (uint32_t)(100000));
       
          /* Waiting for end of move command*/
          while(dSPIN_Busy_HW());
        
          /* Go to Home set with ReleaseSW command */
          commandArray[DEVICE_1] = (uint8_t) dSPIN_GO_HOME;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, 0);
        
          /* Wait until not busy - busy pin test */
          while(dSPIN_Busy_HW());
          /********************************************************************/
          /* End example of ReleaseSw Command */
          /********************************************************************/
    
          /********************************************************************/
          /* Start example of StepClock Command */
          /********************************************************************/
        
          /* Enable Step Clock Mode */
          commandArray[DEVICE_1] = (uint8_t)dSPIN_STEP_CLOCK | (uint8_t)FWD;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, 0);
        
          /* Set PWM period to 500 so PWM Frequency = 1MHz/ 500 = 2KHz */
          /* and so, motor moves at 2000 steps/s */
          dSPIN_PWM_Enable(500);
          dSPIN_Delay(0x00FFFFFF);
          dSPIN_PWM_DISABLE();
        
          /* Get Status to clear FLAG due to step clock mode */
          dSPIN_All_Slaves_Get_Status(number_of_slaves, responseArray);
        
          /********************************************************************/
          /* End example of StepClock Command */
          /********************************************************************/

          /********************************************************************/
          /* Start example of FLAG interrupt management */
          /********************************************************************/
          /* Interrupt configuration for FLAG signal */
          dSPIN_Flag_Interrupt_GPIO_Config();
          /* Run constant speed of 400 steps/s forward direction */
          dSPIN_One_Slave_Run(DEVICE_1, number_of_slaves, FWD, Speed_Steps_to_Par(400));        
        
          /* Tentative to write to the current motor absolute position register */
          /* while the motor is running */
          commandArray[DEVICE_1] = dSPIN_ABS_POS;
          argumentArray[DEVICE_1] = 100;
          dSPIN_All_Slaves_Set_Param(number_of_slaves, commandArray, argumentArray);       
          dSPIN_Delay(0x00FFFFFF);
          /* Get Status to clear FLAG due to non-performable command */
          dSPIN_All_Slaves_Get_Status(number_of_slaves, responseArray);
          /* Perform SoftStop commmand */
          commandArray[DEVICE_1] = dSPIN_SOFT_STOP;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, 0);
          /* Wait until not busy - busy pin test */
          while(dSPIN_Busy_HW());
          /********************************************************************/
          /* End example of FLAG interrupt management */
          /********************************************************************/
      
          /********************************************************************/
          /* Start example of BUSY interrupt management */
          /********************************************************************/
          /* Interrupt configuration for BUSY signal */
          dSPIN_Busy_Interrupt_GPIO_Config();
          /* Move by 100,000 steps forward, range 0 to 4,194,303 */
          dSPIN_One_Slave_Move(DEVICE_1, number_of_slaves, REV, (uint32_t)(100000));
          /* During busy time the POWER LED is switched OFF */
          /* Wait until not busy - busy pin test */
	  while(dSPIN_Busy_HW());
          /* Disable the power bridges */
          commandArray[DEVICE_1] = dSPIN_SOFT_HIZ;
          dSPIN_All_Slaves_Send_Command(number_of_slaves, commandArray, 0);
          /********************************************************************/
          /* End example of BUSY interrupt management */
          /********************************************************************/
        }  /* (number_of_slaves > 1) */
        /**********************************************************************/
        /* End example of DAISY CHAINING */
        /**********************************************************************/
      }  /* (daisy_chain == 0) */

      /************************************************************************/
      /* Start example of motor control using button */
      /************************************************************************/
#if defined(STEVAL_PCC009V2)
      dSPIN_Busy_Interrupt_GPIO_DeConfig();
#endif /* defined(STEVAL_PCC009V2) */
      dSPIN_Buttons_Interrupts_GPIO_Config();
      /************************************************************************/
      /* End example of motor control using button */
      /************************************************************************/        
      while(1);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/** @} */  

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
