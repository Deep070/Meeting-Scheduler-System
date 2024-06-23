#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <sstream>
#include <algorithm>

#define LOCAL_HOST "127.0.0.1"
#define PORT "23871"
#define TCP_SERVER_ADDR "127.0.0.1" 
#define TCP_SERVER_PORT "24871"
#define SERVER_A_ADDRESS "127.0.0.1"
#define SERVER_A_PORT "21871"
#define SERVER_B_ADDRESS "127.0.0.1"
#define SERVER_B_PORT "22871"
#define BACKLOG 10

using namespace std;

int createUDPSocket() 
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) 
    {
        std::cerr << "Error creating socket\n";
        return -1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(PORT));
    serverAddr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
     if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) 
    {
        std::cerr << "Error binding socket to address\n";
        close(sockfd);
        return -1;
    }

    return sockfd;
 }

std::string recvData(int sockfd, const char* serverAddress, const char* serverPort, const char* serverName,int flag)
{
    char buffer[1024];
    sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(serverPort));
    serverAddr.sin_addr.s_addr = inet_addr(serverAddress);
    int bytesReceived = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&serverAddr, &serverAddrLen);
    if (bytesReceived == -1)
    {
        std::cerr << "Error receiving data from " << serverName << "\n";
        return "";
    }
    buffer[bytesReceived] = '\0';
    if(flag == 0)
    {
        std::cout << "Main Server received the username list from " << serverName << " using UDP over port " << PORT << "."<< std::endl;
    }
    else if(flag==1)
    {
        std::cout << "Main Server received from " << serverName << " the intersection result using UDP over port  " << PORT << "." <<std::endl;
    }
    else
    {
        std::cout << "The registration has finished for " << serverName << "." <<std::endl;
    }
    // Send number of bytes received to server
    if (sendto(sockfd, &bytesReceived, sizeof(bytesReceived), 0, (struct sockaddr *)&serverAddr, serverAddrLen) == -1) 
    {
        std::cerr << "Error sending data to " << serverName << "\n";
    }

    return std::string(buffer);
}

int createTCPSocket(int port)
 {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating server socket\n";
        return -1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(atoi(TCP_SERVER_PORT));
     if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding server socket to port\n";
        close(serverSocket);
        return -1;
    }

    if (listen(serverSocket, BACKLOG) == -1) {
        std::cerr << "Error listening on server socket\n";
        close(serverSocket);
        return -1;
    }
    return serverSocket;
}


int acceptClient(int serverSocket, std::vector<std::string>& usernames)
{
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == -1)
    {
        std::cerr << "Error accepting client connection\n";
        close(serverSocket);
        return -1;
    }
     char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);

    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, 1024, 0);
    if (bytesReceived == -1)
    {
        std::cerr << "Error receiving data from client\n";
        close(clientSocket);
        return -1;
    }
     buffer[bytesReceived] = '\0';
    std::istringstream iss(buffer);
    std::string username;
    while (iss >> username) 
    {
        usernames.push_back(username);
    }
    std::cout << "Main Server received the request from client using TCP over port " << TCP_SERVER_PORT << std::endl;
    return clientSocket;
}

void sendDataToServer(const std::vector<std::string>& data, const char* serverAddr, const char* serverPort)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        std::cerr << "Error creating socket\n";
        return;
    }

    struct sockaddr_in serverAddrIn;
    memset(&serverAddrIn, 0, sizeof(serverAddrIn));
    serverAddrIn.sin_family = AF_INET;
    serverAddrIn.sin_port = htons(atoi(serverPort));
    serverAddrIn.sin_addr.s_addr = inet_addr(serverAddr);

    std::string dataStr = "";
    for (size_t i = 0; i < data.size(); ++i)
    {
        dataStr += data[i] + " ";
    }

    int bytesSent = sendto(sockfd, dataStr.c_str(), dataStr.size(), 0, (struct sockaddr*)&serverAddrIn, sizeof(serverAddrIn));
    if (bytesSent == -1)
    {
        std::cerr << "Error sending data to server\n";
    }

    close(sockfd);
}

void sendDataToServer2(const std::string& data, const char* serverAddr, const char* serverPort)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        std::cerr << "Error creating socket\n";
        return;
    }

    struct sockaddr_in serverAddrIn;
    memset(&serverAddrIn, 0, sizeof(serverAddrIn));
    serverAddrIn.sin_family = AF_INET;
    serverAddrIn.sin_port = htons(atoi(serverPort));
    serverAddrIn.sin_addr.s_addr = inet_addr(serverAddr);
    int bytesSent = sendto(sockfd, data.c_str(), data.size(), 0, (struct sockaddr*)&serverAddrIn, sizeof(serverAddrIn));
    if (bytesSent == -1)
    {
        std::cerr << "Error sending data to server\n";
    }

    close(sockfd);
}

