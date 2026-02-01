/**
 * @file main.cpp
 * @brief Mesh Boolean Cutter - 网格布尔切割工具
 * 
 * 基于 MeshLib 和 Qt 的 3D 网格布尔运算可视化软件
 * 
 * 功能：
 * 1. 从文件加载网格模型（STL/OBJ 等格式）
 * 2. 生成圆柱体切割工具（长50mm，直径6mm）
 * 3. 使用 XYZ 按钮控制圆柱体位置
 * 4. 执行布尔差值运算并可视化结果
 */

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    // 创建 Qt 应用程序
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Mesh Boolean Cutter");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("MeshLibDemo");
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    // 运行应用程序事件循环
    return app.exec();
}
