
// Copyright © https://github.com/cfd-dev/OCCT-demo

// OCC Shape共享机制分析验证演示
// 本示例展示了OpenCASCADE中形状共享的机制和原理

#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

// 分析形状的共享信息
class ShapeAnalyzer
{
private:
    std::map<void*, int> ptrCount;  // 指针 -> 引用计数
    std::map<void*, std::string> ptrType;  // 指针 -> 类型名称
    
public:
    // 收集形状中所有拓扑元素的指针
    void CollectShapePointers(const TopoDS_Shape& shape, const std::string& shapeName)
    {
        std::cout << "\n分析形状: " << shapeName << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        
        // 收集顶点
        TopTools_IndexedMapOfShape vertices;
        TopExp::MapShapes(shape, TopAbs_VERTEX, vertices);
        std::cout << "顶点数量: " << vertices.Extent() << std::endl;
        for (int i = 1; i <= vertices.Extent(); ++i)
        {
            const TopoDS_Vertex& v = TopoDS::Vertex(vertices(i));
            void* ptr = v.TShape().get();
            ptrCount[ptr]++;
            ptrType[ptr] = "Vertex";
        }
        
        // 收集边
        TopTools_IndexedMapOfShape edges;
        TopExp::MapShapes(shape, TopAbs_EDGE, edges);
        std::cout << "边数量: " << edges.Extent() << std::endl;
        for (int i = 1; i <= edges.Extent(); ++i)
        {
            const TopoDS_Edge& e = TopoDS::Edge(edges(i));
            void* ptr = e.TShape().get();
            ptrCount[ptr]++;
            ptrType[ptr] = "Edge";
        }
        
        // 收集面
        TopTools_IndexedMapOfShape faces;
        TopExp::MapShapes(shape, TopAbs_FACE, faces);
        std::cout << "面数量: " << faces.Extent() << std::endl;
        for (int i = 1; i <= faces.Extent(); ++i)
        {
            const TopoDS_Face& f = TopoDS::Face(faces(i));
            void* ptr = f.TShape().get();
            ptrCount[ptr]++;
            ptrType[ptr] = "Face";
        }
    }
    
    // 打印共享统计
    void PrintSharingStatistics()
    {
        std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║              拓扑元素共享统计                          ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
        
        std::map<std::string, int> typeCounts;
        std::map<std::string, int> sharedCounts;
        
        for (const auto& pair : ptrCount)
        {
            std::string type = ptrType[pair.first];
            typeCounts[type]++;
            if (pair.second > 1)
            {
                sharedCounts[type]++;
            }
        }
        
        std::cout << "\n元素类型统计：" << std::endl;
        std::cout << "────────────────────────────────────────────────────────" << std::endl;
        std::cout << std::left << std::setw(15) << "类型" 
                  << std::setw(15) << "唯一数量" 
                  << std::setw(15) << "共享数量" 
                  << std::setw(15) << "共享率" << std::endl;
        std::cout << "────────────────────────────────────────────────────────" << std::endl;
        
        for (const auto& pair : typeCounts)
        {
            std::string type = pair.first;
            int total = pair.second;
            int shared = sharedCounts[type];
            double shareRate = (total > 0) ? (100.0 * shared / total) : 0.0;
            
            std::cout << std::left << std::setw(15) << type 
                      << std::setw(15) << total 
                      << std::setw(15) << shared 
                      << std::fixed << std::setprecision(1) 
                      << std::setw(15) << shareRate << "%" << std::endl;
        }
        std::cout << "────────────────────────────────────────────────────────\n" << std::endl;
    }
    
    // 打印详细的共享信息
    void PrintDetailedSharing(int minRefCount = 2)
    {
        std::cout << "\n共享元素详细信息 (引用次数 >= " << minRefCount << ")：" << std::endl;
        std::cout << "────────────────────────────────────────────────────────" << std::endl;
        
        std::map<std::string, std::vector<std::pair<void*, int>>> typeSharing;
        
        for (const auto& pair : ptrCount)
        {
            if (pair.second >= minRefCount)
            {
                std::string type = ptrType[pair.first];
                typeSharing[type].push_back({pair.first, pair.second});
            }
        }
        
        for (const auto& typePair : typeSharing)
        {
            std::cout << "\n" << typePair.first << " 共享情况：" << std::endl;
            for (const auto& item : typePair.second)
            {
                std::cout << "  地址: " << item.first 
                          << ", 引用次数: " << item.second << std::endl;
            }
        }
        std::cout << std::endl;
    }
    