void parseUsernames(const std::string& data, std::vector<std::string>& usernames)
{
    std::istringstream iss(data);
    std::string username;
    while (std::getline(iss, username)) {
        usernames.push_back(username);
    }
}

struct TimeAvailability {
    int start;
    int end;
};

std::vector<TimeAvailability> parseTimeAvailabilityString(std::string timeStr) {
    std::vector<TimeAvailability> timeAvailability;

    // Parse the input string
    size_t pos = 0;
    while ((pos = timeStr.find("[")) != std::string::npos) {
        TimeAvailability timeAvail;
        if (pos == 0) {
            timeStr = timeStr.substr(1);
        } else {
            timeStr = timeStr.substr(pos + 1);
        }
        pos = timeStr.find(",");
        timeAvail.start = atoi(timeStr.substr(0, pos).c_str());
        timeStr = timeStr.substr(pos + 1);
        pos = timeStr.find("]");
        timeAvail.end = atoi(timeStr.substr(0, pos).c_str());
        timeAvailability.push_back(timeAvail);
    }

    return timeAvailability;
}

std::vector<TimeAvailability> findTimeIntersection(std::vector<TimeAvailability>& availability1, std::vector<TimeAvailability>& availability2) 
{
    vector<TimeAvailability> intersection;
    for (vector<TimeAvailability>::iterator interval1 = availability1.begin(); interval1 != availability1.end(); ++interval1)
     {
        for (vector<TimeAvailability>::iterator interval2 = availability2.begin(); interval2 != availability2.end(); ++interval2) 
        {
            int start = max(interval1->start, interval2->start);
            int end = min(interval1->end, interval2->end);
            if (start < end) { // Exclude intervals with start and end values that are the same
                TimeAvailability interval;
                interval.start = start;
                interval.end = end;
                intersection.push_back(interval);
            }
        }
    }
    return intersection;
}

