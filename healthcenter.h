//
//  healthcenter.h
//  EE450_PA2
//
//  Created by Julius Yee on 11/11/17.
//  Copyright © 2017 Julius Yee. All rights reserved.
//

#ifndef healthcenter_h
#define healthcenter_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <vector>
#include <sstream>
#include <fstream>

#define SERVER_PORT "30492"
#define MAX_BUFFER_LENGTH 100
#define BACKLOG 20

struct patientInfo
{
    std::string name;
    std::string IP;
    std::string port;
    std::string doctor;
};

class healthcenter
{
public:
    healthcenter(); //constructor
    void setUpUDP();
private:
    char hostname[256];
    char ip_address[256];
    int sockfd;
    char buffer[MAX_BUFFER_LENGTH];
    std::vector<patientInfo> users;
    std::string doctor1Info;
    std::string doctor2Info;
};

#endif

