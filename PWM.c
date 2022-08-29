#include <tinyprintf.h>

#include <stm32f4/rcc.h>

#include <stm32f4/gpio.h>

#include <stm32f4/nvic.h>

#include <stm32f4/exti.h>

#include <stm32f4/syscfg.h>

#include <stm32f4/tim.h>

#include <stm32f4/adc.h>

#define RED_LED 4
#define BLUE_LED 1
#define GREEN_LED 5

#define WAIT_PSC 1000
#define WAIT_DELAY (APB1_CLK / WAIT_PSC)
#define DELAY_1000 (WAIT_DELAY)
#define DELAY_500 (WAIT_DELAY / 2)
#define DELAY_100 (WAIT_DELAY / 10)

#define ADC 3

#define TIM3_DIV 27
#define TIM3_PERIOD 59999

void init_ADC() {
  GPIOA_MODER = SET_BITS(GPIOA_MODER, ADC * 2, 2, 0b11);
  GPIOA_PUPDR = SET_BITS(GPIOA_PUPDR, ADC * 2, 2, 0b01);
  ADC1_SQR3 = 3;
  ADC1_CR1 = 0;
  ADC1_CR2 = ADC_ADON;

}

int ledArray[] = {
  4,
  1,
  5
};

char RGB[] = {
  'R',
  'B',
  'G'
};
int colourArray[] = {
  0,
  0,
  0
};
int colorPowers[] = {
  100,
  100,
  100
};

void delay(int x) {
  if (x == 500) {
    TIM4_ARR = DELAY_500;
  } else if (x == 100) {
    TIM4_ARR = DELAY_100;
  } else if (x == 10) {
    TIM4_ARR = DELAY_100 / 10;
  }

  TIM4_SR = 0;
  while ((TIM4_SR & TIM_UIF) == 0) {
    __asm("nop");
  }

}

void init_SM() {
  GPIOB_MODER = SET_BITS(GPIOB_MODER, 2 * RED_LED, 2, 0b10);
  GPIOB_AFRL = SET_BITS(GPIOB_AFRL, 4 * RED_LED, 4, 2);
  GPIOB_OTYPER &= ~(1 << RED_LED);

  GPIOB_MODER = SET_BITS(GPIOB_MODER, 2 * GREEN_LED, 2, 0b10);
  GPIOB_AFRL = SET_BITS(GPIOB_AFRL, 4 * GREEN_LED, 4, 2);
  GPIOB_OTYPER &= ~(1 << GREEN_LED);

  GPIOB_MODER = SET_BITS(GPIOB_MODER, 2 * BLUE_LED, 2, 0b10);
  GPIOB_AFRL = SET_BITS(GPIOB_AFRL, 4 * BLUE_LED, 4, 2);
  GPIOB_OTYPER &= ~(1 << BLUE_LED);
}

void init_TIM3() {

  TIM3_PSC = 9;
  TIM3_ARR = 21000;
  TIM3_CR1 = TIM_CEN | TIM_ARPE;

  //
  TIM3_CCMR1 = TIM_CCS1S_OUT | TIM_OC1M_PWM1;
  TIM3_CCER = TIM_CC1E;
  TIM3_CCR1 = 0;

  TIM3_CCMR1 |= TIM_CCS2S_OUT | TIM_OC2M_PWM1;
  TIM3_CCER |= TIM_CC2E;
  TIM3_CCR2 = 0;

  TIM3_CCMR2 |= TIM_CCS4S_OUT | TIM_OC4M_PWM1;
  TIM3_CCER |= TIM_CC4E;
  TIM3_CCR4 = 0;
  // TIM3_CCMR1 = TIM_CCS1S_OUT | TIM_OC1M_PWM1;
  // TIM3_CCER = TIM_CC1E;
}

void init_TIM4() {

  TIM4_PSC = WAIT_PSC;
  TIM4_ARR = DELAY_100;
  TIM4_EGR = TIM_UG;
  TIM4_SR = 0;
  //TIM4_CR1 = TIM_ARPE;
  TIM4_CR1 = 0;
  TIM4_CR1 = TIM4_CR1 | TIM_CEN;

}

void start_SM() {
  TIM3_CCR1 = TIM3_ARR;
}

void stop_SM() {
  TIM3_CCR1 = 0;
}

