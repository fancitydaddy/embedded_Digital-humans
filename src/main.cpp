#include <CubismFramework.cpp>
#include <CubismModelSettingJson.hpp>
#include <Rendering/OpenGL/CubismRenderer_OpenGLES2.hpp>
#include <string>

using namespace Live2D::Cubism::Framework;

void LoadModel(const std::string& modelFilePath)
{
    // 创建模型设置对象
    auto* modelSetting = new CubismModelSettingJson(modelFilePath.c_str());

    // 创建Cubism模型
    auto* model = CubismModel::Create(modelSetting);

    // 设置渲染器
    auto* renderer = new Csm::CubismRenderer_OpenGLES2();
    renderer->Initialize(model);
    
    // 你可以在此处加载模型的资源，如纹理、参数等

    // 渲染模型
    renderer->Draw();
}

void InitializeCubism()
{
    // Initialize Cubism Framework
    Live2D::Cubism::Framework::CubismFramework::Initialize();
}

// 释放Live2D Cubism SDK
void ReleaseCubism()
{
    Live2D::Cubism::Framework::CubismFramework::Dispose();
}

void MainLoop()
{
    // 初始化Live2D
    InitializeCubism();

    // 加载模型
    LoadModel("path/to/your/model/model3.json");

    // 主循环
    while (true)
    {
        // 获取音频电平值
        float audioLevel = GetAudioLevelExampleFunction();

        // 更新嘴型参数
        for (int i = 0; i < _modelSetting->GetLipSyncParameterCount(); ++i) {
            _model->AddParameterValue(_modelSetting->GetLipSyncParameterId(i), audioLevel, 0.8f);
        }

        // 更新和渲染模型
        // 这里假设你有一个窗口或OpenGL上下文
        // ...

        // Swap buffers
        // ...
    }

    // 释放Live2D
    ReleaseCubism();
}