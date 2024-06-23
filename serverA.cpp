#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
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

using namespace std;

#define LOCAL_HOST "127.0.0.1"
#define PORT "21871"
#define destinationHostname "127.0.0.1"
#define destinationPort "23871"
#define BACKLOG 10

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
     
    //binding the socket
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) 
    {
        std::cerr << "Error binding socket to address\n";
        close(sockfd);
        return -1;
    }

    return sockfd;
 }

struct TimeAvailability {
    int start;
    int end;
};

struct User {
    string username;
    vector<TimeAvailability> timeAvailability;
};

bool isValidTimeAvailability(const TimeAvailability& timeAvail) 
{
    // Check if start and end times are non-negative integers
    if (timeAvail.start < 0 || timeAvail.end < 0) {
        return false;
    }
    // Check if start time is less than end time
    if (timeAvail.start >= timeAvail.end) {
        return false;
    }
    // Check if end time is within the allowed range
    if (timeAvail.start > 100 || timeAvail.end > 100) {
        return false;
    }
    return true;
}

vector<User> readInputFile(const string& fileName)
{
    vector<User> users;
    ifstream inputFile(fileName.c_str());
    if (!inputFile.is_open()) {
        cout << "Error opening file: " << fileName << endl;
        return users;
    }
    string line;
    while (getline(inputFile, line)) 
    {

        stringstream ss(line);
        string username, timeStr;
        getline(ss, username, ';');
        getline(ss, timeStr);
        // Remove spaces from username
        username.erase(remove_if(username.begin(), username.end(), ::isspace), username.end());
        // Parse time availability
        vector<TimeAvailability> timeAvailability;
        if (!timeStr.empty()) {
                size_t pos = 0;
                int temp=0;
                while ((pos = timeStr.find("[")) != string::npos) 
                {
                    TimeAvailability timeAvail;
                    if(temp==0)
                    {
                        timeStr = timeStr.substr(pos + 2);
                        temp=1;

                    }
                    else
                    {
                        timeStr = timeStr.substr(pos + 1);

                    }
                    timeAvail.start = atoi(timeStr.c_str());
                    pos = timeStr.find(",");
                    timeStr = timeStr.substr(pos + 1);
                    pos = timeStr.find("]");
                    timeAvail.end = atoi(timeStr.substr(0, pos).c_str());
                    timeAvailability.push_back(timeAvail);
                }
            // Validate time availability
            for (vector<TimeAvailability>::const_iterator it = timeAvailability.begin(); it != timeAvailability.end(); ++it) {
                if (!isValidTimeAvailability(*it)) {
                    cout << "Invalid time availability for user: " << username << endl;
                    continue;
                }
            }
        }
        // Create User object and add to users vector
        User user;
        user.username = username;
        user.timeAvailability = timeAvailability;
        users.push_back(user);
    }
    inputFile.close();
    return users;
}


//converting timeAvailabilities To String
std::string timeAvailabilitiesToString(const std::vector<TimeAvailability>& availabilities) 
{
    std::stringstream ss;
    for (std::vector<TimeAvailability>::const_iterator it = availabilities.begin(); it != availabilities.end(); ++it) {
        ss << "[" << it->start << ", " << it->end << "] ";
    }
    return ss.str();
}


int sendUDPData(int sockfd, const void* data, size_t dataSize, const char* hostname, const char* port) 
{
    struct addrinfo hints, *serverInfo, *p;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if ((rv = getaddrinfo(hostname, port, &hints, &serverInfo)) != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return -1;
    }
    int numBytesSent = 0;
    bool sentPacket = false;
    for (p = serverInfo; p != NULL; p = p->ai_next) 
    {
        // Extract usernames from data buffer
        const User* users = static_cast<const User*>(data);
        std::vector<std::string> usernames;
        for (size_t i = 0; i < dataSize / sizeof(User); i++) 
        {
            usernames.push_back(users[i].username);
        }
        // Convert usernames to a string separated by newline
        std::string usernamesStr = "";
        for (size_t i = 0; i < usernames.size(); i++) 
        {
            usernamesStr += usernames[i] + "\n";
        }

        // Send usernames only
        if ((numBytesSent = sendto(sockfd, usernamesStr.c_str(), usernamesStr.length(), 0, p->ai_addr, p->ai_addrlen)) == -1) 
        {
            perror("sendto");
            continue;
        }
        sentPacket = true;
        break;
    }
     // Wait for response from receiver
    char buf[1024];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    int numBytesReceived = recvfrom(sockfd, buf, sizeof buf, 0, (struct sockaddr *)&their_addr, &addr_len);
    if (numBytesReceived == -1) 
    {
        perror("recvfrom");
        return -1;
    }
    std::cout << "ServerA finished sending a list of usernames to Main Server."<< std::endl;
    return numBytesSent;
}





