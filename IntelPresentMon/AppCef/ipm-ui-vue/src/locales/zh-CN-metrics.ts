export interface LocalizedText {
    readonly name: string;
    readonly description: string;
}

export interface LocalizedMetricText extends LocalizedText {
    readonly originalName: string;
}

function metric(
    originalName: string,
    name: string,
    description: string,
): LocalizedMetricText {
    return { originalName, name, description };
}

// Keys match PM_METRIC values in PresentMonAPI2/PresentMonAPI.h.
export const zhCNMetricTextById: Readonly<Record<number, LocalizedMetricText>> = {
    0: metric('Application', '应用程序', '生成该帧的进程名称。'),
    1: metric('Swap Chain Address', '交换链地址', '用于呈现该帧的交换链地址。'),
    2: metric('GPU Vendor', 'GPU 厂商', 'GPU 的厂商名称。'),
    3: metric('GPU Name', 'GPU 名称', 'GPU 的设备名称。'),
    4: metric('CPU Vendor', 'CPU 厂商', 'CPU 的厂商名称。'),
    5: metric('CPU Name', 'CPU 名称', 'CPU 的设备名称。'),
    6: metric('CPU Start Time', 'CPU 开始时间', 'CPU 开始处理该帧的时间。'),
    7: metric('CPU Start QPC', 'CPU 开始 QPC', 'CPU 开始处理该帧时的 QueryPerformanceCounter (QPC) 值。'),
    8: metric('FrameTime-App', '应用帧时间', '从该帧开始到 CPU 开始处理下一帧所经过的时间。'),
    9: metric('Ms CPU Busy', 'CPU 忙碌时间', 'CPU 在呈现该帧前用于处理该帧的时间。'),
    10: metric('Ms CPU Wait', 'CPU 等待时间', 'CPU 在开始下一帧前等待的时间。'),
    11: metric('FPS-Display', '显示帧率', '屏幕显示新帧的速率。'),
    12: metric('FPS-Presents', 'Present 调用帧率', '应用程序调用 Present() 的速率。'),
    13: metric('Ms GPU Time', 'GPU 时间', 'GPU 处理该帧所用的总时间。'),
    14: metric('Ms GPU Busy', 'GPU 忙碌时间', 'GPU 主动处理该帧的时间，即至少一个 GPU 引擎正在执行目标进程工作的时间。'),
    15: metric('Ms GPU Wait', 'GPU 等待时间', 'GPU 在处理该帧期间处于空闲状态的时间。'),
    16: metric('Dropped Frames', '丢帧', '指示该帧是否未显示。'),
    17: metric('Displayed Time', '显示时长', '该帧在屏幕上的显示时长；如果未显示，则值为“NA”。'),
    18: metric('Sync Interval', '同步间隔', '应用程序呈现该帧时提供的同步间隔。驱动程序之后可能根据控制面板覆盖设置等因素修改该值。'),
    19: metric('Present Flags', 'Present 标志', '应用程序呈现该帧时提供的 Present 标志。'),
    20: metric('Present Mode', '呈现模式', '系统用于呈现该帧的模式。'),
    21: metric('Present Runtime', '呈现运行时', '用于呈现该帧的 API。'),
    22: metric('Allows Tearing', '允许画面撕裂', '值为 1 表示屏幕可能显示不完整帧，值为 0 表示仅显示完整帧。'),
    23: metric('Ms GPU Latency', 'GPU 延迟', '从该帧开始到 GPU 开始处理该帧所经过的时间。'),
    24: metric('Display Latency', '显示延迟', '从该帧开始到该帧显示在屏幕上所经过的时间。'),
    25: metric('Ms Click To Photon Latency', '点击到显示延迟', '从参与生成该帧的最早鼠标点击，到该帧显示在屏幕上所经过的时间。'),
    26: metric('GPU Sustained Power Limit', 'GPU 持续功率限制', 'GPU 的持续功率上限。'),
    27: metric('GPU Power', 'GPU 功耗', '图形处理器消耗的功率。'),
    28: metric('GPU Voltage', 'GPU 电压', '图形适配器的工作电压。'),
    29: metric('GPU Frequency', 'GPU 频率', 'GPU 核心的时钟频率。'),
    30: metric('GPU Temperature', 'GPU 温度', 'GPU 的温度。'),
    31: metric('GPU Fan Speed', 'GPU 风扇转速', 'GPU 散热风扇的转速。'),
    32: metric('GPU Utilization', 'GPU 利用率', '当前使用的 GPU 处理能力比例。'),
    33: metric('3D/Compute Utilization', '3D/计算利用率', '当前使用的 3D/计算处理能力比例。'),
    34: metric('Media Utilization', '媒体引擎利用率', '当前使用的媒体处理能力比例。'),
    35: metric('GPU Power Limited', 'GPU 功率受限', '由于 GPU 超过最大功率限制，GPU 频率受到限制。'),
    36: metric('GPU Temperature Limited', 'GPU 温度受限', '由于 GPU 超过最高温度限制，GPU 频率受到限制。'),
    37: metric('GPU Current Limited', 'GPU 电流受限', '由于 GPU 超过最大电流限制，GPU 频率受到限制。'),
    38: metric('GPU Voltage Limited', 'GPU 电压受限', '由于 GPU 超过最大电压限制，GPU 频率受到限制。'),
    39: metric('GPU Utilization Limited', 'GPU 利用率受限', '由于 GPU 利用率较低，GPU 频率受到限制。'),
    40: metric('GPU Memory Power', 'GPU 显存功耗', 'GPU 显存消耗的功率。'),
    41: metric('GPU Memory Voltage', 'GPU 显存电压', 'GPU 显存的工作电压。'),
    42: metric('GPU Memory Frequency', 'GPU 显存频率', 'GPU 显存的时钟频率。'),
    43: metric('GPU Memory Effective Frequency', 'GPU 显存有效频率', 'GPU 显存可达到的有效数据传输速率。'),
    44: metric('GPU Memory Temperature', 'GPU 显存温度', 'GPU 显存的温度。'),
    45: metric('GPU Memory Size', 'GPU 显存容量', 'GPU 显存的总容量。'),
    46: metric('GPU Memory Size Used', 'GPU 显存已用容量', 'GPU 显存的已用容量。'),
    47: metric('GPU Memory Utilization', 'GPU 显存利用率', 'GPU 显存的已用百分比。'),
    48: metric('GPU Memory Max Bandwidth', 'GPU 显存最大带宽', 'GPU 显存的最大总带宽。'),
    49: metric('GPU Memory Write Bandwidth', 'GPU 显存写入带宽', 'GPU 显存写入操作的最大带宽。'),
    50: metric('GPU Memory Read Bandwidth', 'GPU 显存读取带宽', 'GPU 显存读取操作的最大带宽。'),
    51: metric('GPU Memory Power Limited', 'GPU 显存功率受限', '由于显存模块超过最大功率限制，显存频率受到限制。'),
    52: metric('GPU Memory Temperature Limited', 'GPU 显存温度受限', '由于显存模块超过最高温度限制，显存频率受到限制。'),
    53: metric('GPU Memory Current Limited', 'GPU 显存电流受限', '由于显存模块超过最大电流限制，显存频率受到限制。'),
    54: metric('GPU Memory Voltage Limited', 'GPU 显存电压受限', '由于显存模块超过最大电压限制，显存频率受到限制。'),
    55: metric('GPU Memory Utilization Limited', 'GPU 显存利用率受限', '由于显存流量较低，显存频率受到限制。'),
    56: metric('CPU Utilization', 'CPU 利用率', '当前使用的 CPU 处理能力比例。'),
    57: metric('CPU Power Limit', 'CPU 功率限制', 'CPU 的功率上限。'),
    58: metric('CPU Power', 'CPU 功耗', 'CPU 消耗的功率。'),
    59: metric('CPU Temperature', 'CPU 温度', '对所有具有温度采样值的物理 CPU 核心求得的平均温度。'),
    60: metric('CPU Frequency', 'CPU 频率', 'CPU 的时钟频率。'),
    61: metric('CPU Core Utility', 'CPU 核心效用率', '各 CPU 核心正在使用的处理能力。'),
    62: metric('FPS-App', '应用帧率', '应用程序渲染并在屏幕上显示新帧的速率。'),
    63: metric('Frame Type', '帧类型', '该帧是由应用程序渲染，还是由驱动程序或 SDK 生成。'),
    64: metric('Ms Animation Error', '动画误差', '上一帧的 CPU 时间差与显示时间差之间的差值。'),
    65: metric('Ms All Input To Photon Latency', '输入到显示延迟', '从参与生成该帧的最早键盘或鼠标操作，到该帧显示在屏幕上所经过的时间。'),
    66: metric('Instrumented Latency', '插桩延迟', '从插桩记录的该帧起点，到该帧显示在屏幕上所经过的时间。'),
    67: metric('Animation Time', '动画时间', 'CPU 开始为该帧执行动画工作的时间。'),
    68: metric('GPU Effective Frequency', 'GPU 有效频率', 'GPU 核心的有效时钟频率。'),
    69: metric('GPU Voltage Regulator Temperature', 'GPU 稳压器温度', 'GPU 稳压器的温度。'),
    70: metric('GPU Memory Effective Bandwidth', 'GPU 显存有效带宽', '根据当前时钟频率，显存模块能够维持的数据传输速率。'),
    71: metric('GPU Overvoltage Percent', 'GPU 超压百分比', 'GPU 超压增量相对于最大允许增量的比例。'),
    72: metric('GPU Temperature Percent', 'GPU 温度百分比', 'GPU 温度相对于热余量的比例。'),
    73: metric('GPU Power Percent', 'GPU 功率百分比', 'GPU 功耗相对于默认最大功率的比例。'),
    74: metric('GPU Fan Speed Percent', 'GPU 风扇转速百分比', 'GPU 风扇转速相对于该风扇最大转速的比例。'),
    75: metric('GPU Card Power', 'GPU 板卡功耗', '图形适配器整块板卡的总功耗。'),
    76: metric('Time In Seconds', '呈现开始时间（秒）', '调用 Present() 时的时间，以秒为单位。'),
    77: metric('Present Start QPC', 'Present 开始 QPC', '调用 Present() 时的 QueryPerformanceCounter (QPC) 值。'),
    78: metric('Ms Between Presents', 'Present 调用间隔', '本次 Present() 调用与上一次调用之间的时间。'),
    79: metric('Ms In Present API', 'Present API 内耗时', '在 Present() 调用内部花费的时间。'),
    80: metric('Ms Between Display Change', '显示变化间隔', '上一帧从开始显示到本次 Present() 结果显示之间的时间。'),
    81: metric('Ms Until Displayed', '显示等待时间', '从调用 Present() 到该帧显示在屏幕上所经过的时间。'),
    82: metric('Ms Render Present Latency', '渲染呈现延迟', '从调用 Present() 到 GPU 完成该帧工作所经过的时间。'),
    83: metric('Ms Between Simulation Start', '模拟开始间隔', '上一帧与当前帧开始模拟处理的时间间隔。'),
    84: metric('Ms PC Latency', 'PC 延迟', '从 PC 接收到输入，到帧被发送至显示器所经过的时间。'),
    85: metric('FrameTime-Display', '显示帧时间', '上一帧与当前帧显示之间的时间。'),
    86: metric('Ms Between App Start', '应用帧开始间隔', '从该帧开始到 CPU 开始处理下一帧所经过的时间。'),
    87: metric('FrameTime-Presents', 'Present 帧时间', '本次 Present() 调用与上一次调用之间的时间。'),
    88: metric('Ms Flip Delay', '翻转延迟', '对 Present() 结果显示时间施加的额外延迟。'),
    89: metric('PSO Compile Count', 'PSO 编译次数', '归因于该帧的管线状态对象（PSO）编译启动次数。'),
    90: metric('Ms PSO Compile Time', 'PSO 编译时间', '归因于该帧的 PSO 编译时长总和；并发编译会分别计时。'),
    91: metric('PSO Compile Busy Percent', 'PSO 编译忙碌百分比', '该帧周期内至少有一个 PSO 编译处于活动状态的时间比例。'),
    92: metric('Process ID', '进程 ID', '生成该帧的进程 ID。'),
    93: metric('Session Start QPC', '会话开始 QPC', 'ETW 跟踪会话开始时的 QPC 值，由收到的第一个事件时间戳推导。'),
    94: metric('CPU Core Temperature', 'CPU 核心温度', '每个物理 CPU 核心的温度。'),
};

