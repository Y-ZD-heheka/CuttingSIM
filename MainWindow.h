/**
 * @file MainWindow.h
 * @brief 主窗口类
 * 
 * 包含所有 UI 控件：文件加载、圆柱体位置控制、布尔运算按钮等
 */

#pragma once

#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <memory>
#include "CutterVisualizer.h"
#include "CylinderGenerator.h"
#include "BooleanOperator.h"

// 前置声明
namespace MR {
    class Mesh;
}

/**
 * @brief 主窗口类
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    /**
     * @brief 加载目标网格文件
     */
    void onLoadTargetMesh();
    
    /**
     * @brief 保存结果网格
     */
    void onSaveResult();
    
    /**
     * @brief 执行布尔切割运算
     */
    void onExecuteCut();
    
    /**
     * @brief 重置圆柱体位置
     */
    void onResetCutter();
    
    /**
     * @brief 圆柱体位置改变
     */
    void onCutterPositionChanged();
    
    /**
     * @brief XYZ 移动按钮点击
     */
    void onMoveXPlus();
    void onMoveXMinus();
    void onMoveYPlus();
    void onMoveYMinus();
    void onMoveZPlus();
    void onMoveZMinus();
    
    /**
     * @brief 步长改变
     */
    void onStepSizeChanged(double value);
    
    /**
     * @brief 可视化模式改变
     */
    void onVisualModeChanged(int index);

private:
    void setupUI();
    void createMenus();
    void updateCutterMesh();
    void updateInfoLabel();
    
    // 中央可视化组件
    CutterVisualizer* visualizer_ = nullptr;
    
    // 圆柱体生成器
    CylinderGenerator cylinderGen_;
    
    // 布尔运算器
    BooleanOperator booleanOp_;
    
    // 网格数据
    std::shared_ptr<MR::Mesh> targetMesh_;
    std::shared_ptr<MR::Mesh> cutterMesh_;
    std::shared_ptr<MR::Mesh> resultMesh_;
    
    // 圆柱体位置
    MR::Vector3f cutterPosition_;
    
    // UI 控件
    QDoubleSpinBox* spinX_ = nullptr;
    QDoubleSpinBox* spinY_ = nullptr;
    QDoubleSpinBox* spinZ_ = nullptr;
    QDoubleSpinBox* spinStep_ = nullptr;
    
    QPushButton* btnLoad_ = nullptr;
    QPushButton* btnSave_ = nullptr;
    QPushButton* btnCut_ = nullptr;
    QPushButton* btnReset_ = nullptr;
    
    QPushButton* btnXPlus_ = nullptr;
    QPushButton* btnXMinus_ = nullptr;
    QPushButton* btnYPlus_ = nullptr;
    QPushButton* btnYMinus_ = nullptr;
    QPushButton* btnZPlus_ = nullptr;
    QPushButton* btnZMinus_ = nullptr;
    
    QComboBox* comboVisualMode_ = nullptr;
    QLabel* infoLabel_ = nullptr;
    
    // 当前加载的文件路径
    QString currentFilePath_;
    
    // 步长
    float stepSize_ = 1.0f;
};
