#include "ush.h"

void mx_exit(t_shell *shell) {
    int exit_code = EXIT_SUCCESS;  // TODO exit code
    mx_free_shell(shell);
    tcsetattr(0, TCSANOW, &shell->backup);
    exit(exit_code);
}
