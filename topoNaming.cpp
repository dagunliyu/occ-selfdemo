
// Copyright © https://github.com/cfd-dev/OCCT-demo

// OCC拓扑命名问题演示
// 本示例展示了OpenCASCADE中拓扑命名的概念和常见问题

#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
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
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pnt.hxx>
#include <gp_Ax2.hxx>
#include <Precision.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <WNT_Window.hxx>
#include <V3d_View.hxx>
#include <windows.h>
#include <Quantity_Color.hxx>
#include <AIS_ColoredShape.hxx>

// 打印形状的拓扑信息
void PrintTopologyInfo(const TopoDS_Shape& shape, const std::string& shapeName)
{
    std::cout << "\n========== " << shapeName << " 拓扑信息 ==========" << std::endl;
    
    // 统计各类拓扑元素数量
    TopTools_IndexedMapOfShape vertices, edges, faces, solids;
    TopExp::MapShapes(shape, TopAbs_VERTEX, vertices);
    TopExp::MapShapes(shape, TopAbs_EDGE, edges);
    TopExp::MapShapes(shape, TopAbs_FACE, faces);
    TopExp::MapShapes(shape, TopAbs_SOLID, solids);
    
    std::cout << "实体数量: " << solids.Extent() << std::endl;
    std::cout << "面数量: " << faces.Extent() << std::endl;
    std::cout << "边数量: " << edges.Extent() << std::endl;
    std::cout << "顶点数量: " << vertices.Extent() << std::endl;
    
    // 打印前几个顶点的坐标
    std::cout << "\n前5个顶点坐标:" << std::endl;
    int vertexCount = std::min(5, vertices.Extent());
    for (int i = 1; i <= vertexCount; ++i)
    {
        const TopoDS_Vertex& vertex = TopoDS::Vertex(vertices(i));
        gp_Pnt pt = BRep_Tool::Pnt(vertex);
        std::cout << "  顶点" << i << ": (" 
                  << pt.X() << ", " << pt.Y() << ", " << pt.Z() << ")" << std::endl;
    }
    
    std::cout << "======================================\n" << std::endl;
}

// 根据位置查找特定面（示例：查找Z=某值的面）
TopoDS_Face FindFaceByPosition(const TopoDS_Shape& shape, double targetZ, double tolerance = 1e-6)
{
    TopExp_Explorer faceExp(shape, TopAbs_FACE);
    while (faceExp.More())
    {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        
        // 检查该面的所有顶点是否都在目标Z高度附近
        TopExp_Explorer vertexExp(face, TopAbs_VERTEX);
        bool allVerticesMatch = true;
        int vertexCount = 0;
        
        while (vertexExp.More())
        {
            TopoDS_Vertex vertex = TopoDS::Vertex(vertexExp.Current());
            gp_Pnt pt = BRep_Tool::Pnt(vertex);
            if (std::abs(pt.Z() - targetZ) > tolerance)
            {
                allVerticesMatch = false;
                break;
            }
            vertexCount++;
            vertexExp.Next();
        }
        
        if (allVerticesMatch && vertexCount > 0)
        {
            return face;
        }
        
        faceExp.Next();
    }
    
    return TopoDS_Face(); // 未找到
}

