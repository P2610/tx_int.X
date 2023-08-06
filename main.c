/*
 * File:   main.c
 * Author: hidden02
 *
 * Created on August 5, 2023, 10:33 PM
 */

#include "fuses.h"
#include <xc.h>
#include <string.h>

#define rx_buffer_len 5
char rx_buffer[rx_buffer_len] = "";
char rx_buffer_ptr = 0x00;

#define tx_buffer_len 10
char tx_buffer[tx_buffer_len] = "";
char tx_buffer_ptr = 0x00;

#define response_len 10
char response[response_len] = "test_r";

void usart_conf(void){
    TRISC = 0x80;
    TXSTA = 0x04;
    RCSTA = 0x90;
    BAUDCON = 0x08;
    SPBRGH = 0x00;  //9600bps@4MHz
    SPBRG = 0x67;
    //SPBRGH = 0x04;  //9600bps@48MHz
    //SPBRG = 0xE1;    
    return;
}

void interrupt_conf(void){
    INTCON = 0xC0;
    INTCON2 = 0x00;
    INTCON3 = 0X80;
    PIE1 = 0x30;
    return;
}

void ports_conf(void){
    TRISB = 0x00;
}

void set_usart_response(char * res, char res_len){
    for(char i = 0; i < tx_buffer_len; i++){
        if(res[i] == '\0' || i >= res_len)
            break;
        tx_buffer[i] = res[i];        
    }
    return;
}

void start_tx(){
    tx_buffer_ptr = 0;
    TXREG = tx_buffer[tx_buffer_ptr];
    tx_buffer_ptr++;
    TXSTAbits.TXEN = 1;    
}

void switch_cmd (char * ptr){
    if(!strncmp(ptr, "STOP\r", rx_buffer_len)){           //Detener el sistema
        PORTBbits.RB0 = 0;
        set_usart_response("OK\r", 3);
        start_tx();
    }else if(!strncmp(ptr, "STRT\r", rx_buffer_len)){     //Iniciar el sistema
        PORTBbits.RB0 = 1;
        set_usart_response("OK\r", 3);
        start_tx();
    }
}

void main(void) {
    interrupt_conf();
    usart_conf();
    ports_conf();
    while(1){
        
    }
    return;
}

void __interrupt(high_priority) tcInt(void){
    if(PIE1bits.RCIE && PIR1bits.RCIF){
        rx_buffer[rx_buffer_ptr] = RCREG;
        rx_buffer_ptr++;
        if(rx_buffer_ptr >= rx_buffer_len){
            rx_buffer_ptr = 0;
            switch_cmd(rx_buffer);
        }
    }else if(PIE1bits.TXIE && PIR1bits.TXIF){
        PIR1bits.TXIF = 0;
        if(tx_buffer_ptr >= tx_buffer_len || tx_buffer[tx_buffer_ptr] == '\0'){
            tx_buffer_ptr = 0;
            TXSTAbits.TXEN = 0;
        }else{
            TXREG = tx_buffer[tx_buffer_ptr];
            tx_buffer_ptr++;
        }
    }
}