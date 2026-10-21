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
}

void cflags(Cmd *cmd) {
    cmd_append(cmd, "-std=c23");
    cmd_append(cmd, "-Wall");
    cmd_append(cmd, "-Wextra");
    cmd_append(cmd, "-Werror");
    cmd_append(cmd, "-Werror");
    cmd_append(cmd, "-Wno-missing-field-initializers");
    cmd_append(cmd, "-Wno-sign-compare");
    cmd_append(cmd, "-g");
    cmd_append(cmd, "-fsanitize=address");
#ifdef __APPLE__
    cmd_append(cmd, "-fuse-ld=lld");
#endif // __APPLE__
    // cmd_append(cmd, "--save-temps");
}

void libs(Cmd *cmd){
#ifdef _WIN32
    cmd_append(cmd, "-IC:/msys64/clang64/include"); 
    cmd_append(cmd, "-LC:/msys64/clang64/lib");
#elif __APPLE__
    cmd_append(cmd, "-I/opt/homebrew/Cellar/raylib/6.0/include");
    cmd_append(cmd, "-L/opt/homebrew/Cellar/raylib/6.0/lib");
#endif // _WIN32
    cmd_append(cmd, "-lraylib");
}

bool build() {
    cc(&cmd);
    cflags(&cmd);
    libs(&cmd);
    if (!cmd_run(&cmd)) return false;
    return true;
}

int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);

    if (!build()) return 1;
    return 0;
}