int sendUDPData2(int sockfd, const std::string availabilities, const char* hostname, const char* port) 
{
    struct addrinfo hints, *serverInfo, *p;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if ((rv = getaddrinfo(hostname, port, &hints, &serverInfo)) != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return -1;
    }
    int numBytesSent = 0;
    bool sentPacket = false;
    for (p = serverInfo; p != NULL; p = p->ai_next) 
    {
        // Extract usernames from data buffer
        
        // Send usernames only
        if ((numBytesSent = sendto(sockfd, availabilities.c_str(), availabilities.length(), 0, p->ai_addr, p->ai_addrlen)) == -1) 
        {
            perror("sendto");
            continue;
        }
        sentPacket = true;
        break;
    }
     // Wait for response from receiver
    char buf[1024];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    int numBytesReceived = recvfrom(sockfd, buf, sizeof buf, 0, (struct sockaddr *)&their_addr, &addr_len);
    if (numBytesReceived == -1) 
    {
        perror("recvfrom");
        return -1;
    }
    std::cout << "Server A finished sending the response to Main Server \n"; 
    return numBytesSent;
}

//finding the common time between two vector timeavalability
vector<TimeAvailability> findTimeIntersection(vector<TimeAvailability>& availability1, vector<TimeAvailability>& availability2) 
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

//print time timeAvailabilty
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

//print time timeAvailabilty
void printAvailability(const std::vector<TimeAvailability>& availability) {
    std::cout << "[";
    for (size_t i = 0; i < availability.size(); ++i) {
        std::cout << "[" << availability[i].start << "," << availability[i].end << "]";
        if (i != availability.size() - 1) {
            std::cout << ",";
        }
    }
    std::cout << "]";
}

