# Atividade dos semáforos de pedestres e veículos

### 1.1 Requisitos do sistema:
#### Requisitos gerais:
O sistema de pedestres deve estar sincronizado com o de carros.
#### Semáforo de pedestres:
- O sistema deve controlar dois leds, led0(verde) e led2(vermelho).
- O sistema deve alternar entre vermelho e verde continuamente com 4 segundos aceso em cada led.
- O sistema deve possuir um modo noturno que ao ser ativado, acendera o led vermelho por 1 segundo a cada segundo.
- O sistema deve suportar um botão de pedestres que, ao ser ativado enquanto o sinal estiver vermelho, acende o led verde e apaga o led vermelho.
- O sistema deve utilizar logs para verificação do funcionamento
#### Semáforo de carros:
- o sistema deve controlar 2 leds, inclusive um estado com os dois ligados: led0(verde, led2(vermelho, led0+led2(amarelo).
- o sistema deve alternar entre verde, amarelo e vermelho por 3 segundos, 1 segundo e 4 segundos, respectivamente.
- o sistema deve possuir um modo noturno, de tal forma que, quando ativado, faz o semáforo piscar 2 segundos(1 aceso, 1 apagado).
- o sistema deve suportar um botão de pedestre, que, quando ativado, automaticamente faz o semáforo ficar vermelho.
- o sistema deve utilizar logs para verificar funcionamento.
### 1.2 Teste de aceitação:
- Funcionamento dos semáforos sincronizados e com todas as implementações:

https://github.com/user-attachments/assets/924230dc-e3ef-42bf-a075-931eecf1e2e9

- Teste do modo noturno compartilhado:

### 2.1 Arquitetura do sistema
#### Semáforo de pedestres:
- Thread_verde deve controlar o funcionamento do led verde normalmente, no modo noturno e ao pressionar o botão de pedestres.
- Thread_vermelho deve controlar o funcionamento do led vermelho normalmente, no modo noturno e ao pressionar o botão de pedestres.
- Main deve ser responsável pela configuração do botão de pedestres.
- A ISR deve ser responsável por verificar se o botão foi pressionado.
#### Semáforo de carros:
- semáforo é utilizado para evitar condições de race condition.
- função botao pressionado avisa threads quando botão for ativado.
- thread verde inicia o ciclo, podendo ser interrompida caso botao pressionado, indo direto para vermelho, se não, vai pra amarelo(tem modo noturno, onde thread fica desabilitada)
- thread amarela controla o led amarelo, podendo ser interrompida com a ativação do botão, indo para o vermelho de qualquer modo( o modo noturno funciona nessa thread).
- thread vermelha controla o led vermelho, não pode ser interrompida, sendo a ultima parte do ciclo, retornando para verde9possui modo noturno que desabilita o funcionamento da thread).
- main é responsável por configurar leds e botão, além de adquirir o estado do botão, para ser usado na função botao pressionado.
### 2.2 Testes de sistema:
- Teste da sincronização do botão de pedestres com os semáforos:

https://github.com/user-attachments/assets/8970db8b-4cc3-4908-b54e-aa0fb874b5c5

### 3.1 Projeto de componentes
- Quatro jumpers serão necessários: um para conectar os terras das placas, outro para conectar os botões, um para pressionar os botões e outro para sincronização das placas.
### 3.2 Teste de integração

- Teste da sincronização dos semáforos:

https://github.com/user-attachments/assets/3c78e3e5-cf35-48c7-a66d-0da6bc750feb

### 4.1 Implementação/codificação
#### Semáforo de pedestres:
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
#define VERMELHO_LIGADO 4000 // Vermelho ficará aceso por 2 segundos

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

#### Semáforo de carros:

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
int noturno = 0;
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


### 4.2 Teste de unidade
#### Semáforo de pedestres:
- Teste do funcionamento dos leds:
<img width="890" height="107" alt="image1" src="https://github.com/user-attachments/assets/fdec272f-dc59-4649-bd4e-974267ca8fa1" />

https://github.com/user-attachments/assets/e10793e0-84b4-4064-900d-72118fdb3aaa

- Teste do funcionamento do modo noturno:
<img width="875" height="103" alt="image2" src="https://github.com/user-attachments/assets/bd180323-7b77-4d55-a7d7-caae7caca5c3" />

https://github.com/user-attachments/assets/804ac90a-1270-451a-99e6-40a59235c3e1

- Teste do funcionamento do botão de pedestres:
<img width="869" height="103" alt="image3" src="https://github.com/user-attachments/assets/0fda4089-2515-4103-80d9-ff4eb8a0c281" />

https://github.com/user-attachments/assets/879c80d3-79f9-479c-b4d3-5d8c186c942a

#### Semáforo de pedestres:
- funcionamento do semáforo;




https://github.com/user-attachments/assets/d350267c-2cac-4d79-b845-ea66ffc21353



  
- funcionamento modo noturno:



https://github.com/user-attachments/assets/cb822949-2e8d-4c5b-9294-8bb88fb9dbb4

