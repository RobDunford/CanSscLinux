#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define UPDATE_INTERVAL 20 // Update interval in seconds

pthread_mutex_t userIpLock;
bool running;

void fileWrite(uint16_t msgId, char *data, uint8_t len)
{
    char filename[50];
    snprintf(filename, sizeof(filename), "../data/%u.txt", msgId);

    // Open file in write mode
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }
    fprintf(file, "%s", data);
    printf("%d %s\n", msgId, data);
    // flush the file
    fflush(file);
    // Close file
    fclose(file);
}

// char line[200];
void *menu_display(void *arg)
{
    char *line = NULL;
    size_t len = 0;
    char menuText[] = {"\nUsage: Type file number then the text you want in the file,"
    " start your data with a suitable delimiter to enable file number conversion\n"};

    running = true; // cross thread variable only written in this thread

    printf("%s", menuText);

    while (1)
    {
        // https://man7.org/linux/man-pages/man3/getline.3.html
        getline(&line, &len, stdin);

        // Parsing the input string
        int filenum = atoi(line);
        int numtemp = filenum;

        int numlen = 0;
        while (numtemp > 0)
        {
            numtemp /= 10;
            numlen++;
        }
        pthread_mutex_lock(&userIpLock);
        // if (line[0]=='x')
        //     running= false;

        fileWrite(filenum, line + numlen, len);
        pthread_mutex_unlock(&userIpLock);
    }
    free(line);
    return NULL;
}

int main()
{
    FILE *file;
    time_t current_time;
    struct tm *time_info;
    char time_string[20];

    pthread_t display_thread;

    pthread_create(&display_thread, NULL, menu_display, NULL);

    if (pthread_mutex_init(&userIpLock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }

    while (running)
    {
        // Get current time
        time(&current_time);
        time_info = localtime(&current_time);

        // Format time as a string
        strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", time_info);
        pthread_mutex_lock(&userIpLock);
        fileWrite(1, time_string, 20);
        pthread_mutex_unlock(&userIpLock);

        // Wait for the specified interval
        sleep(UPDATE_INTERVAL);
    }
    //  Terminate the display thread (optional)
    pthread_cancel(display_thread);
    pthread_join(display_thread, NULL);
    pthread_mutex_destroy(&userIpLock);
    return 0;
}