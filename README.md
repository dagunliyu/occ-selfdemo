# OpenCASCADE 自学演示项目 (OCC-SELFDEMO)

本项目包含一系列OpenCASCADE (OCC)的演示程序，用于学习和理解OCC的核心功能。

## 项目简介

OpenCASCADE是一个开源的3D建模内核，广泛应用于CAD/CAM/CAE领域。本项目通过多个独立的示例程序，展示了OCC的各种功能和概念。

## 演示程序

### 1. Proj2d - 2D投影演示
**文件**: `proj2d.cpp`

演示了如何将点投影到2D曲线（圆）上。

**主要功能**:
- 创建2D几何圆
- 使用`GeomAPI_ProjectPointOnCurve`进行点到曲线的投影
- 可视化原始点、投影点和圆
- 计算投影距离和参数

### 2. Proj3d - 3D投影演示
**文件**: `proj3d.cpp`

演示了如何将点投影到3D曲面（球面）上。

**主要功能**:
- 创建3D几何球面
- 使用`GeomAPI_ProjectPointOnSurf`进行点到曲面的投影
- UV参数计算
- 3D可视化

### 3. ProjParallel - 并行计算演示
**文件**: `projParallel.cpp`

演示了使用OpenMP和TBB进行并行计算的性能对比。

**主要功能**:
- 大规模点投影（800万个点）
- OpenMP并行实现
- TBB并行实现
- 性能对比和加速比分析
- 并行效率测试

### 4. TopoNaming - 拓扑命名演示 ⭐ 新增
**文件**: `topoNaming.cpp`

演示了OpenCASCADE中的拓扑命名问题及其解决方法。

**主要功能**:
- **拓扑命名问题演示**: 展示当形状被修改时，原有的拓扑索引如何失效
- **稳定命名方法**: 使用几何特征（位置、方向等）而非索引来识别拓扑元素
- **拓扑关联关系**: 展示面-边-顶点之间的关联关系
- **可视化**: 用不同颜色标识不同的面，直观展示拓扑结构
- **布尔运算影响**: 展示布尔运算如何改变拓扑结构

**核心概念**:
- 拓扑命名问题是参数化CAD建模中的关键挑战
- 形状修改后，索引可能改变，导致依赖操作失败
- 通过几何特征识别可以提高稳定性

## 构建说明

### 依赖项
- CMake 3.10+
- C++17 编译器
- OpenCASCADE
- VTK
- TBB (Intel Threading Building Blocks)
- OpenMP

### 构建步骤
```bash
mkdir build
cd build

# 配置CMake，指定依赖库路径
cmake .. \
  -DOCC_DIR=/path/to/opencascade \
  -DVTK_DIR=/path/to/vtk \
  -DTBB_DIR=/path/to/tbb

# 编译
cmake --build .
```

### 运行演示
```bash
# 运行2D投影演示
./Proj2d

# 运行3D投影演示
./Proj3d

# 运行并行计算演示
./ProjParallel

# 运行拓扑命名演示
./TopoNaming
```

## 学习路径建议

1. **初学者**: 从`Proj2d`开始，理解基本的几何创建和投影概念
2. **进阶**: 学习`Proj3d`，掌握3D几何和曲面操作
3. **性能优化**: 通过`ProjParallel`了解并行计算在几何计算中的应用
4. **高级主题**: 学习`TopoNaming`，理解参数化建模的核心挑战

## 拓扑命名问题详解

拓扑命名问题（Topological Naming Problem）是参数化CAD系统中的一个经典难题：

### 问题描述
当一个参数化模型被修改时（例如改变尺寸、添加/删除特征），模型的拓扑结构可能发生变化。原来用于引用特定几何元素（面、边、顶点）的"名称"或"索引"可能会：
- 指向不同的几何元素
- 变得无效（元素不存在）
- 导致后续操作失败

### 影响
这会导致：
- 参数化历史失效
- 特征重建失败
- 模型破损

### 解决方案
本演示展示了几种稳定命名的策略：
1. 基于几何特征的识别（位置、方向）
2. 拓扑关联关系追踪
3. 几何不变量使用

## 参考资源

- [OpenCASCADE官方文档](https://dev.opencascade.org/doc)
- [OpenCASCADE技术论坛](https://dev.opencascade.org/forums)

## 许可证

本项目遵循原始演示代码的许可证。

## 贡献

欢迎提交问题和改进建议！
