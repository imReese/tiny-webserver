#include "config.h"

int main(int argc, char *argv[]) {

    string user = "root";
    string password = "213514";
    string databasename = "root";

    Config config;
    config.parse_arg(argc, argv);

    Webserver server;

    server.init();

    //
    server.log_wirte();

    //
    server.sql_pool();

    //
    server.thread_pool();

    //
    server.trig_mode();

    //
    server.eventListen();

    // 
    server.eventLoop();

    return 0;
}