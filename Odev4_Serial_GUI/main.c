#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"

#include "lcd.h" 

volatile uint8_t sn = 0, dk = 0, sa = 0;

void initmikro(void);
void timerkesme(void);
void lcdyaz(void);
void uartZamanOku(void);

int main(void)
{
    initmikro();

    LCD_sil();
    satir_sutun(1, 1);
    lcd_yaz("Saat Bekleniyor");

    // PC'den başlangıç saatini AL (format: !HH:MM:SS)
    uartZamanOku();

    LCD_sil();
    lcdyaz();   // ilk saati yaz

    // Sayaç bu saatten devam etsin
    TimerEnable(TIMER0_BASE, TIMER_A);

    while (1)
    {
        // her şey kesmede
    }
}

void initmikro(void)
{
    // 40 MHz – lcd.h'ndeki delay’lerle uyumlu, yeterince stabil
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL |
                   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // LCD
    baslangic();
    LCD_sil();

    // UART0 (PA0-RX, PA1-TX)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600,
                        UART_CONFIG_WLEN_8 |
                        UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_NONE);
    UARTEnable(UART0_BASE);

    // TIMER0: 1 saniyelik periyodik kesme
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() - 1);

    TimerIntRegister(TIMER0_BASE, TIMER_A, timerkesme);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    IntEnable(INT_TIMER0A);
    IntMasterEnable();
}

// PC'den !HH:MM:SS formatında saat okur
void uartZamanOku(void)
{
    char buffer[9];
    char c;
    int i;

    // 1) RX FIFO'da ne varsa hepsini temizle
    while (UARTCharsAvail(UART0_BASE))
    {
        UARTCharGetNonBlocking(UART0_BASE);
    }

    // 2) '!' gelene kadar bekle (başlangıç karakteri)
    do
    {
        c = UARTCharGet(UART0_BASE);   // bloklayıcı okuma
    } while (c != '!');

    // 3) Ardından gelen 8 karakteri oku → HH:MM:SS
    for (i = 0; i < 8; i++)
    {
        buffer[i] = UARTCharGet(UART0_BASE);
    }
    buffer[8] = '\0';

    // 4) String'ten sa, dk, sn hesapla
    sa = (buffer[0] - '0') * 10 + (buffer[1] - '0');
    dk = (buffer[3] - '0') * 10 + (buffer[4] - '0');
    sn = (buffer[6] - '0') * 10 + (buffer[7] - '0');
}

void timerkesme(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    if (++sn == 60)
    {
        sn = 0;
        if (++dk == 60)
        {
            dk = 0;
            if (++sa == 24) sa = 0;
        }
    }

    lcdyaz();
}

void lcdyaz(void)
{
    char zaman[9];

    zaman[0] = '0' + (sa / 10);
    zaman[1] = '0' + (sa % 10);
    zaman[2] = ':';
    zaman[3] = '0' + (dk / 10);
    zaman[4] = '0' + (dk % 10);
    zaman[5] = ':';
    zaman[6] = '0' + (sn / 10);
    zaman[7] = '0' + (sn % 10);
    zaman[8] = '\0';

    satir_sutun(1, 1);
    lcd_yaz(zaman);
}