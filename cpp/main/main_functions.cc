#include "main_functions.h"

#include "image_provider.h"
#include "model_settings.h"
#include "materials_model_data.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "esp_log.h"
#include "config.h"


static const char *TAG = "Classifier";

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spiffs.h"
#include "driver/i2c.h"
#include "ssd1306.h"
#include "string.h"
#include "sdkconfig.h"

#define AS312_PIN           19

extern void startCameraServer();

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SSD1306_t dev;
extern "C" {
#include "ssd1306.h"
}

#include "font8x8_basic.h"

//MQTT
#include "mqtt_client.h"
//MQTT ENDE

//WLAN
#include <string.h>
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define EXAMPLE_ESP_MAXIMUM_RETRY  5
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static int s_retry_num = 0;
static int old_classnum = 666;
static int tf_initialized = 0;
static esp_mqtt_client_config_t mqtt_cfg;
static esp_mqtt_client_handle_t client;

bool setupSensor() {
    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (uint64_t) 1 << 19;
    io_conf.pull_up_en = (gpio_pullup_t) 1;
    gpio_config(&io_conf);
    return true;
}

void init_display() {
    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, true);
    //ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, 0, "Custom Item", 11, true);
    ssd1306_display_text(&dev, 2, "Classifier", 10, true);

}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI("WLAN", "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI("WLAN", "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;

        ESP_LOGI("WLAN", "got ip:"
        IPSTR, IP2STR(&event->ip_info.ip));

        char buff[16];
        snprintf(buff, sizeof(buff), IPSTR, IP2STR(&event->ip_info.ip));

        ssd1306_clear_line(&dev, 6, true);
        ssd1306_display_text(&dev, 6, buff, strlen(buff), true);



        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    const char EXAMPLE_ESP_WIFI_SSID[] = WIFI_SSID; //"MySSID"
    const char EXAMPLE_ESP_WIFI_PASS[] = WIFI_PASSWD; //"MyWlanPassword"

    //const char EXAMPLE_ESP_WIFI_SSID[] = "HUAWEI P40 Pro"; //"MySSID"
    //const char EXAMPLE_ESP_WIFI_PASS[] = "e1783ac1b30f"; //"MyWlanPassword"

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {};
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    strcpy((char *) wifi_config.sta.ssid, (const char *) EXAMPLE_ESP_WIFI_SSID);
    strcpy((char *) wifi_config.sta.password, (const char *) EXAMPLE_ESP_WIFI_PASS);


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("WLAN", "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI("WLAN", "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI("WLAN", "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE("WLAN", "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

esp_err_t init_nvs() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    return err;
}

bool init_wlan() {
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    return true;
}

bool init_mqtt() {
    mqtt_cfg = {};
    mqtt_cfg.keepalive = 120;
    mqtt_cfg.uri = (char *) MQTT_URI;
    mqtt_cfg.username = (char *) MQTT_USER;
    mqtt_cfg.password = (char *) MQTT_PASS;
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
    return true;
}

namespace {
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;

// An area of memory to use for input, output, and intermediate arrays.
    constexpr int kTensorArenaSize = 320 * 1024; //320
//static uint8_t tensor_arena[kTensorArenaSize];


}  // namespace

// The name of this function is important for Arduino compatibility.

int32_t check_stop_flag(esp_err_t err) {
    int32_t stop_flag = 0;
    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");

        // Read
        printf("Reading stop_flag from NVS ... ");
        // value will default to 0, if not set yet in NVS
        err = nvs_get_i32(my_handle, "stop_flag", &stop_flag);
        switch (err) {
            case ESP_OK:
                printf("Done\n");
                printf("stop_flag = %d\n", stop_flag);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // Close
        nvs_close(my_handle);
    }

    printf("\n");

    return stop_flag;
}


bool init_tensorflow() {
    tf_initialized = 1;
    uint8_t *tensor_arena = (uint8_t *) malloc(320 * 1024 * sizeof(uint8_t)); // 320
    static const char *TAG = "Classifier";

    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    model = tflite::GetModel(g_materials_model_data);
    ESP_LOGW(TAG, "output %s", "After GetModel");
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "Model provided is schema version %d not equal "
                             "to supported version %d.",
                             model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

    static tflite::MicroMutableOpResolver<11> micro_op_resolver(error_reporter);
    micro_op_resolver.AddAveragePool2D();
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddDepthwiseConv2D();
    micro_op_resolver.AddFullyConnected();
    micro_op_resolver.AddMean();
    micro_op_resolver.AddPad();
    micro_op_resolver.AddMul();
    micro_op_resolver.AddSub();
    micro_op_resolver.AddAdd();
    micro_op_resolver.AddQuantize();
    micro_op_resolver.AddSoftmax();

    static tflite::MicroInterpreter static_interpreter(
            model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;
    ESP_LOGW(TAG, "output %s", "After Build an interpreter");

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    ESP_LOGW(TAG, "allocate status %d", allocate_status);
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return false;
    }
    ESP_LOGW(TAG, "output %s", "After Allocate Memory");

    input = interpreter->input(0);
    return true;
}

void setup() {
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    setupSensor();

    old_classnum = 666;
    ESP_LOGW("heap", "Total heap: %d", esp_get_free_heap_size());
    init_display();
    init_wlan();

    if (check_stop_flag(init_nvs())) {
        ESP_LOGW("LOOP", "no_stop_flag: %d", esp_get_free_heap_size());
        init_tensorflow();
        init_mqtt();
    } else {
        ESP_LOGW("LOOP", "Start Camera: %d", esp_get_free_heap_size());
        ESP_LOGW("LOOP", "Start Camera: %d", esp_get_free_internal_heap_size());
        startCameraServer();
    }
}


void loop() {
    vTaskDelay(50 / portTICK_PERIOD_MS);
    if (tf_initialized) {
        if (gpio_get_level(GPIO_NUM_19)) {
            ssd1306_clear_line(&dev, 4, true);
            ssd1306_display_text(&dev, 4, "Classifying", 11, true);
            static const char *TAG = "Classifier";

            uint8_t image_data[kMaxImageSize];
            if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels,
                                      image_data)) {
                TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
            }
            ESP_LOGW(TAG, "Captured Image");
            for (int i = 0; i < (kMaxImageSize); ++i) {
                int pixel_val = (int) image_data[i];
                input->data.int8[i] = (int8_t)(pixel_val - 128);
            }
            ESP_LOGW(TAG, "Converted Image");
            if (kTfLiteOk != interpreter->Invoke()) {
                ESP_LOGW(TAG, "output %s", "Run the model on this input and make sure it succeeds.");
                TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
            }
            ESP_LOGW(TAG, "Ran Model");
            TfLiteTensor *output = interpreter->output(0);
            ESP_LOGW(TAG, "output %s", "Process the inference results.");
            RespondToDetection(output);
        }
    }
}

