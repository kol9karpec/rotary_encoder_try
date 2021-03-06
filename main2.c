#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LED_BIT 7
#define LED_DDR DDRB
#define LED_PORT PORTB


#define DPIN12 6
#define DPIN11 5
#define DPIN21 INT0
#define DPIN20 INT1
#define DPIN19 INT2

#define ENCODER_IN1 DPIN21
#define ENCODER_IN2 DPIN20
#define ENCODER_IN1_DDR DDRD
#define ENCODER_IN2_DDR DDRD
#define ENCODER_IN1_PORT PORTD
#define ENCODER_IN2_PORT PORTD
#define ENCODER_IN1_PIN PIND
#define ENCODER_IN2_PIN PIND

#define BIT(x) (1 << (x))
#define GET_BIT(r,n) ((r >> n)&1)

void init_encoder_right();
void init_led();
void init_pwm_timer();
void pwm_tick(uint8_t flag);

volatile int presc = 1;
volatile int flag = 1;

int main(void) {
	cli();

	init_encoder_right();
	init_led();
	init_pwm_timer();

	sei();
	while(1) {

	}


	return 0;
}

void pwm_tick(uint8_t flag) {
	if((flag == 1) && (OCR0A != 0xff)) {
		OCR0A++;
	} else if(OCR0A != 0x00) {
		OCR0A--;
	}
}

void init_encoder_right() {
	//Setting DDR for encoder pins to input mode (12 and 11 pins)
	ENCODER_IN1_DDR &= ~(BIT(ENCODER_IN1));
	ENCODER_IN2_DDR &= ~(BIT(ENCODER_IN2));
	
	//Pullups to '0'
	ENCODER_IN1_PORT &= ~(BIT(ENCODER_IN1));
	ENCODER_IN2_PORT &= ~(BIT(ENCODER_IN2));

	//Enabling interrupt for INT0/Digital pin 21
	EICRA |= BIT(ISC00) | BIT(ISC01); //Interrupt on rising edge
	EIMSK |= BIT(INT0);
}

ISR(PCINT0_vect) { //interrupt to handle encoder rotate
	int in1_cur = GET_BIT(ENCODER_IN1_PIN,ENCODER_IN1);
	int in2_cur = GET_BIT(ENCODER_IN2_PIN,ENCODER_IN2);

	if(in1_cur == in2_cur) {
		pwm_tick(1);
	} else {
		pwm_tick(-1);
	}
}

void init_led() {
	LED_DDR |= BIT(LED_BIT);
	LED_PORT |= BIT(LED_BIT);
}

void init_pwm_timer() {
	//Setting up TIMER0 to PWM output
	TCCR0A |= BIT(COM0A0); //Toggle OC0A on compare match
	
	//Fast PWM mode with TOP = OCRA
	TCCR0A |= BIT(WGM00) | BIT(WGM01);
	TCCR0B |= BIT(WGM02);
	
	//Setting prescaler
	TCCR0B |= BIT(CS00); //prescaler = 1

	//Output Compare Match value
	OCR0A = 1;
}
