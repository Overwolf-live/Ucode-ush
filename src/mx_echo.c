#include "ush.h"

static void check_flags(char **word, t_echo *echo, int *flags) {
    echo->E = false;
    echo->e = false;
    echo->n = false;
    bool E = false;
    bool e = false;
    bool n = false;
    bool flag = true;
    for (int i = 1; word[i]; i++) {
        if (word[i][0] == '-' && strlen(word[i]) > 1) {
            for (int j = 1; j < (int)strlen(word[i]) && flag; j++) {
                switch (word[i][j]) {
                    case 'E':
                        echo->E = true;
                        break;
                    case 'e':
                        echo->e = true;
                        break;
                    case 'n':
                        echo->n = true;
                        break;
                    default:
                        if (!E) echo->E = false;
                        if (!e) echo->e = false;
                        if (!n) echo->n = false;
                        flag = false;
                        break;
                }
            }
            if (flag)
                *flags = *flags + 1;
        }
        if (echo->E) E = true;
        if (echo->e) e = true;
        if (echo->n) n = true;
        flag = true;
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
        char *temp = getenv(var);
        if (temp)
            var_value = strdup(temp);
    }
    mx_strdel(&var);
    if (!var_value)
        return;
    *result = mx_strrejoin(*result, var_value);
    mx_strdel(&var_value);
}

void mx_echo(t_shell *shell) {
    char **words = mx_strsplit(shell->line, ' ');
    if (!words[1]) {
        mx_free_words(words);
        return;
    }

    t_echo *echo = malloc(sizeof(t_echo));
    int flags = 0, words_count = 0;
    check_flags(words, echo, &flags);
    while (words[++words_count]);
    words_count--;
    if (words_count == flags) {
        mx_free_words(words);
        free(echo);
        return;
    }
    if (strcmp(words[flags + 1], "~") == 0 && !words[flags + 2]) {
        printf("%s\n\r", getenv("HOME"));
        mx_free_words(words);
        free(echo);
        return;
    }
    if (strcmp(words[flags + 1], "~+") == 0 && !words[flags + 2]) {
        printf("%s\n\r", getenv("PWD"));
        mx_free_words(words);
        free(echo);
        return;
    }
    if (strcmp(words[flags + 1], "~-") == 0 && !words[flags + 2]) {
        printf("%s\n\r", getenv("OLDPWD"));
        mx_free_words(words);
        free(echo);
        return;
    }

    char *sequenses = "abfnrtv";
    char *escapes = "\a\b\f\n\r\t\v";
    int backslash = 0;

    int quote = 0;
    int brace1 = 0;
    int brace2 = 0;
    int bracket1 = 0;
    int bracket2 = 0;
    char *dollar_sequense = NULL;
    bool dollar = false;
    char *result = NULL;

    char p[2];
    for (int i = flags + 1; words[i]; i++) {
        for (int j = 0; words[i][j]; j++) {
            if (words[i][j] == '\\') {
                if (backslash < 2)
                    backslash++;
                if (backslash == 2 && (!words[i][j + 1] || echo->E)) {
                    backslash = 0;
                    result = mx_strrejoin(result, "\\");
                }
                continue;
            }
            if (!echo->E) {
                if (backslash == 2) {
                    bool correct = false;
                    for (int a = 0; sequenses[a]; a++) {
                        if (words[i][j] == sequenses[a]) {
                            MX_C_TO_P(escapes[a], p);
                            result = mx_strrejoin(result, p);
                            backslash = 0;
                            correct = true;
                            break;
                        }
                    }
                    if (!correct) {
                        result = mx_strrejoin(result, "\\");
                        MX_C_TO_P(words[i][j], p);
                        result = mx_strrejoin(result, p);
                        backslash = 0;
                    }
                    continue;
                }
            }
            if (words[i][j] == '"' || words[i][j] == '\'') {
                if (backslash == 1) {
                    MX_C_TO_P(words[i][j], p);
                    result = mx_strrejoin(result, p);
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
                        mx_strdel(&result);
                        mx_strdel(&dollar_sequense);
                        mx_free_words(words);
                        free(echo);
                        shell->exit_code = EXIT_FAILURE;
                        return;
                    }
                    else if (words[i][j + 1] != '}') {
                        dollar = false;
                        env_var_handler(shell, &result, dollar_sequense);
                    }
                    continue;
                }
                else if (words[i][j] == ')') {
                    bracket2++;
                    if (bracket1 != bracket2 && words[i][j + 1] != ')') {
                        printf("ush: parse error near `)'\n\r");
                        mx_strdel(&result);
                        mx_strdel(&dollar_sequense);
                        mx_free_words(words);
                        free(echo);
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
                        env_var_handler(shell, &result, dollar_sequense);
                    }
                    continue;
                }
            }
            MX_C_TO_P(words[i][j], p);
            result = mx_strrejoin(result, p);
        }
        if (words[i + 1])
            result = mx_strrejoin(result, " ");
    }
    if (quote % 2 != 0) {
        printf("quote doesn't close\n\r");
        mx_free_words(words);
        mx_strdel(&result);
        free(echo);
        shell->exit_code = EXIT_FAILURE;
        return;
    }
    if (!result)
        result = strdup("");
    printf("%s", result);
    if (echo->n)
        printf("%s%s%%%s", MX_BLACK_F, MX_WHITE_B, MX_RESET);
    printf("\n\r");

    mx_strdel(&result);
    mx_free_words(words);
    free(echo);
    shell->exit_code = EXIT_SUCCESS;
}
