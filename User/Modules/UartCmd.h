#ifndef UARTCMD_H
#define UARTCMD_H

#include "STM32X.h"
#include "Command.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

const CmdNode_t * UARTCMD_InitMenu(void);
void UARTCMD_CrossDeinit(CmdLine_t * line);

#endif //UARTCMD_H
