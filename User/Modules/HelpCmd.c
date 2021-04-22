
#include "HelpCmd.h"
#include "ConfigCmd.h"
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

static const CmdNode_t gHelpNode;

/*
 * PUBLIC FUNCTIONS
 */

const CmdNode_t * HelpCmd_InitMenu(void)
{
	return &gHelpNode;
}


/*
 * PRIVATE FUNCTIONS
 */

/*
 * FUNCTION NODES
 */


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
			" - '1': true\r\n"
			" - '0': false\r\n"
	},
	{
		.name = "number",
		.text =
			"Number arguments expect a positive integer.\r\n"
			"They may be entered in hex using the 0x prefix.\r\n"
			"They also support a simple engineering notation for large numbers.\r\n"
			"For example:\r\n"
			" - '128': 128\r\n"
			" - '0x80': 128\r\n"
			" - '9k6': 9600\r\n"
			" - '8m': 8000000\r\n"
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
			"If delimited using \' or \" the argument will be parsed as string. Enter '" CMD_HELP_NODE " string' for more info\r\n"
			"The following examples are equivalent:\r\n"
			" - '61736369690D0A'\r\n"
			" - '[61 73 63 69 69 0D 0A]'\r\n"
			" - '<61-73-63-69-69-0d-0a>'\r\n"
			" - '\"ascii\\r\\n\"'\r\n"
			"These same formatting options are available for byte command responses.\r\n"
			"Enter 'config set format " CMD_HELP_SYMBOL "' for more info on responses\r\n"
	}
};

static void HelpCmd_Help(CmdLine_t * line, CmdArgValue_t * args)
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
			"This is a tree based command line interface.\r\n"
			"This tool uses a tree based menu structure.\r\n"
			"Entering '<path> " CMD_HELP_SYMBOL "' provides information about the specifed path.\r\n"
			"Enter '" CMD_HELP_SYMBOL "' to learn about items in the root menu.\r\n"
		;
		Cmd_Prints(line, CmdReply_Info, message);
	}
}

static const CmdNode_t gHelpNode = {
	.type = CmdNode_Function,
	.name = CMD_HELP_NODE,
	.help = "Provides information on some topics.",
	.func = {
		.arglen = LENGTH(gHelpArgs),
		.args = gHelpArgs,
		.callback = HelpCmd_Help
	}
};

/*
 * INTERRUPT ROUTINES
 */