int main()
{
    int sockfd = createUDPSocket();
    if (sockfd == -1) 
    {
        std::cerr << "Error creating socket\n";
        return -1;
    }
    std::cout << "ServerA is up and running using UDP on port " << PORT << "." << std::endl;
    string fileA = "a.txt";
    vector<User> usersA = readInputFile(fileA);
    const void* data = &usersA[0];
        size_t dataSize = usersA.size() * sizeof(User);
    if (sendUDPData(sockfd, data, dataSize, destinationHostname, destinationPort) == -1) {
        cerr << "Error sending UDP data\n";
    }

    // receive data from client
    const int MAX_BUFFER_SIZE = 1024;
    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in clientAddress;
    socklen_t len = sizeof(clientAddress);

    while (true) 
    {
        ssize_t recvlen = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddress, &len);

        if (recvlen == -1) {
            std::cerr << "Error receiving UDP data\n";
            close(sockfd);
            return -1;
        }
        // copy the received data to a vector of string
        std::vector<std::string> usernamesA;
        std::string receivedData(buffer, recvlen);
        std::istringstream iss(receivedData);
        std::string username;
        while (iss >> username) {
            usernamesA.push_back(username);
        }

        if (!usernamesA.empty())
         {
            
            std::cout << "Server A received the usernames from Main Server using UDP over port "<< PORT << std::endl;

            vector<User> usersWithMatchingNames;

            for (size_t i = 0; i < usernamesA.size(); ++i) 
            {
                for (size_t j = 0; j < usersA.size(); ++j) 
                {
                    if (usersA[j].username == usernamesA[i]) 
                    {
                        usersWithMatchingNames.push_back(usersA[j]);
                    }
                }
            }

            vector<TimeAvailability> intersection = usersWithMatchingNames[0].timeAvailability; // Start with the time availability of the first user
            for (int i = 1; i < usersWithMatchingNames.size(); ++i) 
            {
                intersection = findTimeIntersection(intersection, usersWithMatchingNames[i].timeAvailability);
            }
            std::string availabilitiesStr;
            if(intersection.empty())
            {
                availabilitiesStr="isEmpty";
            }
            else
            {
              availabilitiesStr = timeAvailabilitiesToString(intersection);
              std::cout << "Found the intersection result: ";
                std::cout << "[";
                for (int i = 0; i < intersection.size(); i++)
                 {
                    std::cout << "[" << intersection[i].start << "," << intersection[i].end << "]";

                    if (i != intersection.size() - 1)
                    {
                        std::cout << ",";
                    }
                }
                std::cout << "] for ";
                // print the received data
                std::vector<std::string>::iterator it;
                 for (int i = 0; i < usernamesA.size(); i++)
                { 
                    if (i != usernamesA.size() - 1)
                    {
                        std::cout << usernamesA[i] << ",";
                    }
                    else
                    {
                    std::cout << usernamesA[i]<< ".";
                    }
                }
                std::cout<<std::endl;

                
            }
            if (sendUDPData2(sockfd, availabilitiesStr, destinationHostname, destinationPort) == -1) 
            {
                cerr << "Error sending UDP data\n";
            }

            ssize_t recv = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddress, &len);

            if (recv == -1) 
            {
                std::cerr << "Error receiving UDP data\n";
                close(sockfd);
                return -1;
            }

            // copy the received data to a string
            std::string receivedData(buffer, recv);
            vector<TimeAvailability> change =parseTimeAvailabilityString(receivedData);
            cout << change[0].start << change[0].end << endl;
            for (int i = 0; i < usersWithMatchingNames.size(); i++) 
            {
                // intersection = findTimeIntersection(intersection, usersWithMatchingNames[i].timeAvailability);
                for(int j = 0; j< usersWithMatchingNames[i].timeAvailability.size(); j++){
                    if (change[0].start == usersWithMatchingNames[i].timeAvailability[j].start){
                        if (change[0].end == usersWithMatchingNames[i].timeAvailability[j].end){
                            usersWithMatchingNames[i].timeAvailability.erase(usersWithMatchingNames[i].timeAvailability.begin()+j);
                            break;
                        }
                        else{
                            usersWithMatchingNames[i].timeAvailability[j].start = change[0].end;
                            break;
                        }
                    }
                    else if (change[0].end == usersWithMatchingNames[i].timeAvailability[j].end){
                        usersWithMatchingNames[i].timeAvailability[j].end = change[0].start;
                    }
                    else if (change[0].start > usersWithMatchingNames[i].timeAvailability[j].start && change[0].end < usersWithMatchingNames[i].timeAvailability[j].end){
                        int temp = usersWithMatchingNames[i].timeAvailability[j].end;
                        usersWithMatchingNames[i].timeAvailability[j].end = change[0].start;
                        TimeAvailability newSlot;
                        newSlot.start = change[0].end;
                        newSlot.end = temp;
                        usersWithMatchingNames[i].timeAvailability.insert(usersWithMatchingNames[i].timeAvailability.begin()+j+1, newSlot);
                        break;
                    }
                }
            }

            
            for (size_t i = 0; i < usersWithMatchingNames.size(); ++i) 
            {
                for (size_t j = 0; j < usersA.size(); ++j) 
                {
                    if (usersA[j].username == usersWithMatchingNames[i].username) 
                    {
                        // Store the old timeAvailability for printing later
                        std::vector<TimeAvailability> oldAvailability = usersA[j].timeAvailability;
                        // Update the timeAvailability with the new values
                        usersA[j].timeAvailability.assign(usersWithMatchingNames[i].timeAvailability.begin(),usersWithMatchingNames[i].timeAvailability.end());

                        // Print the update
                        std::cout << usersA[j].username << ": updated from ";
                        printAvailability(oldAvailability);
                        std::cout << " to ";
                         printAvailability(usersA[j].timeAvailability);
                         std::cout << std::endl;
                     }
                 }
             }
            string message="Registration finished";
            if (sendUDPData2(sockfd, message, destinationHostname, destinationPort) == -1) 
            {
                cerr << "Error sending UDP data\n";
            }
        }
    }
        close(sockfd);

        return 0;   
}

