
// Copyright © https://github.com/cfd-dev/OCCT-demo

// FreeCAD风格的拓扑命名演示
// 本示例展示了类似FreeCAD使用的拓扑命名机制
// FreeCAD使用基于哈希和几何特征的稳定命名方案

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Geom_Surface.hxx>
#include <Standard_TypeDef.hxx>

// FreeCAD风格的拓扑元素命名类
// FreeCAD使用形如 "Face1", "Edge3" 等的名称，但内部使用几何哈希保证稳定性
class TopoNameGenerator
{
private:
    std::map<std::string, TopoDS_Shape> nameToShapeMap;
    std::map<size_t, std::string> hashToNameMap;
    int faceCounter;
    int edgeCounter;
    int vertexCounter;
    
    // 将浮点数舍入到固定精度以确保哈希稳定性
    // 这避免了浮点精度差异导致相同几何产生不同哈希的问题
    double RoundForHash(double value, double precision = 1e-6) const
    {
        return std::round(value / precision) * precision;
    }
    
    // 计算浮点数的稳定哈希
    size_t HashDouble(double value) const
    {
        double rounded = RoundForHash(value);
        return std::hash<double>{}(rounded);
    }
    
    // 计算面的几何哈希（基于面积、中心点、法向）
    size_t ComputeFaceHash(const TopoDS_Face& face)
    {
        size_t hash = 0;
        
        // 计算面积
        GProp_GProps props;
        BRepGProp::SurfaceProperties(face, props);
        double area = props.Mass();
        
        // 获取中心点
        gp_Pnt center = props.CentreOfMass();
        
        // 获取曲面类型
        BRepAdaptor_Surface surface(face);
        GeomAbs_SurfaceType surfType = surface.GetType();
        
        // 组合哈希值，添加类型标识避免与边、顶点冲突
        hash ^= std::hash<int>{}(1); // 类型标识：1=面
        hash ^= HashDouble(area);
        hash ^= HashDouble(center.X()) << 1;
        hash ^= HashDouble(center.Y()) << 2;
        hash ^= HashDouble(center.Z()) << 3;
        hash ^= std::hash<int>{}(static_cast<int>(surfType)) << 4;
        
        // 如果是平面，添加法向信息
        if (surfType == GeomAbs_Plane)
        {
            gp_Dir normal = surface.Plane().Axis().Direction();
            hash ^= HashDouble(normal.X()) << 5;
            hash ^= HashDouble(normal.Y()) << 6;
            hash ^= HashDouble(normal.Z()) << 7;
        }
        
        return hash;
    }
    
    // 计算边的几何哈希（基于长度、中点）
    size_t ComputeEdgeHash(const TopoDS_Edge& edge)
    {
        size_t hash = 0;
        
        // 添加类型标识避免与面、顶点冲突
        hash ^= std::hash<int>{}(2); // 类型标识：2=边
        
        GProp_GProps props;
        BRepGProp::LinearProperties(edge, props);
        double length = props.Mass();
        gp_Pnt center = props.CentreOfMass();
        
        hash ^= HashDouble(length);
        hash ^= HashDouble(center.X()) << 1;
        hash ^= HashDouble(center.Y()) << 2;
        hash ^= HashDouble(center.Z()) << 3;
        
        return hash;
    }
    
    // 计算顶点的几何哈希（基于坐标）
    size_t ComputeVertexHash(const TopoDS_Vertex& vertex)
    {
        size_t hash = 0;
        
        // 添加类型标识避免与面、边冲突
        hash ^= std::hash<int>{}(3); // 类型标识：3=顶点
        
        gp_Pnt pt = BRep_Tool::Pnt(vertex);
        
        hash ^= HashDouble(pt.X());
        hash ^= HashDouble(pt.Y()) << 1;
        hash ^= HashDouble(pt.Z()) << 2;
        
        return hash;
    }

public:
    TopoNameGenerator() : faceCounter(1), edgeCounter(1), vertexCounter(1) {}
    
