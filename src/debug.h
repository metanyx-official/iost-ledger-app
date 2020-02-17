#ifndef LEDGER_APP_IOST_DEBUG_H
#define LEDGER_APP_IOST_DEBUG_H

#include <stdint.h>

extern void debug_init_stack_canary();

extern uint32_t debug_get_stack_canary();

extern void debug_check_stack_canary();

#endif // LEDGER_APP_IOST_DEBUG_H
