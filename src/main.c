#include "runners/default.c"

static int main_symbol_test(int a) { return -1; }

int main(int argc, char** argv) {
    return selene_main(argc, argv);
}
