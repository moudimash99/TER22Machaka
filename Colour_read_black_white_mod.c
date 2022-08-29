#include <tinyprintf.h>
#include <stm32f4/rcc.h>
#include <stm32f4/gpio.h>
#include <stm32f4/nvic.h>
#include <stm32f4/exti.h>
#include <stm32f4/syscfg.h>
#include <stm32f4/tim.h>
#include <stm32f4/adc.h>


// GPIOD
#define RED_LED 1
#define BLUE_LED 2
#define GREEN_LED 3

int ledArray[] = {
  1,
  2,
  3
};

char RGB[] = {'G','B','R'};
int colourArray[] = {
  0,
  0,
  0
};
int modifArray[] = {
  0,
  0,
  0
};
int whiteArray[] = {
  0,
  0,
  0
};
int blackArray[] = {
  0,
  0,
  0
};

#define ADC 3

// #define WAIT_PSC 1000
// #define WAIT_DELAY (APB1_CLK / WAIT_PSC)
// #define DELAY_1000 (WAIT_DELAY)
// #define DELAY_500 (WAIT_DELAY / 2)
// #define DELAY_100 (WAIT_DELAY / 10)

#define WAIT_PSC 1000
#define WAIT_DELAY (APB1_CLK / WAIT_PSC)
#define DELAY_1000 (WAIT_DELAY)
#define DELAY_500 (WAIT_DELAY/2)
#define DELAY_100 (WAIT_DELAY/10)

void init_ADC() {
  GPIOA_MODER = SET_BITS(GPIOA_MODER, ADC * 2, 2, 0b11);
  GPIOA_PUPDR = SET_BITS(GPIOA_PUPDR, ADC * 2, 2, 0b01);
  ADC1_SQR3 = 3;
  ADC1_CR1 = 0;
  ADC1_CR2 = ADC_ADON;

}
void init_TIM4() {
  TIM4_CR1 = 0;
  TIM4_PSC = WAIT_PSC;
  TIM4_ARR = DELAY_100;
  TIM4_EGR = TIM_UG;
  TIM4_SR = 0;
  //TIM4_CR1 = TIM_ARPE;
  TIM4_CR1 = TIM_CEN;

}
void turnOffLED(int led) {
  GPIOD_ODR = GPIOD_ODR & ~(1 << led);
}
void turnOnLED(int led) {
  GPIOD_ODR = GPIOD_ODR | (1 << led);
}
void init_LED() {
  // GPIO init
  for (int i = 0; i < 3; i++) {
    GPIOD_MODER = SET_BITS(GPIOD_MODER, 2 * ledArray[i], 2, GPIO_MODER_OUT);
    GPIOD_OTYPER &= ~(1 << ledArray[i]);
    turnOnLED(ledArray[i]);
  }

}
void Calibrate(){
	delay_long(5000); //delay 5s
	for(int i = 0;i<=2;i++){
     turnOffLED(ledArray[i]);
     delay(200);
     int avgRead = get_reading(1000);          //number is the number of scans to take for average, this whole function is redundant, one reading works just as well.
     whiteArray[i] = avgRead;
     turnOnLED(ledArray[i]);
     delay(200);
  }
	///turn onn leds so that you say that we are in the next phase



	delay_long(5000);
	for(int i = 0;i<=2;i++){
		  turnOffLED(ledArray[i]);
		 delay(200);
		 int avgRead = get_reading(1000);          //number is the number of scans to take for average, this whole function is redundant, one reading works just as well.
		 blackArray[i] = avgRead;
		 turnOnLED(ledArray[i]);
		 delay(200);
	}
  delay_long(5000);
}
int handle_adc(){
    int val = 0;
    ADC1_CR2 |= ADC_SWSTART;
    while((ADC1_SR & ADC_EOC) == 0)__asm("nop");
    val = ADC1_DR;

		return val;

}

int get_reading(int count){
	int sum = 0;
	for (int i = 0; i < count; i++) {
		int t = handle_adc();
		sum += t;
	}
	int f = sum / count;

	return f;
}

void init() {

  init_LED();
  init_ADC();
  init_TIM4();
}

void switchLED(int turn) {
  switch (turn) {
  case 0:
    turnOnLED(GREEN_LED);
    break;
  case 1:
    turnOffLED(GREEN_LED);
    break;
  default:
    break;
  }
}
void delay_long(int x) {
  int v = x / 100;
  for (int i = 0 ; i < v; i++){
    delay(100);
  }
}

void delay(int x) {
  if (x == 500) {
    TIM4_ARR = DELAY_500;
  } else if (x == 100) {
    TIM4_ARR = DELAY_100;
  }

  TIM4_SR = 0;
  while ((TIM4_SR & TIM_UIF) == 0) {
    __asm("nop");
  }

}
void printResults(int colourArray[]){
  // printf("\n");
  // printf("==================================================================\n");
  for (int i = 0; i < 1; i++) {
  printf("%c value was %d \n", RGB[i],colourArray[i]);
}
// printf("==================================================================\n");
// printf("\n");
// delay_long(2000);
}
void printResultsMod(){
  for (int i = 0; i < 3; i++) {
  printf("%c Modified value was %d \n", RGB[i],modifArray[i]);
}
}


int main() {
  printf("Starting...\n");

  // RCC init
  RCC_AHB1ENR |= RCC_GPIODEN;
  RCC_AHB1ENR |= RCC_GPIOAEN;
  RCC_APB1ENR |= RCC_TIM4EN;
  RCC_APB2ENR |= RCC_ADC1EN;
  TIM4_CR1 = TIM4_CR1 | TIM_CEN;

  printf("Starting...\n");
  init();
  // Calibrate();
  // turnOnLED(GREEN_LED);
  int turn = 0;

  while (1) {
    for (int i = 0; i < 3; i++) {

      delay(100);
      turnOffLED(ledArray[i]);

      delay(100);

      int val = get_reading(15);
      int greyDiff = whiteArray[i] - blackArray[i];                    //the highest possible return minus the lowest returns the area for values in between
      int ModifVal = (val - blackArray[i]) / (greyDiff) * 255; //the reading returned minus the lowest value divided by the possible range multiplied by 255 will give us a value roughly between 0-255 representing the value for the current reflectivity(for the colour it is exposed to) of what is being scanned

      colourArray[i] = val;
      modifArray[i] = ModifVal;
      turnOnLED(ledArray[i]);

    }
    // printf("%d  was read ...\n", colourArray[0]);

		printResults(colourArray);
    // printResultsMod();
  }

}
