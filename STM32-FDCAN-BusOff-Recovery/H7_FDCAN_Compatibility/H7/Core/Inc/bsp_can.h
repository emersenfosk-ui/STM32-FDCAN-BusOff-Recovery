#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "main.h"

/* 引用 CubeMX 在 main.c 中定义好的句柄，避免重复定义 */
extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

void bsp_InitCan1(void);
void bsp_InitCan2(void);

void can1_SendPacket(uint8_t *_DataBuf, uint8_t _Len);
void can2_SendPacket(uint8_t *_DataBuf, uint8_t _Len);

/* 新增：发送CANFD帧的函数声明 */
void can1_SendFD_Packet(uint8_t *_DataBuf, uint8_t _Len);

#endif