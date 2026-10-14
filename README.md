# CompVision-Assignment

基于 **MFC + OpenCV** 的计算机视觉（数字图像处理）课程作业演示程序。程序采用自绘深色主题界面，通过顶部弹出菜单调用 40+ 种图像处理算法，支持撤销 / 重做、参数配置热加载。

## 功能特性

### 文件
- 打开 / 保存图像到指定目录 / 另存为 / 保存分析结果
- 撤销 / 重做 / 恢复原始图像

### 基本变换
- 灰度化、二值化（固定阈值 / Otsu 自动阈值）
- 平移、缩放、旋转
- 灰度反转、色彩反转
- 线性灰度变换、分段线性灰度变换
- 对数变换、指数变换、Gamma 校正

### 直方图增强
- 离散直方图均衡化（DHE）
- 离散直方图规定化（DHS，需选择参考图）
- CLAHE 局部直方图均衡化

### 滤波
- 邻域最大值 / 最小值 / 中值 / 均值滤波（可配置核大小）
- 双边滤波（保边去噪）
- 巴特沃斯低通 / 高通滤波（频域，需先执行傅里叶变换）

### 形态学
- 腐蚀 / 膨胀 / 开运算 / 闭运算

### 噪声
- 加性高斯白噪声（AWGN）
- 泊松-高斯混合噪声
- 信号相关联合噪声（ELD-SFRN）

### 边缘检测与分割
- Robert / Prewitt / Sobel / Frie-Chen 一阶算子
- Laplacian / LOG / Canny 二阶算子（Canny 为手写实现）
- 分水岭分割（距离变换 / 形态学梯度 / Canny 三种模式）
- 阈值自动估计（边缘密度自适应、Canny 双阈值 Otsu 估计）

### 检测
- Hough 直线检测
- Harris 角点检测

### 特征匹配（两张同一物体不同角度的照片）
- 打开第二张图像作为待匹配图像（复用参考图槽位）
- ORB 特征匹配：ORB 检测子 + BFMatcher 汉明距离
- SIFT 特征匹配：SIFT 检测子 + FLANN(KD-Tree)
- AKAZE 特征匹配：AKAZE 检测子 + BFMatcher 汉明距离
- 统一流程：KNN 取两个最近邻 → Lowe 比值检验 → RANSAC 单应性内点统计 → 两图并排连线可视化
- 结果输出到主显示区与分析结果槽位，可用「文件 - 保存分析结果到」导出

### 频域
- 离散傅里叶变换（DFT）/ 逆变换（IDFT）

### 立体视觉（第五章）
- 相机标定：棋盘格角点检测（含亚像素细化）、单目标定（K / 畸变系数 / 逐幅图重投影误差）、
  双目标定（R、T、E、F 与基线 |T|）、立体校正（`stereoRectify`）与畸变校正
- 立体校正：由标定结果生成校正映射并重映射左右图，并排显示并叠加水平线检查极线对齐
- 视差计算：SGBM 与 BM 两条路径，参数（视差范围、匹配窗口、P1/P2、左右一致性阈值、
  唯一性比率、斑点滤除、中值滤波）全部可在 `properties.ini` 中调整；含无效视差掩膜
- 视差转深度：按讲义式 $Z=fB/d$ 输出深度图，并给出中位深度与理论分辨率 $\Delta Z=Z^2\Delta d/(fB)$

### 分析
- 彩色直方图分析
- 灰度直方图分析（含均值 / 方差 / 信息熵统计）

## 项目结构

