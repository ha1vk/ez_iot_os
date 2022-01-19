#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <cli.h>
#include <stdio.h>

#include "ezos_libc.h"
#include "ezos_time.h"
#include "ezos_mem.h"
#include "ezos_thread.h"

#define RET_CHAR '\n'
#define END_CHAR '\r'
#define PROMPT   "# "
#define EXIT_MSG "exit"

#define csp_printf(...) ezos_printf(__VA_ARGS__)
#define fflush(...) do {} while (0)

#if (AOS_CLI_MINI_SIZE > 0)
char *cli_mini_support_cmds[] = { "netmgr", "help", "sysver",
                                  "reboot", "time", "ota" };
#endif

#ifdef CONFIG_EZIOT_COMPONENT_CLI_USE_CONSTRUCTOR
__attribute__((constructor(102))) void ezos_cli_init(void) 
{
    aos_cli_init(0);
}
#endif

static struct cli_st *cli     = NULL;
static int volatile cliexit   = 0;
char              esc_tag[64] = { 0 };
static unsigned char esc_tag_len = 0;
static ez_thread_t g_cli_thread = NULL;

int cli_getchar(char *inbuf);
int cli_putstr(char *msg);

static const struct cli_command *cli_command_get(int idx)
{
    if (!(idx >= 0 && idx < cli->num_commands)) {
        return (struct cli_command *)-1;
    }

    return cli->dynamic_cmds[idx];
}

/* Find the command 'name' in the cli commands table.
 * If len is 0 then full match will be performed else upto len bytes.
 * Returns: a pointer to the corresponding cli_command struct or NULL.
 */
static const struct cli_command *lookup_command(char *name, int len)
{
    int i = 0;
    int n = 0;

    while (i < cli->num_commands && n < cli->num_commands) {
        const struct cli_command *cmd = cli_command_get(i);
        if (cmd->name == NULL) {
            i++;
            continue;
        }
        /* See if partial or full match is expected */
        if (len != 0) {
            if (!ezos_strncmp(cmd->name, name, len)) {
                return cmd;
            }
        } else {
            if (!ezos_strcmp(cmd->name, name)) {
                return cmd;
            }
        }

        i++;
        n++;
    }

    return NULL;
}


/*proc one cli cmd and to run the according funtion
* Returns: 0 on success:
           1 fail
*/
int proc_onecmd(int argc, char *argv[])
{
    int                       i = 0;
    const char               *p;
    const struct cli_command *command = NULL;

    if (argc < 1) {
        return 0;
    }

    // if (!cli->echo_disabled) 
    {
        csp_printf("\r\n");
        fflush(stdout);
    }

    /*
     * Some comamands can allow extensions like foo.a, foo.b and hence
     * compare commands before first dot.
     */
    i = ((p = ezos_strchr(argv[0], '.')) == NULL) ? 0 : (p - argv[0]);

    command = lookup_command(argv[0], i);
    if (command == NULL) {
        return 1;
    }

    cli->outbuf = ezos_malloc(OUTBUF_SIZE);
    if (NULL == cli->outbuf) {
        aos_cli_printf("Error! cli alloc mem fail!\r\n");
        return 1;
    }
    ezos_memset(cli->outbuf, 0, OUTBUF_SIZE);

    command->function(cli->outbuf, OUTBUF_SIZE, argc, argv);
    aos_cli_printf("%s", cli->outbuf);

    ezos_free(cli->outbuf);
    cli->outbuf = NULL;
    return 0;
}


/* Parse input line and locate arguments (if any), keeping count of the number
 * of arguments and their locations.  Look up and call the corresponding cli
 * function if one is found and pass it the argv array.
 *
 * Returns: 0 on success: the input line contained at least a function name and
 *          that function exists and was called.
 *          1 on lookup failure: there is no corresponding function for the
 *          input line.
 *          2 on invalid syntax: the arguments list couldn't be parsed
 */
