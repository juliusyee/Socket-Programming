//
//  doctor.cpp
//  doctor
//
//  Created by Julius Yee on 11/15/17.
//  Copyright © 2017 Julius Yee. All rights reserved.
//

#include "doctor.h"
#include <iostream>
using namespace std;

doctor::doctor(){num_subscribers = 0;}

void doctor::sendToHealthCenter(int doc_num)
{
    /////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////CREATE UDP SOCKET FOR DOCTOR AND BIND//////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    
    doctor_port[0] = '4';
    doctor_port[1] = '0'+ doc_num;
    doctor_port[2] = '4';
    doctor_port[3] = '9';
    doctor_port[4] = '2';
    doctor_port[5] = '\0';

    ///////////////SET UP UDP SOCKET FOR DOCTOR///////////
    //Initialize variables
    struct addrinfo hints, *hs, *p;
    int rv;
    int yex = 1;
    //Set up the properties of the socket
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    //check that we get valid address info
    if ((rv = getaddrinfo(NULL,doctor_port, &hints, &hs)) !=0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }
    
    //Loop through and bind to the first
    for (p = hs; p != NULL; p = p->ai_next)
    {
        if((sockfdUDP = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1)
        {
            perror("DOCTOR SOCKET FAILED");
            continue;
        }
        setsockopt(sockfdUDP, SOL_SOCKET, SO_REUSEADDR, &yex, sizeof(int));
        if(::bind(sockfdUDP,p->ai_addr,p->ai_addrlen)==-1)
        {
            close(sockfdUDP);
            continue;
        }
        break;
    }
    if (p == NULL )
    {
        perror("DOCTOR UDP failed to bind");
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////SEND DATAGRAM TO HEALTH CENTER WITH DOC# & PASSWORD///////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    stringstream tt;
    tt << doc_num;
    string toSend;
    toSend.append("doctor");
    toSend.append(tt.str());
    if (doc_num == 1)
    {
        toSend.append(" aaaaa");
    }
    else
    {
        toSend.append(" bbbbb");
    }
    int length = strlen(toSend.c_str());
    unsigned short hp = 30492;
    struct sockaddr_in recv_Addr;
    recv_Addr.sin_family = AF_INET;
    recv_Addr.sin_port = htons(hp);
    recv_Addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sendto(sockfdUDP,toSend.c_str(),length,0,(struct sockaddr*) &recv_Addr, sizeof recv_Addr);
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //////////////////RECEIVE DATAGRAM CONTAINING PATIENT INFO FROM HEALTH CENTER////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    struct sockaddr_storage address;
    socklen_t fromlen;
    fromlen = sizeof address;
    
    int byte_count = recvfrom(sockfdUDP, buffer, MAX_BUFFER_SIZE - 1 ,0, (struct sockaddr *)&address,&fromlen);
    
    if (byte_count == -1)
    {
        perror("ERROR RECVFROM UDP DOCTOR FROM HEALTHCENTER");
    }
    if (byte_count > 0)
    {
        buffer[byte_count] = '\0';
        string received_message = buffer;

        std::istringstream ss (received_message);
         std::string line1;
         while(std::getline(ss,line1))
         {
             std::istringstream tt(line1);
            subscribed_patient pat;
            tt >> pat.name >> pat.port;
            listOfPatients.push_back(pat);
             num_subscribers++;
         }
        cout << "doctor" << doc_num << ": " << buffer;
    }
    
    close(sockfdUDP);
    /////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////READ FROM SCHEDULE TEXT FILE////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    //Get username and passwords for doctors
    ifstream inFile;
    if (doc_num == 1)
    {
        inFile.open("doctor1.txt");
    }
    else
    {
        inFile.open("doctor2.txt");
    }
    if (!inFile)
    {
        cerr << "Unable to open file datafile.txt";
        exit(1);   // call system to stop
    }
    //Read the available day/times and store them into vector
    string line;
    while (std::getline(inFile, line))
    {
        appointment a;
        std::istringstream iss(line);
        iss >> a.day >> a.time;
        listOfAppointments.push_back(a);
    }
}

void doctor::interactWithPatient(int doc_num)
{
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////SET UP DOCTOR'S DYNAMICALLY ASSIGNED TCP PORT//////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    int yes = 1;
    
    //create socket
    sockfdTCP = socket(AF_INET,SOCK_STREAM,0);
    if (sockfdTCP == -1)
    {
        perror("ERROR PATIENT TCP SOCKET");
    }
    //Create dynamically assigned TCP port with local host as IP address
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = 0;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    //bind socket to dynamically assigned TCP port number
    setsockopt(sockfdTCP, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    
    if (::bind(sockfdTCP, (struct sockaddr*) &address, sizeof address) == -1)
    {
        perror("PATIENT TCP BIND ERROR");
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////
    ///////BASED ON # OF SUBSCRIBERS, PERFORM THE REQUIRED ACTIONS WITH TCP PORT/////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    
    //if no patient subscribers
    if (num_subscribers == 0)
    {
        cout << "Doctor" << doc_num << " has no peer subscribers!" << endl;
        return;
    }
    else if (num_subscribers == 1) // only 1 patient subscriber
    {
        cout << "Doctor" << doc_num << " has only one patient subscriber." << endl;
        //Create a TCP connection to that patient
        int oneSock, numbytes;
        struct addrinfo hints, *cl, *p;
        int rv;
        
        //Set the socket properties
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        
        //Acquire the address info
        if((rv = getaddrinfo("127.0.0.1",listOfPatients[0].port.c_str(),&hints,&cl))!=0)
        {
            fprintf(stderr, "DOCTOR TCP getaddrinfo: %s\n", gai_strerror(rv));
        }
        
        //loop through and connect to first socket we can and connect
        for (p = cl; p != NULL; p = p->ai_next)
        {
            if((oneSock = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
            {
                perror("DOCTOR TCP SOCKET CREATION ERROR TO SEND");
                continue;
            }
            
            if(connect(oneSock,(struct sockaddr *) p->ai_addr,p->ai_addrlen) == -1)
            {
                close(oneSock);
                perror("DOCTOR TPC CONNECT");
                continue;
            }
            break; //we only break if a socket was founded and connected with no errors
        }
        
        //if we come out of the loop and p is null, means nothing was connected
        if(p == NULL)
        {
            fprintf(stderr,"TCP: Doctor failed to connect to patient\n");
        }
        
        //Recevie ACK message for successful connection
        if((numbytes = recv(oneSock,buffer,MAX_BUFFER_SIZE-1,0))== -1)
        {
            perror("Doctor TCP Recv Error");
        }
        buffer[numbytes] = '\0';
        cout << "doctor" << doc_num << ": " << buffer << endl;
        
        //Send to welcome message to patient
        std::stringstream ss;
        string messageToSend = "1 welcome to doctor";
        ss << doc_num;
        messageToSend.append(ss.str());
        messageToSend.append(" group");
        int length = strlen(messageToSend.c_str());
        send(oneSock,messageToSend.c_str(),length,0);
        if((numbytes = recv(oneSock,buffer,MAX_BUFFER_SIZE-1,0))== -1)
        {
            perror("Doctor TCP Recv Error");
        }
        buffer[numbytes] = '\0';
        cout << "doctor" << doc_num << ": " << buffer << endl;
    }
    else //more than 1 patient subscriber
    {
        cout << "Doctor" << doc_num << " has " << num_subscribers << " patients!" << endl;
        //Create a TCP connection to that patient
        int oneSock, numbytes;
        struct addrinfo hints, *cl, *p;
        int rv;
        
        //Set the socket properties
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        
        //Acquire the address info
        if((rv = getaddrinfo("127.0.0.1",listOfPatients[0].port.c_str(),&hints,&cl))!=0)
        {
            fprintf(stderr, "DOCTOR TCP getaddrinfo: %s\n", gai_strerror(rv));
        }
        
        //loop through and connect to first socket we can and connect
        for (p = cl; p != NULL; p = p->ai_next)
        {
            if((oneSock = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
            {
                perror("DOCTOR TCP SOCKET CREATION ERROR TO SEND");
                continue;
            }
            
            if(connect(oneSock,(struct sockaddr *) p->ai_addr,p->ai_addrlen) == -1)
            {
                close(oneSock);
                perror("DOCTOR TPC CONNECT");
                continue;
            }
            break; //we only break if a socket was founded and connected with no errors
        }
        
        //if we come out of the loop and p is null, means nothing was connected
        if(p == NULL)
        {
            fprintf(stderr,"TCP: Doctor failed to connect to patient\n");
        }
        
        //Recevie ACK message for successful connection
        if((numbytes = recv(oneSock,buffer,MAX_BUFFER_SIZE-1,0))== -1)
        {
            perror("Doctor TCP Recv Error");
        }
        buffer[numbytes] = '\0';
        cout << "doctor" << doc_num << ": " << buffer << endl;
        
        //Send to welcome message to patient
        std::stringstream sss;
        string messageToSend = "";
        sss << num_subscribers;
        messageToSend.append(sss.str());
        messageToSend.append(" welcome to doctor");
        std::stringstream ss;
        ss << doc_num;
        messageToSend.append(ss.str());
        messageToSend.append(" group");
        int length = strlen(messageToSend.c_str());
        send(oneSock,messageToSend.c_str(),length,0);
        
        //Get ACK message for welcome message
        if((numbytes = recv(oneSock,buffer,MAX_BUFFER_SIZE-1,0))== -1)
        {
            perror("Doctor TCP Recv Error");
        }
        buffer[numbytes] = '\0';
        cout << "doctor" << doc_num << ": " << buffer << endl;
        
        //Construct message about patient subscribers and send
        string other_patients = "";
        for (int i = 1; i < listOfPatients.size(); i++)
        {
            other_patients.append(listOfPatients[i].name);
            other_patients.append(" ");
            other_patients.append(listOfPatients[i].port);
            other_patients.append("\r\n");
        }
        int length2 = strlen(other_patients.c_str());
        send(oneSock,other_patients.c_str(),length2,0);
        
        //Get ACK message for other patient info
        if((numbytes = recv(oneSock,buffer,MAX_BUFFER_SIZE-1,0))== -1)
        {
            perror("Doctor TCP Recv Error");
        }
        buffer[numbytes] = '\0';
        cout << "doctor" << doc_num << ": " << buffer << endl;
        
        //Construct message about other patient subscribers and send
        string schedule = "";
        for (int i = 0; i < listOfPatients.size(); i++)
        {
            schedule.append(listOfAppointments[i].day);
            schedule.append(" ");
            schedule.append(listOfAppointments[i].time);
            schedule.append("\r\n");
        }
        int length3 = strlen(schedule.c_str());
        send(oneSock,schedule.c_str(),length3,0);
        
        //Get ACK message for other patient info
        if((numbytes = recv(oneSock,buffer,MAX_BUFFER_SIZE-1,0))== -1)
        {
            perror("Doctor TCP Recv Error");
        }
        buffer[numbytes] = '\0';
        cout << "doctor" << doc_num << ": " << buffer << endl;
    }
}


//////////////////////////////////////////Main Function/////////////////////////////
int main(void)
{
    doctor d;
    int doctor_num = 0;
    int pid;
    while (doctor_num < 2)
    {
        if ((pid = fork()) == 0)
        {
            d.sendToHealthCenter(doctor_num + 1);
            d.interactWithPatient(doctor_num + 1);
            break;
        }
        else
        {
            doctor_num++;
        }
    }
    wait(NULL);
}



