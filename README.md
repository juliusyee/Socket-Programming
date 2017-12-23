# Socket-Programming
Simulation of a peer-to-peer file-sharing network via ad hoc connections using TCP and UDP sockets 



In this project, I created UDP sockets for a healthcenter, 4 patients (via fork()), and 2 doctors (via fork()). I created 3 classes named healthcenter, patient, and doctor, all of which handle the activities of each type of person/entity involved in communications. In addition, each of the 4 patients and 2 doctors have TCP sockets that has a dynamically assigned port and local IP address. The UDP sockets are all hardcoded port numbers and a local IP address. I successfully completed all three phases of the project as described in the rubric:

1) Registration of patients with healthcenter server
2) Authentication of the doctors by the healthcenter server
3)Peer to Peer schedule sharing

Code Files:

healthcenter.h : header file for the healthcenter class
healthcenter.cpp : cpp file containing the main function to complete the required actions by a healthcenter server; contains the 			   implementations for the setUpUDP method
doctor.h : header file for the doctor class
doctor.cpp : cpp file containing the main function to complete the required actions by a doctor; contains the implementations for the 		     sendToHealthCenter and interactWithPatient methods
patient.h : header file for the patient class
patient.cpp : cpp file containing the main function to complete the required actions by a patient; contains the implementations for 		      the sendToHealthCenter method

In order to run the programs, compile each on Ubuntu : g++ -o outputname file.cpp -lnsl -lresolv
where outputname can be whatever and file.cpp is healthcenter.cpp, patient.cpp, or doctor.cpp
Compile all 3 cpp files and then run ./outputname & in the correct order (healthcenter, then patient, then doctor)

The format of all messages exchanged are as follows: ex) patient1: message
where message is what was received in patient1's socket

No code was used from anywhere else other than references from Beej's Socket Programming Tutorial Guide.