static int handle_input(char *inbuf)
{
    struct
    {
        unsigned inArg : 1;
        unsigned inQuote : 1;
        unsigned done : 1;
    } stat;
    static char *argvall[CLI_MAX_ONCECMD_NUM][CLI_MAX_ARG_NUM];
    int          argcall[CLI_MAX_ONCECMD_NUM] = { 0 };
    /*
    static char *argv[CLI_MAX_ONCECMD_NUM][CLI_MAX_ARG_NUM];
    int argc = 0;*/
    int  cmdnum = 0;
    int *pargc  = &argcall[0];
    int  i      = 0;
    int  ret    = 0;

    ezos_memset((void *)&argvall, 0, sizeof(argvall));
    ezos_memset((void *)&argcall, 0, sizeof(argcall));
    ezos_memset(&stat, 0, sizeof(stat));

    do {
        switch (inbuf[i]) {
            case '\0':
                if (stat.inQuote) {
                    return 2;
                }
                stat.done = 1;
                break;

            case '"':
                if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) {
                    ezos_memcpy(&inbuf[i - 1], &inbuf[i], ezos_strlen(&inbuf[i]) + 1);
                    --i;
                    break;
                }
                if (!stat.inQuote && stat.inArg) {
                    break;
                }
                if (stat.inQuote && !stat.inArg) {
                    return 2;
                }

                if (!stat.inQuote && !stat.inArg) {
                    stat.inArg   = 1;
                    stat.inQuote = 1;
                    (*pargc)++;
                    argvall[cmdnum][(*pargc) - 1] = &inbuf[i + 1];
                } else if (stat.inQuote && stat.inArg) {
                    stat.inArg   = 0;
                    stat.inQuote = 0;
                    inbuf[i]     = '\0';
                }
                break;

            case ' ':
                if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) {
                    ezos_memcpy(&inbuf[i - 1], &inbuf[i], ezos_strlen(&inbuf[i]) + 1);
                    --i;
                    break;
                }
                if (!stat.inQuote && stat.inArg) {
                    stat.inArg = 0;
                    inbuf[i]   = '\0';
                }
                break;

            case ';':
                if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) {
                    ezos_memcpy(&inbuf[i - 1], &inbuf[i], ezos_strlen(&inbuf[i]) + 1);
                    --i;
                    break;
                }
                if (stat.inQuote) {
                    return 2;
                }
                if (!stat.inQuote && stat.inArg) {
                    stat.inArg = 0;
                    inbuf[i]   = '\0';

                    if (*pargc) {
                        if (++cmdnum < CLI_MAX_ONCECMD_NUM) {
                            pargc = &argcall[cmdnum];
                        }
                    }
                }

                break;

            default:
                if (!stat.inArg) {
                    stat.inArg = 1;
                    (*pargc)++;
                    argvall[cmdnum][(*pargc) - 1] = &inbuf[i];
                }
                break;
        }
    } while (!stat.done && ++i < INBUF_SIZE && cmdnum < CLI_MAX_ONCECMD_NUM &&
             (*pargc) < CLI_MAX_ARG_NUM);

    if (stat.inQuote) {
        return 2;
    }

    for (i = 0; i <= cmdnum && i < CLI_MAX_ONCECMD_NUM; i++) {
        ret |= proc_onecmd(argcall[i], argvall[i]);
    }

    return ret;
}

/* Perform basic tab-completion on the input buffer by string-matching the
 * current input line against the cli functions table.  The current input line
 * is assumed to be NULL-terminated.
 */
static void tab_complete(char *inbuf, unsigned int *bp)
{
    int         i, n, m;
    const char *fm = NULL;

    aos_cli_printf("\r\n");

    /* show matching commands */
    for (i = 0, n = 0, m = 0; i < cli->num_commands
            && n < cli->num_commands; i++) {
        const struct cli_command *cmd = cli_command_get(i);
        if (cmd->name != NULL) {
            if (!ezos_strncmp(inbuf, cmd->name, *bp)) {
                m++;
                if (m == 1) {
                    fm = cmd->name;
                } else if (m == 2)
                    aos_cli_printf("%s  %s  ", fm, cmd->name);
                else
                    aos_cli_printf("%s  ", cmd->name);
            }
            n++;
        }
    }

    /* there's only one match, so complete the line */
    if (m == 1 && fm) {
        n = ezos_strlen(fm) - *bp;
        if (*bp + n < INBUF_SIZE) {
            ezos_memcpy(inbuf + *bp, fm + *bp, n);
            *bp += n;
            inbuf[(*bp)++] = ' ';
            inbuf[*bp]     = '\0';
        }
    }
    if (m >= 2) {
        aos_cli_printf("\r\n");
    }

    /* just redraw input line */
    aos_cli_printf("%s%s", PROMPT, inbuf);
}


