## 毕设
version: 3.0
### 已实现功能
1. 工业相机
   1. 相机触发控制
   2. 相机图像处理
   3. 相机数据导出
   4. 相机参数设置
2. Root模块
   1. 二维绘图功能
   2. 三维绘图功能
   3. 画布多图功能
   4. 图片清理功能
3. GLIA模块
   1. c++仿真测试
   2. 项目接口
4. 总控模块
   1. 基本使用功能
### 待实现功能
1. 工业相机
   1. 相机多开功能
   2. 图像多格式数据导出功能
   3. 高深度处理功能
~~2. Root模块~~
   1. ~~绘图风格设置~~
3. ~~GLIA模块~~
4. ART模块
   1. 输出周期设置
   2. cyclePoints和调制频率之间的数学关系
5. 总控模块
   1. 采集周期获取
   2. 根据采集周期设置ART模块
### 待解决功能
1. 编写fGLIA函数处理文件夹中的数据
2. 测试同步性
### 同步性方法
1. 配置采集卡为单点输出模式---one_demand
2. 工业相机为软触发模式
3. 一个时刻一个时刻采样
4. ~~重写fGLIA函数~~
### step采集
1. 打开相机，相机设置软触发
2. 打开采集卡，设置为单点输出
3. 根据相机帧率生成调制数据
4. 进入循环
   1. 取出一帧调制数据
   2. 写入采集卡
   3. 采集卡输出
   4. 读回采集卡数据
   5. 相机采集
5. 调用算法