# STM32H743 FDCAN BusOff 自愈实验

## 实验目的
验证 H7 在总线故障（无 ACK）时进入 Error Passive，并实现自动恢复。

## 硬件架构
- MCU：STM32H743VIT6 + TJA1044GT
- 对端：CANable + Cangaroo
- 总线：经典 CAN 2.0，250kbps，采样点 87.5%

## 实验步骤
1. H7 周期发送 0x123，CANable 监听并 ACK
2. 拔掉 CANable，制造总线无应答故障
3. H7 的 TEC 迅速累积到 128，进入 Error Passive
4. 检测到 EP=1，主动复位 CAN 控制器
5. 重新插上 CANable，通信恢复

## 实验现象
（附串口打印截图）

## 结论
FDCAN 在 Error Passive 状态下 TEC 会被硬件冻结，无法达到 BusOff。
因此自愈逻辑不能依赖 BusOff 回调，必须在检测到 EP=1 时主动复位 CAN 控制器。

## 关键代码
- `can_app.c`：错误检测 + 主动恢复
- `bsp_can.c`：FDCAN 过滤器、接收中断

## 踩坑记录
1. FDCAN 时钟源被 CubeMX 覆盖
2. FDCAN 在 Error Passive 下 TEC 冻结
3. CANable 波特率每次打开会重置
