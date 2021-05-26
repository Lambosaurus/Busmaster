
#include "GpioCmd.h"
#include "GPIO.h"
#include "ComCmd.h"
#include "UartCmd.h"

/*
 * PRIVATE DEFINITIONS
 */

#define PX_PINS		(P0_PIN | P1_PIN | P2_PIN | P3_PIN)

/*
 * PRIVATE TYPES
 */

typedef enum {
	PinState_None 	= 0,
	PinState_Output = (1 << 0),
	PinState_Input 	= (1 << 1),
	PinState_Pullup = (1 << 2),
} PinState_t;

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

static bool gGpioEnabled;
static const CmdNode_t gGpioMenu;

/*
 * PUBLIC FUNCTIONS
 */

const CmdNode_t * GPIOCMD_InitMenu(void)
{
	gGpioEnabled = false;
	return &gGpioMenu;
}

void GPIOCMD_CrossDeinit(CmdLine_t * line)
{
	if (gGpioEnabled)
	{
		GPIO_Deinit(PX_GPIO, PX_PINS);
		Cmd_Prints(line, CmdReply_Warn, "gpio deinitialised\r\n");
		gGpioEnabled = false;
	}
}

/*
 * PRIVATE FUNCTIONS
 */

static uint32_t GPIOCMD_GetPin(CmdLine_t * line, uint32_t pin)
{
	switch (pin)
	{
	case 0:
		return P0_PIN;
	case 1:
		return P1_PIN;
	case 2:
		return P2_PIN;
	case 3:
		return P3_PIN;
	default:
		Cmd_Prints(line, CmdReply_Error, "pin must be 0 - 3\r\n");
		return 0;
	}
}

/*
 * FUNCITON NODES
 */

static void GPIOCMD_Init(CmdLine_t * line, CmdArgValue_t * argv)
{
	UARTCMD_CrossDeinit(line);
	if (gGpioEnabled)
	{
		GPIO_Deinit(PX_GPIO, PX_PINS);
	}
	gGpioEnabled = true;
	COMCMD_PrintOk(line);
}

static const CmdNode_t gGpioInitNode = {
	.type = CmdNode_Function,
	.name = "init",
	.func = {
		.arglen = 0,
		.callback = GPIOCMD_Init,
	}
};

static void GPIOCMD_Deinit(CmdLine_t * line, CmdArgValue_t * argv)
{
	if (gGpioEnabled)
	{
		GPIO_Deinit(PX_GPIO, PX_PINS);
		gGpioEnabled = false;
	}
	COMCMD_PrintOk(line);
}

static const CmdNode_t gGpioDeinitNode = {
	.type = CmdNode_Function,
	.name = "deinit",
	.func = {
		.arglen = 0,
		.callback = GPIOCMD_Deinit,
	}
};

static const CmdArg_t gGpioWriteArgs[] = {
	{
		.name = "pin",
		.type = CmdArg_Number,
	},
	{
		.type = CmdArg_Bool,
		.name = "enable",
	}
};

static void GPIOCMD_Write(CmdLine_t * line, CmdArgValue_t * argv)
{
	uint32_t pin_number = argv[0].number;
	bool enable = argv[1].boolean;
	if (!gGpioEnabled)
	{
		COMCMD_PrintNoInit(line, "gpio");
		return;
	}
	uint32_t pin = GPIOCMD_GetPin(line, pin_number);
	if (!pin)
	{
		return;
	}
	GPIO_EnableOutput(PX_GPIO, pin, enable);
	COMCMD_PrintOk(line);
}

static const CmdNode_t gGpioWriteNode = {
	.type = CmdNode_Function,
	.name = "write",
	.func = {
		.arglen = LENGTH(gGpioWriteArgs),
		.args = gGpioWriteArgs,
		.callback = GPIOCMD_Write,
	}
};

static const CmdArg_t gGpioReadArgs[] = {
	{
		.name = "pin",
		.type = CmdArg_Number,
	}
};

static void GPIOCMD_Read(CmdLine_t * line, CmdArgValue_t * argv)
{
	uint32_t pin_number = argv[0].number;
	if (!gGpioEnabled)
	{
		COMCMD_PrintNoInit(line, "gpio");
		return;
	}
	uint32_t pin = GPIOCMD_GetPin(line, pin_number);
	if (!pin)
	{
		return;
	}
	bool set = GPIO_Read(PX_GPIO, pin);
	Cmd_Printf(line, CmdReply_Info, "%d\r\n", set);
}

static const CmdNode_t gGpioReadNode = {
	.type = CmdNode_Function,
	.name = "read",
	.func = {
		.arglen = LENGTH(gGpioReadArgs),
		.args = gGpioReadArgs,
		.callback = GPIOCMD_Read,
	}
};

static const CmdNode_t * gGpioFunctions[] = {
		&gGpioInitNode,
		&gGpioDeinitNode,
		&gGpioWriteNode,
		&gGpioReadNode,
};

static const CmdNode_t gGpioMenu = {
	.type = CmdNode_Menu,
	.name = "gpio",
	.menu = {
		.nodes = gGpioFunctions,
		.count = LENGTH(gGpioFunctions)
	}
};


/*
 * INTERRUPT ROUTINES
 */