// 演示拓扑命名问题：修改形状后，原有的索引可能失效
void DemonstrateTopologicalNamingProblem()
{
    std::cout << "\n############## 拓扑命名问题演示 ##############\n" << std::endl;
    
    // 场景1：创建初始形状 - 一个简单的盒子
    std::cout << "场景1：创建基础盒子 (100x100x100)" << std::endl;
    TopoDS_Shape box1 = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape();
    PrintTopologyInfo(box1, "盒子1");
    
    // 记录第一个面的索引
    TopTools_IndexedMapOfShape faces1;
    TopExp::MapShapes(box1, TopAbs_FACE, faces1);
    std::cout << "盒子1的第1个面索引: 1, 总面数: " << faces1.Extent() << std::endl;
    
    // 场景2：创建稍微不同的盒子（不同尺寸）
    std::cout << "\n场景2：创建不同尺寸的盒子 (150x100x100)" << std::endl;
    TopoDS_Shape box2 = BRepPrimAPI_MakeBox(150.0, 100.0, 100.0).Shape();
    PrintTopologyInfo(box2, "盒子2");
    
    TopTools_IndexedMapOfShape faces2;
    TopExp::MapShapes(box2, TopAbs_FACE, faces2);
    std::cout << "盒子2的第1个面索引: 1, 总面数: " << faces2.Extent() << std::endl;
    
    // 问题：虽然两个盒子的"第1个面"索引相同，但可能对应不同的几何面！
    std::cout << "\n*** 拓扑命名问题 ***" << std::endl;
    std::cout << "虽然两个盒子的第1个面索引都是1，但它们可能对应不同的几何位置！" << std::endl;
    std::cout << "这在参数化建模中会导致严重问题。" << std::endl;
    
    // 场景3：通过布尔运算修改形状
    std::cout << "\n场景3：对盒子进行布尔运算（切除圆柱）" << std::endl;
    gp_Ax2 cylinderAxis(gp_Pnt(50, 50, 0), gp_Dir(0, 0, 1));
    TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(cylinderAxis, 20.0, 120.0).Shape();
    TopoDS_Shape cutResult = BRepAlgoAPI_Cut(box1, cylinder).Shape();
    PrintTopologyInfo(cutResult, "切除后的形状");
    
    TopTools_IndexedMapOfShape facesCut;
    TopExp::MapShapes(cutResult, TopAbs_FACE, facesCut);
    std::cout << "布尔运算后，面的数量变化: " << faces1.Extent() 
              << " -> " << facesCut.Extent() << std::endl;
    std::cout << "原来的面索引在新形状中可能失效或指向不同的面！" << std::endl;
    
    std::cout << "\n############################################\n" << std::endl;
}

// 演示使用几何特征来稳定识别拓扑元素
void DemonstrateStableNaming()
{
    std::cout << "\n############## 稳定命名方法演示 ##############\n" << std::endl;
    
    // 创建一个盒子
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape();
    
    std::cout << "使用几何特征（如位置）来识别面，而不是依赖索引：\n" << std::endl;
    
    // 查找顶面（Z=100的面）
    TopoDS_Face topFace = FindFaceByPosition(box, 100.0);
    if (!topFace.IsNull())
    {
        std::cout << "成功找到顶面 (Z=100)" << std::endl;
        
        // 获取该面的中心点作为验证
        TopExp_Explorer vertexExp(topFace, TopAbs_VERTEX);
        gp_Pnt center(0, 0, 0);
        int count = 0;
        while (vertexExp.More())
        {
            gp_Pnt pt = BRep_Tool::Pnt(TopoDS::Vertex(vertexExp.Current()));
            center.SetX(center.X() + pt.X());
            center.SetY(center.Y() + pt.Y());
            center.SetZ(center.Z() + pt.Z());
            count++;
            vertexExp.Next();
        }
        if (count > 0)
        {
            center.SetX(center.X() / count);
            center.SetY(center.Y() / count);
            center.SetZ(center.Z() / count);
            std::cout << "顶面中心点: (" 
                      << center.X() << ", " << center.Y() << ", " << center.Z() << ")" << std::endl;
        }
    }
    else
    {
        std::cout << "未找到顶面" << std::endl;
    }
    
    // 查找底面（Z=0的面）
    TopoDS_Face bottomFace = FindFaceByPosition(box, 0.0);
    if (!bottomFace.IsNull())
    {
        std::cout << "成功找到底面 (Z=0)" << std::endl;
    }
    
    std::cout << "\n这种基于几何特征的识别方法比单纯使用索引更可靠！" << std::endl;
    std::cout << "############################################\n" << std::endl;
}

// 演示拓扑关联关系
void DemonstrateTopologicalRelations()
{
    std::cout << "\n############## 拓扑关联关系演示 ##############\n" << std::endl;
    
    // 创建一个简单盒子
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape();
    
    std::cout << "面与边的关联关系：" << std::endl;
    TopTools_IndexedMapOfShape faces;
    TopExp::MapShapes(box, TopAbs_FACE, faces);
    
    for (int i = 1; i <= std::min(3, faces.Extent()); ++i)
    {
        const TopoDS_Face& face = TopoDS::Face(faces(i));
        std::cout << "\n面 " << i << " 的边：" << std::endl;
        
        TopExp_Explorer edgeExp(face, TopAbs_EDGE);
        int edgeCount = 0;
        while (edgeExp.More())
        {
            edgeCount++;
            edgeExp.Next();
        }
        std::cout << "  共有 " << edgeCount << " 条边" << std::endl;
    }
    
    // 构建边->顶点映射
    TopTools_IndexedMapOfShape edges;
    TopExp::MapShapes(box, TopAbs_EDGE, edges);
    
    std::cout << "\n前3条边的顶点：" << std::endl;
    for (int i = 1; i <= std::min(3, edges.Extent()); ++i)
    {
        const TopoDS_Edge& edge = TopoDS::Edge(edges(i));
        TopoDS_Vertex v1, v2;
        TopExp::Vertices(edge, v1, v2);
        
        gp_Pnt p1 = BRep_Tool::Pnt(v1);
        gp_Pnt p2 = BRep_Tool::Pnt(v2);
        
        std::cout << "  边 " << i << ": (" 
                  << p1.X() << "," << p1.Y() << "," << p1.Z() << ") -> ("
                  << p2.X() << "," << p2.Y() << "," << p2.Z() << ")" << std::endl;
    }
    
    std::cout << "\n############################################\n" << std::endl;
}