export const zhCNMetricTextByOriginalName: Readonly<Record<string, LocalizedText>> =
    {
        ...Object.fromEntries(
            Object.values(zhCNMetricTextById).map(({ originalName, name, description }) => [
                originalName,
                { name, description },
            ]),
        ),
        // This metrics.csv entry does not currently have a public PM_METRIC id.
        'Video Busy': {
            name: '视频引擎忙碌时间',
            description: 'GPU 视频编码和解码引擎主动处理该帧的时间。',
        },
    };

// Keys match PM_STAT values in PresentMonAPI2/PresentMonAPI.h.
export const zhCNStatTextById: Readonly<Record<number, LocalizedText>> = {
    0: {
        name: '无',
        description: '空统计方式，通常用于查询静态指标或处理逐帧事件。',
    },
    1: {
        name: '平均值',
        description: '滑动窗口内所有观测值的算术平均值。',
    },
    2: {
        name: '第 99 百分位',
        description: '滑动窗口内有 99% 的观测值小于或等于该值。',
    },
    3: {
        name: '第 95 百分位',
        description: '滑动窗口内有 95% 的观测值小于或等于该值。',
    },
    4: {
        name: '第 90 百分位',
        description: '滑动窗口内有 90% 的观测值小于或等于该值。',
    },
    5: {
        name: '第 1 百分位',
        description: '滑动窗口内有 1% 的观测值小于或等于该值。',
    },
    6: {
        name: '第 5 百分位',
        description: '滑动窗口内有 5% 的观测值小于或等于该值。',
    },
    7: {
        name: '第 10 百分位',
        description: '滑动窗口内有 10% 的观测值小于或等于该值。',
    },
    8: {
        name: '最大值',
        description: '滑动窗口内观测值的最大值。',
    },
    9: {
        name: '最小值',
        description: '滑动窗口内观测值的最小值。',
    },
    10: {
        name: '中点值',
        description: '滑动窗口中最接近时间中点的观测值。',
    },
    11: {
        name: '中点线性插值',
        description: '在滑动窗口中最接近时间中点的两个观测值之间进行线性插值。',
    },
    12: {
        name: '最新值',
        description: '滑动窗口中时间最近的观测值。',
    },
    13: {
        name: '最早值',
        description: '滑动窗口中时间最早的观测值。',
    },
    14: {
        name: '计数',
        description: '滑动窗口内满足指定条件的观测值数量，例如布尔字段为真的次数。',
    },
    15: {
        name: '非零平均值',
        description: '滑动窗口内排除所有零值后的帧样本平均值。',
    },
};

// Keys match PM_METRIC_AVAILABILITY values in PresentMonAPI2/PresentMonAPI.h.
export const zhCNMetricAvailabilityDescriptionById: Readonly<Record<number, string>> = {
    0: '该指标在所选设备上可用。',
    1: '该指标在所选设备上不可用。',
    2: '数据源 API 不支持该指标。',
    3: '该设备不支持此指标。',
    4: 'PresentMon 当前尚未实现该指标。',
};

export function getLocalizedMetricText(
    id: number,
    originalName: string,
): LocalizedText | undefined {
    return zhCNMetricTextById[id] ?? zhCNMetricTextByOriginalName[originalName];
}

export function getLocalizedStatText(id: number): LocalizedText | undefined {
    return zhCNStatTextById[id];
}

export function getLocalizedMetricAvailabilityDescription(id: number): string | undefined {
    return zhCNMetricAvailabilityDescriptionById[id];
}
