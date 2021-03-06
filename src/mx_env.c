#include "ush.h"

static void check_flags(char **word, t_env *env, int *flags) {
    env->i = false;
    env->v = false;
    env->P = false;
    env->u = false;
    for (int i = 1; word[i]; i++) {
        if (word[i][0] == '-' && strlen(word[i]) > 1) {
            for (int j = 1; j < (int)strlen(word[i]); j++) {
                switch (word[i][j]) {
                    case 'i':
                        env->i = true;
                        break;
                    case 'v':
                        env->v = true;
                        break;
                    case 'P':
                        env->P = true;
                        break;
                    case 'u':
                        env->u = true;
                        break;
                    default:
                        *flags = -1;
                        return;
                }
            }
            *flags = *flags + 1;
        }
    }
}

static void env_var_handler(t_shell *shell, char **result, char *var) {
    char *var_value = NULL;
    if (strcmp(var, "$") == 0) {
        pid_t curr_id = getpid();
        var_value = mx_itoa(curr_id);
    }
    else if (strcmp(var, "?") == 0) {
        var_value = mx_itoa(shell->exit_code);
    }
    else {
        var_value = strdup(getenv(var));
    }
    mx_strdel(&var);
    if (!var_value)
        return;
    *result = mx_strrejoin(*result, var_value);
    mx_strdel(&var_value);
}

