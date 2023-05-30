#ifndef PACKAGEDATA_H
#define PACKAGEDATA_H
#include "packagedata.h"
#include "crc32.h"
#include <stdio.h>
#include <string.h>
// pkg = (NODE_Package *)malloc(N_PACKAGES * sizeof(NODE_Package));

void createPackage(NODE_Package *mi_pack, uint8_t header, uint8_t command, uint8_t length, uint8_t dat0, uint8_t dat1, uint8_t dat2, uint8_t dat3, uint8_t fin)
{
    uint8_t input_arr[] = {header, command, length, dat0, dat1, dat2, dat3, fin};
    mi_pack->header = header;
    mi_pack->command = command;
    mi_pack->length = length;
    mi_pack->data[0] = dat0;
    mi_pack->data[1] = dat1;
    mi_pack->data[2] = dat2;
    mi_pack->data[3] = dat3;
    mi_pack->end = fin;
    mi_pack->crc32 = calc_crc32(input_arr, sizeof(input_arr));
}

void PackageToString(NODE_Package *pack, char *msg_pack)
{
    // Convertir cada campo a una cadena y almacenarlo en el buffer temporal
    char buffer[MSG_TAM_STR];
    int offset = 0;
    // Convertir cada campo a una cadena y almacenarlo en el buffer temporal
    snprintf(buffer, MSG_TAM_STR, "%02X%02X%02X", pack->header, pack->command, pack->length);
    strcpy(msg_pack, buffer);
    offset += strlen(buffer);
    for (int i = 0; i < PAYLOAD_LEN; i++)
    {
        snprintf(buffer, MSG_TAM_STR, "%02X", pack->data[i]);
        // strcat(msg_pack, buffer);
        strcat(msg_pack + offset, buffer);
        offset += strlen(buffer);
    }
    snprintf(buffer, MSG_TAM_STR, "%02X%08lX", pack->end, pack->crc32);
    strcat(msg_pack, buffer);
    printf("%s\n", msg_pack);
}

void StringToPackage(NODE_Package *pack, char *msg_pack)
{
    uint8_t info[8];
    uint8_t i, c = 0;
    char cad2[2]; // 0xAF
    // 5A120000000000B23613A09vû?
    // 5A120000000000B23613A097
    for (i = 0; i < 8; i++)
    {
        strncpy(cad2, msg_pack + c, 2);
        info[i] = strtoul(cad2, NULL, 16);
        c += 2;
    }
    createPackage(pack, info[0], info[1], info[2], info[3], info[4], info[5], info[6], info[7]);
}

uint32_t getCrc32b(NODE_Package pkg)
{
    return pkg.crc32;
}

void showPackage(NODE_Package *pkg)
{
    //uartClrScr(0);
    //uartGotoxy(0, 0, 0);
    ESP_LOGI("Header", "0x%01X\n", pkg->header);
    ESP_LOGI("Command", "0x%01X\n", pkg->command);
    ESP_LOGI("Length", "0x%01X\n", pkg->length);
    if (pkg->length > 0)
    {
        ESP_LOGI("data0", "0x%01x\n", pkg->data[0]);
        ESP_LOGI("data1", "0x%01x\n", pkg->data[1]);
        ESP_LOGI("data2", "0x%01x\n", pkg->data[2]);
        ESP_LOGI("data3", "0x%01x\n", pkg->data[3]);
    }else
    {
        ESP_LOGI("dataS", "NULL\n");
    }
    ESP_LOGI("End", "0x%01X\n", pkg->end);
    ESP_LOGI("CRC32", "0x%08lX\n", pkg->crc32);
}

#endif