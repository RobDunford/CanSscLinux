/*This code was inspired by https://www.youtube.com/watch?v=tqlBv6inzUY and
 * https://github.com/libcsp/libcsp */

/*Compiled with gnu99 instead of gcc99, ifreq not accessable with gcc99
 * https://stackoverflow.com/questions/10433982/why-does-c99-complain-about-storage-sizes
 */

#include "can.h"
#include "canssc/canssc.h"
#include <errno.h>
#include <fcntl.h>
#include <libsocketcan.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

int sockfd;
int nbytes;

struct can_frame rxFrame;
bool canWaiting = false;

void serialShowCan(uint32_t can_id, uint8_t* message, uint32_t size,
                   char* dirStr)
{
    canId_t canId;
    canId.val = can_id;
    char trans[9] = {
        "SGPDNRAF"}; // T_SIMPLE,T_GET,T_PUT,T_DATA,T_NEXT,T_REPEAT,T_ABORT,T_DONE,
    char dataStr[9] = {0};

    if (size > 8)
        size = 8;
    strncpy(dataStr, (const char*)message, size);

    if (canId.split.transport == T_DATA)
    {
        // printf("%s%c%u-%s\n", dirStr, trans[canId.split.transport],
        // canId.split.frame, dataStr);
        printf("%s %u %s\n", dirStr, canId.split.frame, dataStr);
    }
    else
    {
        printf("%s%c-", dirStr, trans[canId.split.transport]);
        for (uint32_t i = 0; i < size; i++)
        {
            printf("[%02X]", message[i]);
        }
        printf("\n");
    }
}

void* CanRead(void* arg)
{
    while (1)
    {
        nbytes = read(sockfd, &rxFrame, sizeof(struct can_frame));
        canWaiting = true;
        while (canWaiting)
        {
        }
    }
}

void CanStart(void)
{
    pthread_t canReadThread;

    struct sockaddr_can sockaddr;
    struct ifreq ifr; // interface request variable

    /* Set interface up - this may require increased OS privileges */
    // char device[] = {"can0"};

    // can_do_stop(device);
    // can_set_bitrate(device, 500000);
    // can_set_restart_ms(device, 100);
    // can_do_start(device);

    /*Create the CAN socket */
    /* Notes:
    A socket is an endpoint for sending and receiving data on a computer
    network. PF_CAN stands for "Protocol Family CAN" SOCK_RAW allows direct
    access to lower-level protocols Using SOCK_RAW means that your application
    is responsible for handling the CAN protocol's own headers and payload,
    providing more control over the communication. CAN_RAW This is the specific
    protocol to be used with the socket CAN_RAW indicates that the raw CAN
    protocol is being used, as opposed to a higher-level protocol like ISO-TP*/
    sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd < 0)
    {
        perror("Error while opening socket %d");
        return;
    }
    /*Name of the CAN interface */
    strcpy(ifr.ifr_name, "can0");

    ioctl(sockfd, SIOCGIFINDEX, &ifr); // system call

    /* Bind the socket to the CAN interface */
    sockaddr.can_family = AF_CAN;           // "Address Family CAN"
    sockaddr.can_ifindex = ifr.ifr_ifindex; // pass the system interface index
    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
    {
        perror("Error in socket bind");
        printf("Try sudo ip link set up can0 type can bitrate 500000\n");
    }
    /* Read CAN frames and print them */
    // create a thread for the can read. don't bother with a queue as the socket
    // should already have one get one msg and pause until CanGet has had a
    // chance to pick it up.
    pthread_create(&canReadThread, NULL, CanRead,
                   NULL); // when will this one join again??
}

// True if there is a can frame to process
bool CanGet(uint32_t* id, char* msg, uint8_t* len)
{
    if (canWaiting)
    {
        if (rxFrame.can_dlc >= 0)
        {
            *id = rxFrame.can_id;
            *len = rxFrame.can_dlc;
            memcpy(msg, rxFrame.data, rxFrame.can_dlc);

            serialShowCan(rxFrame.can_id, rxFrame.data, rxFrame.can_dlc, "Rx");
        }
        canWaiting = false;
        return true;
    }
    return false;
}

// True if data ACKed, always gonna say yes for the moment
bool CanPut(uint32_t* id, char* msg, uint8_t len)
{
    bool sent = false;
    struct can_frame frame = {.can_id = *id | CAN_EFF_FLAG, .len = len};

    memcpy(frame.data, msg, frame.len);

    serialShowCan(*id, msg, len, "Tx");
    uint32_t waiting_ms = 0;

    while (sent == false)
    {
        int written;

        written = write(sockfd, &frame, sizeof(frame));
        if (written < 0)
        {
            if (errno == EINVAL)
            {
                // returned if a method receives an invalid argument.
                return false; // CSP_ERR_TX;
            }
            else if (errno == ENOBUFS)
            {
                /* If no space available, wait for 5 ms and try again */
                usleep(5000);
                waiting_ms += 5;
            }
            else if (errno == EAGAIN || errno == EINTR)
            {
                /* Acceptable, since something interrupted us, try again */
                waiting_ms += 5;
            }
            else
            {
                // csp_print("%s[%s]: write() failed, encountered an error
                // during write(). %d - '%s'\n", __func__, ctx->name, errno,
                // strerror(errno));
                return false; // CSP_ERR_TX;
            }

            if (waiting_ms >= 1000)
            {
                /* We finally got tired of waiting, give up */
                // csp_print("%s[%s]: write() failed, we have been waiting for
                // CAN buffers for too long (>1000 ms)\n", __func__, ctx->name);
                return false; // CSP_ERR_TX;
            }
        }
        else
        {
            sent = true;
        }
    }

    return true;
}