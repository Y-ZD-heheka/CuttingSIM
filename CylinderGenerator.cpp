/**
 * @file CylinderGenerator.cpp
 * @brief 圆柱体网格生成器实现
 */

#include "CylinderGenerator.h"
#include <MRMesh/MRMeshBuilder.h>
#include <MRMesh/MRConstants.h>
#include <MRMesh/MRVector3.h>
#include <MRMesh/MRAffineXf3.h>
#include <MRMesh/MRMatrix3.h>
#include <cmath>
#include <algorithm>

CylinderGenerator::CylinderGenerator()
{
    // 默认参数：长50mm，直径6mm
    params_.length = 50.0f;
    params_.diameter = 6.0f;
    params_.segments = 64;
}

void CylinderGenerator::setParams(const CylinderParams& params)
{
    params_ = params;
}

MR::Mesh CylinderGenerator::generate() const
{
    const float radius = params_.getRadius();
    const float halfLength = params_.length / 2.0f;
    const int segments = params_.segments;
    
    if (segments < 3 || radius <= 0 || params_.length <= 0)
    {
        return MR::Mesh();
    }
    
    MR::VertCoords points;
    MR::Triangulation tris;
    
    // 顶点数量：顶部中心 + 顶部圆周 + 底部中心 + 底部圆周 + 侧面
    // 创建两个端面的顶点和圆周顶点
    const int vertCount = 2 + segments * 2;  // 顶部中心 + 底部中心 + 上下圆周
    points.reserve(vertCount);
    
    // 添加顶部中心点 (Z+)
    const MR::VertId topCenterId(0);
    points.emplace_back(0.0f, 0.0f, halfLength);
    
    // 添加底部中心点 (Z-)
    const MR::VertId bottomCenterId(1);
    points.emplace_back(0.0f, 0.0f, -halfLength);
    
    // 添加顶部圆周顶点
    std::vector<MR::VertId> topRingIds;
    topRingIds.reserve(segments);
    for (int i = 0; i < segments; ++i)
    {
        const float angle = 2.0f * MR::PI_F * i / segments;
        const float x = radius * std::cos(angle);
        const float y = radius * std::sin(angle);
        topRingIds.push_back(MR::VertId(points.size()));
        points.emplace_back(x, y, halfLength);
    }
    
    // 添加底部圆周顶点
    std::vector<MR::VertId> bottomRingIds;
    bottomRingIds.reserve(segments);
    for (int i = 0; i < segments; ++i)
    {
        const float angle = 2.0f * MR::PI_F * i / segments;
        const float x = radius * std::cos(angle);
        const float y = radius * std::sin(angle);
        bottomRingIds.push_back(MR::VertId(points.size()));
        points.emplace_back(x, y, -halfLength);
    }
    
    // 创建三角形面
    // 顶部端面 (Z+)
    for (int i = 0; i < segments; ++i)
    {
        int next = (i + 1) % segments;
        tris.push_back({topCenterId, topRingIds[next], topRingIds[i]});
    }
    
    // 底部端面 (Z-)
    for (int i = 0; i < segments; ++i)
    {
        int next = (i + 1) % segments;
        tris.push_back({bottomCenterId, bottomRingIds[i], bottomRingIds[next]});
    }
    
    // 侧面
    for (int i = 0; i < segments; ++i)
    {
        int next = (i + 1) % segments;
        // 侧面由两个三角形组成一个四边形
        tris.push_back({topRingIds[i], topRingIds[next], bottomRingIds[i]});
        tris.push_back({topRingIds[next], bottomRingIds[next], bottomRingIds[i]});
    }
    
    // 构建网格
    MR::Mesh mesh;
    mesh.topology = MR::MeshBuilder::fromTriangles(tris);
    mesh.points = std::move(points);
    
    return mesh;
}

MR::Mesh CylinderGenerator::generateAt(const MR::Vector3f& position, 
                                        const MR::Vector3f& direction) const
{
    MR::Mesh mesh = generate();
    
    if (mesh.points.empty())
    {
        return mesh;
    }
    
    // 计算旋转矩阵，使圆柱体从Z轴对齐到目标方向
    MR::Vector3f defaultDir(0, 0, 1);
    MR::Vector3f targetDir = direction.normalized();
    
    MR::AffineXf3f transform;
    
    // 如果方向相同或相反，只需平移
    if (std::abs(MR::dot(defaultDir, targetDir)) > 0.9999f)
    {
        transform = MR::AffineXf3f::translation(position);
    }
    else
    {
        // 计算旋转轴和角度
        MR::Vector3f rotationAxis = MR::cross(defaultDir, targetDir).normalized();
        float cosAngle = MR::dot(defaultDir, targetDir);
        float angle = std::acos(std::clamp(cosAngle, -1.0f, 1.0f));
        
        // 创建旋转矩阵
        MR::Matrix3f rotationMatrix = MR::Matrix3f::rotation(rotationAxis, angle);
        
        // 组合变换：先旋转，再平移
        transform = MR::AffineXf3f(rotationMatrix, position);
    }
    
    mesh.transform(transform);
    return mesh;
}
