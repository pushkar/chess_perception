/*
 * log.h
 *
 *  Created on: Jul 1, 2010
 *      Author: pushkar
 */

#ifndef LOG_H_
#define LOG_H_

#include "../sensors/mesa.h"

// Logging
void logwrite_init(const char* filename, mesa_t frame);
void logwrite_logframe(mesa_t frame);
void logwrite_close();

// Reading
void logread_init(const char* filename, mesa_t frame);
void logread_logframe(mesa_t *frame);
void logread_close();

#endif /* LOG_H_ */
