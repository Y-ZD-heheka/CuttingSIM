/**
 * @file CutterVisualizer.cpp
 * @brief 3D 切割可视化器实现
 */

#include "CutterVisualizer.h"
#include <MRMesh/MRMeshSave.h>
#include <MRMesh/MRBox.h>
#include <QPainter>
#include <QMouseEvent>
#include <cmath>
#include <vector>
#include <algorithm>

// 简单的3D向量操作
struct Vec3 {
    float x, y, z;
    Vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
    Vec3(const MR::Vector3f& v) : x(v.x), y(v.y), z(v.z) {}
    
    Vec3 operator-(const Vec3& o) const { return Vec3(x-o.x, y-o.y, z-o.z); }
    Vec3 operator+(const Vec3& o) const { return Vec3(x+o.x, y+o.y, z+o.z); }
    Vec3 operator*(float s) const { return Vec3(x*s, y*s, z*s); }
    
    Vec3 cross(const Vec3& o) const {
        return Vec3(y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x);
    }
    float dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalized() const {
        float len = length();
        if (len > 0) return *this * (1.0f/len);
        return *this;
    }
};

CutterVisualizer::CutterVisualizer(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    
    offset_ = QPoint(width()/2, height()/2);
}

CutterVisualizer::~CutterVisualizer() = default;

void CutterVisualizer::setTargetMesh(const std::shared_ptr<MR::Mesh>& mesh)
{
    targetMesh_ = mesh;
    update();
}

void CutterVisualizer::setCutterMesh(const std::shared_ptr<MR::Mesh>& mesh)
{
    cutterMesh_ = mesh;
    update();
}

void CutterVisualizer::setResultMesh(const std::shared_ptr<MR::Mesh>& mesh)
{
    resultMesh_ = mesh;
    update();
}

void CutterVisualizer::clearAll()
{
    targetMesh_.reset();
    cutterMesh_.reset();
    resultMesh_.reset();
    update();
}

void CutterVisualizer::setVisualMode(VisualMode mode)
{
    visualMode_ = mode;
    update();
}

void CutterVisualizer::updateView()
{
    update();
}

bool CutterVisualizer::saveResult(const QString& filename)
{
    if (!resultMesh_ || resultMesh_->points.empty())
    {
        return false;
    }
    
    auto result = MR::MeshSave::toAnySupportedFormat(*resultMesh_, filename.toStdString());
    return result.has_value();
}

void CutterVisualizer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 计算所有网格的包围盒，用于自动缩放
    float maxBound = 50.0f;
    bool hasMesh = false;
    
    auto checkMesh = [&](const std::shared_ptr<MR::Mesh>& mesh) {
        if (mesh && !mesh->points.empty()) {
            auto bbox = mesh->computeBoundingBox();
            maxBound = std::max(maxBound, std::max({bbox.max.x - bbox.min.x,
                                                     bbox.max.y - bbox.min.y,
                                                     bbox.max.z - bbox.min.z}));
            hasMesh = true;
        }
    };
    
    if (visualMode_ == VisualMode::Original || visualMode_ == VisualMode::All)
        checkMesh(targetMesh_);
    if (visualMode_ == VisualMode::Cutter || visualMode_ == VisualMode::All)
        checkMesh(cutterMesh_);
    if (visualMode_ == VisualMode::Result || visualMode_ == VisualMode::All)
        checkMesh(resultMesh_);
    
    if (!hasMesh) {
        painter.drawText(rect(), Qt::AlignCenter, "No mesh loaded");
        return;
    }
    
    // 自动缩放
    float baseScale = std::min(width(), height()) / (maxBound * 1.5f);
    float finalScale = baseScale * scale_;
    
    // 绘制坐标轴
    drawAxes(painter);
    
    // 渲染各个网格
    if (visualMode_ == VisualMode::All || visualMode_ == VisualMode::Original) {
        if (targetMesh_ && !targetMesh_->points.empty()) {
            renderMesh(painter, *targetMesh_, QColor(100, 150, 255), 0.7f);
        }
    }
    
    if (visualMode_ == VisualMode::All || visualMode_ == VisualMode::Cutter) {
        if (cutterMesh_ && !cutterMesh_->points.empty()) {
            renderMesh(painter, *cutterMesh_, QColor(255, 100, 100), 0.5f);
        }
    }
    
    if (visualMode_ == VisualMode::All || visualMode_ == VisualMode::Result) {
        if (resultMesh_ && !resultMesh_->points.empty()) {
            renderMesh(painter, *resultMesh_, QColor(100, 255, 150), 1.0f);
        }
    }
}