    // 为形状生成稳定的拓扑命名
    void GenerateNames(const TopoDS_Shape& shape, const std::string& prefix = "")
    {
        // 为所有面生成名称
        TopTools_IndexedMapOfShape faces;
        TopExp::MapShapes(shape, TopAbs_FACE, faces);
        
        for (int i = 1; i <= faces.Extent(); ++i)
        {
            const TopoDS_Face& face = TopoDS::Face(faces(i));
            size_t hash = ComputeFaceHash(face);
            
            // 检查是否已经有这个几何的名称（避免重复命名相同几何）
            if (hashToNameMap.find(hash) == hashToNameMap.end())
            {
                std::ostringstream oss;
                oss << prefix << "Face" << faceCounter++;
                std::string name = oss.str();
                
                hashToNameMap[hash] = name;
                nameToShapeMap[name] = face;
            }
            // 如果哈希已存在，说明是相同或非常相似的几何，使用已有名称
        }
        
        // 为所有边生成名称
        TopTools_IndexedMapOfShape edges;
        TopExp::MapShapes(shape, TopAbs_EDGE, edges);
        
        for (int i = 1; i <= edges.Extent(); ++i)
        {
            const TopoDS_Edge& edge = TopoDS::Edge(edges(i));
            size_t hash = ComputeEdgeHash(edge);
            
            // 检查是否已经有这个几何的名称（避免重复命名相同几何）
            if (hashToNameMap.find(hash) == hashToNameMap.end())
            {
                std::ostringstream oss;
                oss << prefix << "Edge" << edgeCounter++;
                std::string name = oss.str();
                
                hashToNameMap[hash] = name;
                nameToShapeMap[name] = edge;
            }
            // 如果哈希已存在，说明是相同或非常相似的几何，使用已有名称
        }
        
        // 为所有顶点生成名称
        TopTools_IndexedMapOfShape vertices;
        TopExp::MapShapes(shape, TopAbs_VERTEX, vertices);
        
        for (int i = 1; i <= vertices.Extent(); ++i)
        {
            const TopoDS_Vertex& vertex = TopoDS::Vertex(vertices(i));
            size_t hash = ComputeVertexHash(vertex);
            
            // 检查是否已经有这个几何的名称（避免重复命名相同几何）
            if (hashToNameMap.find(hash) == hashToNameMap.end())
            {
                std::ostringstream oss;
                oss << prefix << "Vertex" << vertexCounter++;
                std::string name = oss.str();
                
                hashToNameMap[hash] = name;
                nameToShapeMap[name] = vertex;
            }
            // 如果哈希已存在，说明是相同或非常相似的几何，使用已有名称
        }
    }
    
    // 通过名称查找形状
    TopoDS_Shape FindShapeByName(const std::string& name)
    {
        auto it = nameToShapeMap.find(name);
        if (it != nameToShapeMap.end())
        {
            return it->second;
        }
        return TopoDS_Shape();
    }
    
    // 通过几何哈希查找名称（用于追踪）
    std::string FindNameByGeometry(const TopoDS_Face& face)
    {
        size_t hash = ComputeFaceHash(face);
        auto it = hashToNameMap.find(hash);
        if (it != hashToNameMap.end())
        {
            return it->second;
        }
        return "";
    }
    
    std::string FindNameByGeometry(const TopoDS_Edge& edge)
    {
        size_t hash = ComputeEdgeHash(edge);
        auto it = hashToNameMap.find(hash);
        if (it != hashToNameMap.end())
        {
            return it->second;
        }
        return "";
    }
    
    // 打印所有命名
    void PrintAllNames()
    {
        std::cout << "\n拓扑元素命名表：" << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        
        for (const auto& pair : nameToShapeMap)
        {
            std::cout << "  " << pair.first;
            
            // 添加几何信息
            if (pair.second.ShapeType() == TopAbs_FACE)
            {
                TopoDS_Face face = TopoDS::Face(pair.second);
                GProp_GProps props;
                BRepGProp::SurfaceProperties(face, props);
                gp_Pnt center = props.CentreOfMass();
                std::cout << " (面积: " << std::fixed << std::setprecision(2) 
                          << props.Mass() << ", 中心: " 
                          << center.X() << "," << center.Y() << "," << center.Z() << ")";
            }
            else if (pair.second.ShapeType() == TopAbs_EDGE)
            {
                TopoDS_Edge edge = TopoDS::Edge(pair.second);
                GProp_GProps props;
                BRepGProp::LinearProperties(edge, props);
                std::cout << " (长度: " << std::fixed << std::setprecision(2) << props.Mass() << ")";
            }
            
            std::cout << std::endl;
        }
        
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    }
    
    // 获取命名数量统计
    void PrintStatistics()
    {
        int faceCount = 0, edgeCount = 0, vertexCount = 0;
        
        for (const auto& pair : nameToShapeMap)
        {
            if (pair.second.ShapeType() == TopAbs_FACE) faceCount++;
            else if (pair.second.ShapeType() == TopAbs_EDGE) edgeCount++;
            else if (pair.second.ShapeType() == TopAbs_VERTEX) vertexCount++;
        }
        
        std::cout << "命名统计：" << std::endl;
        std::cout << "  面: " << faceCount << std::endl;
        std::cout << "  边: " << edgeCount << std::endl;
        std::cout << "  顶点: " << vertexCount << std::endl;
    }
};

