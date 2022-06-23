idf_component_register(SRCS "AT24C.c" "webserver.c" "Config.c" "task_COM.c" "task_RotSensor.c" "task_RotatorTasks.c" "RotTask.c" "LoopBuffer.c" "RotSensor.c" "motor.c" "Lib595.c" "COM.c" "my_network.c" "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES "driver" "libesphttpd" "esp_wifi" "nvs_flash" "tcpip_adapter" )
