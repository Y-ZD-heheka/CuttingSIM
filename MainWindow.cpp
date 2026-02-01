/**
 * @file MainWindow.cpp
 * @brief 主窗口实现
 */

#include "MainWindow.h"
#include <MRMesh/MRMeshLoad.h>
#include <MRMesh/MRMeshSave.h>
#include <MRMesh/MRBox.h>
#include <MRMesh/MRMeshBuilder.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QFrame>
#include <QSplitter>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , cutterPosition_(0, 0, 0)
{
    setWindowTitle("Mesh Boolean Cutter - MeshLib + Qt");
    resize(1200, 800);
    
    // 创建圆柱体网格（初始位置）
    MR::Mesh cylinder = cylinderGen_.generate();
    cutterMesh_ = std::make_shared<MR::Mesh>(std::move(cylinder));
    
    setupUI();
    createMenus();
    
    // 创建初始场景（长方体）
    createInitialScene();
    
    // 初始化可视化器
    visualizer_->setCutterMesh(cutterMesh_);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    // 创建中央部件
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 主布局
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // 左侧面板（控制面板）
    QFrame* leftPanel = new QFrame();
    leftPanel->setFrameStyle(QFrame::StyledPanel);
    leftPanel->setMaximumWidth(350);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);
    
    // ===== 文件操作组 =====
    QGroupBox* fileGroup = new QGroupBox("File Operations (文件操作)");
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);
    
    QHBoxLayout* fileBtnLayout = new QHBoxLayout();
    btnLoad_ = new QPushButton("Load Mesh (加载模型)");
    btnSave_ = new QPushButton("Save Result (保存结果)");
    btnSavePiece_ = new QPushButton("Save Piece (保存碎片)");
    btnSave_->setEnabled(false);
    btnSavePiece_->setEnabled(false);
    fileBtnLayout->addWidget(btnLoad_);
    fileBtnLayout->addWidget(btnSave_);
    fileBtnLayout->addWidget(btnSavePiece_);
    fileLayout->addLayout(fileBtnLayout);
    
    infoLabel_ = new QLabel("No mesh loaded (未加载模型)");
    infoLabel_->setWordWrap(true);
    fileLayout->addWidget(infoLabel_);
    
    leftLayout->addWidget(fileGroup);
    
    // ===== 圆柱体参数组 =====
    QGroupBox* cutterGroup = new QGroupBox("Cutter Parameters (切割工具参数)");
    QGridLayout* cutterLayout = new QGridLayout(cutterGroup);
    
    // 显示圆柱体参数（只读）
    CylinderParams params = cylinderGen_.getParams();
    cutterLayout->addWidget(new QLabel("Length (长度):"), 0, 0);
    cutterLayout->addWidget(new QLabel(QString("%1 mm").arg(params.length)), 0, 1);
    cutterLayout->addWidget(new QLabel("Diameter (直径):"), 1, 0);
    cutterLayout->addWidget(new QLabel(QString("%1 mm").arg(params.diameter)), 1, 1);
    
    leftLayout->addWidget(cutterGroup);
    
    // ===== 位置控制组 =====
    QGroupBox* posGroup = new QGroupBox("Cutter Position (切割器位置)");
    QGridLayout* posLayout = new QGridLayout(posGroup);
    
    // X 控制
    posLayout->addWidget(new QLabel("X:"), 0, 0);
    spinX_ = new QDoubleSpinBox();
    spinX_->setRange(-500, 500);
    spinX_->setDecimals(2);
    spinX_->setSingleStep(0.1);
    spinX_->setValue(0);
    posLayout->addWidget(spinX_, 0, 1);
    
    btnXMinus_ = new QPushButton("-X");
    btnXPlus_ = new QPushButton("+X");
    posLayout->addWidget(btnXMinus_, 0, 2);
    posLayout->addWidget(btnXPlus_, 0, 3);
    
    // Y 控制
    posLayout->addWidget(new QLabel("Y:"), 1, 0);
    spinY_ = new QDoubleSpinBox();
    spinY_->setRange(-500, 500);
    spinY_->setDecimals(2);
    spinY_->setSingleStep(0.1);
    spinY_->setValue(0);
    posLayout->addWidget(spinY_, 1, 1);
    
    btnYMinus_ = new QPushButton("-Y");
    btnYPlus_ = new QPushButton("+Y");
    posLayout->addWidget(btnYMinus_, 1, 2);
    posLayout->addWidget(btnYPlus_, 1, 3);
    
    // Z 控制
    posLayout->addWidget(new QLabel("Z:"), 2, 0);
    spinZ_ = new QDoubleSpinBox();
    spinZ_->setRange(-500, 500);
    spinZ_->setDecimals(2);
    spinZ_->setSingleStep(0.1);
    spinZ_->setValue(0);
    posLayout->addWidget(spinZ_, 2, 1);
    
    btnZMinus_ = new QPushButton("-Z");
    btnZPlus_ = new QPushButton("+Z");
    posLayout->addWidget(btnZMinus_, 2, 2);
    posLayout->addWidget(btnZPlus_, 2, 3);
    
    // 步长控制
    posLayout->addWidget(new QLabel("Step (步长):"), 3, 0);
    spinStep_ = new QDoubleSpinBox();
    spinStep_->setRange(0.01, 100);
    spinStep_->setDecimals(2);
    spinStep_->setSingleStep(0.1);
    spinStep_->setValue(1.0);
    posLayout->addWidget(spinStep_, 3, 1);
    
    leftLayout->addWidget(posGroup);
    
    // ===== 操作按钮组 =====
    QGroupBox* actionGroup = new QGroupBox("Actions (操作)");
    QVBoxLayout* actionLayout = new QVBoxLayout(actionGroup);
    
    btnCut_ = new QPushButton("Execute Cut (执行切割)");
    btnCut_->setEnabled(false);
    btnCut_->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    actionLayout->addWidget(btnCut_);
    
    btnReset_ = new QPushButton("Reset Position (重置位置)");
    actionLayout->addWidget(btnReset_);
    
    leftLayout->addWidget(actionGroup);
    
    // ===== 可视化模式组 =====
    QGroupBox* viewGroup = new QGroupBox("Visualization (可视化)");
    QVBoxLayout* viewLayout = new QVBoxLayout(viewGroup);
    
    comboVisualMode_ = new QComboBox();
    comboVisualMode_->addItem("All (全部显示)", static_cast<int>(VisualMode::All));
    comboVisualMode_->addItem("Target Only (仅目标)", static_cast<int>(VisualMode::Original));
    comboVisualMode_->addItem("Cutter Only (仅切割器)", static_cast<int>(VisualMode::Cutter));
    comboVisualMode_->addItem("Result Only (仅结果)", static_cast<int>(VisualMode::Result));
    viewLayout->addWidget(comboVisualMode_);
    
    viewLayout->addWidget(new QLabel("Mouse: Left drag = rotate, Wheel = zoom"));
    
    leftLayout->addWidget(viewGroup);
    
    // 添加弹性空间
    leftLayout->addStretch();
    
    // ===== 3D 可视化器 =====
    visualizer_ = new CutterVisualizer();
    visualizer_->setMinimumSize(600, 500);
    
    // 添加到主布局
    mainLayout->addWidget(leftPanel, 0);
    mainLayout->addWidget(visualizer_, 1);
    
    // ===== 连接信号槽 =====
    connect(btnLoad_, &QPushButton::clicked, this, &MainWindow::onLoadTargetMesh);
    connect(btnSave_, &QPushButton::clicked, this, &MainWindow::onSaveResult);
    connect(btnSavePiece_, &QPushButton::clicked, this, &MainWindow::onSaveCutPiece);
    connect(btnCut_, &QPushButton::clicked, this, &MainWindow::onExecuteCut);
    connect(btnReset_, &QPushButton::clicked, this, &MainWindow::onResetCutter);
    
    connect(spinX_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onCutterPositionChanged);
    connect(spinY_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onCutterPositionChanged);
    connect(spinZ_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onCutterPositionChanged);
    
    connect(btnXPlus_, &QPushButton::clicked, this, &MainWindow::onMoveXPlus);
    connect(btnXMinus_, &QPushButton::clicked, this, &MainWindow::onMoveXMinus);
    connect(btnYPlus_, &QPushButton::clicked, this, &MainWindow::onMoveYPlus);
    connect(btnYMinus_, &QPushButton::clicked, this, &MainWindow::onMoveYMinus);
    connect(btnZPlus_, &QPushButton::clicked, this, &MainWindow::onMoveZPlus);
    connect(btnZMinus_, &QPushButton::clicked, this, &MainWindow::onMoveZMinus);
    
    connect(spinStep_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onStepSizeChanged);
    
    connect(comboVisualMode_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onVisualModeChanged);
}

