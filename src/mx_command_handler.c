#include  "ush.h"

static void put_alias_value(t_shell *shell, t_key_value *alias) {
    char *new_line = strdup(alias->value);
    char *line_copy = shell->line;
    for(unsigned long i = 0; i < strlen(alias->name); i++) {
        line_copy++;
    }
    new_line = mx_strrejoin(new_line, line_copy);
    free(shell->line);
    shell->line = new_line;
}

void mx_command_handler(t_shell *shell) {
    if (!shell->line) {
        printf("\n\r");
        return;
    }
    char **words = mx_strsplit(shell->line, ' ');
    if (!words) {
        mx_free_words(words);
        return;
    }
    char *builtins[] = MX_BUILTINS_ARRAY;
    /*void (*functions[]) (t_shell*) = {
        &mx_alias, &mx_bg, &mx_cd, &mx_chdir, &mx_echo, &mx_env, &mx_exit, &mx_export,
        &mx_false, &mx_fg, &mx_jobs, &mx_kill, &mx_pwd, &mx_set, &mx_true, &mx_unset, &mx_which};*/

    void (*functions[]) (t_shell*) = {
        &mx_alias, NULL, &mx_cd, &mx_chdir, &mx_echo, &mx_env, &mx_exit, &mx_export,
        &mx_false, NULL, NULL, &mx_kill, &mx_pwd, NULL, &mx_true, &mx_unset, &mx_which};

    if (shell->new_line)
        printf("\n\r");

    int words_count = 0;
    while (words[++words_count]);
    t_key_value *copy = shell->aliases;
    bool found = true;
    if (!copy)
        found = false;
    while(copy) {
        char **copy_split = mx_strsplit(copy->name, ' ');
        int copy_name_count = 0;
        while (copy_split[++copy_name_count]);
        if (words_count != copy_name_count)
            found = false;
        else {
            for (int i = 0; words[i]; i++)
                if (strcmp(words[i], copy_split[i]) != 0)
                    found = false;
        }
        mx_free_words(copy_split);
        if (found) {
            put_alias_value(shell, copy);
            break;
        }
        else
            copy = copy->next;
    }
    if (found) {
        mx_free_words(words);
        words = mx_strsplit(shell->line, ' ');
    }
    for (int i = 0; i < MX_BUILTINS_COUNT; i++) {
        if (strcmp(words[0], builtins[i]) == 0) {
            functions[i](shell);
            break;
        }
        else if (i == MX_BUILTINS_COUNT - 1) {
            printf("ush: command not found: %s\n\r", words[0]);
            shell->exit_code = EXIT_FAILURE;
        }
    }
    mx_free_words(words);
}
