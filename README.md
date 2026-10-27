# Demonstração de race condition com Zephyr RTOS e PlatformIO

Este projeto demonstra a perda de atualizações em um contador lógico de pacotes
de telemetria compartilhado por duas threads no Zephyr RTOS. A configuração
incluída usa a placa NXP FRDM-K64F (`frdm_k64f`), suportada pela plataforma
Freescale Kinetis do PlatformIO.

## Como a falha é reproduzida

Em cada rodada, ambas as threads leem o contador antes de qualquer uma poder
gravá-lo. A thread principal libera as duas gravações somente depois de receber
as duas confirmações de leitura. As threads então gravam o mesmo valor
incrementado: uma das duas atualizações se perde em todas as rodadas.

As leituras e gravações individuais usam `atomic_get()` e `atomic_set()`, mas a
sequência completa ler-modificar-gravar não é atômica. Os semáforos organizam
intencionalmente a intercalação para tornar a falha reproduzível; eles não
protegem o contador. Com 1.000 rodadas, o resultado esperado é 1.000, em vez dos
2.000 incrementos solicitados.

Para evitar a falha em uma aplicação real, proteja a sequência completa
ler-modificar-gravar com um mutex ou substitua-a por uma operação atômica de
incremento, como `atomic_inc()`, se isso atender aos requisitos do projeto.

## Preparar o PlatformIO

1. Instale o Visual Studio Code e a extensão **PlatformIO IDE** (ou instale o
   PlatformIO Core).
2. Abra esta pasta como projeto no PlatformIO. A configuração já seleciona
   `freescalekinetis`, `zephyr` e `frdm_k64f`; na primeira compilação, o
   PlatformIO baixará as ferramentas e os pacotes necessários.
3. Conecte a FRDM-K64F via USB e instale os drivers da interface de depuração
   CMSIS-DAP, se solicitado pelo sistema operacional.

## Compilar, gravar e visualizar

No terminal integrado, na raiz do projeto:

```sh
pio run
pio run -t upload
pio device monitor
```

Também é possível usar os comandos **Build**, **Upload** e **Monitor** da barra
de ferramentas do PlatformIO no VS Code. O monitor serial está configurado para
115200 baud. Após a execução, o terminal deve mostrar 1.000 pacotes registrados,
2.000 incrementos solicitados e 1.000 atualizações perdidas.

Para outra placa suportada pela plataforma PlatformIO Freescale Kinetis, ajuste
`board` em `platformio.ini` e confira a configuração UART correspondente no
device tree da placa.
