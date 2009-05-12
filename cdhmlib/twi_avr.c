/* twi.avr.c */
/* avr.twi - TWI AVR module. {{{
 *
 * Copyright (C) 2005 Demonchy Clément
 *
 * Robot APB Team/Efrei 2006.
 *        Web: http://assos.efrei.fr/robot/
 *      Email: robot AT efrei DOT fr
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * }}} */

#include "twi_avr.h"
#include "myusart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <compat/twi.h>
// #include "modules/uart/uart.h"

// TODO mettre l'etat partout 
// quand on fait les demandes, et les liberations qui vont bien dans
// l'interruption

#define TWI_SL_RCPT_SIZE 16
#define TWI_SL_SEND_SIZE 16

#if 1
#define twi_debug(x) 	 do{}while(0)
#define twi_debug_hex(x) do{}while(0)
#else
#define twi_debug(x) usart_TxQueuePut(x)
#define twi_debug_hex(x) usart_TxQueuePutHex(x)
#endif

enum
{
    TWI_SUCCESS,
    TWI_BUSY,
    TWI_FREE,
    TWI_FAILURE
};

#ifdef TWI_SLAVE_ENABLE
/* données reçues du master */
static volatile uint8_t rcpt_buf_sl[TWI_SL_RCPT_SIZE];
static volatile uint8_t data_ready_sl;
/* données à envoyer au master */
/* 2 buffers 1 pour envoyer vers l'user et l'autre vers l'i2c */
static volatile uint8_t send_buf_sl1[TWI_SL_SEND_SIZE];
static volatile uint8_t send_buf_sl2[TWI_SL_SEND_SIZE];
static volatile uint8_t *send_sys_sl, *send_user_sl;
static volatile uint8_t update_sl; /* lock pour savoir si on peut switcher les buffers sans risque */
static volatile uint8_t send_switch_sl;
#endif /* TWI_SLAVE_ENABLE */

void
twi_init (uint8_t addr)
{
#ifdef TWI_SLAVE_ENABLE
    data_ready_sl = 0;
    send_switch_sl = 0;
    update_sl = 0;
    send_sys_sl = send_buf_sl1;
    send_user_sl = send_buf_sl2;
    TWAR = addr;
    TWSR = 0x00;
    TWCR = _BV(TWEA);
#endif /* TWI_SLAVE_ENABLE */
    /* Active la twi et les interruptions de la twi */
    TWCR |= _BV (TWEN) | _BV (TWIE); 
}

#ifdef TWI_SLAVE_ENABLE
uint8_t 
twi_sl_poll (uint8_t *buffer, uint8_t size)
{
    // XXX state
    if (data_ready_sl)
      {
        data_ready_sl = 0;
        while (size > 0){
            size--;
            buffer[size] = rcpt_buf_sl[size];
        }
        /* de nouveau dispo pour traiter de nouvelles requetes */
        TWCR |= _BV (TWEA); 
        return 1;
      }
    else
        return 0;
}
#endif  /* TWI_SLAVE_ENABLE */

#ifdef TWI_SLAVE_ENABLE
void
twi_sl_update (uint8_t *buffer, uint8_t size)
{
    // XXX state
    update_sl = 1;
    send_switch_sl = 1; /* demander un swap des buffers */
    while (size >0){
        size--;
        send_user_sl[size] = buffer[size];
    }
    update_sl = 0;
}
#endif  /* TWI_SLAVE_ENABLE */


SIGNAL (SIG_2WIRE_SERIAL)
{
#ifdef TWI_SLAVE_ENABLE
    static uint8_t send_idx_sl = 0;
    static uint8_t rcpt_idx_sl = 0;
    static uint8_t b;
#endif  /* TWI_SLAVE_ENABLE */
    uint8_t status = TWSR & 0xF8;
    twi_debug('S');
    twi_debug_hex(status);
    switch (status)
      {
#ifdef TWI_SLAVE_ENABLE
        /***** slave transmitter mode *****/
        /* START + SLA|W + ACK 
         * on a recu un start pour une ecriture et on a acquité 
         * choisi le buffer d'envoie
         * envoie le premier byte 
         */
      case TW_ST_SLA_ACK:
      case TW_ST_ARB_LOST_SLA_ACK:
        twi_debug('a');
        if (send_switch_sl && !update_sl) 
          {
            volatile uint8_t *tmp = send_sys_sl;
            send_sys_sl = send_user_sl;
            send_user_sl = tmp;
            send_switch_sl = 0;
          }
        send_idx_sl = 0;
        /* NO BREAK */
        /* Recu un ack apres l'envoie d'un bit */
      case TW_ST_DATA_ACK:
        twi_debug('b');
        b = send_sys_sl[send_idx_sl++];
        TWDR = b;
        twi_debug_hex(b);
        if (send_idx_sl == TWI_SL_SEND_SIZE)
            TWCR &= ~_BV(TWEA);
        TWCR |= _BV(TWINT); 
        break;
      case TW_ST_DATA_NACK:
      case TW_ST_LAST_DATA:
        twi_debug('c');
        TWCR |= _BV (TWEA);
        TWCR |= _BV(TWINT);
        break;
        /***** slave receiver mode *****/
        /* START + SLA|W + ACK */
      case TW_SR_SLA_ACK:
      case TW_SR_ARB_LOST_SLA_ACK:
      case TW_SR_GCALL_ACK:
      case TW_SR_ARB_LOST_GCALL_ACK:
        twi_debug('d');
        data_ready_sl = 0;
        rcpt_idx_sl = 0;
        if (TWI_SL_RCPT_SIZE == 1)
            TWCR &= ~_BV(TWEA);
        TWCR |= _BV(TWINT);
        break;
        /* DATA + ACK */
      case TW_SR_DATA_ACK:
      case TW_SR_GCALL_DATA_ACK:
        twi_debug('e');
        rcpt_buf_sl[rcpt_idx_sl++] = TWDR;
        if (TWI_SL_RCPT_SIZE - rcpt_idx_sl == 1)
            TWCR &= ~_BV(TWEA);
        TWCR |= _BV(TWINT);
        break;
        /* DATA + NACK */
      case TW_SR_DATA_NACK:
      case TW_SR_GCALL_DATA_NACK:
        twi_debug('f');
        //uart0_putc ('o');
        rcpt_buf_sl[rcpt_idx_sl++] =  TWDR;
        /* NO BREAK */
        /* STOP */
      case TW_SR_STOP:
        twi_debug('g');
        //uart0_putc ('q');
        data_ready_sl = 1;
        TWCR |= _BV(TWINT);
        break;
#endif /* TWI_SLAVE_ENABLE */
      case 0x00: /* bus error */
       twi_debug('h');
        TWCR &= ~ _BV(TWSTA);
        TWCR |= _BV(TWSTO);
        TWCR |= _BV(TWINT);
        /* Reenable */
        TWCR = _BV(TWEA);
        TWCR |= _BV (TWEN) | _BV (TWIE); 
      }
}

