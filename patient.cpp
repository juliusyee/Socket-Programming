//
//  patient.cpp
//  EE450_PA2
//
//  Created by Julius Yee on 10/30/17.
//  Copyright © 2017 Julius Yee. All rights reserved.
//

#include "patient.h"
#include <string>
using namespace std;

patient::patient(){}

//This function creates the UDP socket and binds it to its UDP port number for the patient
void patient::sendToHealthCenter(int pn)
{
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////CREATE PATIENT UDP SOCKET AND BIND//////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    
    patientUDP_port[0] = '5';
    patientUDP_port[1] = '0'+ pn;
    patientUDP_port[2] = '4';
    patientUDP_port[3] = '9';
    patientUDP_port[4] = '2';
    patientUDP_port[5] = '\0';
    ///////////////SET UP UDP SOCKET FOR PATIENT CLIENT///////////
    //Initialize variables
    struct addrinfo hints, *hs, *p;
    int rv;
    int yez = 1;
    //Set up the properties of the socket
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    //check that we get valid address info
    if ((rv = getaddrinfo(NULL,patientUDP_port, &hints, &hs)) !=0)
    {
        perror("ERROR PATIENT UDP: GETADDRINFO");
    }

    //Loop through and bind to the first
    for (p = hs; p != NULL; p = p->ai_next)
    {
        if((sockfdUDP = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1)
        {
            perror("SERVER SOCKET FAILED");
            continue;
        }
        setsockopt(sockfdUDP, SOL_SOCKET, SO_REUSEADDR, &yez, sizeof(int));
        if(::bind(sockfdUDP,p->ai_addr,p->ai_addrlen)==-1)
        {
            close(sockfdUDP);
            continue;
        }
        break;
    }
    if (p == NULL )
    {
        perror("Patient UDP failed to bind");
    }
    
    //Obtain IP address
    hostent* host = gethostbyname("localhost");
    in_addr** addresses = (in_addr **)host->h_addr_list;
    
    for (int i = 0; addresses[i] != NULL; i++)
    {
        strcpy(ip_address, inet_ntoa(*addresses[i]));
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////SET UP PATIENT'S DYNAMICALLY ASSIGNED TCP PORT//////////////////////
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
    
    //Obtain the port number that this patient was dynamically assigned to
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    unsigned short u;
    if (getsockname(sockfdTCP, (struct sockaddr *)&sin, &len) == -1)
    {
        perror("PATIENT TCP: getsockname");
    }
    else
    {
        u = ntohs(sin.sin_port);
    }
    /////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////SEND DATAGRAM TO HEALTH CENTER UDP SOCKET//////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    
    struct addrinfo hintz, *cl, *q;
    int x;
    //Acquire the address info of the health center
    memset(&hintz,0,sizeof(hintz));
    hintz.ai_family = AF_INET;
    hintz.ai_socktype = SOCK_DGRAM;

    if((x = getaddrinfo(NULL,HEALTH_CENTER_PORT,&hintz,&cl))!=0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(x));
        //perror("PATIENT ERROR GET ADDRINFO FOR CONNECTION TO HEALTH CENTER");
    }

    //Message to send to health center server preparation
    std::stringstream ss;
    string toSend;
    toSend.append("Patient");
    ss << pn;
    toSend.append(ss.str());
    
    toSend.append(" ");
    string str(ip_address);
    toSend.append(str);
    
    toSend.append(" ");
    std::stringstream zz;
    zz << u;
    TCPport = zz.str();
    toSend.append(TCPport);
    
    std::stringstream tt;
    int num = (rand()%2) + 1;
    tt << num;
    toSend.append(" Doctor");
    toSend.append(tt.str());
    int length = strlen(toSend.c_str());

    int cl_socket;
    int j;
    for(q = cl; q != NULL; q = q->ai_next)
    {
        if((cl_socket = socket(q->ai_family,q->ai_socktype,q->ai_protocol)) == -1)
        {
            perror("PHASE 1: PATIENT ERROR SOCKET");
            continue;
        }
        
        if ((j = connect(cl_socket,(struct sockaddr *) q->ai_addr,q->ai_addrlen) == -1))
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(j));
            //perror("CLIENT ERROR SOCKET connect");
            continue;
        }

        if (int y = send(cl_socket,toSend.c_str(),length,0) < 0)
        {
            close(cl_socket);
            continue;
        }
        //cout << "message sent" << endl;
        break;
    }
    
    close(sockfdUDP);
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //////////////RECEIVE TCP CONNECTION AND MSG FROM DOCTORS/PEER PATIENTS /////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    std::vector<subscribed_patient> listOfPatients;
    std::vector<appointment> listOfAppointments;
    int new_fd;
    struct sockaddr_storage their_addr;
    int num_bytes;
    if (listen(sockfdTCP,BACK_LOG) == -1)
    {
        perror("PATIENT TCP LISTEN ERROR");
    }
    //Main accept loop
    while (1)
    {
        struct sockaddr_storage their_addr;
        socklen_t addr_size = sizeof(their_addr);
        new_fd =  accept(sockfdTCP,(struct sockaddr*) &their_addr,&addr_size);
        if(new_fd == -1)
        {
            perror("PATIENT ACCEPT ERROR");
            continue;
        }
        
        //Send ACK message
        std::stringstream ss;
        string ackMessage = "ACK message from patient";
        ss << pn;
        ackMessage.append(ss.str());
        int length = strlen(ackMessage.c_str());
        if( send(new_fd,ackMessage.c_str(),length,0) == -1)
        {
            perror("PATIENT TCP ACK MESSAGE SEND ERROR");
        }
        
        //Receive welcome message from doctor/patient peer
        if((num_bytes = recv(new_fd,buffer,MAX_BUFFER_SIZE-1,0))== -1)
        {
            perror("PATIENT TCP RECV ERROR");
        }
        buffer[num_bytes] = '\0';
        string received_msg (buffer);
        std::stringstream stream(buffer);
        int a;
        string b,c,d;
        stream >> a >> b >> c >> d;
        cout << "patient" << pn << ": " <<  buffer << endl;
        
        //if I'm the last patient OR only patient  to be join doctor group
        if (a == 1)
        {
            //Send ACK message
            if( send(new_fd,ackMessage.c_str(),length,0) == -1)
            {
                perror("PATIENT TCP ACK MESSAGE SEND ERROR");
            }
            cout << "Patient" << pn << " joined " << d << endl;
        }
        else //Not the last patient, so will pass on schedule and list of other peer subscribers
        {
            //Send ACK message
            if( send(new_fd,ackMessage.c_str(),length,0) == -1)
            {
                perror("PATIENT TCP ACK MESSAGE SEND ERROR");
            }
            
            //Receive peer subscriber info and process
            if((num_bytes = recv(new_fd,buffer,MAX_BUFFER_SIZE-1,0))== -1)
            {
                perror("PATIENT TCP RECV ERROR");
            }
            buffer[num_bytes] = '\0';
            string received_message = buffer;
            
            std::istringstream ss (received_message);
            std::string line1;
            while(std::getline(ss,line1))
            {
                std::istringstream tt(line1);
                subscribed_patient pat;
                tt >> pat.name >> pat.port;
                listOfPatients.push_back(pat);
            }
            
            cout << "patient" << pn << ": " << received_message;
            
            //Send ACK message
            if( send(new_fd,ackMessage.c_str(),length,0) == -1)
            {
                perror("PATIENT TCP ACK MESSAGE SEND ERROR");
            }
            
            //Receive schedule info and process
            if((num_bytes = recv(new_fd,buffer,MAX_BUFFER_SIZE-1,0))== -1)
            {
                perror("PATIENT TCP RECV ERROR");
            }
            buffer[num_bytes] = '\0';
            string scheduleInfo = buffer;
            std::istringstream sched (scheduleInfo);
            std::string line2;
            while(std::getline(sched,line2))
            {
                std::istringstream st(line2);
                appointment a;
                st >> a.day >> a.time;
                listOfAppointments.push_back(a);
            }
            
            cout << "patient" << pn << ": " << scheduleInfo;
            //Send ACK message
            if( send(new_fd,ackMessage.c_str(),length,0) == -1)
            {
                perror("PATIENT TCP ACK MESSAGE SEND ERROR");
            }
            
            ////////////////////////////////////////////////
            ///////SENDING INFO TO NEXT PATIENTS////////////
            ////////////////////////////////////////////////
            
            //Create welcome message, patient message, & schedule message
            string patientMessage = "";
            string scheduleMessage = "";
            
            for (int i = 1; i < listOfPatients.size(); i++)
            {
                patientMessage.append(listOfPatients[i].name);
                patientMessage.append(" ");
                patientMessage.append(listOfPatients[i].port);
                patientMessage.append("\r\n");
            }
            
            for (int j = 1; j < listOfAppointments.size(); j++)
            {
                scheduleMessage.append(listOfAppointments[j].day);
                scheduleMessage.append(" ");
                scheduleMessage.append(listOfAppointments[j].time);
                scheduleMessage.append("\r\n");
            }
            a = a -1;
            std::stringstream sv;
            string welcomeMessage = "";
            sv << a;
            welcomeMessage.append(sv.str());
            welcomeMessage.append(" welcome to ");
            welcomeMessage.append(d);
            welcomeMessage.append(" group");
            
            //Send all the info to patient peer subscriber
            int oneSock, numbytes;
            struct addrinfo hints, *hl, *w;
            int rv;
            
            //Set the socket properties
            memset(&hints,0,sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;
            
            //Acquire the address info
            if((rv = getaddrinfo("127.0.0.1",listOfPatients[0].port.c_str(),&hints,&hl))!=0)
            {
                fprintf(stderr, "PATIENT PEER TCP getaddrinfo: %s\n", gai_strerror(rv));
            }
            
            //loop through and connect to first socket we can and connect
            for (w = hl; w != NULL; w = w->ai_next)
            {
                if((oneSock = socket(w->ai_family,w->ai_socktype,w->ai_protocol)) == -1)
                {
                    perror("PATIENT PEER TCP SOCKET CREATION ERROR TO SEND");
                    continue;
                }
                
                if(connect(oneSock,(struct sockaddr *) w->ai_addr,w->ai_addrlen) == -1)
                {
                    close(oneSock);
                    perror("PATIENT PEER TPC CONNECT");
                    continue;
                }
                break; //we only break if a socket was founded and connected with no errors
            }
            
            //if we come out of the loop and p is null, means nothing was connected
            if(w == NULL)
            {
                fprintf(stderr,"TCP: PATIENT failed to connect to PEER\n");
            }
    
            //Recevie ACK message for successful connection
            if((numbytes = recv(oneSock,buffer,MAX_BUFFER_SIZE-1,0))== -1)
            {
                perror("PATIENT TCP Recv Error");
            }
            buffer[numbytes] = '\0';
            cout << "patient" << pn << ": " << buffer << endl;
            
            //Send welcome message
            int z = strlen(welcomeMessage.c_str());
            send(oneSock,welcomeMessage.c_str(),z,0);
            
            //Receive ACK message for welcome message
            if((numbytes = recv(oneSock,buffer,MAX_BUFFER_SIZE-1,0))== -1)
            {
                perror("PATIENT PEER TCP Recv Error");
            }
            buffer[numbytes] = '\0';
            cout << "patient" << pn << ": " << buffer << endl;
            
            //Send patient info
            int l = strlen(patientMessage.c_str());
            send(oneSock,patientMessage.c_str(),l,0);
            
            //Receive ACK message for patient info
            if((numbytes = recv(oneSock,buffer,MAX_BUFFER_SIZE-1,0))== -1)
            {
                perror("PATIENT PEER TCP Recv Error");
            }
            buffer[numbytes] = '\0';
            cout << "patient" << pn << ": " << buffer << endl;
            
            //Send schedule info
            int l2 = strlen(scheduleMessage.c_str());
            send(oneSock,scheduleMessage.c_str(),l2,0);
            
            //Receive ACK message for doctor info
            if((numbytes = recv(oneSock,buffer,MAX_BUFFER_SIZE-1,0))== -1)
            {
                perror("PATIENT PEER TCP Recv Error");
            }
            buffer[numbytes] = '\0';
            cout << "patient" << pn << ": " << buffer << endl;
        }
        //exit(1);
    }
}

//////////////////////////////MAIN FUNCTION//////////////////////////////////////
int main(void)
{
    patient p1;
    
    int patient_number = 0;
    int pid;
    while(patient_number < 4)
    {
        if ((pid = fork()) == 0)
        {
            p1.sendToHealthCenter(patient_number + 1);
            break;
        }
        else
        {
            patient_number++;
        }
    }
    wait(NULL);
    
    
}



