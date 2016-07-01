#include "nvm0common.h"

void test_nvm0nvm()
{
    nvm_structure_build();
    nvm_system_init();
    nvm_system_close();
    nvm_structure_destroy();
}
