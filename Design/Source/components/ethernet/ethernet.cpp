#include "ethernet.h"

#include "esp_eth_netif_glue.h"

#include "esp_log.h"
#include "esp_check.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "ethernet_init.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/spi_master.h"

#define WIZNET_RST  GPIO_NUM_9
#define WIZNET_INT  GPIO_NUM_10
#define WIZNET_MOSI GPIO_NUM_11
#define WIZNET_MISO GPIO_NUM_12
#define WIZNET_CLK  GPIO_NUM_13
#define WIZNET_CS   GPIO_NUM_14

namespace Ethernet
{
    typedef struct {
        uint8_t spi_cs_gpio;
        int8_t int_gpio;
        uint32_t polling_ms;
        int8_t phy_reset_gpio;
        uint8_t phy_addr;
        uint8_t *mac_addr;
    } spi_eth_module_config_t;

    spi_host_device_t SPI_HOST = SPI2_HOST;
    esp_eth_handle_t esp_eth_handle;
    spi_eth_module_config_t spi_eth_module_config =
    {
        .spi_cs_gpio = WIZNET_CS,
        .int_gpio = WIZNET_INT,
        .polling_ms = 0,
        .phy_reset_gpio = WIZNET_RST,
        .phy_addr = 1,
        .mac_addr = 0,
    };

    namespace
    {
        static const char *TAG = "w5500_eth";
        static bool gpio_isr_svc_init_by_eth = false; // indicates that we initialized the GPIO ISR service

        /**
         * @brief SPI bus initialization (to be used by Ethernet SPI modules)
         *
         * @return
         *          - ESP_OK on success
         */
        static esp_err_t spi_bus_init()
        {
            esp_err_t ret = ESP_OK;
            spi_bus_config_t buscfg = {
                .mosi_io_num = WIZNET_MOSI,
                .miso_io_num = WIZNET_MISO,
                .sclk_io_num = WIZNET_CLK,
                .quadwp_io_num = -1,
                .quadhd_io_num = -1,
            };


        #if (CONFIG_EXAMPLE_ETH_SPI_INT0_GPIO >= 0) || (CONFIG_EXAMPLE_ETH_SPI_INT1_GPIO > 0)
            // Install GPIO ISR handler to be able to service SPI Eth modules interrupts
            ret = gpio_install_isr_service(0);
            if (ret == ESP_OK) {
                gpio_isr_svc_init_by_eth = true;
            } else if (ret == ESP_ERR_INVALID_STATE) {
                ESP_LOGW(TAG, "GPIO ISR handler has been already installed");
                ret = ESP_OK; // ISR handler has been already installed so no issues
            } else {
                ESP_LOGE(TAG, "GPIO ISR handler install failed");
                goto err;
            }
        #endif

            ESP_GOTO_ON_ERROR(spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO), err, TAG, "SPI host #%d init failed", SPI_HOST);

        err:
            return ret;
        }

        /**
         * @brief Ethernet SPI modules initialization
         *
         * @param[in] spi_eth_module_config specific SPI Ethernet module configuration
         * @param[out] mac_out optionally returns Ethernet MAC object
         * @param[out] phy_out optionally returns Ethernet PHY object
         * @return
         *          - esp_eth_handle_t if init succeeded
         *          - NULL if init failed
         */
        static esp_eth_handle_t eth_init_spi(spi_eth_module_config_t *spi_eth_module_config, esp_eth_mac_t **mac_out, esp_eth_phy_t **phy_out)
        {
            esp_eth_handle_t ret = NULL;

            // Init common MAC and PHY configs to default
            eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
            eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

            // Update PHY config based on board specific configuration
            phy_config.phy_addr = spi_eth_module_config->phy_addr;
            phy_config.reset_gpio_num = spi_eth_module_config->phy_reset_gpio;

            // Configure SPI interface for specific SPI module
            spi_device_interface_config_t spi_devcfg = {
                .mode = 0,
                .clock_speed_hz = 36 * 1000 * 1000,
                .spics_io_num = spi_eth_module_config->spi_cs_gpio,
                .queue_size = 20
            };

            // Init vendor specific MAC config to default, and create new SPI Ethernet MAC instance
            // and new PHY instance based on board configuration
            eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(SPI_HOST, &spi_devcfg);
            w5500_config.int_gpio_num = spi_eth_module_config->int_gpio;
            w5500_config.poll_period_ms = spi_eth_module_config->polling_ms;
            esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
            esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);

