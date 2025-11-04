# Atividade dos semáforos de pedestres e veículos

## Parte do Guilherme:
Desenvolvi o semáforo de pedestres por partes para facilitar os testes de funcionamento.
A primeira coisa que desenvolvi foi código responsável por acender os leds alternadamente entre vermelho e verde por 4 e 2 segundos, respectivamente.
Abaixo estão os logs e video do funcionamento:

https://github.com/user-attachments/assets/5c3b0fde-9ecb-440e-b60b-4d25283910c5

<img width="908" height="113" alt="image3" src="https://github.com/user-attachments/assets/ed680df1-d8a8-472d-bca9-e67100b3e473" />



## Trabalho em Dupla
- A atividade deve ser realizada **em duplas**.
- Cada membro deve contribuir ativamente no desenvolvimento.
- **Commits no repositório Git** devem ser feitos de forma individual, permitindo a avaliação das contribuições de cada aluno.

## Descrição do Sistema
O sistema deve controlar semáforos de dois tipos: pedestres e carros, incluindo **modo noturno** e **botão de travessia de pedestres**.

### 1. Semáforo de Pedestres
- Contém dois LEDs: verde e vermelho.
- Comportamento:
  - Verde acende por 4 segundos.
  - Vermelho acende por 2 segundos.
- Controle deve ser feito por **duas threads independentes**, garantindo exclusão mútua (**mutex**) entre verde e vermelho.
- Deve ser utilizado o microcontrolador de um integrante.

### 2. Semáforo de Veículos
- Contém três LEDs: verde, amarelo e vermelho.
- Comportamento:
  - Verde acende por 3 segundos.
  - Amarelo acende por 1 segundo.
  - Vermelho acende por 4 segundos.
- Controle deve ser feito por **três threads independentes**, garantindo exclusão mútua (**mutex**) entre os LEDs.
- Deve ser validado que há **sincronismo** entre o semáforo de pedestres e o semáforo de veículos.
- Deve ser utilizado o microcontrolador do outro integrante.

### 3. Modo Noturno
- Um modo alternativo em que os semáforos piscam:
  - Carros: amarelo piscando a cada 2 segundos (1 segundo aceso, 1 segundo apagado).
  - Pedestres: vermelho piscando a cada 2 segundos (1 segundo aceso, 1 segundo apagado).

### 4. Botão de Travessia
- Permite que pedestres acionem o semáforo:
  - Pedestre verde é ativado.
  - Semáforo de carros é bloqueado de forma segura.
- O sistema deve suportar uma única via
- Controle de acesso deve garantir **consistência entre semáforos**, evitando conflito de LEDs.

## Requisitos de Projeto
1. Cada LED deve ser controlado por **uma thread**.
2. Threads devem utilizar **mutex** para evitar conflitos.
3. Modos de operação:
   - Normal (dia)
   - Noturno
   - Travessia acionada pelo botão
4. **Validação do código**
   - Alunos devem elaborar **planos de testes completos**, seguindo o **modelo V**, para garantir o correto funcionamento de cada parte e do sistema completo.
   - Testes devem incluir:
     - Testes unitários (cada semáforo)
     - Testes de integração (interação entre semáforos)
     - Testes de sistema (modos noturno e botão de travessia)
6. **Uso de IA generativa**
   - Sugestão de uso para auxiliar na geração de código ou testes.
   - A avaliação será baseada **na qualidade dos testes e nas evidências registradas no repositório**, não apenas na implementação do código.
7. **Controle de contribuições individuais**
   - Cada membro da dupla deve realizar **commits separados** no repositório.
   - A avaliação individual considerará o histórico de commits e a participação de cada aluno no desenvolvimento e validação do sistema.