void CutterVisualizer::renderMesh(QPainter& painter, const MR::Mesh& mesh, 
                                   const QColor& color, float opacity)
{
    // 简单的正交投影渲染
    float baseScale = std::min(width(), height()) / 150.0f * scale_;
    QPoint center(width()/2 + offset_.x(), height()/2 + offset_.y());
    
    // 计算旋转矩阵
    float cx = std::cos(rotX_);
    float sx = std::sin(rotX_);
    float cy = std::cos(rotY_);
    float sy = std::sin(rotY_);
    
    auto rotate = [&](const Vec3& v) -> Vec3 {
        // 绕X轴旋转
        float y1 = v.y * cx - v.z * sx;
        float z1 = v.y * sx + v.z * cx;
        // 绕Y轴旋转
        float x2 = v.x * cy + z1 * sy;
        float z2 = -v.x * sy + z1 * cy;
        return Vec3(x2, y1, z2);
    };
    
    auto project = [&](const Vec3& v) -> QPoint {
        Vec3 r = rotate(v);
        return QPoint(center.x() + r.x * baseScale, 
                      center.y() - r.y * baseScale);  // Y轴翻转
    };
    
    // 简单的面片渲染
    QPen pen(color);
    pen.setWidth(1);
    QColor fillColor = color;
    fillColor.setAlphaF(opacity * 0.3f);
    painter.setPen(pen);
    
    // 计算每个面的深度并排序（简单的画家算法）
    struct FaceDepth {
        MR::FaceId face;
        float depth;
        std::vector<QPoint> points;
    };
    std::vector<FaceDepth> faces;
    
    for (auto f : mesh.topology.getValidFaces()) {
        auto verts = mesh.topology.getTriVerts(f);
        
        Vec3 v0(mesh.points[verts[0]]);
        Vec3 v1(mesh.points[verts[1]]);
        Vec3 v2(mesh.points[verts[2]]);
        
        // 计算面中心深度
        Vec3 center = (v0 + v1 + v2) * (1.0f/3.0f);
        Vec3 rotated = rotate(center);
        
        FaceDepth fd;
        fd.face = f;
        fd.depth = rotated.z;
        fd.points = {project(v0), project(v1), project(v2)};
        faces.push_back(fd);
    }
    
    // 按深度排序（从远到近）
    std::sort(faces.begin(), faces.end(), 
              [](const FaceDepth& a, const FaceDepth& b) { return a.depth > b.depth; });
    
    // 绘制面
    for (const auto& fd : faces) {
        if (fd.points.size() >= 3) {
            QPolygon polygon;
            for (const auto& p : fd.points) {
                polygon << p;
            }
            painter.setBrush(fillColor);
            painter.drawPolygon(polygon);
        }
    }
    
    // 绘制边
    painter.setBrush(Qt::NoBrush);
    for (const auto& fd : faces) {
        if (fd.points.size() >= 3) {
            for (size_t i = 0; i < fd.points.size(); ++i) {
                QPoint p1 = fd.points[i];
                QPoint p2 = fd.points[(i+1) % fd.points.size()];
                painter.drawLine(p1, p2);
            }
        }
    }
}

void CutterVisualizer::drawAxes(QPainter& painter)
{
    float axisLength = 80.0f * scale_;
    QPoint origin(30, height() - 30);
    
    float cx = std::cos(rotX_);
    float sx = std::sin(rotX_);
    float cy = std::cos(rotY_);
    float sy = std::sin(rotY_);
    
    auto rotate = [&](const Vec3& v) -> Vec3 {
        float y1 = v.y * cx - v.z * sx;
        float z1 = v.y * sx + v.z * cx;
        float x2 = v.x * cy + z1 * sy;
        float y2 = y1;
        return Vec3(x2, y2, 0);
    };
    
    auto project = [&](const Vec3& v) -> QPoint {
        Vec3 r = rotate(v);
        return QPoint(origin.x() + r.x, origin.y() - r.y);
    };
    
    // X轴 - 红色
    QPen xPen(Qt::red);
    xPen.setWidth(2);
    painter.setPen(xPen);
    painter.drawLine(origin, project(Vec3(axisLength, 0, 0)));
    painter.drawText(project(Vec3(axisLength + 10, 0, 0)), "X");
    
    // Y轴 - 绿色
    QPen yPen(Qt::green);
    yPen.setWidth(2);
    painter.setPen(yPen);
    painter.drawLine(origin, project(Vec3(0, axisLength, 0)));
    painter.drawText(project(Vec3(0, axisLength + 10, 0)), "Y");
    
    // Z轴 - 蓝色
    QPen zPen(Qt::blue);
    zPen.setWidth(2);
    painter.setPen(zPen);
    QPoint zEnd = project(Vec3(0, 0, axisLength));
    // Z轴需要特殊处理以显示深度感
    painter.drawLine(origin, QPoint(origin.x(), origin.y() - axisLength * 0.7f));
    painter.drawText(QPoint(origin.x(), origin.y() - axisLength * 0.7f - 10), "Z");
}

void CutterVisualizer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    offset_ = QPoint(0, 0);
}

void CutterVisualizer::mousePressEvent(QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (event->buttons() & Qt::LeftButton) {
#else
    if (event->button() == Qt::LeftButton) {
#endif
        isDragging_ = true;
        lastMousePos_ = event->pos();
    }
}

void CutterVisualizer::mouseMoveEvent(QMouseEvent* event)
{
    if (isDragging_) {
        QPoint delta = event->pos() - lastMousePos_;
        rotY_ += delta.x() * 0.01f;
        rotX_ += delta.y() * 0.01f;
        lastMousePos_ = event->pos();
        update();
    }
}

void CutterVisualizer::mouseReleaseEvent(QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (!(event->buttons() & Qt::LeftButton)) {
#else
    if (event->button() == Qt::LeftButton) {
#endif
        isDragging_ = false;
    }
}

void CutterVisualizer::wheelEvent(QWheelEvent* event)
{
    float delta = event->angleDelta().y() / 120.0f;
    scale_ *= std::pow(1.1f, delta);
    scale_ = std::clamp(scale_, 0.1f, 10.0f);
    update();
}

void CutterVisualizer::projectVertex(const MR::Vector3f& vertex, QPoint& point)
{
    // 辅助函数，实际实现在 renderMesh 中
    Q_UNUSED(vertex)
    Q_UNUSED(point)
}
