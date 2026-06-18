#include <cpu.h>
#include <ram_mem.h>
#include <disk_storage.h>
#include <image_accelerator.h>
#include <routing.h>
#include <systemc.h>
#include <iostream>

using namespace std;

int sc_main(int argc, char* argv[]) {
    // Instantiate modules
    cpu cpu("cpu");
    ram_mem ram("ram");
    disk_storage disk("disk");
    image_accelerator accelerator("accelerator");
    routing bus("bus");

    // connect modules
    // Initiators to bus
    cpu.socket.bind(bus.target_cpu);
    accelerator.init_socket.bind(bus.target_accelator);

    // bus to targets
    bus.init_ram_mem.bind(ram.target_socket);
    bus.init_disk_storage.bind(disk.target_socket);
    bus.init_accelator.bind(accelerator.target_socket);

    cout<<"Starting image proccessing simulation"<<endl;
    sc_start();
    cout<<"Finishing image proccessing simulation"<<endl;
    return 0;
}