```
MFC_PicViewer/
├── MFC_PicViewer.cpp / .h            # 应用入口 (CWinApp)
├── MFC_PicViewerDlg.cpp / .h         # 主对话框：UI 布局、菜单、命令分发
├── MFC_PicViewer_OptionDlg.cpp / .h  # 参数配置窗口（占位）
├── ImageDocument.cpp / .h            # 图像状态：原始/当前/分析/参考图 + 撤销重做双栈
├── MyTransforms.cpp / .h             # 变换 / 直方图增强 / 边缘检测 / 分水岭 / DFT / Hough / Harris
├── MyFilter.cpp / .h                 # 空间域滤波 + 巴特沃斯频域滤波 + 双边滤波
├── MyMorphology.cpp / .h             # 形态学：腐蚀 / 膨胀 / 开 / 闭
├── MyFeatureMatch.cpp / .h           # 特征匹配：ORB / SIFT / AKAZE + 比值检验 + RANSAC
├── MyCalibration.cpp / .h            # 相机标定：棋盘格检测 / 单目与双目标定 / 立体校正
├── MyStereo.cpp / .h                 # 立体匹配：SGBM 与 BM 视差 / 后处理 / 视差转深度
├── MyNoise.cpp / .h                  # AWGN / 泊松-高斯 / ELD-SFRN 噪声
├── MyOpenCV.cpp / .h                 # 仿射变换封装
├── MyMenu.cpp / .h                   # 自绘弹出菜单
├── MyButton.cpp / .h                 # 自绘三态按钮
├── MyImageStatic.cpp / .h            # 图像显示控件（缩放/拖拽）
├── MyEditControl.cpp / .h            # 信息栏编辑框
├── MyStatic.cpp / .h                 # 自绘静态文本
├── MyINIReader.h                     # 轻量 INI 解析器（UTF-8 BOM 兼容）
├── properties.ini                    # 算法参数配置
└── res/                              # 图标与按钮图片
```

## 构建

- 环境：Windows + Visual Studio 2022（v143）、OpenCV 4.x（本机路径 `E:\OpenCV\opencv\build`，链接 `opencv_world4110[d].lib`）
- 打开 `MFC_PicViewer.sln`，选择 x64 + Debug / Release 直接生成
- 运行前确保 OpenCV DLL 在系统 PATH 中，或拷贝到 exe 同目录

## 使用

- 顶部菜单栏：**文件 / 编辑 / 增强 / 滤波 / 噪声 / 分析 / 配置 / 匹配 / 立体视觉**
- 点击菜单栏文字弹出对应功能菜单
- 特征匹配流程：「文件 → 打开」载入第一张 → 「匹配 → 打开第二张图像」载入另一角度照片 → 选择 ORB / SIFT / AKAZE 之一
- 立体视觉流程：「文件 → 打开」载入左图 → 「立体视觉 → 打开右图」载入右图 →
  「相机标定」选择棋盘格图像目录（文件名形如 `left01.jpg` / `right01.jpg`）→
  「立体校正」→「SGBM 视差」→「视差转深度」；结果同样存入分析结果槽位，可用「文件 - 保存分析结果到」导出
- 修改 `properties.ini` 后，通过「配置 → 参数载入」热加载，无需重新编译
- 底部信息栏输出每次操作的日志与统计结果

## 配置参数（properties.ini）

