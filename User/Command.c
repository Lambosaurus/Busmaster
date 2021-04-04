
#include "Command.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "NParse.h"

/*
 * PRIVATE DEFINITIONS
 */

#define CMD_MAX_LINE	256

/*
 * PRIVATE TYPES
 */

typedef struct {
	uint32_t index;
	const char * str;
	uint32_t size;
	bool delimiter;
}Token_t;

typedef struct {
	const char * str;
	char delimiter;
	uint32_t size;
}CmdToken_t;

/*
 * PRIVATE PROTOTYPES
 */

static void Cmd_Run(CmdLine_t * line, const CmdNode_t * node, const char * str);
static void Cmd_Execute(CmdLine_t * line, const char * str);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void Cmd_Init(CmdLine_t * line, const CmdNode_t * root, void (*print)(const uint8_t * data, uint32_t size), void * memory, uint32_t memsize)
{
	line->bfr.index = 0;
	line->bfr.data = memory;
	line->bfr.size = CMD_MAX_LINE;
	line->root = root;
	line->print = print;

	line->mem.heap = memory + CMD_MAX_LINE;
	line->mem.size = memsize - CMD_MAX_LINE;
	line->mem.head = line->mem.heap;
}

uint32_t Cmd_Memfree(CmdLine_t * line)
{
	return line->mem.size - (line->mem.head - line->mem.heap);
}

void * Cmd_Malloc(CmdLine_t * line, uint32_t size)
{
	if (Cmd_Memfree(line) < size)
	{
		char * warn = "MEMORY OVERRUN\r\n";
		line->print((uint8_t *)warn, strlen(warn));
	}
	// Ignore overrun and do it anyway.....
	void * ptr = line->mem.head;
	line->mem.head += size;
	return ptr;
}

void Cmd_FreeAll(CmdLine_t * line)
{
	line->mem.head = line->mem.heap;
}

void Cmd_Free(CmdLine_t * line, void * ptr)
{
	if (ptr >= line->mem.heap)
	{
		line->mem.head = ptr;
	}
}

void Cmd_Parse(CmdLine_t * line, const uint8_t * data, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		char ch = (char)data[i];
		switch (ch)
		{
		case 0:
		case '\n':
		case '\r':
			line->bfr.data[line->bfr.index] = 0;
			if (line->bfr.index)
			{
				Cmd_Execute(line, line->bfr.data);
			}
			line->bfr.index = 0;
			break;
		default:
			if (line->bfr.index < line->bfr.size - 1)
			{
				// Need to leave room for at least a null char.
				line->bfr.data[line->bfr.index++] = ch;
			}
			else
			{
				// Discard the line
				line->bfr.index = 0;
			}
			break;
		}
	}
}

void Cmd_Printf(CmdLine_t * line, const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    // Take whatevers left - because we will immediately free it.
    uint32_t free = Cmd_Memfree(line);
    char * bfr = Cmd_Malloc(line, free);
    uint32_t written = vsnprintf(bfr, free, fmt, ap);
    va_end(ap);
    line->print((uint8_t *)bfr, written);
    Cmd_Free(line, bfr);
}

/*
 * PRIVATE FUNCTIONS
 */

static bool Cmd_ParseToken(const char ** str, CmdToken_t * token)
{
	const char * head = *str;
	while (1)
	{
		// Find the first char
		char ch = *head;
		if (ch == ' ' || ch == '\t')
		{
			head++;
		}
		else if (ch == 0)
		{
			return false;
		}
		else
		{
			break;
		}
	}
	char startc = *head;
	if (startc == '"' || startc == '\'' || startc == '`')
	{
		token->delimiter = startc;
		// Check if its a quoted token
		head++;
		token->str = head;
		while (1)
		{
			char ch = *head;
			if (ch == startc)
			{
				break;
			}
			else if (ch == 0)
			{
				return false;
			}
			else
			{
				str++;
			}
		}
	}
	else // Non quoted token
	{
		token->delimiter = 0;
		token->str = head;
		while (1)
		{
			char ch = *head;
			if (ch == ' ' || ch == '\t' || ch == 0)
			{
				break;
			}
			else
			{
				head++;
			}
		}
	}

	token->size = head - token->str;
	*str = head;
	return true;
}

static bool Cmd_NextToken(CmdLine_t * line, const char ** str, CmdToken_t * token)
{
	if (Cmd_ParseToken(str, token))
	{
		// Copy token from a ref to an allocated buffer
		char * bfr = Cmd_Malloc(line, token->size + 1);
		memcpy(bfr, token->str, token->size);
		bfr[token->size] = 0;
		token->str = bfr;
		return true;
	}
	return false;
}

static void Cmd_Execute(CmdLine_t * line, const char * str)
{
	Cmd_Run(line, line->root, str);
	Cmd_FreeAll(line);
}

