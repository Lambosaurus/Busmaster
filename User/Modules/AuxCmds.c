
#include "AuxCmds.h"
#include "ComCmd.h"
#include "ConfigCmd.h"

#include "ADC.h"
#include "GPIO.h"
#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

static const CmdNode_t gVoutNode;
static const CmdNode_t gVrefNode;
static const CmdNode_t gTempNode;
static const CmdNode_t gHelpNode;

/*
 * PUBLIC FUNCTIONS
 */

const CmdNode_t * AUXCMD_InitVref(void)
{
	return &gVrefNode;
}

const CmdNode_t * AUXCMD_InitVout(void)
{
	GPIO_EnableOutput(VOUT_EN_GPIO, VOUT_EN_PIN, !gConfig.default_vout);
	return &gVoutNode;
}

const CmdNode_t * AUXCMD_InitTemp(void)
{
	return &gTempNode;
}

const CmdNode_t * AUXCMD_InitHelp(void)
{
	return &gHelpNode;
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * FUNCTION NODES
 */

static void AUXCMD_Vref(CmdLine_t * line, CmdArgValue_t * args)
{
	uint32_t vref = ADC_ReadVRef();
	CmdReplyLevel_t level = CmdReply_Info;
	if (vref < 2200 || vref > 3400)
	{
		level = CmdReply_Warn;
	}
	Cmd_Printf(line, level, "%lumV\r\n", vref);
}

static const CmdNode_t gVrefNode = {
	.type = CmdNode_Function,
	.name = "vref",
	.func = {
		.arglen = 0,
		.callback = AUXCMD_Vref
	}
};

static void AUXCMD_Temp(CmdLine_t * line, CmdArgValue_t * args)
{
	int32_t temp = ADC_ReadDieTemp();
	CmdReplyLevel_t level = CmdReply_Info;
	if (temp <= 0 || temp > 40)
	{
		level = CmdReply_Warn;
	}
	Cmd_Printf(line, level, "%d degC\r\n", temp);
}

static const CmdNode_t gTempNode = {
	.type = CmdNode_Function,
	.name = "temp",
	.func = {
		.arglen = 0,
		.callback = AUXCMD_Temp
	}
};

static const CmdArg_t gVoutArgs[] = {
	{
		.type = CmdArg_Bool,
		.name = "enable",
	}
};

static void AUXCMD_Vout(CmdLine_t * line, CmdArgValue_t * args)
{
	bool enable = args[0].boolean;
	GPIO_Write(VOUT_EN_GPIO, VOUT_EN_PIN, !enable);
	COMCMD_PrintOk(line);
}

static const CmdNode_t gVoutNode = {
	.type = CmdNode_Function,
	.name = "vout",
	.func = {
		.arglen = LENGTH(gVoutArgs),
		.args = gVoutArgs,
		.callback = AUXCMD_Vout
	}
};

static const CmdArg_t gHelpArgs[] = {
	{
		.type = CmdArg_String | CmdArg_Optional,
		.name = "topic",
	}
};

typedef struct {
	const char * name;
	const char * text;
} HelpTopic_t;

const HelpTopic_t gHelpTopics[] = {
	{
		.name = "boolean",
		.text =
			"Boolean arguments must be one of the following:\r\n"
			" - \"1\": true\r\n"
			" - \"0\": false\r\n"
	},
	{
		.name = "number",
		.text =
			"Number arguments expect a positive integer.\r\n"
			"They may support a simple engineering notation for large numbers.\r\n"
			"For example:\r\n"
			" - \"128\": 128\r\n"
			" - \"9k6\": 9600\r\n"
			" - \"8m\": 8000000\r\n"
	},
	{
		.name = "string",
		.text =
			"String arguments support an arbitrary text input.\r\n"
			"\" or ' may be used to protect strings.\r\n"
			"Standard escape sequences will be decoded, such as \\n, \\\" or \\\".\r\n"
			"Arbitrary bytes may be specified with the \\x?? notation.\r\n"
			"For example:\r\n"
			" - 'some text'\r\n"
			" - 'more\\x20text\\r\\n'\r\n"
	},
	{
		.name = "bytes",
		.text =
			"Binary arguments support an arbitrary binary input.\r\n"
			"This can be submitted as a hex string. The hex string may be delimited with [ or < symbols\r\n"
			"The hex bytes may also be separated using ' ', ',', '-', ':' delimiters.\r\n"
			"If delimited using \' or \" the argument will be parsed as string. Enter 'help string' for more info\r\n"
			"The following examples are equivalent:\r\n"
			" - '61736369690D0A'\r\n"
			" - '[61 73 63 69 69 0D 0A]'\r\n"
			" - '<61-73-63-69-69-0d-0a>'\r\n"
			" - '\"ascii\\r\\n\"'\r\n"
			"These same formatting options are available for byte command responses.\r\n"
			"Enter 'config set format ?\?' for more info on responses\r\n"
	}
};

static void AUXCMD_Help(CmdLine_t * line, CmdArgValue_t * args)
{
	if (args[0].present)
	{
		const char * topic = args[0].str;

		for (uint32_t i = 0; i < LENGTH(gHelpTopics); i++)
		{
			if (strcmp(gHelpTopics[i].name, topic) == 0)
			{
				Cmd_Prints(line, CmdReply_Info, gHelpTopics[i].text);
				return;
			}
		}

		Cmd_Prints(line, CmdReply_Warn, "Unknown topic. Try one of the following:\r\n");
		for (uint32_t i = 0; i < LENGTH(gHelpTopics); i++)
		{
			Cmd_Printf(line, CmdReply_Info, " - %s\r\n", gHelpTopics[i].name);
		}
	}
	else
	{
		const char * message =
			"Welcome to the Busmaster command line utility\r\n"
			"This tool uses a tree based menu structure.\r\n"
			"Enter \"?\" to list \r\n"
		;
		Cmd_Prints(line, CmdReply_Info, message);
	}
}

static const CmdNode_t gHelpNode = {
	.type = CmdNode_Function,
	.name = "help",
	.func = {
		.arglen = LENGTH(gHelpArgs),
		.args = gHelpArgs,
		.callback = AUXCMD_Help
	}
};

/*
 * INTERRUPT ROUTINES
 */