| 分组 | 参数 | 默认值 | 说明 |
|------|------|--------|------|
| transform | scale_x / scale_y | 2.0 / 4.0 | 缩放系数 |
| transform | rotate_theta | 90.0 | 旋转角度（度） |
| transform | shift_x / shift_y | 50 / 50 | 平移像素 |
| transform | logK / expK / gamma | 2.0 / 0.5 / 1.0 | 对数 / 指数 / Gamma 变换参数 |
| transform | grayscale_l / grayscale_r | 100 / 155 | 线性灰度变换输出区间 |
| filter | filter_kernel | 7 | 空间域滤波核大小（奇数） |
| filter | butterworth_D0 / butterworth_level | 500 / 1 | 巴特沃斯截止频率 / 阶数 |
| filter | bilateral_d / sigma_color / sigma_space | 9 / 75 / 75 | 双边滤波参数 |
| morphology | morphology_kernel | 3 | 形态学核大小 |
| enhance | clahe_clip_limit / clahe_tile_size | 2.0 / 8 | CLAHE 对比度限制 / 分块尺寸 |
| noise | noise_AWGN_sigma | 25 | AWGN 标准差 |
| noise | noise_Poisson_Gaussian_gain / sigma | 4 / 5 | 泊松增益 / 高斯标准差 |
| noise | noise_ELD_SFRN_alpha / sigmaT / sigmaG / q | 4 / 3 / 2 / 1 | 信号相关噪声参数 |
| edge | if_auto_estimate_threshold | 1 | 是否自动估计边缘阈值 |
| edge | default_edge_* | 见 ini | 各边缘算子默认阈值 |
| detect | hough_threshold / min_line_length / max_line_gap | 100 / 50 / 10 | Hough 直线检测参数 |
| detect | harris_threshold / k | 0.01 / 0.04 | Harris 角点相对阈值 / 响应系数 |
| feature_match | match_ratio | 0.75 | Lowe 比值检验阈值（越小越严格） |
| feature_match | match_max_features | 1000 | 检测子保留的最大特征点数 |
| feature_match | match_ransac_threshold | 3.0 | RANSAC 单应性重投影阈值（像素） |
| feature_match | match_max_draw_matches | 100 | 可视化中绘制的连线数量上限 |
| calibration | board_width / board_height | 9 / 6 | 棋盘格内角点数（列 / 行） |
| calibration | square_size | 0.025 | 方格边长（米） |
| calibration | subpix_win / use_subpix | 11 / 1 | 角点亚像素细化窗口 / 是否启用 |
| calibration | stereo_fix_intrinsics | 1 | 双目标定是否固定各自内参 |
| stereo | stereo_mode | 1 | 1 = SGBM，2 = BM |
| stereo | stereo_num_disparities / stereo_block_size | 64 / 9 | 视差搜索范围 / 匹配窗口 |
| stereo | stereo_p1 / stereo_p2 | 0 / 0 | SGBM 平滑项（0 表示按窗口自动取值） |
| stereo | stereo_disp12_max_diff | 0 | 左右一致性检查阈值（0 表示关闭） |
| stereo | stereo_uniqueness_ratio | 10 | 唯一性比率（百分数） |
| stereo | stereo_speckle_window_size / stereo_speckle_range | 100 / 2 | 斑点滤除窗口与视差范围 |
| stereo | stereo_median_size | 5 | 视差中值滤波窗口 |

## 架构说明（2026-08 重构后）

1. **ImageDocument 数据层**：原始图 / 当前图 / 分析图 / 参考图与文件路径统一管理，撤销 / 重做采用双栈实现，取代原全局变量。
2. **图像操作注册表**：每个算法以 `ImageOpDef{ id, run, log, needsFreqDomain, toBgr }` 注册，由 `RunOperation()` 统一执行「检查 → 计算 → 类型转换 → 入栈 → 显示 → 日志」。新增算法只需注册条目 + 添加菜单项。
3. **声明式菜单表**：9 个弹出菜单收敛为 `kMenuBars` 数据表 + 一个 `ShowPopupMenu()`。
4. **AppParams 参数结构体**：算法参数统一收敛，由 `properties.ini` 驱动。
5. **立体视觉会话状态**：标定 → 校正 → 视差 → 深度 的结果在主对话框内依次传递
   （`m_calib_left` / `m_calib_stereo` / `m_calib_rect` / `m_rect_left` / `m_disparity`），
   结果统一显示在主显示区并存入分析结果槽位。
6. **窗口圆角**：优先请求 Windows 11 的 DWM 圆角（抗锯齿），系统不支持时回退为窗口区域裁剪（见 `ApplyRoundedCorners()`）。

## 已知说明

- 「区域增长」为占位实现，点击无效果（算法分支未实现，重构后安全跳过）。
- 「另存为」的 JPEG 分支沿用历史行为，写入上次「保存到」推导的目录而非用户所选路径（历史遗留 bug，未修改）。
