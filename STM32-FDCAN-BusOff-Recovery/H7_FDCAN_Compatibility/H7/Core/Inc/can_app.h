#ifndef __CAN_APP_H
#define __CAN_APP_H

#include "main.h"
#include "bsp_can.h"

// 定义要测试的报文ID（和F4、Cangaroo约定好）
#define CAN_ID_F4_TO_H7       0x111  // F4发给H7
#define CAN_ID_H7_TO_F4       0x222  // H7发给F4 (CAN2.0标准帧)
#define CAN_ID_H7_FD_TO_F4    0x333  // H7发给F4 (CANFD帧，测试F4是否会报错)

// 定义测试状态机
typedef enum {
    CAN_TEST_IDLE = 0,    // 空闲
    CAN_TEST_SEND_20,     // 循环发送经典CAN 2.0报文
    CAN_TEST_SEND_FD,     // 循环发送CANFD报文
    CAN_TEST_ERROR,       // 错误处理
} CAN_TestState_t;

// 对外接口
void APP_CAN_Init(void);
void APP_CAN_Task_10ms(void);
void APP_CAN_HandleRx(FDCAN_HandleTypeDef *hfdcan, uint32_t id, uint8_t *data, uint8_t len);

#endif