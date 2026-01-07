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

### 4. TopoNaming - 拓扑命名演示
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

### 5. TopoNamingFreeCAD - FreeCAD风格拓扑命名演示
**文件**: `topoNamingFreeCAD.cpp`

演示了类似FreeCAD使用的基于几何哈希的拓扑命名机制。

**主要功能**:
- **几何哈希计算**: 基于面积、中心点、法向等几何特征计算唯一哈希
- **稳定命名生成**: 为拓扑元素生成稳定的名称（Face1, Edge1等）
- **命名追踪**: 通过几何哈希在形状修改后追踪相同的几何元素
- **特征历史模拟**: 模拟FreeCAD的特征树（Feature Tree）机制
- **布尔运算稳定性**: 展示命名在布尔运算中的稳定性

**核心思想**:
- 不使用简单索引，而是基于几何特征计算哈希
- 相同几何 → 相同哈希 → 稳定命名
- 类似FreeCAD的TNP（Topological Naming Problem）解决方案

**三个演示场景**:
1. **基本命名**: 展示相同几何得到相同哈希
2. **布尔运算**: 展示运算后通过几何特征追踪元素
3. **特征历史**: 模拟多步骤建模过程中的命名更新

### 6. ShapeSharing - Shape共享机制分析验证
**文件**: `shapeSharing.cpp`

演示了OpenCASCADE中形状共享机制的原理和应用。

**主要功能**:
- **共享分析**: 分析形状中拓扑元素的共享情况
- **内存优化**: 展示共享机制带来的内存节省
- **TShape与Location分离**: 理解几何数据和位置信息的分离存储
- **复制行为**: 对比浅复制和深复制的区别
- **修改影响**: 展示修改操作对共享的影响

**六个演示场景**:
1. **独立形状**: 独立创建的形状不共享拓扑元素
2. **布尔运算**: 布尔运算中的共享行为
3. **复制操作**: 浅复制vs深复制的共享差异
4. **复合形状**: 相邻面之间共享边和顶点（内存节省50%+）
5. **修改影响**: 写时复制（Copy-on-Write）机制
6. **TShape与Location**: 几何共享与变换分离

**核心概念**:
- TShape存储几何和拓扑数据，Location存储位置信息
- 通过智能指针共享TShape，避免重复存储
- 理解共享机制对性能和内存的影响

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

# 运行FreeCAD风格拓扑命名演示
./TopoNamingFreeCAD

# 运行Shape共享机制分析演示
./ShapeSharing
```

## 学习路径建议

1. **初学者**: 从`Proj2d`开始，理解基本的几何创建和投影概念
2. **进阶**: 学习`Proj3d`，掌握3D几何和曲面操作
3. **性能优化**: 通过`ProjParallel`了解并行计算在几何计算中的应用
4. **高级主题**: 学习`TopoNaming`，理解参数化建模的核心挑战
5. **专家级**: 学习`TopoNamingFreeCAD`，掌握FreeCAD的拓扑命名解决方案
6. **内核原理**: 学习`ShapeSharing`，深入理解OCC的内存管理和形状共享机制

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
4. **FreeCAD方案**: 基于几何哈希的稳定命名（见`TopoNamingFreeCAD.cpp`）

## FreeCAD拓扑命名方案

FreeCAD采用了创新的方法来解决拓扑命名问题：

### 核心机制
- **几何哈希**: 为每个拓扑元素基于其几何特征（面积、中心、法向等）计算唯一哈希值
- **稳定追踪**: 相同几何产生相同哈希，即使在模型修改后也能追踪
- **特征树**: 在参数化建模的特征历史中维护拓扑命名

### 优势
- 几何相似性保证命名稳定性
- 支持复杂的特征历史操作
- 可以追踪布尔运算后的拓扑元素

详见`topoNamingFreeCAD.cpp`的实现和演示。

## 参考资源

- [OpenCASCADE官方文档](https://dev.opencascade.org/doc)
- [OpenCASCADE技术论坛](https://dev.opencascade.org/forums)
- [FreeCAD Topological Naming Project](https://github.com/realthunder/FreeCAD_assembly3)
- [FreeCAD Wiki - Topological Naming Problem](https://wiki.freecad.org/Topological_naming_problem)

## 许可证

本项目遵循原始演示代码的许可证。

## 贡献

欢迎提交问题和改进建议！
