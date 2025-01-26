#include "runners/default.c"

static int main_symbol_test(int a) { return -1; }

#if defined(MINGW)
int SDL_main(int argc, char** argv) {
#else
int main(int argc, char** argv) {
#endif
    return selene_main(argc, argv);
}
