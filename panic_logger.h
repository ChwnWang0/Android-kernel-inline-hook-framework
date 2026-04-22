#ifndef PANIC_LOGGER_H
#define PANIC_LOGGER_H

/* Panic and crash logging module */

/* Initialize panic logging (registers kprobes) */
int panic_logger_init(void);

/* Cleanup panic logging (unregisters kprobes) */
void panic_logger_exit(void);

#endif
