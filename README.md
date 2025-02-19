# **Projeto de Controle de LEDs e Display com Joystick - RP2040 | BitDogLab**  

## **Vídeo de Demonstração**  
🎥 Link do vídeo de demonstração: https://youtu.be/q7QFI4zNBVQ 

## **Descrição**  
Este projeto foi desenvolvido por mim, **Hilquias Rodrigues de Oliveira**, com o objetivo de demonstrar o uso do **Conversor Analógico-Digital (ADC)** no microcontrolador **RP2040**, além do controle de **LEDs RGB via PWM** e a utilização do **display SSD1306 via I2C**. O projeto foi implementado na **placa de desenvolvimento BitDogLab** e utiliza um **joystick analógico** para ajustar o brilho dos LEDs e movimentar um quadrado no display.  

Os botões do joystick e da placa são gerenciados por **interrupções (IRQ)** e contam com **debouncing via software** para garantir uma resposta precisa.  

## **Funcionalidades**  
✅ **Controle de LEDs RGB via Joystick:**  
- **LED Azul**: brilho ajustado pelo **eixo Y** do joystick.  
- **LED Vermelho**: brilho ajustado pelo **eixo X** do joystick.  
- Ambos são controlados por **PWM**, permitindo transições suaves de brilho.  

✅ **Movimentação de um quadrado no display SSD1306 (128x64)**  
- O **quadrado de 8x8 pixels** se move proporcionalmente aos valores do joystick.  
- Comunicação via **I2C** para exibição gráfica.  

✅ **Botão do Joystick (GPIO 22):**  
- **Alterna o estado do LED Verde.**  
- **Modifica a borda do display** entre diferentes estilos ao ser pressionado.  

✅ **Botão A (GPIO 5):**  
- **Ativa ou desativa os LEDs RGB controlados por PWM.**  

## **Componentes Utilizados**  
🟢 **LED RGB** (GPIOs 11, 12, 13)  
🎮 **Joystick Analógico** (ADC nos GPIOs 26 e 27)  
🟩 **Botão do Joystick** (GPIO 22)  
🅰 **Botão A** (GPIO 5)  
📟 **Display SSD1306 OLED (I2C - GPIOs 14 e 15)**  

## **Aspectos do Projeto**  
✔️ Implementação de **interrupções (IRQ) para os botões**  
✔️ **Debouncing via software** para evitar leituras falsas  
✔️ **Controle de LEDs via PWM**  
✔️ **Conversão A/D (ADC) para capturar valores do joystick**  
✔️ **Comunicação I2C** com o display OLED  
✔️ **Código bem estruturado e comentado**  

## **Como Executar**  
⚙️ **Pré-requisitos:**  
- **Pico SDK** instalado  
- **Extensões Raspberry Pi Pico, CMake e C/C++** no **VSCode**  

📌 **Passos:**  
1️⃣ Clone este repositório e abra a pasta do projeto no VSCode.  
2️⃣ A extensão **Pi Pico** criará automaticamente a pasta `build`.  
3️⃣ Clique em **Compile** na barra inferior do VSCode.  
4️⃣ Verifique se o arquivo **.uf2** foi gerado corretamente na pasta `build`.  
5️⃣ Conecte a **BitDogLab** via USB e coloque-a em **modo BOOTSEL**.  
6️⃣ Arraste o arquivo **.uf2** para a unidade de armazenamento da placa.  
7️⃣ O código será carregado e executado automaticamente.  

🎮 **Interaja com o sistema:**  
- Mova o **joystick** para controlar o brilho dos LEDs RGB e a posição do quadrado no display.  
- Pressione o **botão do joystick** para alternar o **LED Verde** e modificar a borda do display.  
- Pressione o **Botão A** para **ativar ou desativar o controle PWM dos LEDs**.  
