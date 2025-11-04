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
#define VERDE_LIGADO 4000    // Verde ficará aceso por 4 segundos
#define VERMELHO_LIGADO 2000 // Vermelho ficará aceso por 2 segundos

/* Device-tree LED specs */
static const struct gpio_dt_spec led_verde = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_vermelho = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec botao = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback botao_data;
/* --- Sincronização com Semáforos --- */


K_SEM_DEFINE(semaforo_verde, 0, 1);
K_SEM_DEFINE(semaforo_botao, 0, 1);
K_SEM_DEFINE(semaforo_vermelho, 1, 1);
int MODO_NOTURNO = 0;
void botao_pressionado(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    /* Apenas "dá" o semáforo. A thread que o espera vai acordar. */
    /* É seguro chamar k_sem_give de dentro de uma ISR */
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

    /* --- Configuração (feita pela Thread A) --- */
    if (!gpio_is_ready_dt(&led_verde) || !gpio_is_ready_dt(&led_vermelho))
    {
        LOG_INF("Erro: LED nao esta pronto\n");
        return;
    }
    ret = gpio_pin_configure_dt(&led_verde, GPIO_OUTPUT_LOW);
    ret |= gpio_pin_configure_dt(&led_vermelho, GPIO_OUTPUT_LOW);
    if (ret < 0)
    {
        LOG_INF("Erro: Falha ao configurar LEDs\n");
        return;
    }

    LOG_INF("Thread A (Verde) iniciada e aguardando.");

    while (1)
    {
        k_sem_take(&semaforo_verde, K_FOREVER);
        if (MODO_NOTURNO == 0)
        {
            /* 1. Espera pela sua vez (k_sem_take) */
            

            /* 2. Realiza sua ação */
            gpio_pin_set_dt(&led_verde, 1);
            gpio_pin_set_dt(&led_vermelho, 0);

            /* 3. Espera o tempo do LED aceso */
            k_msleep(VERDE_LIGADO); // Espera 4 segundos

            /* 4. Passa o bastão para a Thread B (k_sem_give) */
           
            
        }
        else
        {
            gpio_pin_set_dt(&led_verde, 0);
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

    if (!gpio_is_ready_dt(&led_vermelho))
    {
        return; /* A Thread A já logou o erro se houver */
    }

    LOG_INF("Thread B (Vermelho) iniciada.");

    while (1)
    {
        k_sem_take(&semaforo_vermelho, K_FOREVER);
        if (MODO_NOTURNO == 0)
        {
            k_sem_reset(&semaforo_botao);
            /* 1. Espera pela sua vez (k_sem_take) */
            /* Esta linha bloqueia a thread até a Thread A liberar */
            /* 2. Realiza sua ação */
            gpio_pin_set_dt(&led_verde, 0);
            gpio_pin_set_dt(&led_vermelho, 1);
            k_sem_take(&semaforo_botao, K_MSEC(VERMELHO_LIGADO));
        }
        else
        {   
            gpio_pin_set_dt(&led_verde, 0);
            gpio_pin_toggle_dt(&led_vermelho);
            k_msleep(1000);
        }
        k_sem_give(&semaforo_verde);
    }
}

K_THREAD_DEFINE(thread_a_id, STACK_SIZE, thread_verde,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(thread_b_id, STACK_SIZE, thread_vermelho,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

void main(void)
{
    gpio_pin_configure_dt(&botao, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_interrupt_configure_dt(&botao, GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&botao_data, botao_pressionado, BIT(botao.pin));
    gpio_add_callback(botao.port, &botao_data);
    LOG_INF("Main terminou. Threads com Semáforos (4s/2s) estao rodando.");
}