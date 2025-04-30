# AiGlasses说明文档

---


## 一、简介

### 1. 背景介绍

AI眼镜是集成AI 拍照、录像、语音助手、蓝牙耳机、翻译、导航、墨镜、聊天服务等功能的可穿戴设备。随着消费者对轻量化、便携式智能设备的需求不断增加，智能眼镜作为一种可以解放双手的设备，具有很大的市场潜力。且随着AI技术的成熟和硬件技术的提升，智能眼镜在社交、娱乐、办公和日常使用中具有广泛的应用前景。未来，智能眼镜将具备更强大的功能和更轻量化的设计，形成更为统一的智能生态系统，为用户提供更智能、更便捷的使用体验。



### 2. 规格说明

**Sensor**: IMX681
**ISP** ：multichannel、HDR off
**LDC** ：畸变校正、EIS
**SCL** ：图像缩放
**VENC**：

- 拍照：12M(4032x3024) 1fps，需要jpg编码。
- 录像：3M(2016x1512) 30fps H265



### 3. 场景介绍

SoC端连接Wi-Fi模块，eMMC，sensor，陀螺仪和数字光敏，TWS连接PMIC，充电IC，音频外设和相关用户交互外设。SoC通过Uart向TWS发送命令请求和接收TWS的控制指令，通过I2S接收TWS的音频数据，通过PSPI向TWS发送缩略图。SoC通过Wi-Fi将图像数据传送到手机端。

本章接场景图中灰色部分表示模块掉电状态。


  - **拍照/录像**

  ![](mymedia/aiglases_soc_wifi.svg)

  - **Wi-Fi传输照片/视频文件**

  ![](mymedia/aiglases_wifi_trans.svg)

  - **待机**

  SoC进入STR模式，ddr不断电。

  ![](mymedia/aiglases_suspend.svg)



### 4. demo pipeline

  - **拍照**


                                                                                                       frame mode +------+
                                                                                                     +----------->+ VENC |
                                                                                                     | Port 1     +------+
    +-------+ frame mode +-------+ frame mode +------------+ frame mode +-------+ frame mode +-------+             Dev 8
    |  VIF  +----------->+  ISP  +----------->+  AESTABLE  +----------->+  LDC  +----------->+  SCL  |             Chn 0
    +-------+ Group 0    +-------+ Dev 0      +------------+ Dev 0      +-------+            +-------+             Port 0
              Dev 0                Chn 1                     Chn 1        Dev 0               Dev 1  | frame mode +------+
              Port 0               Port 0                    Port 0       Chn 1               Chn 1  +----------->+ VENC |
                                   (multichn)                             Port 0                       Port 2     +------+
                                                                                                                   Dev 8
                                                                                                                   Chn 1
                                                                                                                   Port 0
VIF和ISP、LDC和SCL、SCL和VENC均以frame mode方式绑定。ISP输出port分辨率为4032x3024，超出ISP单帧最大size 3840x3840，这里开启multichn设置，ISP内部额外创建一个通道，将图像拆分成两部分处理，然后再拼成一张。AESTABLE为ptree模块，在user态获取ISP的output buffer，根据模块设置判断ISP stable状态和处理的帧数，然后灌给下一级模块。两路VENC，分别保存拍照图片和缩略图。

  - **录像**


                                                                                                       ring mode  +------+
                                                                                                     +----------->+ VENC |
                                                                                                     | Port 0     +------+
    +-------+ frame mode +-------+ frame mode +------------+ frame mode +-------+ frame mode +-------+             Dev 0
    |  VIF  +----------->+  ISP  +----------->+  AESTABLE  +----------->+  LDC  +----------->+  SCL  |             Chn 0
    +-------+ Group 0    +-------+ Dev 0      +------------+ Dev 0      +-------+            +-------+             Port 0
              Dev 0                Chn 0                     Chn 0        Dev 0               Dev 1  | frame mode +------+
              Port 0               Port 0                    Port 0       Chn 0               Chn 0  +----------->+ VENC |
                                                                          Port 0                       Port 2     +------+
                                                                                                                   Dev 8
                                                                                                                   Chn 2
                                                                                                                   Port 0
两路VENC，一路与SCL以HW Ring mode方式绑定，出H265视频流，一路与SCL以frame mode方式绑定，出缩略图。