// 演示FreeCAD风格的拓扑命名稳定性
void DemonstrateFreeCADNaming()
{
    std::cout << "\n############## FreeCAD风格拓扑命名演示 ##############\n" << std::endl;
    
    std::cout << "场景1：创建初始盒子并命名" << std::endl;
    TopoDS_Shape box1 = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape();
    
    TopoNameGenerator naming1;
    naming1.GenerateNames(box1, "Box1_");
    naming1.PrintAllNames();
    naming1.PrintStatistics();
    
    std::cout << "\n场景2：创建相同几何的盒子（应该得到相同的哈希）" << std::endl;
    TopoDS_Shape box2 = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape();
    
    TopoNameGenerator naming2;
    naming2.GenerateNames(box2, "Box2_");
    naming2.PrintAllNames();
    
    std::cout << "场景3：创建不同尺寸的盒子（哈希不同）" << std::endl;
    TopoDS_Shape box3 = BRepPrimAPI_MakeBox(150.0, 100.0, 100.0).Shape();
    
    TopoNameGenerator naming3;
    naming3.GenerateNames(box3, "Box3_");
    naming3.PrintAllNames();
    
    std::cout << "\n############################################\n" << std::endl;
}

// 演示拓扑命名在布尔运算中的稳定性
void DemonstrateNamingStabilityWithBooleans()
{
    std::cout << "\n############## 布尔运算中的命名稳定性 ##############\n" << std::endl;
    
    std::cout << "步骤1：创建基础盒子" << std::endl;
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape();
    
    TopoNameGenerator namingBefore;
    namingBefore.GenerateNames(box, "Before_");
    
    std::cout << "\n布尔运算前的命名：" << std::endl;
    namingBefore.PrintStatistics();
    
    // 找到顶面（Z=100）
    TopTools_IndexedMapOfShape facesBefore;
    TopExp::MapShapes(box, TopAbs_FACE, facesBefore);
    TopoDS_Face topFaceBefore;
    
    for (int i = 1; i <= facesBefore.Extent(); ++i)
    {
        TopoDS_Face face = TopoDS::Face(facesBefore(i));
        BRepAdaptor_Surface surface(face);
        if (surface.GetType() == GeomAbs_Plane)
        {
            gp_Pnt center;
            GProp_GProps props;
            BRepGProp::SurfaceProperties(face, props);
            center = props.CentreOfMass();
            
            if (std::abs(center.Z() - 100.0) < 0.1)
            {
                topFaceBefore = face;
                std::string name = namingBefore.FindNameByGeometry(face);
                std::cout << "找到顶面，名称: " << name << std::endl;
                break;
            }
        }
    }
    
    std::cout << "\n步骤2：进行布尔运算（切除圆柱）" << std::endl;
    gp_Ax2 cylinderAxis(gp_Pnt(50, 50, 0), gp_Dir(0, 0, 1));
    TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(cylinderAxis, 20.0, 120.0).Shape();
    TopoDS_Shape cutResult = BRepAlgoAPI_Cut(box, cylinder).Shape();
    
    TopoNameGenerator namingAfter;
    namingAfter.GenerateNames(cutResult, "After_");
    
    std::cout << "\n布尔运算后的命名：" << std::endl;
    namingAfter.PrintStatistics();
    
    // 尝试通过几何找到原来的顶面
    TopTools_IndexedMapOfShape facesAfter;
    TopExp::MapShapes(cutResult, TopAbs_FACE, facesAfter);
    
    std::cout << "\n查找原顶面（通过几何特征）：" << std::endl;
    bool found = false;
    for (int i = 1; i <= facesAfter.Extent(); ++i)
    {
        TopoDS_Face face = TopoDS::Face(facesAfter(i));
        
        // 检查是否是平面且Z坐标接近100
        BRepAdaptor_Surface surface(face);
        if (surface.GetType() == GeomAbs_Plane)
        {
            GProp_GProps props;
            BRepGProp::SurfaceProperties(face, props);
            gp_Pnt center = props.CentreOfMass();
            
            if (std::abs(center.Z() - 100.0) < 0.1)
            {
                std::string name = namingAfter.FindNameByGeometry(face);
                std::cout << "✓ 通过几何特征找到顶面，新名称: " << name << std::endl;
                std::cout << "  面积: " << props.Mass() 
                          << " (原面积100×100=10000, 现在减去圆孔面积)" << std::endl;
                found = true;
                break;
            }
        }
    }
    
    if (!found)
    {
        std::cout << "✗ 未找到匹配的顶面" << std::endl;
    }
    
    std::cout << "\n结论：基于几何哈希的命名可以追踪几何相似的元素，" << std::endl;
    std::cout << "即使在布尔运算后面的几何发生变化（如面积改变），" << std::endl;
    std::cout << "仍可通过几何特征（位置、法向）进行追踪。" << std::endl;
    
    std::cout << "\n############################################\n" << std::endl;
}

