#include <inttypes.h>
#include "esp_log.h"
#define N_PACKAGES 4*2
#define PAYLOAD_LEN 4
#define BUFFER_SIZE 256
#define BYTES_PER_PACKAGE 8
#define MSG_TAM_STR 24
//struct to build a package
typedef struct NODE_Package
{
    uint8_t header, command, length, end;
    uint8_t data[PAYLOAD_LEN];
    uint32_t crc32;
} NODE_Package;

void showPackage(NODE_Package *pkg);
void StringToPackage(NODE_Package *pack, char *msg_pack);
//returns a struct which contains package data
void createPackage(NODE_Package *mi_pack, uint8_t header, uint8_t command, uint8_t length, uint8_t dat0, uint8_t dat1, uint8_t dat2, uint8_t dat3, uint8_t fin);
void PackageToString(NODE_Package *pack, char *msg_pack); 
// function that sends a package to uart
void sendPackage(NODE_Package pack);
//function that receives a package from uart
void receivePackage(NODE_Package pack);
// Get Function
uint32_t getCrc32b(NODE_Package pkg);