            // Init Ethernet driver to default and install it
            esp_eth_handle_t eth_handle = NULL;
            esp_eth_config_t eth_config_spi = ETH_DEFAULT_CONFIG(mac, phy);
            ESP_GOTO_ON_FALSE(esp_eth_driver_install(&eth_config_spi, &eth_handle) == ESP_OK, NULL, err, TAG, "SPI Ethernet driver install failed");

            // The SPI Ethernet module might not have a burned factory MAC address, we can set it manually.
            if (spi_eth_module_config->mac_addr != NULL) {
                ESP_GOTO_ON_FALSE(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, spi_eth_module_config->mac_addr) == ESP_OK,
                                                NULL, err, TAG, "SPI Ethernet MAC address config failed");
            }

            if (mac_out != NULL) {
                *mac_out = mac;
            }
            if (phy_out != NULL) {
                *phy_out = phy;
            }
            return eth_handle;
        err:
            if (eth_handle != NULL) {
                esp_eth_driver_uninstall(eth_handle);
            }
            if (mac != NULL) {
                mac->del(mac);
            }
            if (phy != NULL) {
                phy->del(phy);
            }
            return ret;
        }
    }

    void init()
    {
        esp_err_t ret = ESP_OK;

        ESP_RETURN_VOID_ON_ERROR(spi_bus_init(), TAG, "SPI bus init failed");
        
        // The SPI Ethernet module(s) might not have a burned factory MAC address, hence use manually configured address(es).
        // In this example, Locally Administered MAC address derived from ESP32x base MAC address is used.
        // Note that Locally Administered OUI range should be used only when testing on a LAN under your control!
        uint8_t base_mac_addr[ETH_ADDR_LEN];
        ESP_RETURN_VOID_ON_ERROR(esp_efuse_mac_get_default(base_mac_addr), TAG, "get EFUSE MAC failed");
        uint8_t local_mac_1[ETH_ADDR_LEN];
        esp_derive_local_mac(local_mac_1, base_mac_addr);
        spi_eth_module_config.mac_addr = local_mac_1;

        esp_eth_handle = eth_init_spi(&spi_eth_module_config, NULL, NULL);
        ESP_RETURN_VOID_ON_FALSE(esp_eth_handle, ESP_FAIL, TAG, "SPI Ethernet init failed");

        // Initialize TCP/IP network interface aka the esp-netif (should be called only once in application)
        ESP_RETURN_VOID_ON_ERROR(esp_netif_init(), TAG, "NETIF Init Failed");
        // Create default event loop that running in background
        ESP_RETURN_VOID_ON_ERROR(esp_event_loop_create_default(), TAG, "Event Loop Create Failed");

        esp_netif_t* eth_netif;
        esp_eth_netif_glue_handle_t eth_netif_glue;

        // Create instance(s) of esp-netif for Ethernet(s)

        // Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used and you don't need to modify
        // default esp-netif configuration parameters.
        esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
        eth_netif = esp_netif_new(&cfg);
        eth_netif_glue = esp_eth_new_netif_glue(esp_eth_handle);

        // Attach Ethernet driver to TCP/IP stack
        ESP_RETURN_VOID_ON_ERROR(esp_netif_attach(eth_netif, eth_netif_glue), TAG, "ESP NETIF Attach Failed");

        // Start Ethernet
        ESP_RETURN_VOID_ON_ERROR(esp_eth_start(esp_eth_handle), TAG, "ESP ETH Start Failed");
    }
}

