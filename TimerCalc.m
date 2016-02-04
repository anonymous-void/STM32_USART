psc = 0; %分频数
Tout = 1e-3; %输出时钟周期，单位为毫秒（ms）
Tclk = 7200;  %输入时钟频率，单位千赫兹（kHz）

arr = Tout * Tclk / (psc + 1) - 1