# ESP32-FFT_AudioVisualizer

Projeto de um sistema embarcado que representa graficamente, em tempo real, o conteúdo espectral de sons captados do ambiente. Desenvolvido como parte de um trabalho acadêmico sobre a aplicação das Transformadas de Fourier e de Laplace no processamento de sinais.

📌 Descrição

O sistema utiliza um microfone analógico, filtro passa-baixa e microcontrolador ESP32, que coleta, amostra e processa o sinal de áudio. Por meio da Transformada Rápida de Fourier (FFT), o sinal é convertido para o domínio da frequência e visualizado graficamente em uma matriz de LEDs HUB75 64x32. Cada coluna representa uma banda de frequência, com altura proporcional à intensidade e cor correspondente à faixa espectral.

📷 Demonstração

![Visualização em tempo real](https://github.com/jotapedro7/ESP32-FFT_AudioVisualizer/blob/main/assets/readme/gif_demonstrativo.gif);


🔧 Hardware Utilizado
![image](https://github.com/user-attachments/assets/b84796b0-e1ce-4734-ac03-bcbc087f58bd)

📟 ESP32 38PIN DevKit V1

🎧 Microfone MAX9814 com AGC

🔊 Filtro RC passa-baixa (33 Ω + 220 nF)

🟩 Matriz de LEDs RGB HUB75 P4 64×32 (1/16 scan)

📡 Cabos, resistores e alimentação externa fonte ADC 5v 3A

Esquema de Hardware criada na plataforma Cirkit: https://app.cirkitdesigner.com/project/930f950b-b037-4927-be6b-03ffccbfea08

💻 Bibliotecas Utilizadas

ESP32-HUB75-MatrixPanel-I2S-DMA

arduinoFFT

⚙️ Instalação e Execução

Clone o repositório:

git clone https://github.com/seu-usuario/seu-repositorio.git

Abra o arquivo ESP32_AudioVisualizerFFT.ino na Arduino IDE.

Instale as bibliotecas acima via Gerenciador da IDE.

Conecte o hardware conforme o esquema.

Faça o upload para o ESP32.

Alimente a matriz com fonte externa 5V (3A ou mais recomendada).

🎛️ Funcionamento

O microfone capta o som e envia o sinal para o ESP32 via ADC (GPIO 34).

O sinal passa por filtragem analógica (passa-baixa).

A FFT divide o espectro em 512 bins (faixas), agrupadas em 32 bandas logarítmicas.

A matriz exibe a altura das barras proporcional à energia em cada banda.

A coloração varia de grave (vermelho) a agudo (rosa), facilitando a leitura espectral.





📜 Licença

Este projeto está licenciado sob a licença MIT. Veja o arquivo LICENSE para mais detalhes.
