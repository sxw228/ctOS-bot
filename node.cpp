/*
*  3iRoboticsLIDAR System II
*  Driver Interface
*
*  Copyright 2017 3iRobotics
*  All rights reserved.
*
*	Author: 3iRobotics, Data:2017-09-15
*
*/


#include "C3iroboticsLidar.h"
#include "CSerialConnection.h"

#define DEG2RAD(x) ((x)*M_PI/180.)

#include <stdio.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <unistd.h>  

#define BUFFSIZE 4*700*2


typedef struct _rslidar_data
{
    _rslidar_data()
    {
        signal = 0;
        angle = 0.0;
        distance = 0.0;
    }
    uint8_t signal;
    float   angle;
    float   distance;
}RslidarDataComplete;

using namespace std;
using namespace everest::hwdrivers;


int main(int argc, char * argv[])
{
	

    int client_sockfd = 0;
    int len = 0;
    struct sockaddr_in server_addr;
    int sin_size = 0;
    char buf[BUFFSIZE] = {0};
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("192.168.43.233");
    server_addr.sin_port = htons(6054);
 
    if((client_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket error!\n");
        return -1;
    }
 
    sin_size = sizeof(struct sockaddr_in);
    
    




	int    opt_com_baudrate = 230400;
    string opt_com_path = "/dev/ttyUSB0";

    CSerialConnection serial_connect;
    C3iroboticsLidar robotics_lidar;

    serial_connect.setBaud(opt_com_baudrate);
    serial_connect.setPort(opt_com_path.c_str());
    if(serial_connect.openSimple())
    {
        printf("[AuxCtrl] Open serail port sucessful!\n");
        printf("baud rate:%d\n",serial_connect.getBaud());
    }
    else
    {
        printf("[AuxCtrl] Open serail port %s failed! \n", opt_com_path.c_str());
        return -1;
    }

    printf("C3iroboticslidar connected\n");

    robotics_lidar.initilize(&serial_connect);


    while (1)
    {
		TLidarGrabResult result = robotics_lidar.getScanData();
        switch(result)
        {
            case LIDAR_GRAB_ING:
            {
                break;
            }
            case LIDAR_GRAB_SUCESS:
            {
                TLidarScan lidar_scan = robotics_lidar.getLidarScan();
                size_t lidar_scan_size = lidar_scan.getSize();
                std::vector<RslidarDataComplete> send_lidar_scan_data;
                send_lidar_scan_data.resize(lidar_scan_size);
                RslidarDataComplete one_lidar_data;
                for(size_t i = 0; i < lidar_scan_size; i++)
                {
                    one_lidar_data.signal = lidar_scan.signal[i];
                    one_lidar_data.angle = lidar_scan.angle[i];
                    one_lidar_data.distance = lidar_scan.distance[i];
                    send_lidar_scan_data[i] = one_lidar_data;
		    memcpy(buf+i*8,&(one_lidar_data.angle),sizeof(float));
	            memcpy(buf+i*8+4,&(one_lidar_data.distance),sizeof(float));
		    printf("distance%f\n",one_lidar_data.distance);

                }

               // printf("Lidar count %d!\n", lidar_scan_size);
                  


        

             sendto(client_sockfd, buf, 8*lidar_scan_size, 0 ,(struct sockaddr*)&server_addr, sizeof(struct sockaddr)) ;
        









                break;
            }
            case LIDAR_GRAB_ERRO:
            {
                break;
            }
            case LIDAR_GRAB_ELSE:
            {
                printf("[Main] LIDAR_GRAB_ELSE!\n");
                break;
            }
        }
        //usleep(50);
    }


    close(client_sockfd);






    return 0;
}
