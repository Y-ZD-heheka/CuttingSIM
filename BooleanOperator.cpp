/**
 * @file BooleanOperator.cpp
 * @brief 布尔运算操作器实现
 */

#include "BooleanOperator.h"
#include <chrono>
#include <iomanip>

BooleanOperator::BooleanOperator()
{
}

MR::BooleanOperation BooleanOperator::convertType(BooleanType type) const
{
    switch (type)
    {
        case BooleanType::Union:
            return MR::BooleanOperation::Union;
        case BooleanType::Intersection:
            return MR::BooleanOperation::Intersection;
        case BooleanType::Difference:
            return MR::BooleanOperation::DifferenceAB;
        default:
            return MR::BooleanOperation::Union;
    }
}

std::string BooleanOperator::typeToString(BooleanType type)
{
    switch (type)
    {
        case BooleanType::Union:
            return "Union (并集)";
        case BooleanType::Intersection:
            return "Intersection (交集)";
        case BooleanType::Difference:
            return "Difference (差集 A-B)";
        default:
            return "Unknown";
    }
}

BooleanResult BooleanOperator::execute(const MR::Mesh& meshA, 
                                        const MR::Mesh& meshB, 
                                        BooleanType type)
{
    BooleanResult result;
    
    // 检查输入网格是否有效
    if (meshA.points.empty())
    {
        result.errorMsg = "Mesh A is empty";
        return result;
    }
    
    if (meshB.points.empty())
    {
        result.errorMsg = "Mesh B is empty";
        return result;
    }
    
    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();
    
    // 执行布尔运算
    MR::BooleanResult mrResult = MR::boolean(meshA, meshB, convertType(type));
    
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    result.durationMs = static_cast<float>(elapsed.count());
    
    // 检查运算结果
    if (!mrResult.valid())
    {
        result.errorMsg = mrResult.errorString;
        return result;
    }
    
    // 提取结果网格
    result.mesh = *mrResult;
    result.success = true;
    
    return result;
}

BooleanResult BooleanOperator::difference(const MR::Mesh& meshA, const MR::Mesh& meshB)
{
    return execute(meshA, meshB, BooleanType::Difference);
}
