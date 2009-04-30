#ifndef twi_h
#define twi_h
/* twi.h */
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

#include <inttypes.h>

/** Initialise twi. */
void
twi_init (uint8_t addr);

#ifdef TWI_SLAVE_ENABLE
/** Récupère dans buffer les données recues en tant qu'esclave */
uint8_t 
twi_sl_poll (uint8_t *buffer, uint8_t size);
/** Met à jour le buffer de donnée à envoyer */
void 
twi_sl_update (uint8_t *buffer, uint8_t size);
#endif /* TWI_SLAVE_ENABLE */

#ifdef TWI_MASTER_ENABLE
/** Is the current transaction finished ? */
int8_t 
twi_ms_is_finished (void);
/** Send len bytes of data to address */
int8_t
twi_ms_send (uint8_t address, uint8_t *data, uint8_t len);
/** Read len bytes at addresse en put them in data */
int8_t
twi_ms_read (uint8_t address, uint8_t *data, uint8_t len);
#endif /* TWI_MASTER_ENABLE */

#endif /* twi_h */