    // 重置分析器
    void Reset()
    {
        ptrCount.clear();
        ptrType.clear();
    }
};

// 演示1：两个独立盒子之间没有共享
void DemoIndependentShapes()
{
    std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        演示1：独立形状（无共享）                       ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
    
    TopoDS_Shape box1 = BRepPrimAPI_MakeBox(100, 100, 100).Shape();
    TopoDS_Shape box2 = BRepPrimAPI_MakeBox(100, 100, 100).Shape();
    
    ShapeAnalyzer analyzer;
    analyzer.CollectShapePointers(box1, "盒子1");
    analyzer.CollectShapePointers(box2, "盒子2");
    analyzer.PrintSharingStatistics();
    
    std::cout << "结论：两个独立创建的盒子，即使尺寸相同，也不共享任何拓扑元素。" << std::endl;
    std::cout << "每个盒子都有自己的顶点、边和面实例。" << std::endl;
}

// 演示2：布尔运算后的共享
void DemoBooleanSharing()
{
    std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        演示2：布尔运算中的形状共享                     ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
    
    // 创建两个相交的盒子
    TopoDS_Shape box1 = BRepPrimAPI_MakeBox(100, 100, 100).Shape();
    gp_Trsf transform;
    transform.SetTranslation(gp_Vec(50, 0, 0));
    TopoDS_Shape box2 = BRepBuilderAPI_Transform(box1, transform).Shape();
    
    // 融合操作
    TopoDS_Shape fused = BRepAlgoAPI_Fuse(box1, box2).Shape();
    
    ShapeAnalyzer analyzer;
    analyzer.CollectShapePointers(box1, "原始盒子1");
    analyzer.CollectShapePointers(box2, "平移后盒子2");
    analyzer.CollectShapePointers(fused, "融合结果");
    analyzer.PrintSharingStatistics();
    analyzer.PrintDetailedSharing(2);
    
    std::cout << "结论：布尔融合操作会创建新的拓扑结构，但可能会共享某些未修改的元素。" << std::endl;
}

// 演示3：复制操作的共享行为
void DemoCopySharing()
{
    std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        演示3：复制操作的共享行为                       ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
    
    TopoDS_Shape original = BRepPrimAPI_MakeBox(100, 100, 100).Shape();
    
    // 浅复制（赋值操作）
    TopoDS_Shape shallowCopy = original;
    
    // 深复制（使用BRepBuilderAPI_Copy）
    TopoDS_Shape deepCopy = BRepBuilderAPI_Copy(original).Shape();
    
    std::cout << "\n=== 浅复制（赋值）===" << std::endl;
    ShapeAnalyzer analyzer1;
    analyzer1.CollectShapePointers(original, "原始形状");
    analyzer1.CollectShapePointers(shallowCopy, "浅复制");
    analyzer1.PrintSharingStatistics();
    analyzer1.PrintDetailedSharing(2);
    
    std::cout << "\n=== 深复制（BRepBuilderAPI_Copy）===" << std::endl;
    ShapeAnalyzer analyzer2;
    analyzer2.CollectShapePointers(original, "原始形状");
    analyzer2.CollectShapePointers(deepCopy, "深复制");
    analyzer2.PrintSharingStatistics();
    
    std::cout << "结论：" << std::endl;
    std::cout << "- 浅复制（赋值）：完全共享所有拓扑元素（100%共享率）" << std::endl;
    std::cout << "- 深复制：创建独立的拓扑元素副本（0%共享率）" << std::endl;
}

