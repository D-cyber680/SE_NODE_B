#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "driver/i2c.h"
#include "esp_spiffs.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

#include "string.h"
#include "stdlib.h"
#include <math.h>
#include <limits.h>
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "packagedata.h"
#include "crc32.h"
#include "mpu6050.h"

static const char *TAG = "NODE B: ";

#define I2C_MASTER_SCL_IO 22        /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 21        /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0            /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define ESP_CHANNEL 1

#define HEADER_NODE_A 0x1A
#define HEADER_NODE_B 0x1B
#define END_NODE_A 0x11
#define END_NODE_B 0x22
#define CMD_SHOWTEMP_A 0xEE
#define CMD_SHOWTEMP_B 0xEE

static uint8_t peer_mac[ESP_NOW_ETH_ALEN] = {0x40, 0x91, 0x51, 0xbf, 0xf5, 0x94}; // estacion AP R (40:91:51:bf:f5:94)
int16_t dato = 0;

static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}
//  STA  I (40:22:d8:ee:6d:a4)
static esp_err_t init_wifi(void)
{
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

    esp_wifi_init(&wifi_init_config);
    esp_netif_init();
    esp_event_loop_create_default();
    nvs_flash_init();
    esp_wifi_init(&wifi_init_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_storage(WIFI_STORAGE_FLASH);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi init completed");
    return ESP_OK;
}

void recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len)
{

    ESP_LOGI(TAG, "Data received " MACSTR "%s", MAC2STR(esp_now_info->src_addr), data);
}

void send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS)
    {
        ESP_LOGI(TAG, "ESP_NOW_SEND_SUCCESS");
    }
    else
    {
        ESP_LOGW(TAG, "ESP_NOW_SEND_FAIL");
    }
}

static esp_err_t register_peer(uint8_t *peer_addr)
{
    esp_now_peer_info_t esp_now_peer_info = {};
    memcpy(esp_now_peer_info.peer_addr, peer_mac, ESP_NOW_ETH_ALEN);
    esp_now_peer_info.channel = ESP_CHANNEL;
    esp_now_peer_info.ifidx = ESP_IF_WIFI_STA;

    esp_now_add_peer(&esp_now_peer_info);
    return ESP_OK;
}

static esp_err_t init_esp_now(void)
{
    esp_now_init();
    esp_now_register_recv_cb(recv_cb);
    esp_now_register_send_cb(send_cb);
    ESP_ERROR_CHECK(register_peer(peer_mac));
    ESP_LOGI(TAG, "esp now init completed");
    return ESP_OK;
}

static esp_err_t esp_now_send_data(const uint8_t *peer_addr, const uint8_t *data, uint8_t len)
{
    esp_now_send(peer_addr, data, len);
    return ESP_OK;
}

void app_main(void)
{
    char msg_pack[MSG_TAM_STR];

    ESP_ERROR_CHECK(i2c_master_init());
    mpu6050_init();
    mpu6050_set_rate(200);
    // mpu6050_set_dlpf_mode(5);              // modo 5: low pass filter
    // mpu6050_set_full_scale_gyro_range(1);  // scale 1 en giroscopio
    // mpu6050_set_full_scale_accel_range(1); // scale 1 en acelerometro
    ESP_ERROR_CHECK(init_wifi());
    ESP_ERROR_CHECK(init_esp_now());
    ESP_ERROR_CHECK(register_peer(peer_mac));

    // Creo el apuntador a un paquete vacio
    NODE_Package *pkg = (NODE_Package *)malloc(sizeof(NODE_Package));

    while (mpu6050_test_connection())
    {
        dato = mpu6050_get_temperature() / 340 + 37;
        ESP_LOGI(TAG, "Dato es: %d\n",dato);
        // Creo el paquete con contenido
        createPackage(pkg, HEADER_NODE_B, CMD_SHOWTEMP_B, 2, 0, 0, 0, dato, END_NODE_B);
        // Convierto El paquete a Cadena
        PackageToString(pkg, msg_pack);
        // Convertimos char* -> uint8_t * para poder usar la funcion de esp_now_send_data
        // Enviamos
        esp_now_send_data(peer_mac, (const uint8_t *)msg_pack, 24);
        // showPackage(pkg);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}