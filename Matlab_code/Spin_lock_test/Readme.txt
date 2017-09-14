Matlab代码简介：
drawActualSurf：绘制实测性能曲面，输入数据是经过相应C++处理后的数据。（见C++代码）。有两个选项：绘制x86或者ARM
DrawCtimeLine：绘制实际测试抢锁耗时散点图，以及相应的拟合曲线和理论曲线。有两个选项：绘制x86或者ARM
findMostSimilar：输入理论曲面和测试曲面集，匹配与理论曲面残差最小的测试曲面。
SpeedupNew：不采用c/s参数的方式，固定c，s改变a来绘制理论性能曲面。
Speedup：用c/s参数来绘制理论性能曲面。
DraARM_thoeryCurveNew：利用SpeedupNew来绘制两段的ARM理论曲面
DrawARM_thoeryCurve：利用Speedup来绘制两段的ARM理论曲面

Spin_Lock_Curve:一些存储的原始性能曲面数据和散点图及其拟合图数据。