#if (AOS_CLI_MINI_SIZE <= 0)

static void cli_history_input(void)
{
    char *inbuf   = cli->inbuf;
    int   charnum = ezos_strlen(cli->inbuf) + 1;

    int  his_cur  = cli->his_cur;
    int  left_num = INBUF_SIZE - his_cur;
    char lastchar;
    int  tmp_idx;

    cli->his_idx = his_cur;

    if (left_num >= charnum) {
        tmp_idx  = his_cur + charnum - 1;
        lastchar = cli->history[tmp_idx];
        ezos_strncpy(&(cli->history[his_cur]), inbuf, charnum);

    } else {
        tmp_idx  = (his_cur + charnum - 1) % INBUF_SIZE;
        lastchar = cli->history[tmp_idx];
        ezos_strncpy(&(cli->history[his_cur]), inbuf, left_num);
        ezos_strncpy(&(cli->history[0]), inbuf + left_num, charnum - left_num);
    }
    tmp_idx      = (tmp_idx + 1) % INBUF_SIZE;
    cli->his_cur = tmp_idx;

    /*overwrite*/
    if ('\0' != lastchar) {

        while (cli->history[tmp_idx] != '\0') {
            cli->history[tmp_idx] = '\0';
            tmp_idx               = (tmp_idx + 1) % INBUF_SIZE;
        }
    }
}


static void cli_up_history(char *inaddr)
{
    int index;
    int lastindex = 0;

    lastindex = cli->his_idx;
    index     = (cli->his_idx - 1 + INBUF_SIZE) % INBUF_SIZE;

    while ((cli->history[index] == '\0') && (index != cli->his_idx)) {
        index = (index - 1 + INBUF_SIZE) % INBUF_SIZE;
    }
    if (index != cli->his_idx) {
        while (cli->history[index] != '\0') {
            index = (index - 1 + INBUF_SIZE) % INBUF_SIZE;
        }
        index = (index + 1) % INBUF_SIZE;
    }
    cli->his_idx = index;

    while (cli->history[lastindex] != '\0') {

        *inaddr++ = cli->history[lastindex];
        lastindex = (lastindex + 1) % INBUF_SIZE;
    }
    *inaddr = '\0';

    return;
}

static void cli_down_history(char *inaddr)
{
    int index;
    int lastindex = 0;

    lastindex = cli->his_idx;
    index     = cli->his_idx;

    while ((cli->history[index] != '\0')) {
        index = (index + 1) % INBUF_SIZE;
    }
    if (index != cli->his_idx) {
        while (cli->history[index] == '\0') {
            index = (index + 1) % INBUF_SIZE;
        }
    }
    cli->his_idx = index;

    while (cli->history[lastindex] != '\0') {
        *inaddr++ = cli->history[lastindex];
        lastindex = (lastindex + 1) % INBUF_SIZE;
    }

    *inaddr = '\0';

    return;
}
#endif

