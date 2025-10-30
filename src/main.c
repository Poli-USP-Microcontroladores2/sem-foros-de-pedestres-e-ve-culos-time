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
#error "Unsupported board: led2 devicetree alias is not defined"
#endif

// 🔒 Mutex global para proteger o acesso aos LEDs
K_MUTEX_DEFINE(led_mutex);

int noturno =1;

void amarelo(void)
{
    while (1) {
        k_msleep(3000); // espera o "verde" ficar ligado por 3s


if(noturno ==1 ){ //modo noturno

while(1){
if(noturno ==0){
	break;
}
k_msleep(1000);
gpio_pin_toggle_dt(&led_2);
gpio_pin_toggle_dt(&led);

}

}


else{
        k_mutex_lock(&led_mutex, K_FOREVER);
        gpio_pin_toggle_dt(&led_2); // liga amarelo
        LOG_INF("Ligando amarelo");
        k_mutex_unlock(&led_mutex);

        k_msleep(5000); // amarelo ligado por 5s
}
    }
}

void verde(void)
{
    while (1) {

if(noturno ==1 ){ //modo noturno

	while(1){

}

}
        k_mutex_lock(&led_mutex, K_FOREVER);
        gpio_pin_toggle_dt(&led); // liga verde
        LOG_INF("Ligando verde");
        k_mutex_unlock(&led_mutex);

        k_msleep(8000); // verde ligado por 8s
    }
}

void vermelho(void)
{
    while (1) {
        k_msleep(4000); // espera antes de ligar vermelho

		if(noturno ==1 ){ //modo noturno

	while(1){

}
		}


        k_mutex_lock(&led_mutex, K_FOREVER);
        gpio_pin_toggle_dt(&led); // liga vermelho
        LOG_INF("Ligando vermelho");
        k_mutex_unlock(&led_mutex);

        k_msleep(4000);

        k_mutex_lock(&led_mutex, K_FOREVER);
        gpio_pin_toggle_dt(&led_2); // desliga amarelo
        k_mutex_unlock(&led_mutex);
    }
}

// Threads
K_THREAD_DEFINE(pisca_amarelo, 2048, amarelo, NULL, NULL, NULL, 0, 0, 0);
K_THREAD_DEFINE(pisca_verde, 2048, verde, NULL, NULL, NULL, 0, 0, 0);
K_THREAD_DEFINE(pisca_vermelho, 2048, vermelho, NULL, NULL, NULL, 0, 0, 0);

void main(void)
{
    int ret, ret1;

    // Verifica se os devices estão prontos
    if (!gpio_is_ready_dt(&led)) {
        printk("Error: LED device %s is not ready\n", led.port->name);
        return;
    }

    if (!gpio_is_ready_dt(&led_2)) {
        printk("Error: LED device %s is not ready\n", led_2.port->name);
        return;
    }

    // Configura pinos como saída
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
        k_msleep(1000); // mantém main viva
    }
}
