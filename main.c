#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

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
#define ENCODER_BUTTON DPIN19
#define ENCODER_IN1_DDR DDRD
#define ENCODER_IN2_DDR DDRD
#define ENCODER_BUTTON_DDR DDRD
#define ENCODER_BUTTON_PORT PORTD
#define ENCODER_IN1_PORT PORTD
#define ENCODER_IN2_PORT PORTD
#define ENCODER_IN1_PIN PIND
#define ENCODER_IN2_PIN PIND
#define ENCODER_BUTTON_PIN PIND

#define BIT(x) (1 << (x))
#define GET_BIT(r,n) ((r >> n)&1)

void init_encoder_right();
void init_led();
void init_pwm_timer();
void pwm_tick(int flag);
void USART0_init(unsigned int baud_rate);
void USART0_print(const char * data);

static volatile int presc = 1;
static volatile int flag = 1;

int main(void) {
	cli();

	init_encoder_right();
	init_led();
	init_pwm_timer();
	USART0_init(F_CPU/16/(115200-1));

	sei();
	while(1) {

	}


	return 0;
}

void USART0_print(const char * data) {
	uint8_t i=0;
	uint8_t _strlen = strlen(data);	

	for(;i<_strlen;i++) {
		while(!GET_BIT(UCSR0A,UDRE0));
		UDR0 = data[i];
	}
}


void USART0_init(unsigned int baud_rate) {
	//unsigned ubrr = F_CPU/16/(baud_rate-1);
	
	UBRR0 = baud_rate;

	UCSR0B |= BIT(TXEN0);
}

void pwm_tick(int flag) {
	if((flag > 0) && (OCR0A+flag <= 0xff)) {
		OCR0A+=flag;
	} else if((flag < 0) && (OCR0A+flag >= 0x00)) {
		OCR0A+=flag;
	}
}

void init_encoder_right() {
	//Setting DDR for encoder pins to input mode (12 and 11 pins)
	ENCODER_IN1_DDR &= ~(BIT(ENCODER_IN1));
	ENCODER_IN2_DDR &= ~(BIT(ENCODER_IN2));
	
	//Pullups to '1'
	ENCODER_IN1_PORT |= BIT(ENCODER_IN1);
	ENCODER_IN2_PORT |= BIT(ENCODER_IN2);

	//Enabling interrupt for INT0/Digital pin 21
	EICRA |= BIT(ISC00) | BIT(ISC01); //Interrupt on rising edge
	EIMSK |= BIT(INT0) | BIT(INT2);

	//Init button
	EICRA |= BIT(ISC20) | BIT(ISC21);
	ENCODER_BUTTON_DDR &= ~(BIT(ENCODER_BUTTON));
	ENCODER_BUTTON_PORT |= BIT(ENCODER_BUTTON);
}

ISR(INT0_vect) { //interrupt to handle encoder rotate
	cli();

	int in1_cur = GET_BIT(ENCODER_IN1_PIN,ENCODER_IN1);
	int in2_cur = GET_BIT(ENCODER_IN2_PIN,ENCODER_IN2);
	char * buff = (char*)(malloc(sizeof(char)*0xff));

	if(in1_cur != in2_cur) {
		pwm_tick(5);
	} else {
		pwm_tick(-5);
	}
	//USART0_print(itoa(OCR0A,buff,10));
	free(buff);
	//USART0_print("\n");
	//_delay_ms(5);
	sei();
}

ISR(INT2_vect) {
	if(GET_BIT(EIMSK,INT0)) {
		EIMSK &= ~(BIT(INT0));
		OCR0A = 0;
	} else {
		EIMSK |= BIT(INT0);
		OCR0A = 0xff;
	}
	//USART0_print("int2_vect interrupt!\n");
	_delay_ms(500);
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
	OCR0A = 0;
}
