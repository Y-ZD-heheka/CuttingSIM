/**
 * @file CutterVisualizer.h
 * @brief 3D 切割可视化器
 * 
 * 使用 MRViewer 进行网格可视化
 */

#pragma once

#include <QWidget>
#include <memory>
#include <MRMesh/MRMesh.h>

// 前置声明 MeshLib 类
namespace MR
{
    class Viewer;
    class RenderMeshObject;
}

/**
 * @brief 可视化模式
 */
enum class VisualMode
{
    Original,       ///< 显示原始模型
    Cutter,         ///< 显示切割工具
    Result,         ///< 显示切割结果
    All             ///< 显示所有
};

/**
 * @brief 3D 切割可视化器类
 * 
 * 用于显示目标网格、切割圆柱体和切割结果
 */
class CutterVisualizer : public QWidget
{
    Q_OBJECT

public:
    explicit CutterVisualizer(QWidget* parent = nullptr);
    ~CutterVisualizer();

    /**
     * @brief 设置目标网格（从文件加载的模型）
     */
    void setTargetMesh(const std::shared_ptr<MR::Mesh>& mesh);
    
    /**
     * @brief 设置切割工具网格（圆柱体）
     */
    void setCutterMesh(const std::shared_ptr<MR::Mesh>& mesh);
    
    /**
     * @brief 设置切割结果网格
     */
    void setResultMesh(const std::shared_ptr<MR::Mesh>& mesh);
    
    /**
     * @brief 清除所有网格
     */
    void clearAll();
    
    /**
     * @brief 设置可视化模式
     */
    void setVisualMode(VisualMode mode);
    
    /**
     * @brief 更新显示
     */
    void updateView();
    
    /**
     * @brief 保存结果到文件
     * @param filename 文件路径
     * @return 是否成功
     */
    bool saveResult(const QString& filename);
    
    /**
     * @brief 获取结果网格
     */
    std::shared_ptr<MR::Mesh> getResultMesh() const { return resultMesh_; }

signals:
    /**
     * @brief 视图更新信号
     */
    void viewUpdated();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void renderMesh(QPainter& painter, const MR::Mesh& mesh, 
                    const QColor& color, float opacity = 1.0f);
    void drawAxes(QPainter& painter);
    void projectVertex(const MR::Vector3f& vertex, QPoint& point);
    
    // 网格数据
    std::shared_ptr<MR::Mesh> targetMesh_;
    std::shared_ptr<MR::Mesh> cutterMesh_;
    std::shared_ptr<MR::Mesh> resultMesh_;
    
    // 显示模式
    VisualMode visualMode_ = VisualMode::All;
    
    // 视图参数
    float scale_ = 1.0f;
    QPoint offset_;
    QPoint lastMousePos_;
    bool isDragging_ = false;
    
    // 旋转角度
    float rotX_ = 0.0f;
    float rotY_ = 0.0f;
};
