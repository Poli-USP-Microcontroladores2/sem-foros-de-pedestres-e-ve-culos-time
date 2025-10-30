#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(meu_modulo, LOG_LEVEL_INF);

#define SLEEP_TIME_MS 500

// Define o LED usando Device Tree
#define LED2_NODE DT_ALIAS(led2)
#define LED0_NODE DT_ALIAS(led0)

// Verifica se o LED está definido no Device Tree
#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#else
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

#if DT_NODE_HAS_STATUS(LED2_NODE, okay)
static const struct gpio_dt_spec led_2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
#else
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

void amarelo(){
	while(1){
	k_msleep(3000); //verde ligado por 3s
	gpio_pin_toggle_dt(&led_2); //liga amarelo
	LOG_INF("ligando amarelo");
	k_msleep(5000);
	}
}

void verde(){
while(1){
	gpio_pin_toggle_dt(&led); //liga verde
LOG_INF("ligando verde");
	k_msleep(8000); //verde ligado por 3s
}

}

void vermelho(){
	while(1){
	k_msleep(4000);
gpio_pin_toggle_dt(&led); //liga só vermelho após 4s(3s verde e 1s amarelo)
LOG_INF("ligando vermelho");
	k_msleep(4000);
	gpio_pin_toggle_dt(&led_2);
	}
}



K_THREAD_DEFINE(pisca_amarelo, 2048, amarelo, NULL, NULL, NULL, 0, 0, 0);
K_THREAD_DEFINE(pisca_verde, 2048, verde, NULL, NULL, NULL,0 , 0, 0);
K_THREAD_DEFINE(pisca_vermelho, 2048, vermelho, NULL, NULL, NULL, 0, 0, 0);


void main(void)
{
    int ret, ret1;

    // Verifica se o device está pronto
    if (!gpio_is_ready_dt(&led)) {
        printk("Error: LED device %s is not ready\n", led.port->name);
        return;
    }

	if (!gpio_is_ready_dt(&led_2)) {
        printk("Error: LED device %s is not ready\n", led.port->name);
        return;
    }



    // Configura o pino como saída
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Error %d: failed to configure LED pin\n", ret);
        return;
    }

	 ret1 = gpio_pin_configure_dt(&led_2, GPIO_OUTPUT_INACTIVE);
    if (ret1 < 0) {
        printk("Error %d: failed to configure LED pin\n", ret1);
        return;
    }



    printk("LED blinking on %s pin %d\n", led.port->name, led.pin);

    while (1) {
    }
}