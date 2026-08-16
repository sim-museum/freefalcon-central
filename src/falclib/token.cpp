#include <cISO646>
#include <string.h>
#include <stdlib.h>
#include "token.h"

// MLR 12/13/2003 - Simple token parsing

// FF_LINUX: \r must be a delimiter. The game's data files are CRLF; Windows
// text-mode fgets strips the \r, Linux keeps it. Without \r here the LAST token
// on every line carries a trailing carriage return, so TokenEnum's
// stricmp(arg, *enumnames) never matches and silently returns the caller's
// default, and TokenStr hands back a name with a \r glued on. Same class as the
// particlesys.ini effect-name bug (5783ca73), but in the SHARED tokenizer.

char *tokenStr = 0;

float TokenF(float def)
{
    return(TokenF(tokenStr, def));
}


float TokenF(char *str, float def)
{
    char *bs;

    tokenStr = 0;

    if (bs = strtok(str, " ,\t\n\r"))
    {
        return((float)atof(bs));
    }

    return(def);
}


int TokenI(int def)
{
    return(TokenI(tokenStr, def));
}


int TokenI(char *str, int def)
{
    char *bs;

    tokenStr = 0;

    if (bs = strtok(str, " ,\t\n\r"))
    {
        return(atoi(bs));
    }

    return(def);
}


int TokenFlags(int def, char *flagstr)
{
    return(TokenFlags(tokenStr, def, flagstr));
}


int TokenFlags(char *str, int def, char *flagstr)
{
    char *arg;
    int flags = 0;

    tokenStr = 0;

    if (arg = strtok(str, " ,\t\n\r"))
    {
        while (*arg)
        {
            int l;

            for (l = 0; l < 32 and flagstr[l]; l++)
            {
                if (*arg == flagstr[l])
                {
                    flags or_eq 1 << l;
                }
            }

            arg++;
        }

        return(flags);
    }

    return(def);
}

int TokenEnum(char **enumnames, int def)
{
    return(TokenEnum(tokenStr, enumnames, def));
}


int TokenEnum(char *str, char **enumnames, int def)
{
    char *arg;
    int i = 0;

    tokenStr = 0;

    if (arg = strtok(str, " ,\t\n\r"))
    {
        while (*enumnames)
        {
            if (stricmp(arg, *enumnames) == 0)
            {
                return i;
            }

            enumnames++;
            i++;
        }
    }

    return def;

}

void SetTokenString(char *str)
{
    tokenStr = str;
}

char *TokenStr(char *def)
{
    return(TokenStr(tokenStr, def));
}


char *TokenStr(char *str, char *def)
{
    char *bs;

    tokenStr = 0;

    if (bs = strtok(str, " :,\t\n\r"))
    {
        return(bs);
    }

    return(def);
}