## 二、 功能介绍

### 1. 拍照

  ![](mymedia/aiglases_pipeline_capture.svg)

- 拍照场景下sensor使用4032x3024分辨率，帧率为15fps。过ISP时做3DNR，因为ISP单帧处理最大size为3840x3840，需要开启multichn的设置，将一张图像分成两部分在两个chn中分开处理，然后再拼成一张。
- AESTABLE模块从ISP模块获取buffer并检查AE状态。当判断AE为stable状态时，将stable的buffer传给LDC。因为拍照只处理一张buffer，基于功耗考虑，提前解绑VIF和ISP模块，disable VIF/ISP的output port，将sensor置为sleepmode。
- LDC模块对输入图像做畸变校正，因为需要出一路拍照图片和一路720p的缩略图，需要接入SCL模块输出两路。
- SCL的output port对NV12格式图像的宽度有限制，最大不能超过3840，对YUYV格式则没有限制，所以SCL内部做图像格式转换输出YUYV的数据。SCL输出图像给VENC做编码。
- 两VENC编码分别保存拍照和缩略图的jpeg文件。保存文件结束会从后向前逐级disable VENC，SCL，LDC的output port。



### 2. 录像

  ![](mymedia/aiglases_pipeline_record.svg)

- 录像场景下sensor使用2016x1512分辨率，帧率为30fps。过ISP时做3DNR。
- AESTABLE模块从ISP模块获取buffer并检查AE状态。当判断AE为stable状态时，将stable的buffer传给LDC，录像一直处理数据指导接收到停止录像指令才会停止。
- LDC模块对输入图像做畸变校正，因为需要出一路H265码流和一路720p的缩略图，需要接入SCL模块输出两路。
- 因为输出图像满足SCL规格，SCL不用做格式转换，仍输出NV12格式图像给VENC做编码。
- 两VENC编码分别将所有图像数据保存H265视频文件和取第一帧图像数据保存jpeg文件。当接收到停止录像指令时，保存文件结束，会解绑VIF和ISP模块，并从后向前逐级disable VENC，SCL，LDC，ISP，VIF的output port，然后将sensor置为sleepmode。。



### 3. 音视频同步

待补充



### 4. 文件传输

  ![](mymedia/aiglases_wifi_server.svg)

拍照和录像生成的缩略图会通过PSPI协议传送到TWS。TWS端蓝牙常开，手机端浏览时，缩略图会由TWS通过蓝牙协议传送到手机端。手机端发送下载照片/视频文件指令前，会先对Soc端Wi-Fi进行配网，SoC端作为server，手机端发起连接并请求下载后，照片/视频文件会由SoC通过网络传送到手机端。



### 5. AESTABLE模块介绍

#### 5.1. 模块处理流程

AESTABLE模块是PTREE封装的一个用户层模块，连接在ISP模块后级，通过检查经ISP模块处理后的图像效果是否达到stable状态，用来控制图像数据是直接传输到下一级模块还是等到图像效果stable后再开始传输到下一级模块，以及控制传输图像数据的张数。

AESTABLE处理流程如下：
  ![](mymedia/aiglases_aestable.svg)

APP通过读取数字光敏lux值，根据ae table查询到shutter，gain值，再设置给ISP达到加速收敛的效果。APP设置shutter，gain值后会循环查询ISP的AE状态。如果AE达到stable状态，AESTABLE模块从ISP的output port获取buffer并注入到下一级的input port。

