//
//  healthcenter.cpp
//  EE450_PA2
//
//  Created by Julius Yee on 10/30/17.
//  Copyright © 2017 Julius Yee. All rights reserved.
//

#include <iostream>
#include "healthcenter.h"


using namespace std;

healthcenter::healthcenter()
{
    doctor1Info = "";
    doctor2Info = "";
}

void healthcenter::setUpUDP()
{
    /////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////SET UP HEALTH CENTER UDP SOCKET AND BIND IT TO PORT///////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    
    int status;
    struct addrinfo *hs, *p;
    struct addrinfo hints;
    int yes = 1;
    //load up address structs with getaddrinfo()
    memset(&hints,0,sizeof hints); //make sure struct is empty
    hints.ai_family = AF_INET; //family type of socket
    hints.ai_socktype = SOCK_DGRAM; //socket type udp protocol
    hints.ai_flags = AI_PASSIVE; // fills in IP
    
    //hs points to linked list of addrinfo structs
    status = getaddrinfo(NULL,SERVER_PORT,&hints, &hs);
    if (status != 0)
    {
        perror("ERROR GETTING ADDRESS INFO");
    }
    
    //Loop through and bind to the first
    for (p = hs; p != NULL; p = p->ai_next)
    {
        if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1)
        {
            perror("SERVER SOCKET FAILED");
            continue;
        }
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if(::bind(sockfd,p->ai_addr,p->ai_addrlen)==-1)
        {
            close(sockfd);
            continue;
        }
        break;
    }
    
    if (p == NULL ){ fprintf(stderr, "Failed to bind UDP socket (health center)");}

    /////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////RECEIVE DATAGRAM FROM PATIENTS/////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    
    int count = 0;
    struct sockaddr_storage address;
    socklen_t fromlen;
    fromlen = sizeof address;
    while(count != 4)
    {
        int byte_count = recvfrom(sockfd, buffer, MAX_BUFFER_LENGTH - 1 ,0, (struct sockaddr *)&address,&fromlen);

        if (byte_count > 0)
        {
            buffer[byte_count] = '\0';
            //printf("\"%s\"\n",buffer);
            string input(buffer);
            std::stringstream stream(input);
            patientInfo patient;
            string name,ip, port,doc;
            stream >> name;
            patient.name = name;
            stream >> ip;
            patient.IP = ip;
            stream >> port;
            patient.port = port;
            stream >> doc;
            patient.doctor = doc;
            users.push_back(patient);
            count++;
            //cout << patient.name << " " << patient.IP << " " << patient.port << " " << patient.doctor << endl;
            cout<< "healthcenter: " << name << " registration is done successfully!" << endl;
        }
    }
    
    //Registration of patients is complete
    cout << "healthcenter: Registration of peers completed! Run the doctors!" << endl;
    
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////RECEIVE DATAGRAMS FROM DOCTORS FOR AUTHENTICATION & REPLY///////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    
    //Prepare the patient info to be sent to the doctors
    for (int i = 0; i < users.size(); i++ )
    {
        if (users[i].doctor == "Doctor1")
        {
            doctor1Info.append(users[i].name);
            doctor1Info.append(" ");
            doctor1Info.append(users[i].port);
            doctor1Info.append("\r\n");
        }
        else
        {
            doctor2Info.append(users[i].name);
            doctor2Info.append(" ");
            doctor2Info.append(users[i].port);
            doctor2Info.append("\r\n");
        }
    }
    
    ///Receive incoming UDP datagrams from patients
    int doc_count = 0;
    int doc_byte_count;
    struct sockaddr_in doc_address;
    socklen_t doc_fromlen;
    doc_fromlen = sizeof doc_address;
    
    //Get username and passwords for doctors
    ifstream inFile;\
    inFile.open("healthcenter.txt");
    if (!inFile)
    {
        cerr << "Unable to open file datafile.txt";
        exit(1);   // call system to stop
    }
    
    string line, doc1, doc2, doc1pass, doc2pass;
    int num_lines = 0;
    while (std::getline(inFile, line) && num_lines < 2)
    {
        std::istringstream iss(line);
        if (num_lines == 0)
        {
            iss >> doc1 >> doc1pass;
        }
        else
        {
            iss >> doc2 >> doc2pass;
        }
        num_lines++;
    }
    
    while(doc_count != 2)
    {
        doc_byte_count = recvfrom(sockfd, buffer, MAX_BUFFER_LENGTH - 1 ,0, (struct sockaddr *)&doc_address,&doc_fromlen);
        
        if (doc_byte_count == -1)
        {
            perror("ERROR RECVFROM UDP HEALTHCENTER FROM DOCTOR");
        }
        if (doc_byte_count > 0)
        {
            buffer[doc_byte_count] = '\0';
            //printf("\"%s\"\n",buffer);
            string input(buffer);
            std::stringstream stream(input);
            string username, password;
            stream >> username;
            stream >> password;
            
            cout << "healthcenter: "<< "incoming message from " << username << endl;
            
            if (((username == doc1) && (password == doc1pass)) || ((username == doc2) && (password == doc2pass)))
            {
                cout << "healthcenter: " << username << " logged in successfully!" << endl;
                if (doc_count == 0)
                {
                    if ( sendto(sockfd,doctor1Info.c_str(),strlen(doctor1Info.c_str()),0,(struct sockaddr*) &doc_address, doc_fromlen) == -1)
                    {
                        perror("Healthcenter failed to send patient info to doctor");
                    }
                }
                else
                {
                    if (sendto(sockfd,doctor2Info.c_str(),strlen(doctor2Info.c_str()),0,(struct sockaddr*) &doc_address, doc_fromlen) == -1)
                    {
                        perror("Healthcenter failed to send patient info to doctor");
                    }
                }
                doc_count++;
            }
            else
            {
                perror("HEALTHCENTER: DOC USERNAME AND PASSWORDS DON'T MATCH");
            }
        }
    }
    close(sockfd);
}

//////////////////////////////Main Function///////////////////////////////////////
int main(void)
{
    healthcenter health_center;
    health_center.setUpUDP();
    
}





