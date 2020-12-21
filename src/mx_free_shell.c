#include "ush.h"

void mx_free_shell(t_shell *shell) {
    //Free history variables
    for (int i = 0; i < shell->history_count; i++)
        free(shell->history[i]);
    free(shell->history);
    for (int i = 0; i < shell->history_count; i++) {
        if(shell->history_copy[i] != NULL) {
            free(shell->history_copy[i]);
            shell->history_copy[i] = NULL;
        }
    }
    free(shell->history_copy);

    //Free line variables
    if (shell->line)
        free(shell->line);
    if (shell->line_temp)
        free(shell->line_temp);

    //Free aliases
    if (shell->aliases) {
        t_key_value *copy = shell->aliases;
        while(copy->next) {
            copy = shell->aliases->next;
            if(shell->aliases->name)
                free(shell->aliases->name);
            if(shell->aliases->value)
                free(shell->aliases->value);
            free(shell->aliases);
            shell->aliases = copy;
        }
        if(shell->aliases->name)
            free(shell->aliases->name);
        if(shell->aliases->value)
            free(shell->aliases->value);
        free(shell->aliases);
    }

    // TODO free other fields
    free(shell);
}
