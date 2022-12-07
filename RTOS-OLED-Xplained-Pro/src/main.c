#include <asf.h>
#include "conf_board.h"

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* prototypes and types                                                 */
/************************************************************************/
int genius_get_sequence(int level, int *sequence);
void pin_toggle(Pio *pio, uint32_t mask);
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq) ;

/************************************************************************/
/* RTOS application funcs                                               */
/************************************************************************/
#define TASK_OLED_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_OLED_STACK_PRIORITY            (tskIDLE_PRIORITY)

#define LED_1_PIO PIOA
#define LED_1_PIO_ID ID_PIOA
#define LED_1_IDX 0
#define LED_1_IDX_MASK (1 << LED_1_IDX)

#define LED_2_PIO PIOC
#define LED_2_PIO_ID ID_PIOC
#define LED_2_IDX 30
#define LED_2_IDX_MASK (1 << LED_2_IDX)

#define LED_3_PIO PIOB
#define LED_3_PIO_ID ID_PIOB
#define LED_3_IDX 2
#define LED_3_IDX_MASK (1 << LED_3_IDX)


#define BUT_11_PIO PIOD
#define BUT_11_PIO_ID ID_PIOD
#define BUT_11_IDX 30
#define BUT_11_IDX_MASK (1u << BUT_11_IDX)


#define BUT_1_PIO PIOD
#define BUT_1_PIO_ID ID_PIOD
#define BUT_1_IDX 28
#define BUT_1_IDX_MASK (1u << BUT_1_IDX)

#define BUT_2_PIO PIOA
#define BUT_2_PIO_ID ID_PIOA
#define BUT_2_IDX 19
#define BUT_2_IDX_MASK (1u << BUT_2_IDX)

#define BUT_3_PIO PIOC
#define BUT_3_PIO_ID ID_PIOC
#define BUT_3_IDX 31
#define BUT_3_IDX_MASK (1u << BUT_3_IDX)


QueueHandle_t xQueueBut;
volatile int vigia = 0;

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

void TC0_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC0, 0);
	pin_toggle(BUT_11_PIO,BUT_11_IDX_MASK);
	
	

}


void but1_callback(void) {
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	int i = 0;
	vigia++;
	xQueueSendFromISR(xQueueBut, &i, &xHigherPriorityTaskWoken);
}

void but2_callback(void) {
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	int i = 1;
	vigia++;
	xQueueSendFromISR(xQueueBut, &i, &xHigherPriorityTaskWoken);
}

void but3_callback(void) {
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	int i = 2;
	vigia++;
	xQueueSendFromISR(xQueueBut, &i, &xHigherPriorityTaskWoken);
}




