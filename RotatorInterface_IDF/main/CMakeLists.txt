idf_component_register(SRCS "MultiLang.c" "AT24C.c" "webserver.c" "Config.c" "task_human_input.c" "task_display.c" "task_COM.c" "task_RotSensor.c" "task_RotatorTasks.c" "RotTask.c" "LoopBuffer.c" "RotSensor.c" "motor.c" "Lib595.c" "COM.c" "my_network.c" "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES "driver" "esp_adc_cal" "libesphttpd" "ButtonDebouncer" "esp_wifi" "nvs_flash" "fatfs" "vfs" "tcpip_adapter"
                       "lvgl" "lvgl_esp32_drivers" "esp_timer")