static bool Cmd_ParseArg(CmdLine_t * line, const CmdArg_t * arg, CmdArgValue_t * value, CmdToken_t * token)
{
	const char * str = token->str;
	switch (arg->type)
	{
	case CmdArg_Number:
		return NParse_Kuint(&str, &value->number) && (*str == 0);
	case CmdArg_Bytes:
	{
		uint32_t maxbytes = token->size;
		uint8_t * bfr = Cmd_Malloc(line, maxbytes);
		value->bytes.data = bfr;
		if (token->delimiter)
		{
			return NParse_String(&str, (char *)bfr, maxbytes, &value->bytes.size) && (*str == 0);
		}
		else
		{
			return NParse_Bytes(&str, bfr, maxbytes, &value->bytes.size) && (*str == 0);
		}
	}
	default:
		return false;
	}
}

static const char * Cmd_ArgTypeStr(const CmdArg_t * arg)
{
	switch (arg->type)
	{
	case CmdArg_Number:
		return "number";
	case CmdArg_Bytes:
		return "bytes";
	default:
		return "UNKNOWN";
	}
}

static void Cmd_PrintMenuHelp(CmdLine_t * line, const CmdNode_t * node)
{
	Cmd_Printf(line, "<menu: %s> contains %d nodes:\r\n", node->name, node->menu.count);
	for (uint32_t i = 0; i < node->menu.count; i++)
	{
		const CmdNode_t * child = &node->menu.nodes[i];
		Cmd_Printf(line, " - %s\r\n", child->name);
	}
}

static void Cmd_PrintFunctionHelp(CmdLine_t * line, const CmdNode_t * node)
{
	Cmd_Printf(line, "<func: %s> takes %d arguments:\r\n", node->name, node->func.arglen);
	for (uint32_t argn = 0; argn < node->func.arglen; argn++)
	{
		const CmdArg_t * arg = &node->func.args[argn];
		Cmd_Printf(line, " - <%s: %s>\r\n", Cmd_ArgTypeStr(arg), arg->name);
	}
}

static void Cmd_RunMenu(CmdLine_t * line, const CmdNode_t * node, const char * str)
{
	CmdToken_t token;
	if (!Cmd_NextToken(line, &str, &token))
	{
		Cmd_Printf(line, "<menu: %s>\r\n", node->name);
	}
	if (strcmp("?", token.str) == 0)
	{
		Cmd_PrintMenuHelp(line, node);
	}
	else
	{
		const CmdNode_t * child = NULL;
		for (uint32_t i = 0; i < node->menu.count; i++)
		{
			if (strcmp(node->menu.nodes[i].name, token.str) == 0)
			{
				child = &node->menu.nodes[i];
				break;
			}
		}
		if (child == NULL)
		{
			Cmd_Printf(line, "'%s' is not an item within <menu: %s>\r\n", token.str, node->name);
		}
		else
		{
			// we may as well free this token before we run the next menu.
			Cmd_Free(line, (void*)token.str);
			Cmd_Run(line, child, str);
		}
	}
}

static void Cmd_RunFunction(CmdLine_t * line, const CmdNode_t * node, const char * str)
{
	CmdArgValue_t args[CMD_MAX_ARGS];
	uint32_t argn = 0;

	CmdToken_t token;
	bool token_ok = Cmd_NextToken(line, &str, &token);

	if (token_ok && strcmp("?", token.str) == 0)
	{
		Cmd_PrintFunctionHelp(line, node);
		return;
	}

	for (argn = 0; argn < node->func.arglen; argn++)
	{
		const CmdArg_t * arg = &node->func.args[argn];

		if (argn > 0)
		{
			// We already parsed our first token.
			token_ok = Cmd_NextToken(line, &str, &token);
		}

		if (!(token_ok && Cmd_ParseArg(line, arg, args + argn, &token)))
		{
			Cmd_Printf(line, "Argument %d is <%s: %s>\r\n", argn+1, Cmd_ArgTypeStr(arg), arg->name);
			return;
		}

		// Unfortunately we cannot free our tokens, as args will malloc on top of them.
	}
	if (argn != node->func.arglen)
	{
		Cmd_Printf(line, "<func: %s> required %d arguments\r\n", node->name, node->func.arglen);
	}
	else
	{
		node->func.callback(line, args);
	}
}

static void Cmd_Run(CmdLine_t * line, const CmdNode_t * node, const char * str)
{
	switch (node->type)
	{
	case CmdNode_Menu:
		Cmd_RunMenu(line, node, str);
		break;
	case CmdNode_Function:
		Cmd_RunFunction(line, node, str);
		break;
	}
}



/*
 * INTERRUPT ROUTINES
 */