// 演示4：复合形状中的共享
void DemoCompoundSharing()
{
    std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        演示4：相邻面之间的边和顶点共享                 ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
    
    // 创建一个盒子
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100, 100, 100).Shape();
    
    // 分析单个盒子内部的共享
    ShapeAnalyzer analyzer;
    
    // 计算理论上的拓扑元素数量
    std::cout << "\n盒子拓扑分析：" << std::endl;
    std::cout << "理论值（如果不共享）：" << std::endl;
    std::cout << "  - 6个面，每个面4条边 = 24条边引用" << std::endl;
    std::cout << "  - 6个面，每个面4个顶点 = 24个顶点引用" << std::endl;
    
    TopTools_IndexedMapOfShape faces, edges, vertices;
    TopExp::MapShapes(box, TopAbs_FACE, faces);
    TopExp::MapShapes(box, TopAbs_EDGE, edges);
    TopExp::MapShapes(box, TopAbs_VERTEX, vertices);
    
    std::cout << "\n实际值（OCC优化后）：" << std::endl;
    std::cout << "  - 唯一面数量: " << faces.Extent() << std::endl;
    std::cout << "  - 唯一边数量: " << edges.Extent() << " (每条边被2个面共享)" << std::endl;
    std::cout << "  - 唯一顶点数量: " << vertices.Extent() << " (每个顶点被3条边共享)" << std::endl;
    
    // 分析每条边被多少个面共享
    TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
    TopExp::MapShapesAndAncestors(box, TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);
    
    std::cout << "\n边的共享详情：" << std::endl;
    std::map<int, int> sharingCount;  // 共享次数 -> 边的数量
    for (int i = 1; i <= edgeFaceMap.Extent(); ++i)
    {
        int faceCount = edgeFaceMap(i).Extent();
        sharingCount[faceCount]++;
    }
    
    for (const auto& pair : sharingCount)
    {
        std::cout << "  被 " << pair.first << " 个面共享的边: " 
                  << pair.second << " 条" << std::endl;
    }
    
    // 计算共享带来的内存节省
    int totalEdgeRefs = faces.Extent() * 4;  // 每个面4条边
    int uniqueEdges = edges.Extent();
    double edgeSaving = 100.0 * (1.0 - (double)uniqueEdges / totalEdgeRefs);
    
    int totalVertexRefs = faces.Extent() * 4;  // 每个面4个顶点
    int uniqueVertices = vertices.Extent();
    double vertexSaving = 100.0 * (1.0 - (double)uniqueVertices / totalVertexRefs);
    
    std::cout << "\n共享带来的内存节省：" << std::endl;
    std::cout << "  - 边: " << std::fixed << std::setprecision(1) 
              << edgeSaving << "%" << std::endl;
    std::cout << "  - 顶点: " << std::fixed << std::setprecision(1) 
              << vertexSaving << "%" << std::endl;
    
    std::cout << "\n结论：OCC通过共享相邻面之间的边和顶点，大幅减少内存使用。" << std::endl;
}

// 演示5：修改形状对共享的影响
void DemoModificationImpact()
{
    std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        演示5：形状修改对共享的影响                     ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
    
    // 创建原始盒子
    TopoDS_Shape original = BRepPrimAPI_MakeBox(100, 100, 100).Shape();
    
    // 创建浅复制
    TopoDS_Shape copy1 = original;
    TopoDS_Shape copy2 = original;
    
    std::cout << "\n步骤1：创建原始形状和两个浅复制" << std::endl;
    ShapeAnalyzer analyzer1;
    analyzer1.CollectShapePointers(original, "原始");
    analyzer1.CollectShapePointers(copy1, "复制1");
    analyzer1.CollectShapePointers(copy2, "复制2");
    analyzer1.PrintSharingStatistics();
    
    // 对复制1进行布尔运算
    gp_Ax2 axis(gp_Pnt(50, 50, 0), gp_Dir(0, 0, 1));
    TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(axis, 20.0, 120.0).Shape();
    TopoDS_Shape modified = BRepAlgoAPI_Cut(copy1, cylinder).Shape();
    
    std::cout << "\n步骤2：对复制1进行布尔切割操作" << std::endl;
    ShapeAnalyzer analyzer2;
    analyzer2.CollectShapePointers(original, "原始");
    analyzer2.CollectShapePointers(modified, "修改后");
    analyzer2.CollectShapePointers(copy2, "复制2（未修改）");
    analyzer2.PrintSharingStatistics();
    
    std::cout << "\n结论：" << std::endl;
    std::cout << "- 修改浅复制会创建新的拓扑结构" << std::endl;
    std::cout << "- 原始形状和其他未修改的复制保持共享" << std::endl;
    std::cout << "- 这体现了OCC的写时复制（Copy-on-Write）特性" << std::endl;
}