static int get_input(char *inbuf, unsigned int *bp)
{
    char c;
    int32_t esc = 0;
    int32_t key1 = -1;
    int32_t key2 = -1;
    unsigned char cli_tag_len = 0;

    if (inbuf == NULL)
    {
        aos_cli_printf("input null\r\n");
        return 0;
    }

    while (cli_getchar(&c) == 1)
    {

        if (c == RET_CHAR || c == END_CHAR)
        { /* end of input line */
            inbuf[*bp] = '\0';
            *bp = 0;

            return 1;
        }

        if (c == 0x1b)
        { /* escape sequence */
            esc = 1;
            key1 = -1;
            key2 = -1;
            continue;
        }

        if (esc)
        {
            if (key1 < 0)
            {
                key1 = c;
                if (key1 != 0x5b)
                {
                    /* not '[' */
                    inbuf[*bp] = 0x1b;
                    (*bp)++;

                    inbuf[*bp] = key1;
                    (*bp)++;

                    // if (!cli->echo_disabled)
                    {
                        aos_cli_printf("\x1b%c", key1); /* Ignore the cli tag */
                    }
                    esc = 0;
                }
                continue;
            }

            if (key2 < 0)
            {
                key2 = c;
                if (key2 == 't')
                {
                    esc_tag[0] = 0x1b;
                    esc_tag[1] = key1;
                    cli_tag_len = 2;
                }
            }

            if (key2 != 0x41 && key2 != 0x42 && key2 != 't')
            {
                /* not UP key, not DOWN key, not ESC_TAG */
                inbuf[*bp] = 0x1b;
                (*bp)++;

                inbuf[*bp] = key1;
                (*bp)++;

                inbuf[*bp] = key2;
                (*bp)++;

                esc_tag[0] = '\x0';
                cli_tag_len = 0;
                esc = 0;

                // if (!cli->echo_disabled)
                {
                    aos_cli_printf("\x1b%c%c", key1, key2);
                }
                continue;
            }

#if CLI_MINIMUM_MODE > 0
            if (key2 == 0x41 || key2 == 0x42)
            {
                /* UP or DWOWN key */
                aos_cli_printf("\r\n" PROMPT "Warning! mini cli mode do not support history cmds!");
            }
#else
            if (key2 == 0x41 || key2 == 0x42)
            {
                /* UP or DWOWN key */
                if (key2 == 0x41)
                {
                    cli_up_history(inbuf);
                }
                else
                {
                    cli_down_history(inbuf);
                }

                *bp = strlen(inbuf);
                esc_tag[0] = '\x0';
                cli_tag_len = 0;
                esc = 0;

                aos_cli_printf("\r\n" PROMPT "%s", inbuf);
                continue;
            }
#endif
            if (key2 == 't')
            {
                /* ESC_TAG */
                /* Reserve 2 bytes space for the following */
                if (cli_tag_len >= sizeof(esc_tag) - 1)
                {
                    esc_tag[0] = '\x0';
                    cli_tag_len = 0;
                    esc = 0;

                    aos_cli_printf("Error: cli tag buffer overflow\r\n");
                    continue;
                }

                esc_tag[cli_tag_len++] = c;
                if (c == 'm')
                {
                    esc_tag[cli_tag_len++] = '\x0';

                    // if (!cli->echo_disabled)
                    {
                        aos_cli_printf("%s", esc_tag);
                    }
                    esc = 0;
                }
                continue;
            }
        }

        inbuf[*bp] = c;
        if ((c == 0x08) || (c == 0x7f))
        {
            if (*bp > 0)
            {
                (*bp)--;

                // if (!cli->echo_disabled)
                {
                    aos_cli_printf("%c %c", 0x08, 0x08);
                }
            }
            continue;
        }

        if (c == '\t')
        {
            inbuf[*bp] = '\0';
            tab_complete(inbuf, bp);
            continue;
        }

        // if (!cli->echo_disabled)
        {
            aos_cli_printf("%c", c);
        }

        (*bp)++;
        if (*bp >= INBUF_SIZE)
        {
            aos_cli_printf("Error: input buffer overflow\r\n");
            aos_cli_printf(PROMPT);
            *bp = 0;
            return 0;
        }
    }

    return 0;
}

/* Print out a bad command string, including a hex
 * representation of non-printable characters.
 * Non-printable characters show as "\0xXX".
 */
static void print_bad_command(char *cmd_string)
{
    if (cmd_string != NULL) {
        aos_cli_printf("command '%s' not found\r\n", cmd_string);
    }
}

static void cli_main_input()
{
    int   ret;
    char *msg = NULL;

    if (get_input(cli->inbuf, &cli->bp)) {
        msg = cli->inbuf;

    // if (ezos_strcmp(msg, EXIT_MSG) == 0) {
    //     cliexit = 1;
    //     return;
    //     }

#if (AOS_CLI_MINI_SIZE <= 0)
        if (ezos_strlen(cli->inbuf) > 0) {
            cli_history_input();
        }
#endif
        // aos_cli_printf("\r\n");
        ret = handle_input(msg);
        if (ret == 1) {
            print_bad_command(msg);
        } else if (ret == 2) {
            aos_cli_printf("syntax error\r\n");
        }

        aos_cli_printf("\r\n");
        esc_tag[0]  = '\x0';
        esc_tag_len = 0;
        aos_cli_printf(PROMPT);
    }
}