int main()
{
    int sockfd = createUDPSocket();
    if (sockfd == -1) 
    {
        std::cerr << "Error creating socket\n";
        return -1;
    }

    std::cout << "Main Server is up and running." << std::endl;

    std::vector<std::string> usernamesA; // Vector to store usernames from Server A
    std::vector<std::string> usernamesB; // Vector to store usernames from Server B

    std::string usersA =recvData(sockfd, SERVER_A_ADDRESS, SERVER_A_PORT, "Server A",0);
    std::string usersB=recvData(sockfd, SERVER_B_ADDRESS, SERVER_B_PORT, "Server B",0);
    parseUsernames(usersA,usernamesA);
    parseUsernames(usersB,usernamesB);


 
    int serverSocket = createTCPSocket(atoi(PORT));
    if (serverSocket == -1)
    {
        return 1;
    }


    std::vector<std::string>::iterator it;
    std::vector<std::string> usernames;

    while (true) 
    {
        int clientSocket=acceptClient(serverSocket, usernames);
        
        std::vector<std::string> onlyInA;
        std::vector<std::string> onlyInB;
        std::vector<std::string> neitherInAOrB;
        // Sort elements in vector c into three separate vectors
         for (it = usernames.begin(); it != usernames.end(); ++it) {
            if (std::find(usernamesA.begin(), usernamesA.end(), *it) != usernamesA.end()) {
                onlyInA.push_back(*it);
            } else if (std::find(usernamesB.begin(), usernamesB.end(), *it) != usernamesB.end()) {
                onlyInB.push_back(*it);
            } else {
                neitherInAOrB.push_back(*it);
            }
        }
        if(!neitherInAOrB.empty())
        {
                for (it = neitherInAOrB.begin(); it != neitherInAOrB.end(); ++it) 
                {
                    std::cout << *it << " ";
                }
                std::cout << "does not exist. Send a reply to Client.";
                std::cout << std::endl;
        }
        if(!onlyInA.empty())
        {
                std::cout << "Found ";
                for (it = onlyInA.begin(); it != onlyInA.end(); ++it) 
                {
                    std::cout << *it << ", ";
                }
                std::cout << "located at Server A. Send to Server A.";
                std::cout << std::endl;
        }
        if(!onlyInB.empty())
        {
                std::cout << "Found ";
                for (it = onlyInB.begin(); it != onlyInB.end(); ++it) 
                {
                    std::cout << *it << ",";
                }
                std::cout << "located at Server B. Send to Server B.";
                std::cout << std::endl;
        }
      
        std::string dataStr = "";
        for (size_t i = 0; i < neitherInAOrB.size(); ++i)
        {
            dataStr += neitherInAOrB[i] + " ";
        }

        if (dataStr.empty())
        {
            dataStr="allexist";
        }
        // Send string to client
        send(clientSocket, dataStr.c_str(),dataStr.length(), 0);

        // Send data to Server A and Server B
        sendDataToServer(onlyInA, SERVER_A_ADDRESS, SERVER_A_PORT);
        sendDataToServer(onlyInB, SERVER_B_ADDRESS, SERVER_B_PORT);
        std::vector<TimeAvailability> serverA;
        
        //reciveing the the common time for users in A
        if(!onlyInA.empty())
        {
            std::string availabilitiesA =recvData(sockfd, SERVER_A_ADDRESS, SERVER_A_PORT, "Server A",1);
            if(availabilitiesA!="isEmpty")
            {
                serverA = parseTimeAvailabilityString(availabilitiesA);
            }
            
            std::cout << "[";
            for (int i = 0; i < serverA.size(); i++)
             {
                std::cout << "[" << serverA[i].start << "," << serverA[i].end << "]";

                if (i != serverA.size() - 1) {
                    std::cout << ",";
                }
            }
            std::cout << "]" << std::endl;
        }

        //reciveing the the common time for users in B
        std::vector<TimeAvailability> serverB;
        if(!onlyInB.empty())
        {
            std::string availabilitiesB =recvData(sockfd, SERVER_B_ADDRESS, SERVER_B_PORT, "Server B",1);
            if(availabilitiesB!="isEmpty\n")
            {
                serverB = parseTimeAvailabilityString(availabilitiesB);
            }

            std::cout << "[";
            for (int i = 0; i < serverB.size(); i++)
             {
                std::cout << "[" << serverB[i].start << "," << serverB[i].end << "]";

                if (i != serverB.size() - 1) {
                    std::cout << ",";
                }
            }
            std::cout << "]" <<std::endl;
        }

        // If users enter the only in userA or userB if both then find intersection of that time
        std::vector<TimeAvailability> final;
        if(serverA.empty())
        {
            final=serverB;
        }
        else if(serverB.empty())
        {
            final=serverA;   
        }
        else
        {
            final = findTimeIntersection(serverA,serverB);
        }
        std::stringstream ss;
        std::string finalTime;
        if(!final.empty())
        {

            cout <<  "Found the intersection between the results from server A and B:" <<endl;
            std::cout << "[";
                for (int i = 0; i < final.size(); i++)
                 {
                    std::cout << "[" << final[i].start << "," << final[i].end << "]";

                    if (i != final.size() - 1) {
                        std::cout << ",";
                    }
                }
            std::cout << "]" <<std::endl;

            for (std::vector<TimeAvailability>::const_iterator it = final.begin(); it != final.end(); ++it) 
            {
                ss << "[" << it->start << ", " << it->end << "] ";
            }
            finalTime=ss.str();
        }
        else
        {
            finalTime="[]";
        }

        //sending the intersection between the results from server A and B:
        send(clientSocket, finalTime.c_str(),finalTime.length(), 0);
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, 1024, 0);
        if (bytesReceived == -1)
        {
            std::cerr << "Error receiving data from client\n";
            close(clientSocket);
            return -1;
        }
        buffer[bytesReceived] = '\0';

        // Convert buffer to string
        std::string receivedData(buffer);

        if(!receivedData.empty())
        {
            // Print the received data
            std::cout << "Received data: " << receivedData << std::endl;
        }

        int flag=0;

        //sending message to schedulue meeting for time entered by user if it conatins user from A and recieving response from it
        if(!onlyInA.empty())
        {
            sendDataToServer2(receivedData, SERVER_A_ADDRESS, SERVER_A_PORT);
            std::string regsA =recvData(sockfd, SERVER_A_ADDRESS, SERVER_A_PORT, "Server A",2);
            flag=1;
        }

        //sending message to schedulue meeting for time entered by user if it conatins user from B and recieving response from it
        if(!onlyInB.empty())
        {
            sendDataToServer2(receivedData, SERVER_B_ADDRESS, SERVER_B_PORT);
            std::string regsB =recvData(sockfd, SERVER_B_ADDRESS, SERVER_B_PORT, "Server B",2);
            flag=1;
        }

        //If meeting is scheduled notifiy the client it is done sucesfully
        if(flag==1)
        {
            string str="true";
            send(clientSocket,str.c_str(),str.length(), 0);
        }







    usernames.clear();

    }
    close(serverSocket);
    close(sockfd);
    return 0;   
}


