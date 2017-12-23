//
//  doctor.h
//  doctor
//
//  Created by Julius Yee on 11/15/17.
//  Copyright © 2017 Julius Yee. All rights reserved.
//

#ifndef doctor_h
#define doctor_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <vector>

#define HEALTH_CENTER_PORT "30492"
#define BACK_LOG 20
#define MAX_BUFFER_SIZE 256

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

class doctor
{
public:
    doctor();
    void sendToHealthCenter(int doc_num);
    void interactWithPatient(int doc_num);
private:
    char buffer[MAX_BUFFER_SIZE];
    char hostname[256];
    char ip_address[256];
    int sockfdUDP;
    int sockfdTCP;
    char doctor_port [6];
    char patient_port[6];
    int num_subscribers;
    std::vector<appointment> listOfAppointments;
    std::vector<subscribed_patient> listOfPatients;
};

#endif /* doctor_h */
