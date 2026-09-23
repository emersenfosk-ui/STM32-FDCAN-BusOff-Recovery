#include "can_app.h"
#include <stdio.h>
#include <string.h>

// 软件FIFO
#define CAN_RX_FIFO_SIZE 16
typedef struct {
    uint32_t id;
    uint8_t data[64];
    uint8_t len;
} Can_RxMsg_t;

Can_RxMsg_t can_rx_fifo[CAN_RX_FIFO_SIZE];
volatile uint8_t fifo_head = 0;
volatile uint8_t fifo_tail = 0;

void APP_Can_PushMsg(uint32_t id, uint8_t *data, uint8_t len) {
    uint8_t next = (fifo_head + 1) % CAN_RX_FIFO_SIZE;
    if (next != fifo_tail) {
        can_rx_fifo[fifo_head].id = id;
        memcpy(can_rx_fifo[fifo_head].data, data, len > 64 ? 64 : len);
        can_rx_fifo[fifo_head].len = len;
        fifo_head = next;
    }
}

uint8_t APP_Can_PopMsg(Can_RxMsg_t *msg) {
    if (fifo_head == fifo_tail) return 0;
    *msg = can_rx_fifo[fifo_tail];
    fifo_tail = (fifo_tail + 1) % CAN_RX_FIFO_SIZE;
    return 1;
}

void APP_CAN_HandleRx(FDCAN_HandleTypeDef *hfdcan, uint32_t id, uint8_t *data, uint8_t len) {
    APP_Can_PushMsg(id, data, len);
}

void APP_CAN_Init(void) {
    printf("CAN APP Init OK\r\n");
}

// 每10ms调用一次，在main的while里轮询
void APP_CAN_Task_10ms(void) {
	
	 static uint32_t send_counter = 0;//若测试经典can->CANFD时就注释掉
//	static uint32_t err_counter = 0;
	 Can_RxMsg_t rx_msg;

    // 只处理接收打印，不做周期性发送
    if (APP_Can_PopMsg(&rx_msg)) {
        printf("Recv ID:0x%03X, Len:%d\r\n", rx_msg.id, rx_msg.len);
    }
		
		
//    err_counter++;
//    if (err_counter >= 100) {
//        err_counter = 0;
//        FDCAN_ErrorCountersTypeDef err;
//        HAL_FDCAN_GetErrorCounters(&hfdcan1, &err);
//        uint32_t psr = hfdcan1.Instance->PSR;
//        uint8_t EP = (psr >> 5) & 1;
//        uint8_t BO = (psr >> 7) & 1;

//        printf("TEC=%d, REC=%d, BO=%d, EP=%d\r\n",
//               err.TxErrorCnt, err.RxErrorCnt, BO, EP);

//        /* 检测到被动错误或BusOff，主动恢复 */
//        if (EP == 1 || BO == 1) {
//            printf("Bus fault detected! Recovering...\r\n");

//            HAL_FDCAN_DeInit(&hfdcan1);
//            HAL_FDCAN_Init(&hfdcan1);

//            FDCAN_FilterTypeDef sFilterConfig = {0};
//            sFilterConfig.IdType = FDCAN_STANDARD_ID;
//            sFilterConfig.FilterIndex = 0;
//            sFilterConfig.FilterType = FDCAN_FILTER_MASK;
//            sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
//            sFilterConfig.FilterID1 = 0x7E0;
//            sFilterConfig.FilterID2 = 0x7FF;
//            HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig);

//            HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
//            HAL_FDCAN_Start(&hfdcan1);

//            printf("Recovered!\r\n");
//        }
//    }


/*若测试经典can->CANFD时就将以下注释掉*/
send_counter++;
if (send_counter >= 100) {
    send_counter = 0;

    uint8_t tx_data[64] = {0};
    for(int i=0; i<64; i++) tx_data[i] = i + 1;   // 填 1~64

    FDCAN_TxHeaderTypeDef TxHeader = {0};
    TxHeader.Identifier = 0x333;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_64;      // 64 字节
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;        // 关键：关 BRS
    TxHeader.FDFormat = FDCAN_FD_CAN;              // 关键：FD 帧
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, tx_data) == HAL_OK) {
        printf("H7 Send FD OK\r\n");
    } else {
        printf("H7 Send FD FAILED\r\n");
    }
}
}