// Simple linux implementation, allows for 1 file at a time so when busy don't
// take on new data! we could maintain a file pointer for each file but start
// simple, multiple can messages are just going to be nasty. OBC will be in
// control of most broadcast messages anyway

#include "datastore.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static FILE *file;
bool writeInProg;
#define TEMP_FILENAME "./data/temp.dat"

int fsize(FILE *file, uint32_t *size)
{
    // Move the file pointer to the end of the file
    if (fseek(file, 0, SEEK_END))
    {
        perror("fseek error:");
        return -1;
    }

    // Get the current position of the file pointer, which is the file size
    long ftellRet = ftell(file);
    *size = (uint32_t)ftellRet; // ftell(file);

    printf("File size: %ld bytes\n", ftellRet);

    // rewind file pointer back to the start
    rewind(file);
    return 0;
}

void DataStoreListFiles(void)
{
    struct dirent *entry;
    struct stat file_info;
    char path[] = "./data";

    DIR *dir = opendir(path);

    printf("Data dir contents \n");
    if (dir == NULL)
    {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL)
    {
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        if (stat(filepath, &file_info) == -1)
        {
            perror("Error getting file information");
            continue;
        }

        if (S_ISREG(file_info.st_mode))
        {
            printf("File: %s, Size: %ld bytes\n", entry->d_name,
                   file_info.st_size);
        }
    }

    closedir(dir);
}

void DataStorePrintFile(uint16_t id)
{
    FILE *file;
    uint8_t buffer[128];
    size_t bytesRead;
    long int offset = 0;

    char filename[20];

    snprintf(filename, sizeof(filename), "./data/%u.txt", id);
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("Data file %s missing\n", filename);
        return;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        // printf("%08lx: ", offset);
        for (uint16_t i = 0; i < bytesRead; i++)
        {
            if (i % 8 == 0)
                printf("\n ");
            printf("%02X ", buffer[i]);
        }
        printf(" |");
        for (uint16_t i = 0; i < bytesRead; i++)
        {
            printf("%c", (buffer[i] >= 32 && buffer[i] <= 126) ? buffer[i] : '.');
        }
        printf("|\n");
        // offset += bytesRead;
    }

    fclose(file);
}

void DataStoreInit(void) { writeInProg = false; }

/*
When a get command is received this check that the msg is in the data store
returns true if msg found
*/
bool DataStoreVerifyGet(uint16_t id, uint32_t *len)
{
    char filename[20];
    writeInProg = false;
    snprintf(filename, sizeof(filename), "./data/%u.txt", id);
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("Data file %s missing\n", filename);
        return false;
    }

    return (fsize(file, len) == 0);
}

uint8_t DataStoreGet(uint16_t id, char *pData, uint16_t maxLen)
{
    // must only be called after VerifyGetRequest as read pointer is set in that
    // method
    size_t returnCode = fread(pData, sizeof(char), maxLen, file);
    if (returnCode > 0)
    {
        return returnCode;
    }
    else
    {
        return 0;
    }
}
/*
When a put command is received the overall size and msg index is in the request.
The ID has to be valid and the data size viable. returns true if msg found
*/
bool DataStoreVerifyPut(uint16_t id, uint32_t len)
{
    // is the id valid? Could have a specific list but will just say yes for now
    //  any msg any size!
    remove(TEMP_FILENAME);//if temp file exists ditch it
    return true;
}

bool DataStorePut(uint16_t id, char *pData, uint8_t len)
{
    // for (uint16_t i = 0; i < len; i++)
    // {
    //     if (i % 8 == 0)
    //         printf("\n ");
    //     printf("%02X ", pData[i]);
    // }
    // Writing to binary file
    file = fopen(TEMP_FILENAME, "wb+");
    if (file == NULL) {
        perror("Error opening file for writing");
        return 1;
    }
    size_t bytesWritten = fwrite(pData, 1, len, file);
    if (bytesWritten != len) {
        perror("Error writing to file");
        fclose(file);
        return 1;
    }
    fclose(file);
    writeInProg = true;
    
    return (true);
}

void DataStoreClose(uint16_t id, bool state)
{
    char dataFileName[20];

    // writeinprog flag set when write file opened
    if (writeInProg)
    {
        // with 4 digits max 'filename' 20 is plenty
        snprintf(dataFileName, sizeof(dataFileName), "./data/%u.txt", id);
        remove(dataFileName);
        rename(TEMP_FILENAME, dataFileName);
    }
   
    file = NULL;

    printf("File %d closed\n", id);
    return;
}