void set_R(int pulse) {
  TIM3_CCR1 = (pulse * TIM3_ARR / 100);
}
void set_G(int pulse) {
  TIM3_CCR2 = (pulse * TIM3_ARR / 100);
}
void set_B(int pulse) {
  TIM3_CCR4 = (pulse * TIM3_ARR / 100);
}

int handle_adc() {
  int val = 0;
  ADC1_CR2 |= ADC_SWSTART;
  while ((ADC1_SR & ADC_EOC) == 0) __asm("nop");
  val = ADC1_DR;

  return val;

}
void PWM_led_on(int i, int power) {
  power = 100 - power;
  if (i == 4) {
    set_R(power);
  } else if (i == 1) {
    set_B(power);
  } else if (i == 5) {
    set_G(power);
  }

}

void PWM_led_off(int i) {
  PWM_led_on(i, 0);
}
int get_reading(int count) {
  int sum = 0;
  for (int i = 0; i < count; i++) {
    int t = handle_adc();
    sum += t;

  }
  int f = sum / count;

  return f;
}

void printResults(int colourArray[]) {
  // printf("\n");
  // printf("==================================================================\n");
  for (int i = 0; i < 3; i++) {
    printf("XR1 %c val was %d \n", RGB[i], colourArray[i]);
  }
}

void Calibrate() {
  int maxVal[] = {
    0,
    0,
    0
  };
  for (int i = 0; i <= 2; i++) {
    PWM_led_on(ledArray[i], 100);
    delay(100);
    delay(100);
    int val = get_reading(100000);
    maxVal[i] = val;
    PWM_led_off(ledArray[i]);
  }
  int minVal = maxVal[0];
  int minValIndex = 0;
  for (int i = 1; i <= 2; i++) {
    if (maxVal[i] < minVal) {
      minVal = maxVal[i];
      minValIndex = i;
    }
  }
  int i = 0;

  printf("%c MIN VAL was %d \n", RGB[minValIndex], minVal);
  for (int i = 0; i <= 2; i++) {

    if (i == minValIndex) {
      colorPowers[i] = 100;
      continue;
    } else {
      int calibsum = 0;
      int valsum = 0;
      for(int h = 0; h < 5;h++){
        int val = 0;
        int max = 100;
        int min = 0;
        int c = 100;
      while (1) {

         c = (min + max) / 2;

        PWM_led_on(ledArray[i], c);
        delay(500);

        val = get_reading(100000);
        if (min == max || min == max - 1 || val == minVal){
          break;
        }

        if (val > minVal){
            printf("VAL WAS HIGHER THAN WE WANT, IT WAS %d,  at %d \n", val,c);
          max = c;
        } else{
          if(val < minVal){
              printf("VAL WAS LOWER THAN WE WANT, IT WAS %d,  at %d %  \n", val,c);

            min = c;
          }
        }
      }
      valsum += val;
      calibsum += c;
    }
    int val = valsum / 5;
    calibsum = calibsum / 5;
    colorPowers[i] = calibsum ;
      printf("Finished with %c : brightness %d ,  at %d , compred with min_brightness %d    \n", RGB[i], calibsum,val,minVal);
    }
    PWM_led_on(ledArray[i], 0);

  }

  // loop over and calib
}

int main() {
  RCC_AHB1ENR |= RCC_GPIOBEN;
  RCC_AHB1ENR |= RCC_GPIOAEN;
  RCC_APB1ENR |= RCC_TIM3EN;
  RCC_APB1ENR |= RCC_TIM4EN;
  RCC_APB2ENR |= RCC_ADC1EN;

  init_SM();
  init_TIM3();
  init_TIM4();
  init_ADC();

  set_G(100);
  set_R(100);
  set_B(100);
  Calibrate();

  while (1) {
    for (int j = 0; j < 3; j++) {
      // for (int i = 9; i < 100; i++) {
      PWM_led_on(ledArray[j], colorPowers[j]);
      //
      //   i += 9;
      // }
      delay(500);
      int val = get_reading(100000);

      //   for (int i = 0; i < 10;i++){
      //   delay(100);
      // }
      colourArray[j] = val;
      PWM_led_on(ledArray[j], 0);

    }
    printResults(colourArray);
  }

  // set_SM(70);
  // while (1) {
  //   __asm("nop");
  // }

  return 0;
}
