#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdint.h>

#include <pwm_z42.h>

/*
 * PWM no LED vermelho da FRDM-KL25Z
 *
 * LED vermelho -> PTB18
 * PTB18 pode ser conectado ao TPM2 canal 0.
 *
 * Objetivo:
 * - Alterar a frequência do PWM
 * - Alterar o duty cycle
 *
 * Frequência escolhida:
 * f_pwm = 1 kHz
 *
 * Fórmula:
 * f_pwm = f_core / (TPM_MODULE * Prescale)
 *
 * Considerando:
 * f_core = 48 MHz
 * Prescale = 128
 *
 * TPM_MODULE = 48.000.000 / (1.000 * 128)
 * TPM_MODULE = 375
 */

// Frequência do clock usada pelo TPM quando selecionamos TPM_PLLFLL
#define TPM_SOURCE_CLOCK_HZ     48000000UL

// Frequência desejada para o PWM
#define PWM_FREQ_HZ             1000UL

// Prescaler escolhido: PS_128
#define TPM_PRESCALER_VALUE     128UL

// Valor calculado para o MOD do TPM
#define TPM_MODULE_RAW          (TPM_SOURCE_CLOCK_HZ / (PWM_FREQ_HZ * TPM_PRESCALER_VALUE))
#define TPM_MODULE              ((uint16_t)TPM_MODULE_RAW)

// Verificação simples para garantir que o MOD cabe em 16 bits
#if (TPM_MODULE_RAW > 65535UL)
#error "TPM_MODULE maior que 16 bits. Escolha uma frequencia maior ou um prescaler maior."
#endif

#if (TPM_MODULE_RAW == 0)
#error "TPM_MODULE igual a zero. Escolha uma frequencia menor ou um prescaler menor."
#endif

// Duty cycles elétricos do PWM
#define DUTY_25_PERCENT         25U
#define DUTY_50_PERCENT         50U
#define DUTY_75_PERCENT         75U

// Converte percentual de duty cycle para valor do registrador CnV
static uint16_t duty_to_cnv(uint8_t duty_percent)
{
    if (duty_percent > 100U) {
        duty_percent = 100U;
    }

    return (uint16_t)((TPM_MODULE * duty_percent) / 100U);
}

int main(void)
{
    /*
     * Inicializa o TPM2:
     *
     * TPM2       -> módulo TPM utilizado
     * TPM_PLLFLL -> fonte de clock associada ao core, 48 MHz
     * TPM_MODULE -> valor do MOD, que define o período do PWM
     * TPM_CLK    -> clock interno do TPM
     * PS_128     -> prescaler de 128
     * EDGE_PWM   -> modo PWM por borda
     */
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    /*
     * Inicializa o canal 0 do TPM2 no pino PTB18.
     *
     * TPM2 canal 0 -> PTB18
     * PTB18        -> LED vermelho da FRDM-KL25Z
     *
     * TPM_PWM_H significa que o pulso ativo do PWM é em nível alto.
     */
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18);

    printk("PWM iniciado no LED vermelho da FRDM-KL25Z\n");
    printk("Frequencia PWM desejada: %lu Hz\n", PWM_FREQ_HZ);
    printk("TPM_MODULE calculado: %u\n", TPM_MODULE);

    while (1)
    {
        /*
         * Duty cycle de 25%
         *
         * Duty = CnV / TPM_MODULE
         */
        pwm_tpm_CnV(TPM2, 0, duty_to_cnv(DUTY_25_PERCENT));
        printk("Duty cycle eletrico: 25%%\n");
        k_msleep(3000);

        /*
         * Duty cycle de 50%
         */
        pwm_tpm_CnV(TPM2, 0, duty_to_cnv(DUTY_50_PERCENT));
        printk("Duty cycle eletrico: 50%%\n");
        k_msleep(3000);

        /*
         * Duty cycle de 75%
         */
        pwm_tpm_CnV(TPM2, 0, duty_to_cnv(DUTY_75_PERCENT));
        printk("Duty cycle eletrico: 75%%\n");
        k_msleep(3000);
    }

    return 0;
}