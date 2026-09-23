idf_component_register(
    SRCS
        "app_main.c"
        "../../stm32_sender/crypto/tinyjambu.c"
        "../../stm32_sender/crypto/tinyjambu_permutation.c"
        "../../stm32_sender/protocol/crc16.c"
        "../../stm32_sender/protocol/uart_frame.c"
    INCLUDE_DIRS
        "."
        "../../common"
        "../../stm32_sender/include"
        "../../stm32_sender/crypto"
        "../../stm32_sender/protocol"
    PRIV_REQUIRES esp_driver_uart
)