void RespondToDetection(TfLiteTensor *output) {
    int8_t highest_score = output->data.int8[0];
    int highest_scoring_class_index = 0;
    for (int i = 1; i < kCategoryCount; ++i) {
        if (output->data.int8[i] >= highest_score) {
            highest_score = output->data.int8[i];
            highest_scoring_class_index = i;
        }
    }
    char classstring[14];
    strcpy(classstring, kCategoryLabels[highest_scoring_class_index]);
    char classnum[3];
    strcpy(classnum, utoa(highest_scoring_class_index, classnum, 10));
    static const char *TAG = "Classifier";



    ESP_LOGW(TAG, "Klassen 0: %d 1: %d", output->data.int8[0], output->data.int8[1]);
    ESP_LOGW(TAG, "Klasse predicted %d", highest_scoring_class_index);

    ssd1306_clear_line(&dev, 4, true);
    ssd1306_display_text(&dev, 4, classstring, strlen(classstring), true);
    for (int i = 0; i < kCategoryCount; ++i) {
        char class_score[4] = {0};
        char topic1[30] = {0};
        char buffer[4];
        strcat(topic1, MQTT_TOPIC);
        strcat(topic1, "/");
        strcat(topic1, "debug/scores/");
        strcat(topic1, itoa(i,buffer,10));
        strcpy(class_score, itoa(output->data.int8[i], class_score, 10));
        esp_mqtt_client_publish(client, topic1, class_score, 0, 1, 0);
        }

    if (old_classnum != highest_scoring_class_index) {

        old_classnum = highest_scoring_class_index;

        char topic2[30] = {0};
        strcat(topic2, MQTT_TOPIC);
        strcat(topic2, "/class_num");
        esp_mqtt_client_publish(client, topic2, classnum, 0, 1, 0);

        char topic3[30] = {0};
        strcat(topic3, MQTT_TOPIC);
        strcat(topic3, "/class_string");
        esp_mqtt_client_publish(client, topic3, classstring, 0, 1, 0);

    }
}