// 可视化演示（如果有窗口支持）
void VisualizeShape(const TopoDS_Shape& shape)
{
    try
    {
        // 初始化可视化系统
        Handle(OpenGl_GraphicDriver) graphicDriver = new OpenGl_GraphicDriver(NULL);
        Handle(V3d_Viewer) viewer = new V3d_Viewer(graphicDriver);
        Handle(AIS_InteractiveContext) context = new AIS_InteractiveContext(viewer);
        
        // 创建带颜色的形状显示
        Handle(AIS_ColoredShape) aisShape = new AIS_ColoredShape(shape);
        
        // 为不同的面设置不同颜色以区分
        TopTools_IndexedMapOfShape faces;
        TopExp::MapShapes(shape, TopAbs_FACE, faces);
        
        Quantity_Color colors[] = {
            Quantity_NOC_RED,
            Quantity_NOC_GREEN,
            Quantity_NOC_BLUE,
            Quantity_NOC_YELLOW,
            Quantity_NOC_CYAN,
            Quantity_NOC_MAGENTA
        };
        
        for (int i = 1; i <= faces.Extent(); ++i)
        {
            const TopoDS_Face& face = TopoDS::Face(faces(i));
            Quantity_Color color = colors[(i - 1) % 6];
            aisShape->SetCustomColor(face, color);
        }
        
        context->Display(aisShape, Standard_True);
        context->SetDisplayMode(aisShape, 1, Standard_True); // 着色模式
        
        // 创建3D视图
        HWND hwnd = GetConsoleWindow();
        if (hwnd != NULL)
        {
            Handle(WNT_Window) window = new WNT_Window(hwnd);
            Handle(V3d_View) view = viewer->CreateView();
            view->SetWindow(window);
            view->SetProj(V3d_XposYnegZpos);
            view->SetTwist(0.0);
            if (!window->IsMapped())
            {
                window->Map();
            }
            view->FitAll();
            view->Redraw();
            
            std::cout << "\n可视化窗口已创建。不同颜色代表不同的面。" << std::endl;
            std::cout << "按Enter键继续..." << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    catch (...)
    {
        std::cout << "可视化初始化失败，跳过可视化部分。" << std::endl;
    }
}

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   OpenCASCADE 拓扑命名问题演示" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::cout << "拓扑命名问题是参数化CAD建模中的一个关键挑战。" << std::endl;
    std::cout << "当模型被修改时，原有的拓扑元素（面、边、顶点）的索引可能会改变，" << std::endl;
    std::cout << "导致依赖这些索引的操作失败或产生错误结果。\n" << std::endl;
    
    // 演示1：拓扑命名问题
    DemonstrateTopologicalNamingProblem();
    
    // 演示2：稳定命名方法
    DemonstrateStableNaming();
    
    // 演示3：拓扑关联关系
    DemonstrateTopologicalRelations();
    
    // 创建一个复杂形状用于可视化
    std::cout << "\n############## 可视化演示 ##############\n" << std::endl;
    std::cout << "创建一个带孔的盒子用于可视化..." << std::endl;
    
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape();
    gp_Ax2 cylinderAxis(gp_Pnt(50, 50, 0), gp_Dir(0, 0, 1));
    TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(cylinderAxis, 20.0, 120.0).Shape();
    TopoDS_Shape finalShape = BRepAlgoAPI_Cut(box, cylinder).Shape();
    
    PrintTopologyInfo(finalShape, "最终形状（带孔盒子）");
    
    VisualizeShape(finalShape);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "演示完成！" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
