/* Mutex reminder:
 * https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/*/

#include "./canssc/canssc.h"
#include "can.h"
#include "datastore.h"
#include "tick.h"
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Include for sleep function

#define MAX_FILENAME_LENGTH 100
#define MAX_BUFFER_SIZE 1000

pthread_mutex_t userIpLock;

bool running;

canSsc_t hostCan;

bool actionRequired = false;

long int get_file_size(FILE* file)
{
    long int size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);
    return size;
}

void* menu_display(void* arg)
{
    char* line = NULL;
    size_t len = 0;
    char menuText[] = {
        "\nUsage: Type (S)imple /(G)et /(P)ut nodes msg \n"
        "ex:'S 5 2' requests a simple msg involving nodes 0 "
        "and 2 (include yourself) sending message 2\n"
        "l to list files \nx to exit\nEnter required to consider input\n"};

    running = true; // cross thread variable only written in this thread
    printf("CanSsc node %d\n", CAN_SSC_NODE_ID);

    printf("%s", menuText);

    while (1)
    {
        // https://man7.org/linux/man-pages/man3/getline.3.html
        getline(&line, &len, stdin);
        // Parsing the input string
        char cType;
        int nodes;
        int msg;
        bool pending = false;

        int retval = sscanf(line, "%c %d %d", &cType, &nodes, &msg);

        if (retval == 3)
        {
            nodes &= 0xFF;
            msg &= 0xFFF;
            canTransport_t msgType = T_ABORT;
            switch (tolower(cType))
            {
                case 's':
                {
                    msgType = T_SIMPLE;
                    break;
                }
                case 'g':
                {
                    msgType = T_GET;
                    break;
                }
                case 'p':
                {
                    msgType = T_PUT;
                    break;
                }
            }
            if (msgType != T_ABORT)
            {
                pthread_mutex_lock(&userIpLock);
                CanSscStart(&hostCan, msgType, 0, nodes, msg);
                pthread_mutex_unlock(&userIpLock);
            }
        }
        else if (retval == 1)
        {
            if (tolower(cType) == 'x')
                running = false;
            else if (tolower(cType) == 'l')
                DataStoreListFiles();
            else
                printf("%s", menuText);
        }
        else
            printf("%s", menuText);
    }
    free(line);
    return NULL;
}

void TransferResult(uint16_t msgId, bool result, uint8_t cause)
{
    if (result)
    {
        printf("Great success! msg %d sent (%d)\n", msgId, cause);
    }
    else
    {
        printf("We regret to inform you that msg %d failed Ref: (%d), please "
               "try again\n",
               msgId, cause);
    }
}

int main()
{
    pthread_t display_thread, working_thread;

    pthread_create(&display_thread, NULL, menu_display, NULL);

    if (pthread_mutex_init(&userIpLock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }

    CanSscInit(&hostCan, TransferResult); // void (*)(uint16_t , bool)

    while (running)
    {
        pthread_mutex_lock(&userIpLock);
        CanSscService(&hostCan);
        pthread_mutex_unlock(&userIpLock);
    }

    // any need to shutdown rx thread?
    //  Terminate the display thread (optional)
    pthread_cancel(display_thread);
    pthread_join(display_thread, NULL);
    pthread_mutex_destroy(&userIpLock);
    return 0;
}