int aos_cli_main(void)
{
    cli_main_input();
    return 0;
}

static void help_cmd(char *buf, int len, int argc, char **argv);
#if (AOS_CLI_MINI_SIZE <= 0)

static void exit_cmd(char *buf, int len, int argc, char **argv);
static void devname_cmd(char *buf, int len, int argc, char **argv);
static void pmem_cmd(char *buf, int len, int argc, char **argv);
static void mmem_cmd(char *buf, int len, int argc, char **argv);

#endif
static void uptime_cmd(char *buf, int len, int argc, char **argv);

const struct cli_command built_ins[] = {
    /*cli self*/
    { "help", "print this", help_cmd },

#if (AOS_CLI_MINI_SIZE <= 0)

    { "p", "print memory", pmem_cmd },
    { "m", "modify memory", mmem_cmd },
    { "exit", "close CLI", exit_cmd },
    { "devname", "print device name", devname_cmd },
#endif

    /*aos_rhino*/
    { "time", "system time", uptime_cmd },
};

/* Built-in "help" command: prints all registered commands and their help
 * text string, if any.
 */
static void help_cmd(char *buf, int len, int argc, char **argv)
{
    int      i, n;
    unsigned int build_in_count = sizeof(built_ins) / sizeof(built_ins[0]);

    aos_cli_printf("====Build-in Commands====\r\n");
    aos_cli_printf("====Support %d cmds once, seperate by ; ====\r\n",
                   CLI_MAX_ONCECMD_NUM);

    for (i = 0; i < build_in_count; i++) {
        const struct cli_command *cmd = &built_ins[i];
        if (cmd->name) {
            aos_cli_printf("%-25s: %s\r\n", cmd->name,
                           cmd->help ? cmd->help : "");
        }
    }
    aos_cli_printf("\r\n");
    aos_cli_printf("====User Commands====\r\n");
    for (i = 0, n = build_in_count; i < cli->num_commands && n < cli->num_commands; i++) {
        const struct cli_command *cmd = cli_command_get(i);
        if (cmd >= built_ins && cmd < built_ins + build_in_count) {
            continue;
        }
        if (cmd->name) {
            aos_cli_printf("%-25s: %s\r\n", cmd->name,
                           cmd->help ? cmd->help : "");
            n++;
        }
    }
}


#if (AOS_CLI_MINI_SIZE <= 0)

static void exit_cmd(char *buf, int len, int argc, char **argv)
{
    cliexit = 1;
    return;
}

static void devname_cmd(char *buf, int len, int argc, char **argv)
{
    aos_cli_printf("device name: %s\r\n", "ezos cli device");
}

static void pmem_cmd(char *buf, int len, int argc, char **argv)
{
    int   i;
    char *pos    = NULL;
    char *addr   = NULL;
    int   nunits = 16;
    int   width  = 4;

    switch (argc) {
        case 4:
            width = ezos_strtol(argv[3], NULL, 0);
            __attribute__ ((fallthrough));
        case 3:
            nunits = ezos_strtol(argv[2], NULL, 0);
            nunits = nunits > 0x400 ? 0x400 : nunits;
            __attribute__ ((fallthrough));
        case 2:
            addr = (char *)ezos_strtol(argv[1], &pos, 0);
            break;
        default:
            break;
    }

    if (pos == NULL || pos == argv[1]) {
        aos_cli_printf("p <addr> <nunits> <width>\r\n"
                       "addr  : address to display\r\n"
                       "nunits: number of units to display (default is 16)\r\n"
                       "width : width of unit, 1/2/4 (default is 4)\r\n");
        return;
    }

    switch (width) {
        case 1:
            for (i = 0; i < nunits; i++) {
                if (i % 16 == 0) {
                    aos_cli_printf("0x%08x:", (unsigned int)addr);
                }
                aos_cli_printf(" %02x", *(unsigned char *)addr);
                addr += 1;
                if (i % 16 == 15) {
                    aos_cli_printf("\r\n");
                }
            }
            break;
        case 2:
            for (i = 0; i < nunits; i++) {
                if (i % 8 == 0) {
                    aos_cli_printf("0x%08x:", (unsigned int)addr);
                }
                aos_cli_printf(" %04x", *(unsigned short *)addr);
                addr += 2;
                if (i % 8 == 7) {
                    aos_cli_printf("\r\n");
                }
            }
            break;
        default:
            for (i = 0; i < nunits; i++) {
                if (i % 4 == 0) {
                    aos_cli_printf("0x%08x:", (unsigned int)addr);
                }
                aos_cli_printf(" %08x", *(unsigned int *)addr);
                addr += 4;
                if (i % 4 == 3) {
                    aos_cli_printf("\r\n");
                }
            }
            break;
    }
}

