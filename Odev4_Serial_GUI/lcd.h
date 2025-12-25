#ifndef _LCD_H
#define _LCD_H

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

// ----- LCD pinleri -----
#define RS GPIO_PIN_1
#define RW GPIO_PIN_2
#define EN GPIO_PIN_3

#define D4 GPIO_PIN_4
#define D5 GPIO_PIN_5
#define D6 GPIO_PIN_6
#define D7 GPIO_PIN_7

// Basit gecikme iÃ§in
long sure = 150000;

// Prototipler
void baslangic(void);
void komut_yaz(void);
void LCD_sil(void);
void veri_yaz(void);
void satir_sutun(unsigned char satir, unsigned char sutun);
void veri(char deger);          // *** DÄ°KKAT: void, deÄŸer dÃ¶ndÃ¼rmÃ¼yor
void lcd_yaz(char *s);          // printf yerine bunu kullanÄ±yoruz

//===========================================================================
void baslangic(void)   // LCD baÅŸlangÄ±Ã§ ayarlarÄ±
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, RS | RW | EN);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, D7 | D6  | D5 | D4);

    GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x00);
    SysCtlDelay(sure);    // min 20 ms

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x20);
    komut_yaz();

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x20);
    komut_yaz();

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x80);
    komut_yaz();
    SysCtlDelay(sure);    // min 37 us

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x00);
    komut_yaz();

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0xD0);
    komut_yaz();
    SysCtlDelay(sure);    // min 37 us

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x00);
    komut_yaz();

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x10);
    komut_yaz();
    SysCtlDelay(sure);    // min 1.52 ms

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x00);
    komut_yaz();

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x20);
    komut_yaz();
}

//===========================================================================
void komut_yaz(void)
{
    // RS=0, RW=0, EN=1 -> EN darbesi
    GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x08);
    SysCtlDelay(10000);
    GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x00);
}

//===========================================================================
void satir_sutun(unsigned char satir, unsigned char sutun)
{
    char total;

    if (satir == 1)
        satir = 0x7F;
    if (satir == 2)
        satir = 0xBF;

    total = satir + sutun;

    GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x00);
    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, (0xF0 & total));
    komut_yaz();
    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, ((total & 0x0F) << 4));
    komut_yaz();
}

//===========================================================================
void LCD_sil(void)   // LCD ekranÄ± sil
{
    GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x00);
    SysCtlDelay(sure);

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x00);
    SysCtlDelay(sure);
    komut_yaz();

    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, 0x10);
    SysCtlDelay(sure);
    komut_yaz();
}

//===========================================================================
// metin yazdÄ±rmak iÃ§in: lcd_yaz("SUDE SEDEF");
void lcd_yaz(char *s)
{
    GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x02); // RS=1, RW=0
    while (*s)
        veri(*s++);
}

//===========================================================================
void veri(char deger)
{
    // YÃ¼ksek 4 bit
    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, (0xF0 & deger));
    veri_yaz();

    // DÃ¼ÅŸÃ¼k 4 bit
    GPIOPinWrite(GPIO_PORTB_BASE, D7 | D6  | D5 | D4, ((deger & 0x0F) << 4));
    veri_yaz();
}

//===========================================================================
void veri_yaz(void)
{
    // RS=1, RW=0, EN=1 -> veri darbesi
    GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x0A);
    SysCtlDelay(10000);
    GPIOPinWrite(GPIO_PORTE_BASE, RS | RW | EN, 0x02);
}

#endif
