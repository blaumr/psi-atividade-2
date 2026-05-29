#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <pwm_z42.h>

// Valor do MOD do TPM.
// Frequência aproximada: fpwm = 48 MHz / (TPM_MODULE * 128)
// Com TPM_MODULE = 1000 e prescaler = 128 => ~375 Hz
#define TPM_MODULE 1000

int main(void)
{
    uint16_t duty_50 = TPM_MODULE / 2;   // 50% de duty cycle

    // Inicializa o TPM2 para gerar PWM
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    // Configura o canal 0 do TPM2 no pino PTB18
    // PTB18 normalmente é o LED vermelho da FRDM-KL25Z
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18);

    // Aplica duty cycle de 50%
    pwm_tpm_CnV(TPM2, 0, duty_50);

    while (1) {
        k_msleep(1000);
    }

    return 0;
}