static void mmem_cmd(char *buf, int len, int argc, char **argv)
{
    void        *addr  = NULL;
    int          width = 4;
    unsigned int value = 0;
    unsigned int old_value;
    unsigned int new_value;

    switch (argc) {
        case 4:
            width = ezos_strtol(argv[3], NULL, 0);
            __attribute__ ((fallthrough));
        case 3:
            value = ezos_strtol(argv[2], NULL, 0);
            __attribute__ ((fallthrough));
        case 2:
            addr = (void *)ezos_strtol(argv[1], NULL, 0);
            break;
        default:
            addr = NULL;
            break;
    }

    if (addr == NULL) {
        aos_cli_printf("m <addr> <value> <width>\r\n"
                       "addr  : address to modify\r\n"
                       "value : new value (default is 0)\r\n"
                       "width : width of unit, 1/2/4 (default is 4)\r\n");
        return;
    }

    switch (width) {
        case 1:
            old_value = (unsigned int)(*(unsigned char volatile *)addr);
            *(unsigned char volatile *)addr = (unsigned char)value;
            new_value = (unsigned int)(*(unsigned char volatile *)addr);
            break;
        case 2:
            old_value = (unsigned int)(*(unsigned short volatile *)addr);
            *(unsigned short volatile *)addr = (unsigned short)value;
            new_value = (unsigned int)(*(unsigned short volatile *)addr);
            break;
        case 4:
        default:
            old_value                      = *(unsigned int volatile *)addr;
            *(unsigned int volatile *)addr = (unsigned int)value;
            new_value                      = *(unsigned int volatile *)addr;
            break;
    }
    aos_cli_printf("value on 0x%x change from 0x%x to 0x%x.\r\n", (unsigned int)addr,
                   old_value, new_value);
}

#endif

static void uptime_cmd(char *buf, int len, int argc, char **argv)
{
    long long ms;
    long long days;
    long long hours;
    long long minutes;
    long long seconds;

    ms = ezos_time(NULL);
    aos_cli_printf("UP time in ms %llu\r\n", ms);
    seconds = ms / 1000;
    minutes = seconds / 60;
    hours = minutes / 60;
    days = hours / 24;
    aos_cli_printf("UP time in %llu days, %llu hours, %llu minutes, %llu seconds\r\n",
            days,
            hours % 24,
            minutes % 60,
            seconds % 60
    );
}

int aos_cli_register_command(const struct cli_command *cmd)
{
    int i;

    if (!cli) {
        return EPERM;
    }

    if (!cmd->name || !cmd->function) {
        return EINVAL;
    }

    if (cli->num_commands >= MAX_DYNAMIC_COMMANDS)
    {
        return ENOMEM;
    }

    /* Check if the command has already been registered.
     * Return 0, if it has been registered.
     */
    for (i = 0; i < cli->num_commands; i++) {
        if (cli_command_get(i) == cmd) {
            return 0;
        }
    }

#if (AOS_CLI_MINI_SIZE > 0)
    for (i = 0; i < sizeof(cli_mini_support_cmds) / sizeof(char *); i++) {
        if (ezos_strcmp(cmd->name, cli_mini_support_cmds[i]) == 0) {
            break;
        }
    }
    if (i == sizeof(cli_mini_support_cmds) / sizeof(char *)) {
        aos_cli_printf("Warning! mini cli mode do not support cmd:%s\r\n",
                       cmd->name);
        return 0;
    }
#endif

    cli->dynamic_cmds[cli->num_commands++] = cmd;

    return 0;
}

int ezos_cli_register_command(const char *name, const char *help, void (*func)(char *, int, int, char **))
{
    struct cli_command *tmp_cli = (struct cli_command *)ezos_malloc(sizeof(struct cli_command));
    if (NULL == tmp_cli)
    {
        return -1;
    }
    tmp_cli->name = name;
    tmp_cli->help = help;
    tmp_cli->function = func;
    return aos_cli_register_command(tmp_cli);
}

