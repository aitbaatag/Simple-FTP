#include "../inc/Client/Client.hpp"
#include <iostream>

int main()
{
  char *ip;

  ip = "127.0.0.1";
    Client client("127.0.0.1" ,2000);

    client.CreateTCPIpv4Socket();
    client.InitSocketAdd(ip);
    client.ConnectToServer();
}
