#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

#include "lcd.h"

volatile uint8_t sn = 0, dk = 0, sa = 0;

void initmikro(void);
void timerkesme(void);
void lcdyaz(void);

int main(void)
{
    initmikro();

    satir_sutun(1,2);
    printf("DIJITAL SAAT");
    lcdyaz();

    while(1) {
    }
}

void initmikro(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL |
                   SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    baslangic();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() - 1);

    TimerIntRegister(TIMER0_BASE, TIMER_A, timerkesme);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);
    IntMasterEnable();

    TimerEnable(TIMER0_BASE, TIMER_A);
}

void timerkesme(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    if (++sn == 60) {
        sn = 0;
        if (++dk == 60) {
            dk = 0;
            if (++sa == 24) sa = 0;
        }
    }

    lcdyaz();

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1,
                 (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ^ GPIO_PIN_1));
}

void lcdyaz(void)
{
    char buf[9];
    buf[0] = '0' + (sa / 10);
    buf[1] = '0' + (sa % 10);
    buf[2] = ':';
    buf[3] = '0' + (dk / 10);
    buf[4] = '0' + (dk % 10);
    buf[5] = ':';
    buf[6] = '0' + (sn / 10);
    buf[7] = '0' + (sn % 10);
    buf[8] = '\0';

    satir_sutun(2,5);
    printf(buf);
}
