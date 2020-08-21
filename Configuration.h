#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define bit(n) (1<<n)

#define UART_RX_BUFFER_SIZE 128
#define UART_TX_BUFFER_SIZE 128


#define MINIMUM_FEED_RATE 300
#define DEFAULT_JUNCTION_DEVIATION 0.02 // mm
#define MINIMUM_JUNCTION_SPEED 0.0 // (mm/min)

#define N_AXIS 3

#define MAX_BUFFER_GCODE_MAMAGER	4
#define MAX_BUFFER_PLANNER_MAMAGER 32
#define MAX_BUFFER_SEGMENT_MAMAGER 32
#define TIME_BETWEN_INTERRUPTS_IN_SEC (1.0E-6) //[sec]
#define TIME_BETWEN_INTERRUPTS_IN_USEC 1       //[usec]

//--------------------------------------------------------------------
// SPI PINS CONFIGURATION --------------------------------------------
//--------------------------------------------------------------------
// SPI Pines del Driver de los motores. (comparten el puerto SPI)
#define DRIVER_PERIPH_SPI      SYSCTL_PERIPH_SSI2
#define DRIVER_PERIPH_SPI_PORT SYSCTL_PERIPH_GPIOB

#define SPI_BASE	 SSI2_BASE
#define GPIO_PORT    GPIO_PORTB_BASE

#define PIN_CLK	  	 GPIO_PB4_SSI2CLK
//#define PIN_CS    	  GPIO_PB5_SSI2FSS
#define PIN_RX	     GPIO_PB6_SSI2RX
#define PIN_TX       GPIO_PB7_SSI2TX
#define SPI_CLK      GPIO_PIN_4
//#define SPI_CS	      GPIO_PIN_5
#define SPI_SDI      GPIO_PIN_6
#define SPI_SDO      GPIO_PIN_7
//--------------------------------------------------------------------
// PINES DE LOS DRIVERS SPI ------------------------------------------
//--------------------------------------------------------------------
#define DRIVER_PERIPH_CS_SPI_PORT SYSCTL_PERIPH_GPIOA
#define DRIVER_CS_SPI_PORT GPIO_PORTA_BASE

#define DRIVERX_CS_SPI_PIN GPIO_PIN_2
#define DRIVERY_CS_SPI_PIN GPIO_PIN_2
#define DRIVERZ_CS_SPI_PIN GPIO_PIN_2
//--------------------------------------------------------------------
// PINES DE LOS MOTORES ----------------------------------------------
//--------------------------------------------------------------------
#define MOTOR_PERIPH_NSLEEP_PORT    SYSCTL_PERIPH_GPIOE
#define MOTOR_PERIPH_RESET_PORT     SYSCTL_PERIPH_GPIOA
#define MOTOR_PERIPH_PULSE_PORT     SYSCTL_PERIPH_GPIOA
#define MOTOR_PERIPH_DIRECTION_PORT SYSCTL_PERIPH_GPIOA

#define MOTOR_NSLEEP_PORT    GPIO_PORTE_BASE
#define MOTOR_RESET_PORT     GPIO_PORTA_BASE
#define MOTOR_PULSE_PORT     GPIO_PORTA_BASE
#define MOTOR_DIRECTION_PORT GPIO_PORTA_BASE

#define MOTORX_NSLEEP_PIN    GPIO_PIN_0
#define MOTORY_NSLEEP_PIN    GPIO_PIN_1
#define MOTORZ_NSLEEP_PIN    GPIO_PIN_2

#define MOTORX_RESET_PIN     GPIO_PIN_0
#define MOTORY_RESET_PIN     GPIO_PIN_1
#define MOTORZ_RESET_PIN     GPIO_PIN_2

#define MOTORX_STEP_PIN      GPIO_PIN_3
#define MOTORY_STEP_PIN      GPIO_PIN_4
#define MOTORZ_STEP_PIN      GPIO_PIN_5

#define MOTORX_DIR_PIN       GPIO_PIN_6
#define MOTORY_DIR_PIN       GPIO_PIN_7
#define MOTORZ_DIR_PIN       GPIO_PIN_7
//--------------------------------------------------------------------
// CONFIGURACION DE LOS MOTORES --------------------------------------
//--------------------------------------------------------------------
// Setting MOTORX.
#define MOTORX_USTEPS 32
#define MOTORX_STEPS_PER_REV 200
#define MOTORX_INIT_VEL	0.25  		// [rev per second]
#define MOTORX_MAX_VEL	1			// [rev per second]
#define MOTORX_MM_PER_REV 157.0796f // [mm]/[rev]
#define MOTORX_MAX_ACELERATION 5000 // [steps]/[sec^2]
//--------------------------------------------------------------------
// Setting MOTORY.
#define MOTORY_USTEPS 32
#define MOTORY_STEPS_PER_REV 200
#define MOTORY_INIT_VEL	0.25  		// [rev per second]
#define MOTORY_MAX_VEL	1			// [rev per second]
#define MOTORY_MM_PER_REV 157.0796f // [mm]/[rev]
#define MOTORY_MAX_ACELERATION 5000 // [steps]/[sec^2]
//--------------------------------------------------------------------
// Setting MOTORZ.
#define MOTORZ_USTEPS 32
#define MOTORZ_STEPS_PER_REV 200
#define MOTORZ_INIT_VEL	0.25  		// [rev per second]
#define MOTORZ_MAX_VEL	1			// [rev per second]
#define MOTORZ_MM_PER_REV 157.0796f // [mm]/[rev]
#define MOTORZ_MAX_ACELERATION 5000 // [steps]/[sec^2]
//--------------------------------------------------------------------

#endif /* CONFIGURATION_H_ */
