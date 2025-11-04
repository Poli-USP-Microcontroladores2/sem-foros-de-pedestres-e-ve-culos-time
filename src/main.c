#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(meu_modulo, LOG_LEVEL_INF);

// === CONFIGURAÇÕES DE HARDWARE ===
#define LED2_NODE DT_ALIAS(led2)
#define LED0_NODE DT_ALIAS(led0)
#define BUTTON_NODE DT_NODELABEL(user_button_0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
static const struct gpio_dt_spec led_verde = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#else
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

#if DT_NODE_HAS_STATUS(LED2_NODE, okay)
static const struct gpio_dt_spec led_vermelho = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
#else
#error "Unsupported board: led2 devicetree alias is not defined"
#endif

// === SEMÁFOROS ===
K_SEM_DEFINE(sem_verde, 1, 1);     
K_SEM_DEFINE(sem_amarelo, 0, 1);
K_SEM_DEFINE(sem_vermelho, 0, 1);
int noturno = 1;
// Função que retorna 1 se botão estiver pressionado
static inline int botao_pressionado(void) {
    return gpio_pin_get_dt(&button); // ativo baixo
}

// === THREAD VERDE ===
void verde(void)
{
    while (1) {
        k_sem_take(&sem_verde, K_FOREVER);
if (noturno == 1) {
    k_sem_give(&sem_amarelo);
while(1){

}
}
        LOG_INF("🟢 Verde ligado");
        gpio_pin_set_dt(&led_verde, 1);

        for (int i = 0; i < 30; i++) { // 3s máximo
            if (botao_pressionado()) {
                LOG_INF("🟢 Verde interrompido pelo botão");
                break;
            }
            k_msleep(100);
        }

        gpio_pin_set_dt(&led_verde, 0);
        k_sem_give(&sem_amarelo);
    }
}

// === THREAD AMARELO ===
void amarelo(void)
{
    while (1) {
        k_sem_take(&sem_amarelo, K_FOREVER);

 if (noturno == 1) {
            while (1) {
                if (noturno == 0)
                    break;
                k_msleep(1000);
                gpio_pin_toggle_dt(&led_verde);
                gpio_pin_toggle_dt(&led_vermelho);
            }
        }

        LOG_INF("🟡 Amarelo ligado");

        for (int i = 0; i < 10; i++) { // 1s total
            gpio_pin_set_dt(&led_verde, 1);
            gpio_pin_set_dt(&led_vermelho, 1);

            if (botao_pressionado()) {
                LOG_INF("🟡 Amarelo interrompido pelo botão");
                break; // vai direto para vermelho
            }
            k_msleep(100);
        }

        gpio_pin_set_dt(&led_verde, 0);
        gpio_pin_set_dt(&led_vermelho, 0);

        k_sem_give(&sem_vermelho);
    }
}

// === THREAD VERMELHO ===
void vermelho(void)
{
    while (1) {
        k_sem_take(&sem_vermelho, K_FOREVER);
if (noturno == 1) {
while(1){

}
}
        LOG_INF("🔴 Vermelho ligado");
        gpio_pin_set_dt(&led_vermelho, 1);

        for (int i = 0; i < 40; i++) { // 4s total
            k_msleep(100);
        }

        gpio_pin_set_dt(&led_vermelho, 0);
        LOG_INF("🔴 Vermelho desligado");

        k_sem_give(&sem_verde);
    }
}

// === THREADS ===
K_THREAD_DEFINE(thread_verde, 2048, verde, NULL, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(thread_amarelo, 2048, amarelo, NULL, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(thread_vermelho, 2048, vermelho, NULL, NULL, NULL, 1, 0, 0);

// === MAIN ===
void main(void)
{
    printk("=== Sistema de Semáforo com Botão ===\n");

    if (!gpio_is_ready_dt(&button)) {
        printk("ERRO: Botão não está disponível!\n");
        return;
    }
    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);

    if (!gpio_is_ready_dt(&led_verde) || !gpio_is_ready_dt(&led_vermelho)) {
        printk("Erro: LED não está pronto\n");
        return;
    }

    int ret = gpio_pin_configure_dt(&led_verde, GPIO_OUTPUT_INACTIVE);
    ret |= gpio_pin_configure_dt(&led_vermelho, GPIO_OUTPUT_INACTIVE);

    if (ret < 0) {
        printk("Erro ao configurar LEDs\n");
        return;
    }

    printk("🚦 Sistema de semáforos iniciado!\n");

    while (1) {
        int estado = botao_pressionado();
        printk("Botão: %s\n", estado ? "PRESSIONADO" : "SOLTO");
        k_sleep(K_MSEC(300));
    }
}
