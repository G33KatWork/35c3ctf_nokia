#include "socket/sim_sock.h"
#include "socket/socket.h"
#include "sim.h"
#include "utils.h"
#include "logging.h"

#include <stdlib.h>

int main(int argc, char const *argv[])
{
    struct simcard *sim = sim_init();
    if(!sim) {
        EMSG("Error initializing SIM\n");
        exit(1);
    }

    IMSG("GSM 11.11 SIM initialized\n");  

    while(1) {
        sockets_poll();
    }

    sim_exit(sim);

    return 0;
}
