#pragma once

#include "ushtd.h"

void mx_alias(t_shell*);
// void mx_bg(t_shell*);
void mx_cd(t_shell*);
void mx_chdir(t_shell*);
// void mx_declare(t_shell*);
void mx_echo(t_shell*);
void mx_env(t_shell*);
void mx_exit(t_shell*);
void mx_export(t_shell*);
void mx_false(t_shell*);
// void mx_fg(t_shell*);
// void mx_jobs(t_shell*);
void mx_kill(t_shell*);
void mx_pwd(t_shell*);
// void mx_set(t_shell*);
void mx_true(t_shell*);
void mx_unset(t_shell*);
void mx_which(t_shell*);

t_shell *mx_shell_init();
void mx_loop(t_shell*);

void mx_command_handler(t_shell*);
void mx_free_words(char**);
void mx_free_shell(t_shell*);