// 演示6：TShape和Location的分离
void DemoTShapeLocation()
{
    std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        演示6：TShape（几何）与Location（位置）分离     ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
    
    // 创建原始形状
    TopoDS_Shape original = BRepPrimAPI_MakeBox(50, 50, 50).Shape();
    
    // 创建多个平移副本
    std::vector<TopoDS_Shape> copies;
    for (int i = 0; i < 5; ++i)
    {
        gp_Trsf transform;
        transform.SetTranslation(gp_Vec(i * 60, 0, 0));
        TopoDS_Shape copy = BRepBuilderAPI_Transform(original, transform, Standard_True).Shape();
        copies.push_back(copy);
    }
    
    std::cout << "\n创建了5个平移的盒子副本" << std::endl;
    
    // 分析共享
    ShapeAnalyzer analyzer;
    analyzer.CollectShapePointers(original, "原始");
    for (size_t i = 0; i < copies.size(); ++i)
    {
        std::ostringstream oss;
        oss << "平移副本" << (i + 1);
        analyzer.CollectShapePointers(copies[i], oss.str());
    }
    analyzer.PrintSharingStatistics();
    analyzer.PrintDetailedSharing(2);
    
    std::cout << "\n说明：" << std::endl;
    std::cout << "BRepBuilderAPI_Transform的第三个参数控制是否复制几何：" << std::endl;
    std::cout << "- Standard_True: 复制几何（不共享TShape）" << std::endl;
    std::cout << "- Standard_False: 共享几何，仅修改Location（位置信息）" << std::endl;
    
    // 演示使用共享几何的变换
    std::cout << "\n\n使用共享几何的变换（copyGeom=False）：" << std::endl;
    std::vector<TopoDS_Shape> sharedCopies;
    for (int i = 0; i < 5; ++i)
    {
        gp_Trsf transform;
        transform.SetTranslation(gp_Vec(i * 60, 0, 0));
        TopoDS_Shape copy = BRepBuilderAPI_Transform(original, transform, Standard_False).Shape();
        sharedCopies.push_back(copy);
    }
    
    ShapeAnalyzer analyzer2;
    analyzer2.CollectShapePointers(original, "原始");
    for (size_t i = 0; i < sharedCopies.size(); ++i)
    {
        std::ostringstream oss;
        oss << "共享副本" << (i + 1);
        analyzer2.CollectShapePointers(sharedCopies[i], oss.str());
    }
    analyzer2.PrintSharingStatistics();
    analyzer2.PrintDetailedSharing(2);
    
    std::cout << "\n结论：" << std::endl;
    std::cout << "- 不复制几何时，所有副本共享相同的TShape（100%共享率）" << std::endl;
    std::cout << "- 每个副本仅存储不同的Location（变换矩阵）" << std::endl;
    std::cout << "- 这大幅节省内存，适合创建相同形状的多个实例" << std::endl;
}

int main()
{
    std::cout << "╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                                                        ║" << std::endl;
    std::cout << "║        OpenCASCADE Shape共享机制分析验证               ║" << std::endl;
    std::cout << "║                                                        ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
    
    std::cout << "\nOpenCASCADE的形状共享机制是其高效内存管理的核心：" << std::endl;
    std::cout << "1. TShape（拓扑形状）：存储实际的几何和拓扑数据" << std::endl;
    std::cout << "2. Location：存储形状的位置和方向信息" << std::endl;
    std::cout << "3. 通过智能指针共享TShape，避免重复存储相同几何" << std::endl;
    
    // 运行所有演示
    DemoIndependentShapes();
    DemoBooleanSharing();
    DemoCopySharing();
    DemoCompoundSharing();
    DemoModificationImpact();
    DemoTShapeLocation();
    
    std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    总结                                ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
    
    std::cout << "\nOCC的Shape共享机制优势：" << std::endl;
    std::cout << "✓ 内存效率：通过共享减少冗余数据" << std::endl;
    std::cout << "✓ 性能优化：减少复制操作的开销" << std::endl;
    std::cout << "✓ 一致性：共享的元素保证几何一致" << std::endl;
    std::cout << "✓ 灵活性：支持浅复制和深复制" << std::endl;
    
    std::cout << "\n注意事项：" << std::endl;
    std::cout << "⚠ 修改共享的形状可能影响其他引用" << std::endl;
    std::cout << "⚠ 需要理解浅复制和深复制的区别" << std::endl;
    std::cout << "⚠ 布尔运算会创建新的拓扑结构" << std::endl;
    
    std::cout << "\n════════════════════════════════════════════════════════" << std::endl;
    
    return 0;
}
