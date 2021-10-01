#include "amtpack.h"
#include "amtpack.cpp"

#include <platform/linux.h>
#include <platform/linux.cpp>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s <asset directory>\n", argv[0]);
        return 1;
    }

    Linux_Parameters params = {};
    if (!LinuxInitialise(&params)) {
        return 1;
    }

    Packer_Context packer = {};
    Initialise(&packer.arena, Platform->GetMemoryAllocator(), Gigabytes(1));

    str8 directory = WrapZ(argv[1]);
    PackAssetsToAmt(&packer, directory);
}
