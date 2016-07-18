#include "nvm0common.h"
#include <unistd.h>
#include <pthread.h>

int main(void)
{
    nvm_structure_build();
    print_nvm_info();
	nvm_system_init();
    sleep(2);
    nvm_system_close();
}
