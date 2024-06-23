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
#include <cstdio>
#include <cstring>

using namespace std;

const char* SERVER_ADDRESS = "127.0.0.1";  // Server IP address
const int SERVER_PORT = 24871;

// Function to create a TCP socket
int createTCPSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Error creating TCP socket\n";
        return -1;
    }
    return sockfd;
}


// Function to connect to the server
bool connectToServer(int sockfd, const std::string& serverAddr, int serverPort) {
    sockaddr_in serverAddrInfo;
    memset(&serverAddrInfo, 0, sizeof(serverAddrInfo));
    serverAddrInfo.sin_family = AF_INET;
    serverAddrInfo.sin_port = htons(serverPort);
    serverAddrInfo.sin_addr.s_addr = inet_addr(serverAddr.c_str());
    if (connect(sockfd, (struct sockaddr*)&serverAddrInfo, sizeof(serverAddrInfo)) == -1) {
        std::cerr << "Error connecting to server\n";
        close(sockfd);
        return false;
    }
    return true;
}

// Function to send data to the server
bool sendToServer(int sockfd, const std::vector<std::string>& data) {
    std::stringstream ss;
    for (std::vector<std::string>::const_iterator it = data.begin(); it != data.end(); ++it) {
        ss << *it << " ";
    }
    std::string dataToSend = ss.str();
    int bytesSent = send(sockfd, dataToSend.c_str(), dataToSend.length(), 0);
    if (bytesSent == -1) {
        std::cerr << "Error sending data to server\n";
        close(sockfd);
        return false;
    }
    return true;
}

bool sendToServer2(int sockfd, const std::string& data)
{
	int bytesSent = send(sockfd, data.c_str(), data.length(), 0);
	if (bytesSent == -1) 
	{
		std::cerr << "Error sending data to server\n";
		close(sockfd);
		return false;
	}
	return true;
}


struct TimeAvailability {
    int start;
    int end;
};

// Function to receive data from the server
std::string receiveFromServer(int sockfd) {
    char buffer[1024] = {0};
    timeval timeout;
    timeout.tv_sec = 3; // 3 seconds timeout
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        std::cerr << "Error setting socket receive timeout\n";
        close(sockfd);
        return "";
    }
    int bytesReceived = recv(sockfd, buffer, sizeof(buffer), 0);
    if (bytesReceived == -1) {
        return "";
    }
    return std::string(buffer);
}

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

bool isTimeAvailable(const vector<TimeAvailability>& availability, const TimeAvailability& input)
{
    for (vector<TimeAvailability>::const_iterator it = availability.begin(); it != availability.end(); ++it) {
        const TimeAvailability& t = *it;
        if (input.end <= t.start || input.start >= t.end) {
            continue;
        } else {
            return true;
        }
    }
    return false;
}

TimeAvailability parseTimeAvailability(const string& input) 
{
    int start, end;
    sscanf(input.c_str(), "[%d,%d]", &start, &end);
    return (TimeAvailability){start, end};
}


int main() {
    int sockfd = createTCPSocket();
    if (sockfd == -1) {
        return -1;
    }

    std::cout << "Client is up and running." << std::endl;
    while (true)
     {
        if (connectToServer(sockfd, SERVER_ADDRESS, SERVER_PORT)) 
        {   
        	struct sockaddr_in client_address;
		    socklen_t client_address_len = sizeof(client_address);
		    if (getsockname(sockfd, (struct sockaddr*) &client_address, &client_address_len) < 0) {
		        std::cerr << "Error getting socket name\n";
		        return 1;
		    }
		    int client_port = ntohs(client_address.sin_port);
        	
            std::cout << "Please enter the usernames to check schedule availability: " << std::endl;
            std::string input;
            std::getline(std::cin, input);
            int count=0;
            // Convert the input usernames into a vector of strings
            std::vector<std::string> usernames;
            std::string username;
           	std::istringstream ss(input);
			while (std::getline(ss, username, ','))
			{
				// Remove leading and trailing white spaces from the username
			    username.erase(0, username.find_first_not_of(' '));
			    username.erase(username.find_last_not_of(' ') + 1);
			    if (count < 10) { // Check if the maximum limit has been reached
			        usernames.push_back(username);
			        count++;
			    } else {
			        break;
			    }
			 }
			
            
            if(sendToServer(sockfd, usernames))
            {
            	std::cout << "Client finished sending the usernames to Main Server" << std::endl;
            }
                // Receiving schedule availability from server
                std::string receivedData = receiveFromServer(sockfd);
                if(receivedData!="allexist")
                {
                    std::cout << "Client received the reply from Main Server using TCP over port = "  << client_port << std::endl;
                    std::cout << receivedData << "does not exist" << std::endl;

                } 
    		std::string receivedAnswer = receiveFromServer(sockfd);
    	
    		if(!receivedAnswer.empty())
    		{	

    			std::cout << "Client received the reply from Main Server using TCP over port" << client_port << std::endl;

    			std::vector<TimeAvailability> final= parseTimeAvailabilityString(receivedAnswer);
    			std::cout << "Time intervals [";
                for (int i = 0; i < final.size(); i++)
                 {
                    std::cout << "[" << final[i].start << "," << final[i].end << "]";

                    if (i != final.size() - 1) {
                        std::cout << ",";
                    }
                }
            	std::cout << "]" <<std::endl;


            	string input;
            	while (true) 
	    		{
			        cout << "Please enter the final meeting time to register a meeting:" << endl;
			        getline(cin, input);
			        TimeAvailability meetingTime = parseTimeAvailability(input);
			        if (isTimeAvailable(final, meetingTime)) {
			            cout << "Meeting scheduled for [" << meetingTime.start << "," << meetingTime.end << "]" << endl;
			            break;
			        } else {
			            cout << "Invalid meeting time entered. Please try again." << endl;
			        }
			    }

			    if(input.empty())
			    {
			    	input="empty";
			    }

			    sendToServer2(sockfd,input);
			    std::string ans = receiveFromServer(sockfd);
			    if(ans=="true")
			    {
			    	cout << "Received the notification that registration has finished." <<endl;
			    }


    		}
    		

    		
    		


    		// Close the socket after each request
			close(sockfd);
    		std::cout << "-----Start a new request-----"<<std::endl;
			

    
   			 // Create a new socket for the next request
   			 sockfd = createTCPSocket(); 
    		if (sockfd == -1)
    		{
    	    	return -1;
    		}
    	}

	}
	return 0;
}
