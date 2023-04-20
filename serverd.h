#pragma once

void init_server();

void demonize();

void signal_handler(int);

void handle_client(int);