void MainWindow::createMenus()
{
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File 菜单
    QMenu* fileMenu = menuBar->addMenu("File (文件)");
    
    QAction* loadAction = fileMenu->addAction("Load Mesh (加载模型)");
    loadAction->setShortcut(QKeySequence::Open);
    connect(loadAction, &QAction::triggered, this, &MainWindow::onLoadTargetMesh);
    
    QAction* saveAction = fileMenu->addAction("Save Result (保存结果)");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveResult);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("Exit (退出)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Help 菜单
    QMenu* helpMenu = menuBar->addMenu("Help (帮助)");
    
    QAction* aboutAction = helpMenu->addAction("About (关于)");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About", 
            "Mesh Boolean Cutter\n"
            "MeshLib + Qt Demo\n\n"
            "Features:\n"
            "- Load STL/OBJ mesh files\n"
            "- Cylinder cutter (50mm x 6mm)\n"
            "- XYZ position control\n"
            "- Boolean difference operation");
    });
}

void MainWindow::onLoadTargetMesh()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Load Mesh File (加载网格文件)",
        QString(),
        "Mesh Files (*.stl *.obj *.ply);;STL Files (*.stl);;OBJ Files (*.obj);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // 加载网格文件
    auto result = MR::MeshLoad::fromAnySupportedFormat(fileName.toStdString());
    
    if (!result.has_value()) {
        QMessageBox::critical(this, "Error (错误)", 
            QString("Failed to load mesh:\n%1").arg(QString::fromStdString(result.error())));
        return;
    }
    
    // 保存加载的网格
    targetMesh_ = std::make_shared<MR::Mesh>(std::move(result.value()));
    currentFilePath_ = fileName;
    
    // 获取并保存目标网格的包围盒
    targetBoundingBox_ = targetMesh_->computeBoundingBox();
    
    // 输出包围盒信息到控制台
    qDebug() << "=== Target Mesh Bounding Box ===";
    qDebug() << "Min Point: (" << targetBoundingBox_.min.x << ", " << targetBoundingBox_.min.y << ", " << targetBoundingBox_.min.z << ")";
    qDebug() << "Max Point: (" << targetBoundingBox_.max.x << ", " << targetBoundingBox_.max.y << ", " << targetBoundingBox_.max.z << ")";
    qDebug() << "Size: (" << (targetBoundingBox_.max.x - targetBoundingBox_.min.x) 
             << "x" << (targetBoundingBox_.max.y - targetBoundingBox_.min.y) 
             << "x" << (targetBoundingBox_.max.z - targetBoundingBox_.min.z) << ")";
    qDebug() << "Center: (" << (targetBoundingBox_.min.x + targetBoundingBox_.max.x) / 2.0f
             << "," << (targetBoundingBox_.min.y + targetBoundingBox_.max.y) / 2.0f
             << "," << (targetBoundingBox_.min.z + targetBoundingBox_.max.z) / 2.0f << ")";
    
    // 更新可视化器
    visualizer_->setTargetMesh(targetMesh_);
    
    // 更新信息显示
    updateInfoLabel();
    
    // 启用切割按钮
    btnCut_->setEnabled(true);
    btnSave_->setEnabled(false);  // 新加载模型后，需要重新切割才能保存
    
    // 清除之前的结果
    resultMesh_.reset();
    cutPieceMesh_.reset();
    visualizer_->setResultMesh(nullptr);
}