// 演示拓扑命名在特征历史中的应用
void DemonstrateNamingInFeatureHistory()
{
    std::cout << "\n############## 特征历史中的拓扑命名 ##############\n" << std::endl;
    
    std::cout << "这模拟了FreeCAD的特征树（Feature Tree）机制\n" << std::endl;
    
    // 特征1：创建基础盒子
    std::cout << "特征1：创建基础盒子 (100x100x100)" << std::endl;
    TopoDS_Shape feature1 = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape();
    TopoNameGenerator naming;
    naming.GenerateNames(feature1, "F1_");
    std::cout << "生成命名..." << std::endl;
    naming.PrintStatistics();
    
    // 特征2：切除圆柱1
    std::cout << "\n特征2：在(30,30)位置切除圆柱1" << std::endl;
    gp_Ax2 cyl1Axis(gp_Pnt(30, 30, 0), gp_Dir(0, 0, 1));
    TopoDS_Shape cyl1 = BRepPrimAPI_MakeCylinder(cyl1Axis, 10.0, 120.0).Shape();
    TopoDS_Shape feature2 = BRepAlgoAPI_Cut(feature1, cyl1).Shape();
    naming.GenerateNames(feature2, "F2_");
    std::cout << "更新命名..." << std::endl;
    naming.PrintStatistics();
    
    // 特征3：切除圆柱2
    std::cout << "\n特征3：在(70,70)位置切除圆柱2" << std::endl;
    gp_Ax2 cyl2Axis(gp_Pnt(70, 70, 0), gp_Dir(0, 0, 1));
    TopoDS_Shape cyl2 = BRepPrimAPI_MakeCylinder(cyl2Axis, 10.0, 120.0).Shape();
    TopoDS_Shape feature3 = BRepAlgoAPI_Cut(feature2, cyl2).Shape();
    naming.GenerateNames(feature3, "F3_");
    std::cout << "更新命名..." << std::endl;
    naming.PrintStatistics();
    
    // 特征4：融合球体
    std::cout << "\n特征4：在角落融合球体" << std::endl;
    TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(gp_Pnt(0, 0, 0), 30.0).Shape();
    TopoDS_Shape feature4 = BRepAlgoAPI_Fuse(feature3, sphere).Shape();
    naming.GenerateNames(feature4, "F4_");
    std::cout << "更新命名..." << std::endl;
    naming.PrintStatistics();
    
    std::cout << "\n最终形状的所有命名：" << std::endl;
    naming.PrintAllNames();
    
    std::cout << "\n说明：" << std::endl;
    std::cout << "FreeCAD使用类似的机制在特征树中追踪拓扑元素。" << std::endl;
    std::cout << "每个特征操作后，系统会重新计算并更新拓扑命名。" << std::endl;
    std::cout << "通过几何哈希，即使形状复杂度增加，相似的几何元素仍可被追踪。" << std::endl;
    
    std::cout << "\n############################################\n" << std::endl;
}

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   FreeCAD风格拓扑命名机制演示" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::cout << "FreeCAD使用基于几何哈希的稳定命名机制来解决拓扑命名问题。" << std::endl;
    std::cout << "本演示展示了类似的实现方法。\n" << std::endl;
    
    std::cout << "核心思想：" << std::endl;
    std::cout << "1. 不使用简单的索引（Face1, Face2...）" << std::endl;
    std::cout << "2. 而是基于几何特征（面积、中心点、法向等）计算哈希" << std::endl;
    std::cout << "3. 相同几何→相同哈希→相同名称（稳定性）" << std::endl;
    std::cout << "4. 几何改变→哈希改变→名称更新（追踪性）\n" << std::endl;
    
    // 演示1：基本命名机制
    DemonstrateFreeCADNaming();
    
    // 演示2：布尔运算中的稳定性
    DemonstrateNamingStabilityWithBooleans();
    
    // 演示3：特征历史
    DemonstrateNamingInFeatureHistory();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "演示完成！" << std::endl;
    std::cout << "\n参考资料：" << std::endl;
    std::cout << "- FreeCAD Topological Naming Project" << std::endl;
    std::cout << "- https://github.com/realthunder/FreeCAD_assembly3" << std::endl;
    std::cout << "- FreeCAD Wiki: Topological naming problem" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