/************************************************************************/
/* TASKS                                                                */
/************************************************************************/
static void task_game(void *pvParameters) {
	gfx_mono_ssd1306_init();
	gfx_mono_draw_string("Level: 10", 0, 0, &sysfont);
	int level = 0;
	int sequencia[512];
	char str[100];
	
	pmc_enable_periph_clk(LED_1_PIO_ID);
	pmc_enable_periph_clk(LED_2_PIO_ID);
	pmc_enable_periph_clk(LED_3_PIO_ID);
	pmc_enable_periph_clk(BUT_11_PIO_ID);
	pmc_enable_periph_clk(BUT_1_PIO_ID);
	pmc_enable_periph_clk(BUT_2_PIO_ID);
	pmc_enable_periph_clk(BUT_3_PIO_ID);

	
	
	
	pio_configure(LED_1_PIO, PIO_OUTPUT_0, LED_1_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED_2_PIO, PIO_OUTPUT_0, LED_2_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED_3_PIO, PIO_OUTPUT_0, LED_3_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT_11_PIO, PIO_OUTPUT_0, BUT_11_IDX_MASK, PIO_DEFAULT);
	
	
	
	pio_configure(BUT_1_PIO, PIO_INPUT, BUT_1_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	pio_configure(BUT_2_PIO, PIO_INPUT, BUT_2_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	pio_configure(BUT_3_PIO, PIO_INPUT, BUT_3_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);

	pio_handler_set(BUT_1_PIO, BUT_1_PIO_ID, BUT_1_IDX_MASK, PIO_IT_FALL_EDGE,
	but1_callback);
	pio_handler_set(BUT_2_PIO, BUT_2_PIO_ID, BUT_2_IDX_MASK, PIO_IT_FALL_EDGE,
	but3_callback);
	pio_handler_set(BUT_3_PIO, BUT_3_PIO_ID, BUT_3_IDX_MASK, PIO_IT_FALL_EDGE,
	but2_callback);

	pio_enable_interrupt(BUT_1_PIO, BUT_1_IDX_MASK);
	pio_enable_interrupt(BUT_2_PIO, BUT_2_IDX_MASK);
	pio_enable_interrupt(BUT_3_PIO, BUT_3_IDX_MASK);

	pio_get_interrupt_status(BUT_1_PIO);
	pio_get_interrupt_status(BUT_2_PIO);
	pio_get_interrupt_status(BUT_3_PIO);

	NVIC_EnableIRQ(BUT_1_PIO_ID);
	NVIC_SetPriority(BUT_1_PIO_ID, 4);

	NVIC_EnableIRQ(BUT_2_PIO_ID);
	NVIC_SetPriority(BUT_2_PIO_ID, 4);

	NVIC_EnableIRQ(BUT_3_PIO_ID);
	NVIC_SetPriority(BUT_3_PIO_ID, 4);
	
	pio_set(LED_1_PIO,LED_1_IDX_MASK);
	pio_set(LED_2_PIO,LED_2_IDX_MASK);
	pio_set(LED_3_PIO,LED_3_IDX_MASK);


	for (;;)  {
		sprintf(str, "level : %d  ", level);
		gfx_mono_draw_string(str, 0, 0, &sysfont);
		
		int n = genius_get_sequence(level,sequencia);
		for(int j=0;j<n;j++){
			if(vigia > 0){
				
				gfx_mono_draw_string("perdeu   ", 0, 0, &sysfont);
				level = 0;
				vigia = 0;
				break;
			}
			if(sequencia[j] == 0){
				//printf("entrei");
				TC_init(TC0, ID_TC0, 0, 1000);
				tc_start(TC0, 0);
				//printf("sai");
				pio_clear(LED_1_PIO,LED_1_IDX_MASK);
				//delay_ms(1000);
				vTaskDelay(500);
				pio_set(LED_1_PIO,LED_1_IDX_MASK);
				tc_stop(TC0,0);
				vTaskDelay(100);
			}
			else if(sequencia[j] == 1){
				TC_init(TC0, ID_TC0, 0, 1500);
				tc_start(TC0, 0);
				pio_clear(LED_2_PIO,LED_2_IDX_MASK);
				//delay_ms(1000);
				vTaskDelay(500);
				pio_set(LED_2_PIO,LED_2_IDX_MASK);
				tc_stop(TC0,0);
				vTaskDelay(100);
			}
			else if(sequencia[j] == 2){
				TC_init(TC0, ID_TC0, 0, 2000);
				tc_start(TC0, 0);
				pio_clear(LED_3_PIO,LED_3_IDX_MASK);
				//delay_ms(1000);
				vTaskDelay(500);
				pio_set(LED_3_PIO,LED_3_IDX_MASK);
				tc_stop(TC0,0);
				vTaskDelay(100);
				
			}
		}
		//printf("n:%d\n",n);
		//printf("s0:%d\n",sequencia[0]);
		//printf("s1:%d\n",sequencia[1]);
		//printf("s2:%d\n",sequencia[2]);
		//pd30
		int tentando = 1;
		int t;
		int p = 0;
		int contagem = 0;
		int vetor[100];
		
		while(tentando){
			if (xQueueReceive(xQueueBut, &t, (TickType_t) 1200)){
				printf("apertado: %d\n",t);
				
				
				
				if(t == 0){
					TC_init(TC0, ID_TC0, 0, 1000);
					tc_start(TC0, 0);
					pio_clear(LED_1_PIO,LED_1_IDX_MASK);
					vTaskDelay(100);
					pio_set(LED_1_PIO,LED_1_IDX_MASK);
					tc_stop(TC0,0);
				}
				else if(t == 1){
					TC_init(TC0, ID_TC0, 0, 1500);
					tc_start(TC0, 0);
					pio_clear(LED_2_PIO,LED_2_IDX_MASK);
					vTaskDelay(100);
					pio_set(LED_2_PIO,LED_2_IDX_MASK);
					tc_stop(TC0,0);
				}
				else if(t == 2){
					TC_init(TC0, ID_TC0, 0, 2000);
					tc_start(TC0, 0);
					pio_clear(LED_3_PIO,LED_3_IDX_MASK);
					vTaskDelay(100);
					pio_set(LED_3_PIO,LED_3_IDX_MASK);
					tc_stop(TC0,0);
					
				}
				
				
				
				
				
				
				
				
				
				
				
				contagem++;
				if(contagem>n){
					vigia=0;
					break;
				}
				if(t == sequencia[p]){
					p++;
					if(contagem == n){
						tentando = 0;
						vigia=0;
						level++;
						vTaskDelay(1000);
					}
					
				}
				else{
					tentando = 0;
					printf("sequncia: %d, t : %d, p: %d",sequencia[p],t,p);
					gfx_mono_draw_string("perdeu   ", 0, 0, &sysfont);
					vTaskDelay(500);
					level = 0;
					vigia=0;
				
					
				}
				
				
				
				
				
			}
			else{
				tentando = 0;
				gfx_mono_draw_string("demorou   ", 0, 0, &sysfont);
				level = 0;
				vigia=0;
				vTaskDelay(500);
				
			}
			
		}
	
		
	}
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/
int genius_get_sequence(int level, int *sequence){
	int n = level + 3;

	for (int i=0; i< n ; i++) {
		*(sequence + i) = rand() % 3;
	}

	return n;
}

void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq) {
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS,
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main(void) {
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	configure_console();
	xQueueBut = xQueueCreate(32, sizeof(int) );

	/* Create task to control oled */
	if (xTaskCreate(task_game, "game", TASK_OLED_STACK_SIZE, NULL, TASK_OLED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create game task\r\n");
	}

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* RTOS não deve chegar aqui !! */
	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