void mx_env(t_shell *shell) {
    char **words = mx_strsplit(shell->line, ' ');
    t_env *env = malloc(sizeof(t_env));
    int flags = 0;
    check_flags(words, env, &flags);

    char *usage = "usage: env [-iv] [-P utilpath] [-u name]\n\r\
           [name=value ...] [utility [argument ...]]\n\r";

    if (flags == -1) {
        printf("env: illegal option -- %c\n\r%s", words[1][1], usage);
        mx_free_words(words);
        free(env);
        shell->exit_code = EXIT_FAILURE;
        return;
    }

    char p[2];
    char **new_vars = NULL;
    char *unsetted = NULL;
    bool output = false;
    // env [name=value ...] [utility [argument ...]]
    if ((words[1] && flags == 0) || (words[2] && env->i)) {
        char *var = NULL;
        char *val = NULL;
        int quote = 0;
        int eq = 0;
        int skip_spaces = 0;
        for (int a = 1 + (int)env->i; words[a] && quote % 2 == 0; a++) {
            if (quote % 2 == 0) {
                for (int j = 0; words[a][j]; j++) {
                    if (words[a][j] == '=') {
                        eq = j + 1;
                        break;
                    }
                    MX_C_TO_P(words[a][j], p);
                    var = mx_strrejoin(var, p);
                }
            }
            // env [utility [argument ...]]
            if (eq == 0 && a == 1) {
                mx_strdel(&shell->line);
                for (int i = 1 + (int)env->i; words[i]; i++) {
                    for (int j = 0; words[i][j]; j++) {
                        MX_C_TO_P(words[i][j], p);
                        shell->line = mx_strrejoin(shell->line, p);
                    }
                    if (words[i + 1])
                        shell->line = mx_strrejoin(shell->line, " ");
                }
                mx_free_words(words);
                free(env);
                shell->new_line = false;
                mx_command_handler(shell);
                shell->new_line = true;
                return;
            }
            // env [name=value ...]
            if (eq > 0 && words[a][eq]) {
                char *sequenses = "abfnrtv";
                char *escapes = "\a\b\f\n\r\t\v";
                int backslash = 0;
                int brace1 = 0;
                int brace2 = 0;
                int bracket1 = 0;
                int bracket2 = 0;
                bool dollar = false;
                char *dollar_sequense = NULL;
                for (int i = a; words[i]; i++) {
                    if (i > a) eq = 0;
                    for (int j = eq; words[i][j]; j++) {
                        if (backslash == 2) {
                            bool correct = false;
                            for (int k = 0; sequenses[k]; k++) {
                                if (words[i][j] == sequenses[k]) {
                                    MX_C_TO_P(escapes[k], p);
                                    val = mx_strrejoin(val, p);
                                    backslash = 0;
                                    correct = true;
                                    break;
                                }
                            }
                            if (!correct) {
                                val = mx_strrejoin(val, "\\");
                                MX_C_TO_P(words[i][j], p);
                                val = mx_strrejoin(val, p);
                                backslash = 0;
                            }
                            continue;
                        }
                        if (words[i][j] == '"' || words[i][j] == '\'') {
                            if (backslash == 1) {
                                MX_C_TO_P(words[i][j], p);
                                val = mx_strrejoin(val, p);
                                backslash = 0;
                                continue;
                            }
                            else {
                                quote++;
                                continue;
                            }
                        }
                        if (words[i][j] == '$') {
                            if (!dollar) {
                                dollar = true;
                                continue;
                            }
                            else {
                                dollar = false;
                                continue;
                            }
                        }
                        if (dollar) {
                            if (words[i][j] == '{') {
                                brace1++;
                                continue;
                            }
                            else if (words[i][j] == '(') {
                                bracket1++;
                                continue;
                            }
                            else if (words[i][j] == '}') {
                                brace2++;
                                if (brace1 != brace2 && words[i][j + 1] != '}') {
                                    printf("zsh: bad substitution\n\r");
                                    mx_strdel(&dollar_sequense);
                                    mx_strdel(&var);
                                    mx_strdel(&val);
                                    mx_free_words(words);
                                    free(env);
                                    shell->exit_code = EXIT_FAILURE;
                                    return;
                                }
                                else if (words[i][j + 1] != '}') {
                                    dollar = false;
                                    env_var_handler(shell, &val, dollar_sequense);
                                }
                                continue;
                            }
                            else if (words[i][j] == ')') {
                                bracket2++;
                                if (bracket1 != bracket2 && words[i][j + 1] != ')') {
                                    printf("zsh: bad substitution\n\r");
                                    mx_strdel(&dollar_sequense);
                                    mx_strdel(&var);
                                    mx_strdel(&val);
                                    mx_free_words(words);
                                    free(env);
                                    shell->exit_code = EXIT_FAILURE;
                                    return;
                                }
                                else if (words[i][j + 1] != ')') {
                                    mx_strdel(&shell->line);
                                    shell->line = strdup(dollar_sequense);
                                    shell->new_line = false;
                                    mx_command_handler(shell);
                                    shell->new_line = true;
                                    mx_strdel(&dollar_sequense);
                                    dollar = false;
                                }
                                continue;
                            }
                            else {
                                MX_C_TO_P(words[i][j], p);
                                dollar_sequense = mx_strrejoin(dollar_sequense, p);
                                if (!words[i][j + 1]) {
                                    dollar = false;
                                    env_var_handler(shell, &val, dollar_sequense);
                                }
                                continue;
                            }
                        }
                        MX_C_TO_P(words[i][j], p);
                        val = mx_strrejoin(val, p);
                    }
                    if (quote % 2 == 0) {
                        break;
                    }
                    if (words[i + 1]) {
                        val = mx_strrejoin(val, " ");
                        skip_spaces++;
                    }
                }
            }
            if (quote % 2 == 0) {
                // printf("%s -> %s\n\r", var, val);  // for test
                if (!val) val = strdup("");
                setenv(var, val, 1);
                a += skip_spaces;
                skip_spaces = 0;
                int vars_len = 0;
                char **temp = NULL;
                if (new_vars) {
                    for (; new_vars[vars_len]; vars_len++);
                    temp = malloc(sizeof(char*) * (vars_len + 1));
                    for (int i = 0; new_vars[i]; i++)
                        temp[i] = strdup(new_vars[i]);
                    temp[vars_len] = NULL;
                }
                mx_free_words(new_vars);
                new_vars = malloc(sizeof(char*) * (vars_len + 2));
                if (temp)
                    for (int i = 0; temp[i]; i++)
                        new_vars[i] = strdup(temp[i]);
                new_vars[vars_len++] = strdup(var);
                new_vars[vars_len] = NULL;
                mx_strdel(&var);
                mx_strdel(&val);
                mx_free_words(temp);
            }
        }
        if (quote % 2 != 0) {
            printf("quote doesn't close\n\r");
            mx_free_words(new_vars);
            mx_free_words(words);
            free(env);
            shell->exit_code = EXIT_FAILURE;
            return;
        }
        else
            output = true;
    }
    else if (env->P) {
        int words_count = 0;
        while (words[++words_count]);
        words_count--;
        if (flags == words_count) {
            printf("env: option requires an argument -- P\n\r%s", usage);
            mx_free_words(new_vars);
            mx_free_words(words);
            free(env);
            shell->exit_code = EXIT_FAILURE;
            return;
        }
        else {
            output = true;
        }
    }
    else if (env->u) {
        int words_count = 0;
        while (words[++words_count]);
        words_count--;
        if (flags == words_count) {
            printf("env: option requires an argument -- u\n\r%s", usage);
            mx_free_words(new_vars);
            mx_free_words(words);
            free(env);
            shell->exit_code = EXIT_FAILURE;
            return;
        }
        else {
            unsetted = getenv(words[words_count]);
            unsetenv(words[words_count]);
            output = true;
        }
    }
    if (!words[1] || env->v) {
        output = true;
    }
    if (output) {
        extern char **environ;
        int env_len = 0;
        for (; environ[env_len]; env_len++);
        char **vars = malloc(sizeof(char*) * env_len + 1);
        int k = 0;
        for (int i = 0; environ[i]; i++) {
            vars[k++] = strdup(environ[i]);
        }
        vars[k] = NULL;

        for (int i = 0; vars[i]; i++) {
            bool is_new = true;
            if (env->i) {
                is_new = false;
                for (int j = 0; new_vars[j]; j++) {
                    char *temp = NULL;
                    for (int k = 0; new_vars[j][k] && vars[i][k]; k++) {
                        MX_C_TO_P(vars[i][k], p);
                        temp = mx_strrejoin(temp, p);
                    }
                    if (temp) {
                        if (strcmp(temp, new_vars[j]) == 0) {
                            is_new = true;
                            mx_strdel(&temp);
                            break;
                        }
                        mx_strdel(&temp);
                    }
                }
            }
            if (is_new)
                printf("%s\n\r", vars[i]);
        }
        if (new_vars)
            for (int i = 0; new_vars[i]; i++)
                unsetenv(new_vars[i]);
        if (unsetted) {
            int words_count = 0;
            while (words[++words_count]);
            words_count--;
            setenv(words[words_count], unsetted, 1);
        }
    }
    mx_free_words(new_vars);
    mx_free_words(words);
    free(env);
    shell->exit_code = EXIT_SUCCESS;
}
