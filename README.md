# mini智能旋钮ESP32版本

### V1.0.2_221205
1. 增加里程界面，动态数字位数的功能
2. 主频下调到80Mhz
3. 新增FFF8特征值，用来适配APP请求阻力
4. 增加结束运动时展示15秒过程中，可以由APP马上发起新运动的功能
5. rpm 阻力 卡路里界面，使用有效位设计，高位为0时，不显示数字
6. 时间界面分钟位，如果小于10，那么最高位不显示
7. 单车卡路里公式更新

### V1.0.3_221206
1. 修正时速异常翻倍的问题

### V1.0.4_221208
1. 支持里程下发同步的逻辑

### V1.0.5_221208
1. 里程同步支持接受到下发同步数据后6秒内不再主动累计数据
2. 添加电池电量服务,默认电量为100
3. 增加发电机EEE0服务的预览
4. 卡路里显示单位切换为0.1cal
5. 支持下发同步设置卡路里单位为0.1，需要注意，每次结束运动后会主动还原单位为1
6. 增加通过NVS读取型号判断是使用内置电机还是外置电机

### V1.0.6.221213
1. 新增过程阻力显示的功能，M1S 0-100阻力约3.6秒，80-100约700毫秒
2. 优化阻力档位在非运动界面可以通过编码器调阻的问题
3. M1S实装，空闲界面十分钟后自动关机
4. M1S实装，暂停界面3秒踏频唤醒设备
5. M1S实装，暂停界面十分钟后自动结算
6. M1S实装，结算界面维持15秒后自动关机的逻辑
7. M1S实装，结算界面3秒踏频回到空闲界面

### V1.0.7.221214
1. 卡路里上报1卡路里或者0.1卡路里受APP下发指令控制
2. M1S修复关机时RGB灯不会熄灭的问题
3. M1S修复非运动状态下卡路里数据异常累计的问题
4. M1S默认隐藏电池电量低指示图标

### V1.0.8.221215
1. 增加蓝牙链接成功蜂鸣器短鸣一声的功能
2. 修正蓝牙图标闪烁逻辑错误的问题，目前未连接时亮0.2s灭0.8s
3. OTA增加动画与对应OTA失败报错E02

### V1.0.9.221215
1. 新增数据触摸切换可以由里程界面切换到阻力界面
2. 警报闪烁变更为警示/报警：亮0.1s，灭0.4s

### V1.1.0.221216
1. 修复旋转编码器到阻力界面后没有回到踏频界面的问题
2. M1M支持电量百分比上报
3. M1M支持阻力上报

### v1.1.1_221217
1. 数码管设置占空比为 15/16，亮度拉满
2. 新增E01报错，不接磁控组，一般约十秒产生报错
3. 新增空闲界面长按3秒主动关机的逻辑

### V1.1.2_221217
1. 修复蓝牙链接成功后，在空闲界面十分钟也无法触发关机的问题
2. M1S运动中持续十分钟无踏频会主动休眠

### V1.1.3_221217
1. 增加M1M/M2M支持关机整机断电的功能
2. 修复报错界面长按3秒不能关机的bug
3. M1S修正暂停界面跳转到结算界面的时间，10min->5min

### V1.1.4_221217
1. 修复M1S骑行不可以唤醒单车的问题

### V1.1.5_221217
1. 结算界面长按无法关机的问题

### V1.1.6_221220
1. 增加M1M与M2M的电量保护策略,现在会根据自发电与插电版区分识别休眠时间了
2. M1M待机界面无踏频休眠时间由10min->30s
3. 增加M1M/M2M电池电量低闪烁提醒
4. M1M/M2M现在会根据蓝牙是否连接来区分对应的暂停进入结算界面的时机
5. M1S修正阻力是下降趋势时，不展示过程阻力的问题。(目前阻力变化测试，如果是小于8%的变化，那么没有机会看到阻力变化过程)
6. rpm下发计算时如果超过150，那么视为150，用来规避功率公式的异常
7. 尝试优化了M1M/M2M过程阻力抖动问题

### V1.1.7_221220
1. M1M/M2M真实阻力增加了四舍五入，用来改善阻力变更时，往回跳一个值的问题

### V1.1.8_221221
1. M1M/M2M增加暂停界面主动息屏节约功耗的能力

### V1.1.9_221222
1. M1S修正暂停界面5分钟自动结算运动时长为10分钟

### V1.2.0_221226
1. M1S显示rpm间隔变更：100ms->1000ms

### V1.2.1_221227
1. 新增RPM滑动求平均，使RPM变化幅度变小
2. 优化了rpm归零的间隔，改善体验

### V1.2.2_221229
1. MIS阻力位置变化 MAX_MOTOR_AD (1855->1955) MIN_MOTOR_AD (1255->1355)

### V1.2.3_221230
1. 显示时间增加5999秒的上限限制防止因时间突破上限导致的异常
2. 距离超过999.9km时显示为999.9
3. 卡路里超过9999.9时显示视为9999.9

### V1.2.8_230102
1. M1S优化单车骑行时WiFi OTA成功率低的问题

### V1.2.9_230104
1. M1M/M2M,关机时增加了有rpm会进行重启的逻辑，优化了用户持续骑行无法正常关机的问题

### V1.3.0_230105
1. M1S/M1M/M2M支持关机黑屏期间，按键重新唤醒设备
2. M1S/M1M/M2M支持关机黑屏期间，rpm重新唤醒设备
3. 开机时额外增加250毫秒延时，用来防止自发电设备上电时电压不稳定导致出现异常
4. 结束时有RPM计数器进行一次清零,修改结算时强制3秒不跳转到待机界面

### V1.3.1_230107
1. M1M/M2M校验优化了与下控板的通讯，现在数据更新更稳定了。
2. M1M/M2M增加下控异常断联，通讯5次后RPM强制归零，用来防止异常情况出现

### V1.3.2_230108
1. 新增了M2M功率公式

### V1.3.3_230109
1. 修改M1M/M2M已连接蓝牙自动结算时间是10min的问题，应该为5min

### V1.3.4_230110
1. 增加电量显示限制，现在最大电量百分比不会超过100%了

### V1.3.5_230110
1. 优化与下电控的通讯方式，避免出现正常使用时RPM归0的情况

### V1.3.6_230112
1. M1M/M2M修复下电控死机时，上电控再也无法唤醒的问题（唤醒IO设置错误）

### V1.4.0_230314
1. 增加E80EV型号的支持

### V1.4.3_230410
1. 修复E80系列旋钮上里程不显示的问题

### V1.4.6_230426
1. 优化E80系列旋钮

### V1.4.7_230503
1. 支持M1C型号

### V1.4.8_230503
1. 与下控通讯的波特率变更为38400

### V1.4.9_230503
1. 修复RPM固定为60的问题