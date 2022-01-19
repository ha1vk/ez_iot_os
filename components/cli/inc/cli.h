#ifndef AOS_CLI_H
#define AOS_CLI_H

#include "ezos_gconfig.h"
#include "ezos_libc.h"

#define CONFIG_AOS_CLI
#define BUILD_BIN 1

#ifndef AOS_CLI_MINI_SIZE
#define AOS_CLI_MINI_SIZE       0
#endif

#if(AOS_CLI_MINI_SIZE > 0)

/*can config to cut mem size*/
#define INBUF_SIZE   64
#define OUTBUF_SIZE  32    /*not use now*/
#define MAX_DYNAMIC_COMMANDS 10

#define CLI_MAX_ARG_NUM    8
#define CLI_MAX_ONCECMD_NUM    1

#else

/*can config to cut mem size*/
#define INBUF_SIZE   256
#define OUTBUF_SIZE  512
#define MAX_DYNAMIC_COMMANDS 128

#define CLI_MAX_ARG_NUM    16
#define CLI_MAX_ONCECMD_NUM    4

#endif


#ifndef CONFIG_AOS_CLI_STACK_SIZE
#define CONFIG_AOS_CLI_STACK_SIZE 2048
#endif


#ifndef FUNCPTR
typedef void (*FUNCPTR)(void);
#endif

/* Structure for registering CLI commands */
struct cli_command {
    const char *name;
    const char *help;

    void (*function)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
};

struct cli_st {
    int initialized;
    int echo_disabled;

    const struct cli_command *dynamic_cmds[MAX_DYNAMIC_COMMANDS]; // dynamic commands

    unsigned int num_commands; // static + dynamic commands
    unsigned int bp; /* buffer pointer */

    char inbuf[INBUF_SIZE];
    char *outbuf;

#if(AOS_CLI_MINI_SIZE <= 0)
    int his_idx;
    int his_cur;
    char history[INBUF_SIZE];
#endif
};

#define CLI_ARGS char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv

#ifdef CONFIG_AOS_CLI

#define cmd_printf(...)                                            \
    do {                                                           \
        if (xWriteBufferLen > 0) {                                 \
            ezos_snprintf(pcWriteBuffer, xWriteBufferLen, __VA_ARGS__); \
            xWriteBufferLen-= ezos_strlen(pcWriteBuffer);            \
            pcWriteBuffer+= ezos_strlen(pcWriteBuffer);              \
        }                                                          \
    } while(0)


/**
 * This function registers a command with the command-line interface.
 *
 * @param[in]  command  The structure to register one CLI command
 *
 * @return  0 on success, error code otherwise.
 */
int aos_cli_register_command(const struct cli_command *command);

/**
 * This function unregisters a command from the command-line interface.
 *
 * @param[in]  command  The structure to unregister one CLI command
 *
 * @return  0 on success,  error code otherwise.
 */
int aos_cli_unregister_command(const struct cli_command *command);

/**
 * Register a batch of CLI commands
 * Often, a module will want to register several commands.
 *
 * @param[in]  commands      Pointer to an array of commands.
 * @param[in]  num_commands  Number of commands in the array.
 *
 * @return  0 on success， error code otherwise.
 */
int aos_cli_register_commands(const struct cli_command *commands, int num_commands);

/**
 * Unregister a batch of CLI commands
 *
 * @param[in]  commands      Pointer to an array of commands.
 * @param[in]  num_commands  Number of commands in the array.
 *
 * @return  0 on success, error code otherwise.
 */
int aos_cli_unregister_commands(const struct cli_command *commands, int num_commands);

/**
 * Print CLI msg
 *
 * @param[in]  buff  Pointer to a char * buffer.
 *
 * @return  0  on success, error code otherwise.
 */
#if defined BUILD_BIN || defined BUILD_KERNEL
/* SINGLEBIN or KERNEL */
int aos_cli_printf(const char *buff, ...);
#else
/* FRAMWORK or APP */
#define aos_cli_printf(fmt, ...) csp_printf("%s" fmt, aos_cli_get_tag(), ##__VA_ARGS__)
#endif

int ezos_cli_register_command(const char *name, const char *help, void (*func)(char*, int, int, char**));

#ifdef CONFIG_EZIOT_COMPONENT_CLI_USE_CONSTRUCTOR
#define EZOS_CLI_EXPORT(name, help, func)                        \
    __attribute__((constructor)) void _export_##func() \
    {                                                       \
        ezos_cli_register_command(name, help, func);              \
    }
#endif

/**
 * CLI initial function
 *
 * @return  0 on success, error code otherwise
 */
int aos_cli_init(int use_thread);

int aos_cli_main(void);

/**
 * Stop the CLI thread and carry out the cleanup
 *
 * @return  0 on success, error code otherwise.
 *
 */
int aos_cli_stop(void);

/**
 * CLI get tag string
 *
 * @return cli tag storing buffer
 */
const char *aos_cli_get_tag(void);

/**
 * get CLI task handle
 *
 * @return task handle
 */
void *aos_cli_task_get(void);

/**
 * create CLI task
 *
 * @return 0 success, others not success
 */
int aos_cli_task_create(void);

#else /* CONFIG_AOS_CLI */

#define cmd_printf(...) do {} while(0)

RHINO_INLINE int aos_cli_register_command(const struct cli_command *command)
{
    return 0;
}

RHINO_INLINE int aos_cli_unregister_command(const struct cli_command *command)
{
    return 0;
}

RHINO_INLINE int aos_cli_register_commands(const struct cli_command *commands,
                                           int num_commands)
{
    return 0;
}

RHINO_INLINE int aos_cli_unregister_commands(const struct cli_command *commands,
                                             int num_commands)
{
    return 0;
}

#define aos_cli_printf csp_printf

RHINO_INLINE int aos_cli_init(void)
{
    return 0;
}

RHINO_INLINE int aos_cli_stop(void)
{
    return 0;
}

RHINO_INLINE void *aos_cli_task_get(void)
{
    return NULL;
}

RHINO_INLINE int aos_cli_task_create(void)
{
    return 0;
}

#endif

#endif /* AOS_CLI_H */

