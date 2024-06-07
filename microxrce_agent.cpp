// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <uxr/agent/AgentInstance.hpp>
//#include"student.h"
#include"student23.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fstream>
#include <chrono>
#include <vector>
#include <thread>
#include <algorithm>
#include "fileInfo.h"
#include<mutex>
#define MAX_CLIENTS 2

using namespace std;
mutex studentsMutex;
student deserializeStudent(const char* buffer) {//反序列化
    student student;
    char name[50], hobby1[30], hobby2[30], hobby3[30];
    int fields = sscanf(buffer, "%29[^,],%ld,%ld,%29[^,],%29[^,],%29[^,]", name, &student.number, &student.grade, hobby1, hobby2, hobby3);
    student.name = string(name);
    student.hobby[0] = hobby1;
    student.hobby[1] = hobby2;
    student.hobby[2] = hobby3;
    return student;
}
// 定义比较函数，按 student 的 number 成员排序
bool compareByNumber(const student& a, const student& b) {
    return a.number < b.number;
}

// 使用 sort 函数对 student 数组进行排序
void sortStudent(std::vector<student>* current_students) {
    sort(current_students->begin(), current_students->end(), compareByNumber);
}
void writeToFileAndSwitch(vector<student>* current_students) {
    sort(current_students->begin(), current_students->end(), compareByNumber);
    ofstream outfile;
    auto now = chrono::system_clock::now();
    auto now_c = chrono::system_clock::to_time_t(now);
    tm* now_tm = localtime(&now_c);

    char filename[30];
    strftime(filename, 30, "%Y%m%d%H%M%S.txt", now_tm);

    outfile.open(filename, ios::out);
    if (!outfile.is_open()) {
        cout << "Error opening file" << endl;
        return;
    }

    for (const auto& student : *current_students) {
        outfile << "Name: " << student.name << " Number: " << student.number << " Grade: " << student.grade << " Hobbies: ";
        for (int j = 0; j < 3; j++) {
            outfile << student.hobby[j] << " ";
        }
        outfile << endl;
    }

    outfile.close();

    fileInfo file;
    file.filename = filename;
    file.timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

    ofstream fileinfoFile;
    fileinfoFile.open("fileInfo.txt", ios::app);
    if (!fileinfoFile.is_open()) {
        cout << "Error opening fileInfo.txt" << endl;
        return;
    }

    fileinfoFile << "Filename: " << file.filename << " Timestamp: " << file.timestamp << endl;
    fileinfoFile.close();

    current_students->clear();
}


void handleClientConnection(int client_socket) {
    static int file_count = 1; // 文件计数器，用于生成文件名
    static int i =1;
    static vector<student> students1;
    static vector<student> students2; 
    static vector<student>* current_students = &students1;
    while(true){
        char buf[255];
        int bytesReceived = recv(client_socket, buf, 255, 0);
        if (bytesReceived == -1) {
            cerr << "Error receiving data from client\n";
            break;
        } else if (bytesReceived == 0) {
            cout << "Client disconnected\n";
            break;
        }

        //memset(buf,0,255);
        //recv(client_socket,buf,512,0);
        //memset(&newstudent,0,sizeof(student));
        //memcpy(&newstudent,buf,sizeof(student));
        //student newstudent = deserializeStudent(buf);
        //memset(&newstudent, 0, sizeof(student));
        studentsMutex.lock();
        
        student newstudent = deserializeStudent(buf);
        int read_size = read(client_socket, buf, sizeof(student));
        if(newstudent.grade != 2 && newstudent.grade != 3) {
            return;
        }
        //cout << "Received student information:" << endl;
        cout <<"filecount:"<<file_count<<" ";
        cout << "Name: " << newstudent.name << " ";
        cout << "Number: " << newstudent.number << " ";
        cout << "Grade: " << newstudent.grade << " ";
        cout << "Hobbies: ";
        for (int j = 0; j < 3; j++) {
            cout << newstudent.hobby[j] << " ";
        }
        cout << endl;

        if (read_size == 0) {
            cout << "Client disconnected" << endl;
            close(client_socket);
        }
        current_students->push_back(newstudent);
        studentsMutex.unlock();
        if (current_students->size() >= 1000) {
            if (current_students == &students1) {
                thread write_thread(writeToFileAndSwitch, &students1);
                write_thread.detach();
                current_students = &students2;
            } else {
                thread write_thread(writeToFileAndSwitch, &students2);
                write_thread.detach();
                current_students = &students1;
            }
        }
    }
    close(client_socket);
}

int startrec23() {
    cout << "This is center" << endl;

    // socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        cout << "Error: socket" << endl;
        return 0;
    }

    // bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8400);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        cout << "Error: bind" << endl;
        return 0;
    }

    // listen for two clients
    if (listen(listenfd, MAX_CLIENTS) == -1) {
        cout << "Error: listen" << endl;
        return 0;
    }

    fd_set fds;
    int max_fd = listenfd;
    int client_sockets[MAX_CLIENTS] = {0};

        while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSocket = accept(listenfd, (struct sockaddr *)&clientAddr, &clientAddrSize);
        if (clientSocket == -1) {
            cerr << "Error accepting client connection\n";
            continue;
        }

        thread clientThread(handleClientConnection, clientSocket);
        clientThread.detach();
    }

    close(listenfd);

    return 0;
}

int main(int argc, char** argv)
{
    thread thread23(startrec23);
    eprosima::uxr::AgentInstance& agent_instance = agent_instance.getInstance();

    if (!agent_instance.create(argc, argv))
    {
        return 1;
    }
    agent_instance.run();
    thread23.join();
    // std::vector<uint8_t> data;
    // eprosima::uxr::FastDDSMiddleware writer;
    // writer.write_data(0x01,&data);
    return 0;
}
