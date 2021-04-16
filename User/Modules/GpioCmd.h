#ifndef GPIOCMD_H
#define GPIOCMD_H

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

const CmdNode_t * GPIOCMD_InitMenu(void);
void GPIOCMD_CrossDeinit(CmdLine_t * line);

#endif //GPIOCMD_H
