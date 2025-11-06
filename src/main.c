#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

#define BUTTON_NODE DT_NODELABEL(user_button_0)

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define STACK_SIZE 1024
#define PRIORITY 5

/* --- Tempos de espera específicos --- */
#define VERDE_LIGADO 4000    // Verde aceso por 4 segundos
#define VERMELHO_LIGADO 4000 // Vermelho aceso por 4 segundos

/* --- GPIOs definidos no device tree --- */
static const struct gpio_dt_spec led_verde = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_vermelho = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec botao = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

/* --- Porta e pino de envio (PTB1) --- */
#define PORTA_ENVIO_NODE DT_NODELABEL(gpiob)
#define PINO_ENVIO_NUM   1
static const struct device *porta_envio;

/* --- Callback e semáforos --- */
static struct gpio_callback botao_data;
K_SEM_DEFINE(semaforo_verde, 0, 1);
K_SEM_DEFINE(semaforo_botao, 0, 1);
K_SEM_DEFINE(semaforo_vermelho, 1, 1);

int MODO_NOTURNO = 1;

void botao_pressionado(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    k_sem_give(&semaforo_botao);
    LOG_INF("Botão pressionado!");
}

/* Thread A: Controla o LED Verde */
void thread_verde(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    int ret;

    if (!gpio_is_ready_dt(&led_verde) || !gpio_is_ready_dt(&led_vermelho) || !device_is_ready(porta_envio)) {
        LOG_INF("Erro: GPIO não está pronto");
        return;
    }

    ret = gpio_pin_configure_dt(&led_verde, GPIO_OUTPUT_LOW);
    ret |= gpio_pin_configure_dt(&led_vermelho, GPIO_OUTPUT_LOW);
    ret |= gpio_pin_configure(porta_envio, PINO_ENVIO_NUM, GPIO_OUTPUT_LOW);
    if (ret < 0) {
        LOG_INF("Erro: Falha ao configurar GPIOs");
        return;
    }

    LOG_INF("Thread A (Verde) iniciada e aguardando.");

    while (1)
    {
        k_sem_take(&semaforo_verde, K_FOREVER);
        if (MODO_NOTURNO == 0)
        {
            /* Liga verde, apaga vermelho */
            gpio_pin_set_dt(&led_verde, 1);
            gpio_pin_set_dt(&led_vermelho, 0);

            /* Envia sinal HIGH = verde aceso */
            gpio_pin_set(porta_envio, PINO_ENVIO_NUM, 1);

            k_msleep(VERDE_LIGADO);
        }
        else
        {
            gpio_pin_set_dt(&led_verde, 0);
            gpio_pin_set(porta_envio, PINO_ENVIO_NUM, 0);
        }
        k_sem_give(&semaforo_vermelho);
    }
}

/* Thread B: Controla o LED Vermelho */
void thread_vermelho(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    if (!gpio_is_ready_dt(&led_vermelho)) {
        return;
    }

    LOG_INF("Thread B (Vermelho) iniciada.");

    while (1)
    {
        k_sem_take(&semaforo_vermelho, K_FOREVER);
        if (MODO_NOTURNO == 0)
        {
            k_sem_reset(&semaforo_botao);

            /* Liga vermelho, apaga verde */
            gpio_pin_set_dt(&led_verde, 0);
            gpio_pin_set_dt(&led_vermelho, 1);

            /* Envia sinal LOW = vermelho aceso */
            gpio_pin_set(porta_envio, PINO_ENVIO_NUM, 0);

            k_sem_take(&semaforo_botao, K_MSEC(VERMELHO_LIGADO));
        }
        else
        {
            gpio_pin_set_dt(&led_verde, 0);
            gpio_pin_toggle_dt(&led_vermelho);
            gpio_pin_set(porta_envio, PINO_ENVIO_NUM, 0); // Modo noturno: sempre LOW
            LOG_INF("Modo noturno funcionando!");
            k_msleep(1000);
        }
        k_sem_give(&semaforo_verde);
    }
}

/* Threads */
K_THREAD_DEFINE(thread_a_id, STACK_SIZE, thread_verde,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(thread_b_id, STACK_SIZE, thread_vermelho,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

void main(void)
{

    porta_envio = DEVICE_DT_GET(PORTA_ENVIO_NODE);

    /* Configura botão */
    gpio_pin_configure_dt(&botao, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_interrupt_configure_dt(&botao, GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&botao_data, botao_pressionado, BIT(botao.pin));
    gpio_add_callback(botao.port, &botao_data);

    LOG_INF("Main terminou. Threads e sinal via PTB1 estão rodando.");
}
