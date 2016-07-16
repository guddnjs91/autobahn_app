#include "nvm0common.h"

int main(void)
{
    nvm_structure_build();
    print_nvm_info();
	nvm_system_init();
    nvm_system_close();
}
