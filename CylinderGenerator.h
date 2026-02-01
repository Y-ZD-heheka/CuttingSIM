/**
 * @file CylinderGenerator.h
 * @brief 圆柱体网格生成器
 * 
 * 生成长50mm，直径6mm的圆柱体网格用于布尔切割
 */

#pragma once

#include <MRMesh/MRMesh.h>
#include <MRMesh/MRVector3.h>
#include <memory>

/**
 * @brief 圆柱体参数结构
 */
struct CylinderParams
{
    float length = 50.0f;   ///< 圆柱体长度 (mm)
    float diameter = 6.0f;  ///< 圆柱体直径 (mm)
    int segments = 32;      ///< 圆周分段数
    
    float getRadius() const { return diameter / 2.0f; }
};

/**
 * @brief 圆柱体网格生成器类
 */
class CylinderGenerator
{
public:
    CylinderGenerator();
    ~CylinderGenerator() = default;
    
    /**
     * @brief 设置圆柱体参数
     */
    void setParams(const CylinderParams& params);
    
    /**
     * @brief 获取当前参数
     */
    const CylinderParams& getParams() const { return params_; }
    
    /**
     * @brief 生成圆柱体网格
     * @return 生成的网格，失败返回空mesh
     */
    MR::Mesh generate() const;
    
    /**
     * @brief 生成圆柱体网格并应用变换
     * @param position 圆柱体中心位置
     * @param direction 圆柱体轴向方向 (默认Z轴)
     * @return 生成的网格
     */
    MR::Mesh generateAt(const MR::Vector3f& position, 
                        const MR::Vector3f& direction = MR::Vector3f(0, 0, 1)) const;
    
private:
    CylinderParams params_;
};
