//
//  patient.h
//  EE450_PA2
//
//  Created by Julius Yee on 11/13/17.
//  Copyright © 2017 Julius Yee. All rights reserved.
//

#ifndef patient_h
#define patient_h

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sstream>
#include <vector>

#define HEALTH_CENTER_PORT "30492"
#define MAX_BUFFER_SIZE 100
#define BACK_LOG 20

struct appointment
{
    std::string day;
    std::string time;
};

struct subscribed_patient
{
    std::string name;
    std::string port;
};


class patient
{
public:
    patient();
    void sendToHealthCenter(int pn);
private:
    char buffer[MAX_BUFFER_SIZE];
    char hostname[256];
    char ip_address[256];
    int sockfdUDP;
    int sockfdTCP;
    char patientUDP_port [6];
    std::string TCPport;
};

#endif