int aos_cli_unregister_command(const struct cli_command *cmd)
{
    int i;
    int remaining_cmds;

    if (!cmd->name || !cmd->function) {
        return EINVAL;
    }

    for (i = 0; i < cli->num_commands; i++) {
        const struct cli_command *c = cli_command_get(i);
        if (c == cmd) {
            cli->num_commands--;
            remaining_cmds = cli->num_commands - i;
            if (remaining_cmds > 0) {
                ezos_memmove(&cli->dynamic_cmds[i], &cli->dynamic_cmds[i + 1],
                        (remaining_cmds * sizeof(struct cli_command *)));
            }
            ezos_free((void *)cmd);
            cli->dynamic_cmds[cli->num_commands] = NULL;
            return 0;
        }
    }

    return -ENOMEM;
}

int aos_cli_register_commands(const struct cli_command *cmds, int num_cmds)
{
    int i;
    int err;

    if (!cli) {
        return EPERM;
    }
    for (i = 0; i < num_cmds; i++) {
        if ((err = aos_cli_register_command(cmds++)) != 0) {
            return err;
        }
    }

    return 0;
}

int aos_cli_unregister_commands(const struct cli_command *cmds, int num_cmds)
{
    int i;
    int err;

    for (i = 0; i < num_cmds; i++) {
        if ((err = aos_cli_unregister_command(cmds++)) != 0) {
            return err;
        }
    }

    return 0;
}

int aos_cli_stop(void)
{
    cliexit = 1;
    if (NULL != g_cli_thread)
    {
        ezos_thread_destroy(g_cli_thread);
        g_cli_thread = NULL;
    }

    return 0;
}

static void cli_main()
{
    aos_cli_printf("\r\n");
    aos_cli_printf(PROMPT);
    while(!cliexit)
    {
        cli_main_input();
    }
}

int aos_cli_init(int use_thread)
{
    int ret;

    cli = (struct cli_st *)ezos_malloc(sizeof(struct cli_st));
    if (cli == NULL) {
        return ENOMEM;
    }

    ezos_memset((void *)cli, 0, sizeof(struct cli_st));

    ret = ezos_thread_create(&g_cli_thread, "cli_thread", cli_main, NULL, 4 * 1024, 10);
    if (0 != ret)
    {
        return -1;
    }

    /* add our built-in commands */
    if ((ret = aos_cli_register_commands(
           &built_ins[0], sizeof(built_ins) / sizeof(built_ins[0]))) != 0) {
        goto init_general_err;
    }
    cli->num_commands = sizeof(built_ins) / sizeof(built_ins[0]);
    cli->initialized = 1;
    cli->echo_disabled = 1;

    return 0;

init_general_err:
    if (cli) {
        ezos_free(cli);
        cli = NULL;
    }
    
    if (NULL != g_cli_thread)
    {
        ezos_thread_destroy(g_cli_thread);
        g_cli_thread = NULL;
    }

    return ret;
}

const char *aos_cli_get_tag(void)
{
    return esc_tag;
}

#if defined BUILD_BIN || defined BUILD_KERNEL
int aos_cli_printf(const char *msg, ...)
{
    va_list ap;

    char *pos, message[256];
    int   sz;
    int   len;

    ezos_memset(message, 0, 256);

    sz = 0;
    if (esc_tag_len) {
        ezos_strcpy(message, esc_tag);
        sz = ezos_strlen(esc_tag);
    }
    pos = message + sz;

    va_start(ap, msg);
    len = ezos_vsnprintf(pos, 256 - sz, msg, ap);
    va_end(ap);

    if (len <= 0) {
        return 0;
    }

    cli_putstr(message);

    return 0;
}
#endif

int cli_getchar(char *inbuf)
{
    *inbuf = (char)ezos_cli_getchar();
    return 1;
}

int cli_putstr(char *msg)
{
    int len, pos;

    len = strlen(msg);
    pos = 0;
    while (pos < len)
    {
        if ((ezos_putchar(*(char *)(msg + pos))) >= 0)
        {
            pos += 1; // move to next data block
            continue;
        }
        break;
    }

    return 0;
}
