#pragma once

typedef struct  s_key_value {            // Struct for a list of exported variables
    char *name;
    char *value;
    struct s_key_value *next;
}               t_key_value;

// typedef struct  s_stack {
//     int*        stack;
//     int         top;        // Index of last added job
//     int         last;       // Last job for fg
//     int         prev_last;
// }              t_stack;

typedef struct s_shell {
    char **history;
    char **history_copy;
    int    history_count;
    int    history_now;

    char  *line;
    char  *line_temp;
    int    line_len;
    int    carriage_pos;
    bool   new_line;

    // t_job   *jobs[MX_JOBS_NUMBER]; // TODO: Jobs
    // t_stack *jobs_stack;
    // int     max_number_job;

    char  *executable;                // Path to the shell executable
    char  *pwd;                       // Path to current working directory
    char  *prompt_name;               // Prompt name
    // int    prompt_status;             // TODO: Figure out what prompt_status is

    t_key_value *aliases;

    struct termios backup;
    // TODO: Explore utility
    // t_export *functions;
    // t_export *aliases;
    // struct termios t_original;
    // struct termios t_custom;
    // bool    custom_terminal;
    // pid_t   shell_pgid;
    // char    *git;
    // int     redir;
    // char    *kernal;
    int exit_flag;                     // ???Defaults 0, cheack if you have suspended jobs
    int exit_code;                     // ???Return if exit
}             t_shell;

/* Executable function flags */
typedef struct s_echo {
    bool E, e, n;
}              t_echo;
typedef struct s_pwd {
    bool P, L;
}              t_pwd;
typedef struct s_cd {
    bool P, L, s;
}              t_cd;
typedef struct s_env {
    bool i, v, P, u;
}              t_env;
typedef struct s_which {
    bool a, s;
}              t_which;
