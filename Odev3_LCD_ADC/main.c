#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"

#include "lcd.h"

volatile uint8_t sn = 0, dk = 0, sa = 0;
uint32_t adcValue = 0;

void initmikro(void);
void timerkesme(void);
void lcdyaz(void);
void adc_oku(void);

int main(void)
{
    initmikro();

    // İlk ekran
    satir_sutun(1, 1);
    lcd_yaz("SAYAC      POT");    // solda sayac, sağ tarafta pot yazısı
    lcdyaz();

    while (1) {

    }
}

void initmikro(void)
{
    // 40 MHz sistem saati
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL |
                   SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // LCD başlat
    baslangic();

    // PF1 LED
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    // ---- ADC0, kanal 3 (PE0 / AIN3) ----
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);      // PE0 analog giriş

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0,
                             ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END); // AIN3 = PE0
    ADCSequenceEnable(ADC0_BASE, 3);

    // ---- TIMER0A: 1 saniyelik periyodik kesme ----
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() - 1); // ~1 saniye

    TimerIntRegister(TIMER0_BASE, TIMER_A, timerkesme);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);
    IntMasterEnable();

    TimerEnable(TIMER0_BASE, TIMER_A);
}


void timerkesme(void)
{
    // Kesme bayrağını temizle (ilk iş)
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Zamanı artır
    if (++sn == 60) {
        sn = 0;
        if (++dk == 60) {
            dk = 0;
            if (++sa == 24) sa = 0;
        }
    }

    // Potu oku
    adc_oku();

    // LCD’yi güncelle
    lcdyaz();

    // PF1 LED’i 1 Hz’de yanıp sönsün (opsiyonel)
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1,
                 (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ^ GPIO_PIN_1));
}


void adc_oku(void)
{
    ADCIntClear(ADC0_BASE, 3);
    ADCProcessorTrigger(ADC0_BASE, 3);

    while (!ADCIntStatus(ADC0_BASE, 3, false)) {
        // dönüşünü bekle
    }

    ADCSequenceDataGet(ADC0_BASE, 3, &adcValue);  // 0–4095 arası değer
}


void lcdyaz(void)
{
    char zaman[9];     // "HH:MM:SS"
    char adcYazi[10];  // "A=4095" gibi

    // --- Zaman dizisi (HH:MM:SS) ---
    zaman[0] = '0' + (sa / 10);
    zaman[1] = '0' + (sa % 10);
    zaman[2] = ':';
    zaman[3] = '0' + (dk / 10);
    zaman[4] = '0' + (dk % 10);
    zaman[5] = ':';
    zaman[6] = '0' + (sn / 10);
    zaman[7] = '0' + (sn % 10);
    zaman[8] = '\0';

    // --- ADC yazısı ---
    // Örnek: "P=2735"
    sprintf(adcYazi, "P=%4lu", (unsigned long)adcValue);

    // Sol üst: sayaç
    satir_sutun(1, 1);
    lcd_yaz(zaman);

    // Sağ alt: ADC değeri (2. satır, 11. sütun)
    satir_sutun(2, 11);
    lcd_yaz(adcYazi);
}