void MainWindow::onSaveResult()
{
    if (!resultMesh_ || resultMesh_->points.empty()) {
        QMessageBox::warning(this, "Warning (警告)", "No result to save (没有可保存的结果)");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Result (保存结果)",
        "result.stl",
        "STL Files (*.stl);;OBJ Files (*.obj);;PLY Files (*.ply)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    auto saveResult = MR::MeshSave::toAnySupportedFormat(*resultMesh_, fileName.toStdString());
    if (saveResult.has_value()) {
        QMessageBox::information(this, "Success (成功)", 
            QString("Result saved to:\n%1").arg(fileName));
    } else {
        QMessageBox::critical(this, "Error (错误)", "Failed to save file (保存失败)");
    }
}

void MainWindow::onSaveCutPiece()
{
    if (!cutPieceMesh_ || cutPieceMesh_->points.empty()) {
        QMessageBox::warning(this, "Warning (警告)", 
            "No cut piece to save (没有可保存的碎片)\n请先执行切割操作");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Cut Piece (保存切割碎片)",
        "cut_piece.stl",
        "STL Files (*.stl);;OBJ Files (*.obj);;PLY Files (*.ply)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    auto saveResult = MR::MeshSave::toAnySupportedFormat(*cutPieceMesh_, fileName.toStdString());
    if (saveResult.has_value()) {
        QMessageBox::information(this, "Success (成功)", 
            QString("Cut piece saved to:\n%1").arg(fileName));
    } else {
        QMessageBox::critical(this, "Error (错误)", "Failed to save file (保存失败)");
    }
}

void MainWindow::onExecuteCut()
{
    if (!targetMesh_ || !cutterMesh_) {
        QMessageBox::warning(this, "Warning (警告)", 
            "Please load a target mesh first (请先加载目标模型)");
        return;
    }
    
    // 执行布尔差集运算 (A - B) - 保留切割后的主体
    BooleanResult result = booleanOp_.difference(*targetMesh_, *cutterMesh_);
    // BooleanResult result = booleanOp_.difference(*cutterMesh_, *targetMesh_);
    
    if (!result.success) {
        QMessageBox::critical(this, "Error (错误)", 
            QString("Boolean operation failed:\n%1").arg(QString::fromStdString(result.errorMsg)));
        return;
    }
    
    // 保存结果网格
    resultMesh_ = std::make_shared<MR::Mesh>(std::move(result.mesh));
    targetMesh_ = resultMesh_;
    visualizer_->setResultMesh(resultMesh_);
    
    // 获取被切掉的碎片 (刀具内部的模型部分)
    BooleanResult pieceResult = booleanOp_.getCutPiece(*targetMesh_, *cutterMesh_);
    
    if (pieceResult.success && !pieceResult.mesh.points.empty()) {
        cutPieceMesh_ = std::make_shared<MR::Mesh>(std::move(pieceResult.mesh));
        btnSavePiece_->setEnabled(true);
        
        qDebug() << "=== Cut Piece Info ===";
        qDebug() << "Vertices:" << cutPieceMesh_->topology.numValidVerts();
        qDebug() << "Faces:" << cutPieceMesh_->topology.numValidFaces();
    }
    
    // 自动切换到结果显示模式
    comboVisualMode_->setCurrentIndex(3);  // Result Only
    
    // 启用保存按钮
    btnSave_->setEnabled(true);
    
    // 显示成功信息
    QString msg = QString("Boolean operation completed in %1 ms\n"
                          "Result: %2 vertices, %3 faces")
                         .arg(result.durationMs, 0, 'f', 2)
                         .arg(resultMesh_->topology.numValidVerts())
                         .arg(resultMesh_->topology.numValidFaces());
    
    QMessageBox::information(this, "Success (成功)", msg);
}

void MainWindow::onResetCutter()
{
    spinX_->setValue(0);
    spinY_->setValue(0);
    spinZ_->setValue(0);
    cutterPosition_ = MR::Vector3f(0, 0, 0);
    updateCutterMesh();
}

void MainWindow::onCutterPositionChanged()
{
    cutterPosition_.x = static_cast<float>(spinX_->value());
    cutterPosition_.y = static_cast<float>(spinY_->value());
    cutterPosition_.z = static_cast<float>(spinZ_->value());
    updateCutterMesh();
}

void MainWindow::updateCutterMesh()
{
    // 重新生成圆柱体在指定位置
    MR::Mesh cylinder = cylinderGen_.generateAt(cutterPosition_);
    cutterMesh_ = std::make_shared<MR::Mesh>(std::move(cylinder));
    visualizer_->setCutterMesh(cutterMesh_);
}

void MainWindow::updateInfoLabel()
{
    if (!targetMesh_) {
        infoLabel_->setText("No mesh loaded (未加载模型)");
        return;
    }
    
    QString text = QString("File: %1\n").arg(currentFilePath_);
    text += QString("Vertices: %1\n").arg(targetMesh_->topology.numValidVerts());
    text += QString("Faces: %1\n").arg(targetMesh_->topology.numValidFaces());
    
    auto bbox = targetMesh_->computeBoundingBox();
    text += QString("Size: %.2f x %.2f x %.2f mm")
                .arg(bbox.max.x - bbox.min.x)
                .arg(bbox.max.y - bbox.min.y)
                .arg(bbox.max.z - bbox.min.z);
    
    infoLabel_->setText(text);
}

void MainWindow::onMoveXPlus()
{
    spinX_->setValue(spinX_->value() + stepSize_);
}

void MainWindow::onMoveXMinus()
{
    spinX_->setValue(spinX_->value() - stepSize_);
}

void MainWindow::onMoveYPlus()
{
    spinY_->setValue(spinY_->value() + stepSize_);
}

void MainWindow::onMoveYMinus()
{
    spinY_->setValue(spinY_->value() - stepSize_);
}

void MainWindow::onMoveZPlus()
{
    spinZ_->setValue(spinZ_->value() + stepSize_);
}

void MainWindow::onMoveZMinus()
{
    spinZ_->setValue(spinZ_->value() - stepSize_);
}

void MainWindow::onStepSizeChanged(double value)
{
    stepSize_ = static_cast<float>(value);
}

void MainWindow::onVisualModeChanged(int index)
{
    Q_UNUSED(index)
    int modeValue = comboVisualMode_->currentData().toInt();
    visualizer_->setVisualMode(static_cast<VisualMode>(modeValue));
}

void MainWindow::createInitialScene()
{
    // 创建长方体：中心点(0, 0, 12.5)，尺寸10×10×25mm
    // 这样圆柱体下半部分(-25到0)与长方体(Z:0-25)相交
    MR::Vector3f boxCenter(0.0f, 0.0f, 15.0f);
    MR::Vector3f boxSize(20.0f, 20.0f, 25.0f);
    
    initialMesh_ = std::make_shared<MR::Mesh>(createBoxMesh(boxCenter, boxSize));
    
    // 同时设置 targetMesh_（用于信息显示和布尔运算）
    targetMesh_ = initialMesh_;
    
    // 将初始场景作为目标网格设置到可视化器
    visualizer_->setTargetMesh(initialMesh_);
    
    // 启用切割按钮
    btnCut_->setEnabled(true);
    
    // 输出调试信息
    qDebug() << "=== Initial Scene (Box) ===";
    qDebug() << "Center: (" << boxCenter.x << ", " << boxCenter.y << ", " << boxCenter.z << ")";
    qDebug() << "Size: " << boxSize.x << "x" << boxSize.y << "x" << boxSize.z;
    qDebug() << "Bounding Box: X[-5,5], Y[-5,5], Z[0,25]";
    
    // 更新信息标签
    updateInfoLabel();
}

MR::Mesh MainWindow::createBoxMesh(const MR::Vector3f& center, const MR::Vector3f& size)
{
    // 计算长方体的8个顶点
    float halfW = size.x / 2.0f;
    float halfH = size.y / 2.0f;
    float halfD = size.z / 2.0f;
    
    // 顶点坐标（相对于中心点）
    // 底面4个顶点 (Z最小)
    MR::VertId v0(0); // (-halfW, -halfH, -halfD) + center
    MR::VertId v1(1); // ( halfW, -halfH, -halfD) + center
    MR::VertId v2(2); // ( halfW,  halfH, -halfD) + center
    MR::VertId v3(3); // (-halfW,  halfH, -halfD) + center
    
    // 顶面4个顶点 (Z最大)
    MR::VertId v4(4); // (-halfW, -halfH,  halfD) + center
    MR::VertId v5(5); // ( halfW, -halfH,  halfD) + center
    MR::VertId v6(6); // ( halfW,  halfH,  halfD) + center
    MR::VertId v7(7); // (-halfW,  halfH,  halfD) + center
    
    MR::VertCoords points;
    points.reserve(8);
    
    // 添加8个顶点（带中心偏移）
    points.emplace_back(center.x - halfW, center.y - halfH, center.z - halfD); // v0
    points.emplace_back(center.x + halfW, center.y - halfH, center.z - halfD); // v1
    points.emplace_back(center.x + halfW, center.y + halfH, center.z - halfD); // v2
    points.emplace_back(center.x - halfW, center.y + halfH, center.z - halfD); // v3
    points.emplace_back(center.x - halfW, center.y - halfH, center.z + halfD); // v4
    points.emplace_back(center.x + halfW, center.y - halfH, center.z + halfD); // v5
    points.emplace_back(center.x + halfW, center.y + halfH, center.z + halfD); // v6
    points.emplace_back(center.x - halfW, center.y + halfH, center.z + halfD); // v7
    
    // 创建12个三角形面（6个面 × 2个三角形）
    MR::Triangulation tris;
    tris.reserve(12);
    
    // 底面 (Z-)
    tris.push_back({v0, v2, v1});
    tris.push_back({v0, v3, v2});
    
    // 顶面 (Z+)
    tris.push_back({v4, v5, v6});
    tris.push_back({v4, v6, v7});
    
    // 前面 (Y-)
    tris.push_back({v0, v1, v5});
    tris.push_back({v0, v5, v4});
    
    // 后面 (Y+)
    tris.push_back({v3, v6, v2});
    tris.push_back({v3, v7, v6});
    
    // 左面 (X-)
    tris.push_back({v0, v4, v7});
    tris.push_back({v0, v7, v3});
    
    // 右面 (X+)
    tris.push_back({v1, v2, v6});
    tris.push_back({v1, v6, v5});
    
    // 构建网格
    MR::Mesh mesh;
    mesh.topology = MR::MeshBuilder::fromTriangles(tris);
    mesh.points = std::move(points);
    
    return mesh;
}
