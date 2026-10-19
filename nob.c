#include <stdbool.h>
#define NOB_IMPLEMENTATION
#include "nob.h"

#define SRC "src/main.c"
#define DST "bin/hello-clay.exe"

Cmd cmd = {0};

void cc(Cmd *cmd) {
    cmd_append(cmd, "clang");
    cmd_append(cmd, SRC);
    cmd_append(cmd, "-o", DST);
    cmd_append(cmd, "-Wall");
    cmd_append(cmd, "-Wextra");
    cmd_append(cmd, "-Wno-missing-field-initializers");
    cmd_append(cmd, "-Wno-sign-compare");
}

void libs(Cmd *cmd){
    cmd_append(cmd, "-IC:/msys64/ucrt64/include"); 
    cmd_append(cmd, "-LC:/msys64/ucrt64/lib");
    cmd_append(cmd, "-lraylib");
}

bool build() {
    cc(&cmd);
    libs(&cmd);
    if (!cmd_run(&cmd)) return false;
    return true;
}

int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);

    if (!build()) return 1;
    return 0;
}