我们提供了控制硬光敏设备的驱动，详细可参考[Light Misc Control说明文档](../../source/iford/light_misc_control/driver/ReadMe.md#lightmisc)。


#### 5.2. 模块属性说明

- **RUN_MODE**
  - shot：拍照模式
  - record：录像模式

- **RUN_MODE_PARAM**
  RUN_MODE为shot模式时：
  - CAPTURE_COUNT：连拍帧数。
  - STABLE_COUNT：图像AE连续为stable状态的帧数阈值，满足该条件后AESTABLE模块才会将之后的图像数据传送给后级。
  - USING_LOW_POWER：low power模式。当Fpipeline stop时会从最后一个模块逐级向前做disable output port的操作。若启用low power模式，当AESTABLE模块的处理帧数达到CAPTURE_COUNT时会通知其前级的VIF和ISP模块先解绑并disable output port。若不启用low power模式，AESTABLE模块不会通知其它模块，VIF和ISP模块的解绑以及各模块的output port在pipeline stop时才会做disable操作。默认开启low power模式。

  RUN_MODE为record模式时：
  - STABLE_COUNT：图像AE连续为stable状态的帧数阈值，满足该条件后AESTABLE模块才会将之后的图像数据传送给后级。

  开始录像后会一直保持录像状态直到接收到结束录像指令。

- **START_MODE**
  - auto：自动模式，由ISP判断接收的图像是否AE达到stable状态，未stable的图像帧都丢弃。
  - force：bypass模式，默认ISP接收的图像都是stable的。



## 三、 环境配置及运行

### 1. 编译环境说明

在project路径下选择deconfig进行整包编译，以`SSZ029D-S01A-S`型号板子为例，使用以下deconfig：

    ipc_iford.emmc.glibc-11.1.0-ext4fs.ssc029d.256.bga8_lpddr4x_defconfig

在project目录执行以下命令进行编译:

    make ipc_iford.emmc.glibc-11.1.0-ext4fs.ssc029d.256.bga8_lpddr4x_defconfig;
    make clean && make image -j8

若需单独编译demo，可进到sample_code目录执行：

    make clean && make applications/ai_glasses/demo

编译完成会在 sample_code/out/arm/app 目录下生成 prog_ai_glasses_demo。


### 2. 运行环境说明

  * <b>dts配置</b>

    已默认配置好，无要求

  * <b>sensor驱动配置</b>

    默认已加载好imx681_mipi.ko，无需修改

  * <b>硬光敏驱动配置</b>

    默认已加载好light_misc_control.ko，无需修改


### 3. 依赖的资源文件

  - <b>demo bin文件</b>

    - prog_ai_glasses_demo：demo执行档，位于板端 /customer/sample_code/bin 目录。

    使用上例给出的deconfig整包编译烧录后，demo会默认打包至板端 /customer/sample_code/bin 目录下。
    若是手动编译demo，请将生成的 prog_ai_glasses_demo 拷贝至板端 /customer/sample_code/bin 目录下。

  - <b>IQ bin</b>

    使用上例给出的deconfig整包编译烧录后，IQ bin会默认打包至板端。

    - imx681_12M15fps_v30.bin：拍照场景sensor api bin，位于板端 /config/iqfile 目录下。

    - imx681_3M30fps_v45.bin：录像场景sensor api bin，位于板端 /config/iqfile 目录下。

    - 12m_alsc_cali.data：拍照场景镜头补偿校正，位于板端 /config/iqfile 目录下。

    - 12m_ne_cali.data：拍照场景3DNR Noise型态分析校正，位于板端 /config/iqfile 目录下。

    - 3m_ne_cali.data：录像场景3DNR Noise型态分析校正，位于板端 /config/iqfile 目录下。

    - CalibPoly_12M.bin: 拍照场景LDC畸变校正，位于板端 /customer/sample_code/bin/resource 目录下。

    - CalibPoly_3M.bin: 录像场景LDC畸变校正，位于板端 /customer/sample_code/bin/resource 目录下。

  - <b>pipeline json文件</b>

    - ai_glasses_aestable_whole.json: demo pipeline配置文件，位于板端 /customer/sample_code/bin/resource 目录下。



### 4. 运行命令

`cd`至`/customer/sample_code/bin`，执行：`./prog_ai_glasses_demo`。
如果有修改 prog_ai_glasses_demo 的路径，需保证 resource 目录和 prog_ai_glasses_demo 在同一目录下。


#### 4.1. 运行参数列表

  - <b>nohb</b>：不发送心跳包。默认应用会发送心跳包给TWS。
  - <b>nostr</b>：在task队列为空时不进入suspend，改为 sleep 10s。默认task队列为空时进susend。
  - <b>nosig</b>：不捕获 SIGINT, SIGKILL, SIGTERM 信号。默认捕获上述信号。
  - <b>nodma</b>：fuart不使用DMA模式。默认fuart使用DMA模式。
  - <b>t</b>：测试模式，同时启用 nohb 和 nostr。
  - <b>w</b>：测试模式，在task处理超100s时强制销毁task。
  - <b>d</b>：命令调试模式。命令说明如下：
    - quit: 退出debug模式。
    - exit: 退出app。
    - add [task_type] [task_param]: 添加新的debug task。
        | task_type | task_param | task类型描述 | task参数说明 | 命令说明 |
        |:---------:|:---------:|---------|-------|---------|
        | 0   |  | E_TASK_NONE | | task列表为空 |
        | 1   | n | E_TASK_PHOTO | 拍照数量 | 新建task，指定拍照张数 |
        | 2   |  | E_TASK_START_REC | | 新建task，开始录像 |
        | 3   |  | E_TASK_STOP_REC | | 新建task，结束录像 |
        | 4   |  | E_TASK_TRANS | | 新建task，SoC开启Wi-Fi AP模式，启动server端服务，等待client连接和请求传输数据 |
        | 5   |  | E_TASK_STOP_TRANS | | 新建task，SoC退出Wi-Fi AP模式，停止server服务 |

  prog_ai_glasses_demo默认启动后一直保持运行状态，直到接收到 SIGINT 或 SIGTERM 信号后退出。



#### 4.2. 应用命令说明

  | 运行场景 | 场景说明 | 启动应用命令 |
  |:---------:|:---------:|---------|
  | 正常工作场景 | SoC端应用运行后会默认2s一次向MCU发送心跳包，并查询本地的task列表，有task要处理就取出task执行；task列表为空就suspend。MCU端有事件触发时会唤醒SoC | 执行 <b><i>./prog_ai_glasses_demo</i></b> |
  | 无MCU，SoC单独运行拍照/录像场景 | SoC端应用运行后进入调试模式，不发送心跳包，不会进入suspend而是以sleep 10s代替。允许用户自己添加拍照/录像task | 执行 <b><i>./prog_ai_glasses_demo -t -d</i></b> , 待应用pipeline初始化结束后按回车进入调试模式命令行。<br><br><b>添加拍照task，连拍3张图片</b>：<br>命令行输入指令：<b><i>add 1 3</i></b><br>此时应用会连续拍摄3次，生成3张照片和照片缩略图。<br><br><b>添加开始录像task</b>：<br>命令行输入指令：<b><i>add 2</i></b><br>此时应用会开始录像，创建录像文件和缩略图文件并持续写入。<br><br><b>添加结束录像task</b>：<br>命令行输入指令：<b><i>add 3</i></b><br>此时应用结束录像，关闭录像文件和缩略图文件。 |



## 四、开发指南

### 1. 电源管理

待补充



### 2. 交互协议

#### 2.1. 串口交互协议

##### 2.1.1. 消息结构

    +-----------------------------+
    |            Header           |     消息头，4 bytes，固定为0x5A5A5A5A
    +-----------------------------+
    |            CmdType          |     消息处理类型，1 byte，0：REQ；1：ACK
    +-----------------------------+
    |            CmdId            |     消息ID，1 byte，详细介绍见2.1.2.章节
    +-----------------------------+
    |        Accept in ACK        |     ACK处理状态，1 byte，0：接收请求；1：拒绝请求。全按0处理，暂无拒绝请求的业务需求。
    +-----------------------------+
    |          SoC state          |     SoC状态，1 byte，详细介绍见2.1.3.章节
    +-----------------------------+
    |          user define        |     用户定义，2 bytes，bit[0~7]: task序列号; bit[8~15]: task参数
    +-----------------------------+
    |             Tail            |     消息尾，4 bytes，固定为0xA5A5A5A5
    +-----------------------------+


##### 2.1.2. 消息类型

| **消息ID** | **说明** | **使用场景** |
|----------|------------|-----------|
| E_TASK_NONE | task队列为空 | MCU to SoC |
| E_TASK_PHOTO | 拍照 | MCU to SoC |
| E_TASK_START_REC | 开始录像 | MCU to SoC |
| E_TASK_STOP_REC | 停止录像 | MCU to SoC |
| E_TASK_TRANS | 开始通过WI-Fi从ftp server下载文件 | MCU to SoC |
| E_TASK_STOP_TRANS | 停止从ftp server下载文件 | MCU to SoC |
| E_TASK_POWEROFF | task队列为空时MCU发送断电请求。暂未使用 | MCU to SoC |
| E_TASK_POWEROFF_OK | SoC响应断电请求 | SoC to MCU |
| E_TASK_REQUEST | SoC请求处理MCU的task队列的第一个task | SoC to MCU |
| E_TASK_DONE | SoC通知MCU当前请求的task已处理完成 | SoC to MCU |
| E_TASK_HEARTBEAT | SoC向MCU发送心跳包 | SoC to MCU |


##### 2.1.3. SoC状态

| **SoC状态** | **说明** |
|----------|------------|
| E_STATE_RESUMING | SoC在唤醒过程中。暂未使用 |
| E_STATE_IDEL | SoC在空闲状态 |
| E_STATE_CAP_PIC | SoC正在拍照 |
| E_STATE_REC | SoC正在录像 |
| E_STATE_TRANS | SoC正在上传文件 |
| E_STATE_SUSPENDING | SoC开始进入suspend状态 |
| E_STATE_SUSPENDED | SoC的suspend流程完成，准备掉电 |


##### 2.1.4. 串口通信说明

- MCU和SoC都有自己的工作队列，队列中存放待处理的task，task的类型和消息ID一致。

- MCU和SoC工作期间SoC会定时向MCU发送心跳包。SoC向MCU发送消息时会携带SoC的状态信息。

- MCU端，当触发拍照，录像，传输文件事件时，会向MCU的工作队列中添加对应的task。若SoC未上电，触发这些事件时会给SoC上电。

- SoC端，开机后运行demo或是设备运行demo后从待机恢复唤醒时，又或是SoC完成前一个task后，SoC会查询自己的工作队列是否为空。
  - 若为空，SoC状态置为E_STATE_IDEL，并向MCU发送 E_TASK_REQUEST 命令。MCU接收到 E_TASK_REQUEST 后会查询自己的工作队列。若工作队列不为空，返回队列中的第一个task给SoC处理；若工作队列为空，返回 E_TASK_NONE 的消息给SoC。SoC收到 E_TASK_NONE 消息后，会发送一个 E_TASK_HEARTBEAT 消息，并携带 E_STATE_SUSPENDING 的状态信息给MCU，然后SoC开始执行suspend操作。
  - 若不为空，SoC从工作队列中取出task进行处理。根据task类型的不同SoC的行为分别如下：
    - 若task的类型为 E_TASK_PHOTO，E_TASK_START_REC 或 E_TASK_TRANS，SoC的状态对应变更为 E_STATE_CAP_PIC，E_STATE_REC 或 E_STATE_TRANS。此时若有心跳包发送，心跳包携带的SoC状态为变更后的状态。当task处理完成后，SOC状态变更为 E_STATE_IDEL，并返回 E_TASK_DONE 的消息给MCU。
    - 若task的类型为 E_TASK_STOP_REC 或 E_TASK_STOP_TRANS，SoC结束当前正在处理的录像task或传输文件task，然后SOC的状态变更为 E_STATE_IDEL，并返回 E_TASK_DONE 的消息给MCU。

- SoC执行suspend操作后，在kernel suspend结束阶段会写串口寄存器发送 E_TASK_POWEROFF_OK 消息给MCU，消息携带的SoC状态为 E_STATE_SUSPENDED。MCU接收后会对SoC执行掉电操作。


##### 2.1.5. 拍照录像串口交互流程示例

  ![](mymedia/aiglases_uart_communicate.svg)



#### 2.2. PSPI数据传输协议

待补充



#### 2.3. I2S数据传输协议

待补充



### 3. API说明

| **API名**                                                    | **功能**            |
| ------------------------------------------------------------ | ------------------- |
| [ST_Common_AiGlasses_Init](####ST_Common_AiGlasses_Init) | PTree模块注册和硬光敏初始化 |
| [ST_Common_AiGlasses_Deinit](####ST_Common_AiGlasses_Deinit) | 硬光敏反初始化和PTree模块清理 |
| [ST_Common_AiGlasses_CreatePipeline](####ST_Common_AiGlasses_CreatePipeline) | 初始化pipeline |
| [ST_Common_AiGlasses_DestroyPipeline](####ST_Common_AiGlasses_DestroyPipeline) | 销毁pipeline |
| [ST_Common_AiGlasses_Capture](####ST_Common_AiGlasses_Capture) | 拍照指令 |
| [ST_Common_AiGlasses_StartRecord](####ST_Common_AiGlasses_StartRecord) | 开始录像 |
| [ST_Common_AiGlasses_StopRecord](####ST_Common_AiGlasses_StopRecord) | 结束录像 |
| [ST_Common_AiGlasses_Uart_Init](####ST_Common_AiGlasses_Uart_Init) | 串口初始化，创建数据接收线程 |
| [ST_Common_AiGlasses_Uart_DeInit](####ST_Common_AiGlasses_Uart_DeInit) | 等待数据接收线程结束，关闭串口 |
| [ST_Common_AiGlasses_Uart_ReadThread_CanExit](####ST_Common_AiGlasses_Uart_ReadThread_CanExit) | 结束数据接收线程 |
| [ST_Common_AiGlasses_Uart_Write](####ST_Common_AiGlasses_Uart_Write) | 串口发送数据 |
| [ST_Common_AiGlasses_Prot_Deal_Uart_Buffer](####ST_Common_AiGlasses_Prot_Deal_Uart_Buffer) | 解析串口接收数据 |
| [ST_Common_AiGlasses_Prot_Make_Cmd_To_Protrcol](####ST_Common_AiGlasses_Prot_Make_Cmd_To_Protrcol) | 构建串口发送数据 |


#### ST_Common_AiGlasses_Init

- 功能

  PTree模块注册和硬光敏初始化

- 语法

  ```
  void ST_Common_AiGlasses_Init(void);
  ```

- 形参

  无

- 返回值

  无


#### ST_Common_AiGlasses_Deinit

- 功能

  硬光敏反初始化和PTree模块清理

- 语法

  ```
  void ST_Common_AiGlasses_Deinit(void);
  ```

- 形参

  无

- 返回值

  无


#### ST_Common_AiGlasses_CreatePipeline

- 功能

  初始化pipeline

- 语法

  ```
  ST_Common_AiGlasses_Handle_t *ST_Common_AiGlasses_CreatePipeline(const char *pJsonPath);
  ```

- 形参

  | **参数名称** | **描述** | **输入/输出** |
  | ------------ | -------- | ------------- |
  | pJsonPath | pipeline配置文件  | 输入     |

- 返回值

  |                     | 描述                                                         |
  | ------------------- | ------------------------------------------------------------ |
  | ST_Common_AiGlasses_Handle_t *   | pipeline实例的指针   |


#### ST_Common_AiGlasses_DestroyPipeline

- 功能

  销毁pipeline

- 语法

  ```
  void ST_Common_AiGlasses_DestroyPipeline(ST_Common_AiGlasses_Handle_t *pInstance);
  ```

- 形参

  | **参数名称** | **描述** | **输入/输出** |
  | ------------ | -------- | ------------- |
  | pInstance | pipeline实例的指针  | 输入     |

- 返回值

  无


#### ST_Common_AiGlasses_Capture

- 功能

  拍照指令

- 语法

  ```
  int ST_Common_AiGlasses_Capture(ST_Common_AiGlasses_Handle_t *pInstance, const char *pSaveFileName, const char *pSaveThumbnail, unsigned char enableLowPower);
  ```

- 形参

  | **参数名称** | **描述** | **输入/输出** |
  | ------------ | -------- | ------------- |
  | pInstance | pipeline实例的指针  | 输入     |
  | pSaveFileName | 保存拍照图片的路径 | 输入 |
  | pSaveThumbnail | 保存拍照缩略图的路径 | 输入 |
  | enableLowPower | 是否开启[LowPower模式](####模块属性说明) | 输入 |

- 返回值

  0：成功

  非0：失败


#### ST_Common_AiGlasses_StartRecord

- 功能

  开始录像

- 语法

  ```
  int ST_Common_AiGlasses_StartRecord(ST_Common_AiGlasses_Handle_t *pInstance, const char *pSaveFileName, const char *pSaveThumbnail);
  ```

- 形参

  | **参数名称** | **描述** | **输入/输出** |
  | ------------ | -------- | ------------- |
  | pInstance | pipeline实例的指针  | 输入          |
  | pSaveFileName | 保存录像文件的路径 | 输入 |
  | pSaveThumbnail | 保存录像缩略图的路径 | 输入 |

- 返回值

  0：成功

  非0：失败


#### ST_Common_AiGlasses_StopRecord

- 功能

  结束录像

- 语法

  ```
  int ST_Common_AiGlasses_StopRecord(ST_Common_AiGlasses_Handle_t *pInstance);
  ```

- 形参

  | **参数名称** | **描述** | **输入/输出** |
  | ------------ | -------- | ------------- |
  | pInstance | pipeline实例的指针  | 输入          |

- 返回值

  0：成功

  非0：失败


#### ST_Common_AiGlasses_Uart_Init

- 功能

  串口初始化，创建数据接收线程

- 语法

  ```
  MI_S32 ST_Common_AiGlasses_Uart_Init(void* protcolTaskHandler, MI_BOOL bEnableDma);
  ```

- 形参

  | **参数名称** | **描述** | **输入/输出** |
  | ------------ | -------- | ------------- |
  | protcolTaskHandler | 命令解析工厂函数指针  | 输入          |
  | bEnableDma | 是否开启DMA | 输入 |

- 返回值

  0：成功

  非0：失败


#### ST_Common_AiGlasses_Uart_DeInit

- 功能

  等待数据接收线程结束，关闭串口

- 语法

  ```
  MI_S32 ST_Common_AiGlasses_Uart_DeInit(void);
  ```

- 形参

  无

- 返回值

  0：成功

  非0：失败


#### ST_Common_AiGlasses_Uart_ReadThread_CanExit

- 功能

  结束数据接收线程

- 语法

  ```
  MI_S32 ST_Common_AiGlasses_Uart_ReadThread_CanExit(void);
  ```

- 形参

  无

- 返回值

  0：成功

  非0：失败


#### ST_Common_AiGlasses_Uart_Write

- 功能

  串口发送数据

- 语法

  ```
  MI_S32 ST_Common_AiGlasses_Uart_Write(char* buf, int len);
  ```

- 形参

  | **参数名称** | **描述** | **输入/输出** |
  | ------------ | -------- | ------------- |
  | buf | 发送数据地址  | 输入          |
  | len | 发送数据长度  | 输入          |

- 返回值

  0：成功

  非0：失败


#### ST_Common_AiGlasses_Prot_Deal_Uart_Buffer

- 功能

  解析接收数据

- 语法

  ```
  MI_S32 ST_Common_AiGlasses_Prot_Deal_Uart_Buffer(char *uartBuf, MI_U32 len, BOOL useDma,
                                      MI_S32 (*protcolTaskHandler)(ST_Protrcol_Task_t *pstProtcolTask));
  ```

- 形参

  | **参数名称** | **描述** | **输入/输出** |
  | ------------ | -------- | ------------- |
  | uartBuf | 接收数据地址  | 输入          |
  | len | 接收数据长度  | 输入          |
  | useDma | 是否开启DMA  | 输入          |
  | protcolTaskHandler | 命令解析工厂函数指针  | 输入          |

- 返回值

  0：成功

  非0：失败


#### ST_Common_AiGlasses_Prot_Make_Cmd_To_Protrcol

- 功能

  根据task信息构建发送数据

- 语法

  ```
  MI_S32 ST_Common_AiGlasses_Prot_Make_Cmd_To_Protrcol(char *msg_buf, ST_Protrcol_Task_t *pstProtTask);
  ```

- 形参

  | **参数名称** | **描述** | **输入/输出** |
  | ------------ | -------- | ------------- |
  | msg_buf | 发送数据  | 输入          |
  | pstProtTask | task结构体指针  | 输入          |

- 返回值

  0：成功

  非0：失败



## 五、性能指标和优化建议

### 1. 功耗

功耗测量和开发优化。待补充。


### 2. 耗时

初始化时间，第一帧出图时间，stable时间，待机唤醒时间，单次拍照/录像耗时。待补充。


### 3. 内存和带宽占用

内存消耗统计，带宽占用评估。待补充。


### 4. cpu loading

cpu loading测量和评估。待补充。


---

