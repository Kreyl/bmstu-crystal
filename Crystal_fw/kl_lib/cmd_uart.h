/*
 * cmd_uart.h
 *
 *  Created on: 15.04.2013
 *      Author: kreyl
 */

#ifndef CMD_UART_H_
#define CMD_UART_H_

#include "stm32f4xx.h"
#include "ch.h"
#include "hal.h"
#include "kl_sprintf.h"
#include "kl_lib_f40x.h"

// Set to true if RX needed
#define UART_RX_ENABLED     FALSE

// UART
#define UART_TXBUF_SIZE     1024

#define UART                USART1
#define UART_GPIO           GPIOA
#define UART_TX_PIN         9
#define UART_AF             AF7 // for all uarts
#define UART_RCC_ENABLE()   rccEnableUSART1(FALSE)

#define UART_DMA_TX         STM32_DMA2_STREAM7
#define UART_DMA_CHNL       4
#define UART_DMA_TX_MODE    STM32_DMA_CR_CHSEL(UART_DMA_CHNL) | \
                            DMA_PRIORITY_LOW | \
                            STM32_DMA_CR_MSIZE_BYTE | \
                            STM32_DMA_CR_PSIZE_BYTE | \
                            STM32_DMA_CR_MINC       | /* Memory pointer increase */ \
                            STM32_DMA_CR_DIR_M2P    | /* Direction is memory to peripheral */ \
                            STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */

#if UART_RX_ENABLED // ==== RX ====
#define UART_RXBUF_SZ       36 // unprocessed bytes
#define UART_CMDDATA_SZ     16 // payload bytes
#define UART_RX_PIN         10
#define UART_RX_REG         UART->DR

#define UART_RX_POLLING_MS  99
#define UART_DMA_RX         STM32_DMA1_STREAM5
#define UART_DMA_RX_MODE    DMA_PRIORITY_LOW | \
                            STM32_DMA_CR_MSIZE_BYTE | \
                            STM32_DMA_CR_PSIZE_BYTE | \
                            STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                            STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                            STM32_DMA_CR_CIRC         /* Circular buffer enable */

#endif

class CmdUart_t {
private:
    char TXBuf[UART_TXBUF_SIZE];
    char *PWrite, *PRead;
    bool IDmaIsIdle;
    int32_t IFullSlotsCount, ITransSize;
    void ISendViaDMA();
#if UART_RX_ENABLED
    int32_t SzOld=0, RIndx=0;
    uint8_t IRxBuf[UART_RXBUF_SZ];
    Cmd_t ICmd[2], *PCmdWrite = &ICmd[0], *PCmdRead = &ICmd[1];
    void CompleteCmd();
#endif
    uint32_t IBaudrate;
public:
    void Printf(const char *S, ...);
    void PrintfI(const char *S, ...);
    void FlushTx() { while(!IDmaIsIdle); }  // wait DMA
    void PrintfNow(const char *S, ...);
    void Init(uint32_t ABaudrate);
    void OnAHBFreqChange();
    void DeInit() { UART->CR1 &= ~USART_CR1_UE; /* UART Disable*/  }
    void Cmd(uint8_t CmdCode, uint8_t *PData, uint32_t Length) { Printf("\r\n#%X,%A", CmdCode, PData, Length, 0); }
    // Inner use
    void IPrintf(const char *format, va_list args);
    void IRQDmaTxHandler();
    void IPutChar(char c);
#if UART_RX_ENABLED
    void PollRx();
    // Command and reply
    void Ack(int32_t Result) { Printf("\r\n#Ack %d", Result); }
#endif
    CmdUart_t() {
        for(uint32_t i=0; i<UART_TXBUF_SIZE; i++) TXBuf[i] = 0;
        PWrite = TXBuf;
        PRead = TXBuf;
        IDmaIsIdle = true;
        IFullSlotsCount = 0;
        ITransSize = 0;
        IBaudrate = 115200;
    }
};

extern CmdUart_t Uart;

#endif /* CMD_UART_H_ */
