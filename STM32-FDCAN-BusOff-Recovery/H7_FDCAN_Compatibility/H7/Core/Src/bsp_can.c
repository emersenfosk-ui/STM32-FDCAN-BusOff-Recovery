 #include "bsp_can.h"
#include "can_app.h"
#include <string.h>
#include "uds.h"
#include <stdio.h>

#ifndef HAL_FDCAN_ERROR_BUS_OFF
#define HAL_FDCAN_ERROR_BUS_OFF 0x20000000U
#endif

/* 接收报文用的全局变量 */
FDCAN_RxHeaderTypeDef g_Can1RxHeader;
uint8_t g_Can1RxData[64];

FDCAN_RxHeaderTypeDef g_Can2RxHeader;
uint8_t g_Can2RxData[64];

void bsp_InitCan1(void)
{
    FDCAN_FilterTypeDef sFilterConfig1;
    sFilterConfig1.IdType = FDCAN_STANDARD_ID;
    sFilterConfig1.FilterIndex = 0;//CANFD需要过滤器
    sFilterConfig1.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig1.FilterID1 = 0x000;  //0x7E0;
    sFilterConfig1.FilterID2 = 0x000;  //0x7FF;
    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK) Error_Handler();

    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) Error_Handler();
}

void bsp_InitCan2(void)
{
    FDCAN_FilterTypeDef sFilterConfig2;
    sFilterConfig2.IdType = FDCAN_STANDARD_ID;
    sFilterConfig2.FilterIndex = 0;
    sFilterConfig2.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig2.FilterID1 = 0x222;
    sFilterConfig2.FilterID2 = 0x7FF;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK) Error_Handler();

    HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK) Error_Handler();//先通知再启动
}

/* 标准 CAN 2.0 发送函数 */
void can1_SendPacket(uint8_t *_DataBuf, uint8_t _Len)
{
    FDCAN_TxHeaderTypeDef TxHeader = {0};
    TxHeader.Identifier = 0x222;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = (uint32_t)_Len << 16;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;//主动错误，只对 CANFD 帧有效，普通 CAN2.0 报文没有 ESI 位。
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;//是否开启比特率切换
    TxHeader.FDFormat = FDCAN_FD_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    //FDCAN_NO_TX_EVENTS：发完就完，不记录发送时间戳等信息。
    //FDCAN_STORE_TX_EVENTS：发完后把发送时间戳等信息存入 Tx Event FIFO，供 CPU 查询
    TxHeader.MessageMarker = 0;
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf);
}

void can2_SendPacket(uint8_t *_DataBuf, uint8_t _Len)
{
    FDCAN_TxHeaderTypeDef TxHeader = {0};
    TxHeader.Identifier = 0x111;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = (uint32_t)_Len << 16;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_FD_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, _DataBuf);
}

/* 新增：CANFD 发送函数（支持64字节，BRS开启） */
void can1_SendFD_Packet(uint8_t *_DataBuf, uint8_t _Len)
{
    FDCAN_TxHeaderTypeDef TxHeader = {0};
    TxHeader.Identifier = 0x333; // 用不同的ID区分，方便Cangaroo抓包区分
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = (uint32_t)_Len << 16;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_ON;      /* 开启可变波特率，数据段要提速 */
    TxHeader.FDFormat = FDCAN_FD_CAN;           /* 真正的 CAN-FD 帧格式，决定"帧的格式（是传统 CAN 还是 FDCAN） */
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf);
}

/* 接收中断回调 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if (hfdcan == &hfdcan1) {
        if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
            HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_Can1RxHeader, g_Can1RxData);
            
            /* 兼容性实验：打印接收到的帧 */
            printf("H7 Recv! ID:0x%03X, Data: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
       g_Can1RxHeader.Identifier,
       g_Can1RxData[0], g_Can1RxData[1], g_Can1RxData[2], g_Can1RxData[3],
       g_Can1RxData[4], g_Can1RxData[5], g_Can1RxData[6], g_Can1RxData[7]);
            
            /* 原有的 UDS 处理暂时注释掉，避免干扰 */
            // if (g_Can1RxHeader.Identifier == 0x7E0) {
            //     UDS_HandleRequest(g_Can1RxData, (g_Can1RxHeader.DataLength >> 16) & 0x0F);
            // }
        }
    }
    
    if (hfdcan == &hfdcan2) {
        if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
            HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_Can2RxHeader, g_Can2RxData);
            APP_CAN_HandleRx(hfdcan, g_Can2RxHeader.Identifier, g_Can2RxData, (g_Can2RxHeader.DataLength >> 16) & 0x0F);
        }
    }
}
/* 修正版本库差异：如果报错 HAL_FDCAN_ERROR_BUS_OFF 未定义，改用检查 ErrorCode 的最后一个位 */
void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) {
    /* 检查总线关闭状态（不同版本库宏定义可能不同，这里用位运算安全判断） */
    if (hfdcan->ErrorCode & HAL_FDCAN_ERROR_BUS_OFF){
        printf("BusOff detected! Recovering...\r\n");
			
        HAL_FDCAN_DeInit(hfdcan);
        HAL_FDCAN_Init(hfdcan);
			
			/* 重新配置过滤器 */
        FDCAN_FilterTypeDef sFilterConfig;
        sFilterConfig.IdType = FDCAN_STANDARD_ID;
        sFilterConfig.FilterIndex = 0;
        sFilterConfig.FilterType = FDCAN_FILTER_MASK;
        sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
        sFilterConfig.FilterID1 = 0x7E0;
        sFilterConfig.FilterID2 = 0x7FF;
        HAL_FDCAN_ConfigFilter(hfdcan, &sFilterConfig);
			
			  /* 重新激活接收中断 */
        HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
			
        HAL_FDCAN_Start(hfdcan);
			
			  printf("BusOff recovered!\r\n");
    }
}

