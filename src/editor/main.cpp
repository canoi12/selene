/*
 * Selene Editor
 *
 * Front-end for the tooling side of the engine. The plain runtime
 * (src/main.c -> selene_main) only knows how to boot a game; everything
 * else lives here as a sub-command:
 *
 *   selene-editor run [path] [args...]   run a game (directory, .zip, .selene)
 *   selene-editor pack <target>          package a game (zip, appimage, ...)
 *   selene-editor edit [path]            open the editor UI (default)
 *   selene-editor version
 *   selene-editor help
 */
#include <cstdio>
#include <cstring>
#include <vector>

#include "selene_runtime.h"

namespace {

using CommandFn = int (*)(const char* prog, int argc, char** argv);

struct Command {
    const char* name;
    const char* args;
    const char* help;
    CommandFn fn;
};

int cmd_help(const char* prog, int argc, char** argv);

/* Run a game in-process through the runtime: argv becomes {prog, path?, args...} */
int cmd_run(const char* prog, int argc, char** argv) {
    std::vector<char*> args;
    args.push_back(const_cast<char*>(prog));
    for (int i = 0; i < argc; i++) args.push_back(argv[i]);
    args.push_back(nullptr);
    return selene_main(static_cast<int>(args.size()) - 1, args.data());
}

int cmd_pack(const char* prog, int argc, char** argv) {
    (void)argc; (void)argv;
    std::fprintf(stderr, "%s pack: not implemented yet\n", prog);
    return 1;
}

int cmd_edit(const char* prog, int argc, char** argv) {
    (void)argc; (void)argv;
    std::fprintf(stderr, "%s edit: not implemented yet\n", prog);
    return 1;
}

int cmd_version(const char* prog, int argc, char** argv) {
    (void)prog; (void)argc; (void)argv;
    std::printf("selene-editor %s (%s)\n", SELENE_VERSION, LUA_RELEASE);
    return 0;
}

const Command commands[] = {
    {"run",     "[path] [args...]", "run a game (directory, .zip or .selene)", cmd_run},
    {"pack",    "<target>",         "package the game in the current directory", cmd_pack},
    {"edit",    "[path]",           "open the editor (default command)", cmd_edit},
    {"version", "",                 "print version information", cmd_version},
    {"help",    "",                 "show this help", cmd_help},
};

int cmd_help(const char* prog, int argc, char** argv) {
    (void)argc; (void)argv;
    std::printf("usage: %s <command> [options]\n\ncommands:\n", prog);
    for (const Command& c : commands)
        std::printf("  %-8s %-18s %s\n", c.name, c.args, c.help);
    return 0;
}

const Command* find_command(const char* name) {
    for (const Command& c : commands)
        if (std::strcmp(c.name, name) == 0) return &c;
    return nullptr;
}

} // namespace

int main(int argc, char** argv) {
    const char* prog = "selene-editor";
    if (argc < 2) return cmd_edit(prog, 0, nullptr);

    const char* name = argv[1];
    if (!std::strcmp(name, "-h") || !std::strcmp(name, "--help")) name = "help";
    else if (!std::strcmp(name, "-v") || !std::strcmp(name, "--version")) name = "version";

    const Command* cmd = find_command(name);
    if (!cmd) {
        std::fprintf(stderr, "%s: unknown command '%s'\n\n", prog, argv[1]);
        cmd_help(prog, 0, nullptr);
        return 1;
    }
    return cmd->fn(prog, argc - 2, argv + 2);
}
