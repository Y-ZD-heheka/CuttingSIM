/**
 * @file BooleanOperator.h
 * @brief 布尔运算操作器
 * 
 * 封装 MeshLib 的布尔运算功能
 */

#pragma once

#include <MRMesh/MRMesh.h>
#include <MRMesh/MRMeshBoolean.h>
#include <optional>
#include <string>

/**
 * @brief 布尔运算类型
 */
enum class BooleanType
{
    Union,          ///< 并集 (A + B)
    Intersection,   ///< 交集 (A ∩ B)
    Difference,     ///< 差集 (A - B)
};

/**
 * @brief 布尔运算结果
 */
struct BooleanResult
{
    MR::Mesh mesh;           ///< 结果网格
    bool success = false;    ///< 是否成功
    std::string errorMsg;    ///< 错误信息（如果失败）
    float durationMs = 0.0f; ///< 运算耗时（毫秒）
};

/**
 * @brief 布尔运算操作器类
 */
class BooleanOperator
{
public:
    BooleanOperator();
    ~BooleanOperator() = default;
    
    /**
     * @brief 执行布尔运算
     * @param meshA 第一个网格（被操作对象）
     * @param meshB 第二个网格（操作对象，如切割工具）
     * @param type 布尔运算类型
     * @return 运算结果
     */
    BooleanResult execute(const MR::Mesh& meshA, 
                          const MR::Mesh& meshB, 
                          BooleanType type);
    
    /**
     * @brief 执行布尔差集运算 (A - B)
     * @param meshA 被切割网格
     * @param meshB 切割工具网格
     * @return 运算结果
     */
    BooleanResult difference(const MR::Mesh& meshA, const MR::Mesh& meshB);
    
    /**
     * @brief 获取切割碎片 (刀具内部的模型部分)
     * @param meshA 被切割网格
     * @param meshB 切割工具网格
     * @return 被切掉的部分网格
     */
    BooleanResult getCutPiece(const MR::Mesh& meshA, const MR::Mesh& meshB);
    
    /**
     * @brief 将布尔类型转换为字符串
     */
    static std::string typeToString(BooleanType type);
    
private:
    /**
     * @brief 将 BooleanType 转换为 MR::BooleanOperation
     */
    MR::BooleanOperation convertType(BooleanType type) const;
};
