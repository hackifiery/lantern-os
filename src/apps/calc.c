#include "api.h"

#define fmtWrite  k->fmtWrite
#define fmtGet    k->fmtGet
#define sfmtWrite k->sfmtWrite
#define strcmp    k->strcmp
#define atoi      k->atoi

static int tokenize(char* str, char** tokens, int max_tokens);

void main(struct KernelAPI *k) {
    char input[256];
    char* tokens[16];
    int res = 0;
    for (;;) {
        fmtWrite("calc -> ");
        fmtGet("%s", input);
        int tokenCount = tokenize(input, tokens, 16);
        for (unsigned int i = 0; i < tokenCount; i++) {
            // fmtWrite("token %d: %s\n", i, tokens[i]);
            char res_str[16] = "";
            sfmtWrite(res_str, "%d", res);
            if (strcmp(tokens[i], "$") == 0) {
                // fmtWrite("Replacing $ with %s\n", res_str);
                tokens[i] = res_str;
            }
        }
        #define cmd(s) else if (strcmp(tokens[0], s) == 0)
        if (tokenCount == 0) continue;
        cmd("add") res = atoi(tokens[1]) + atoi(tokens[2]);
        cmd("sub") res = atoi(tokens[1]) - atoi(tokens[2]);
        cmd("mul") res = atoi(tokens[1]) * atoi(tokens[2]);
        cmd("div") res = atoi(tokens[1]) / atoi(tokens[2]);
        cmd("exit") return;
        else {fmtWrite("Unknown operation: %s\n", tokens[0]); continue;}
        fmtWrite("%d\n", res);
        #undef cmd
    }
}

static int tokenize(char* str, char** tokens, int max_tokens) {
    int count = 0;
    int in_token = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ' && !in_token) {
            if (count < max_tokens) {
                tokens[count++] = &str[i];
                in_token = 1;
            }
        } else if (str[i] == ' ') {
            str[i] = '\0';
            in_token = 0;
        }
    }
    return count;
}