#include <vulkan/vulkan_core.h>
#include "ShaderCompilation/ShaderObjectLayoutBuilder.h"
#include "ShaderCompilation/ShaderCompiler.h"
#include "FileManager.h"

#include "Logger.h"

namespace Pudu
{
    void ShaderCompiler::Init()
    {
        createGlobalSession(m_globalSession.writeRef());

        TargetDesc targetDesc;
        targetDesc.profile = m_globalSession->findProfile("spirv_1_6");
        targetDesc.format = SLANG_SPIRV;

        const char* searchPaths[] = {"Shaders"};

        SessionDesc sessionDesc;
        sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        sessionDesc.searchPathCount = 1;
        sessionDesc.searchPaths = searchPaths;

        slang::CompilerOptionEntry useEntryPointNameOption;
        useEntryPointNameOption.name = slang::CompilerOptionName::VulkanUseEntryPointName;
        useEntryPointNameOption.value = {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr};

        std::array<slang::CompilerOptionEntry, 1> options =
        {
            useEntryPointNameOption
        };

        sessionDesc.compilerOptionEntries = options.data();
        sessionDesc.compilerOptionEntryCount = options.size();

        m_globalSession->createSession(sessionDesc, m_session.writeRef());
    }

    bool PrintDiagnostics(Slang::ComPtr<IBlob> diagnostics)
    {
        if (diagnostics)
        {
            LOG_ERROR_NO_BREAK("Shader compilation error: {}",
                               static_cast<const char*>(diagnostics->getBufferPointer()));

            return true;
        }

        return false;
    }

    ShaderCompilationObject GetFailedCompilationObject()
    {
        ShaderCompilationObject emptyData{};
        emptyData.result = ShaderCompilationResult::Failed;
        return emptyData;
    }

    ShaderCompilationObject ShaderCompiler::Compile(const char* path, const std::vector<const char*>& entryPoints,
                                                    bool compute) const
    {
        Slang::ComPtr<IBlob> diagnostics;
        ShaderCompilationObject compiledData;


        //TODO: This is a kinda dirty solution, it will increase shader compilation time but so far, recreating the session has been the only way i've found to do hot reloading
        TargetDesc targetDesc;
        targetDesc.profile = m_globalSession->findProfile("spirv_1_6");
        targetDesc.format = SLANG_SPIRV;

        const char* searchPaths[] = {"Shaders"};

        SessionDesc sessionDesc;
        sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        sessionDesc.searchPathCount = 1;
        sessionDesc.searchPaths = searchPaths;

        slang::CompilerOptionEntry useEntryPointNameOption;
        useEntryPointNameOption.name = slang::CompilerOptionName::VulkanUseEntryPointName;
        useEntryPointNameOption.value = {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr};

        std::array<slang::CompilerOptionEntry, 1> options =
        {
            useEntryPointNameOption
        };

        sessionDesc.compilerOptionEntries = options.data();
        sessionDesc.compilerOptionEntryCount = options.size();

        Slang::ComPtr<ISession> session;
        m_globalSession->createSession(sessionDesc, session.writeRef());

        IModule* coreModule = session->loadModule("PuduCoreModule", diagnostics.writeRef());
        IModule* baseModule = session->loadModule("PuduGraphicsModule", diagnostics.writeRef());

        if (PrintDiagnostics(diagnostics)) return GetFailedCompilationObject();

        IModule* shaderModule = session->loadModule(path, diagnostics.writeRef());
        if (PrintDiagnostics(diagnostics)) return GetFailedCompilationObject();

        auto dependenciesCount = shaderModule->getDependencyFileCount();

        compiledData.m_dependencies.push_back(path);
        for (auto i = 0; i < dependenciesCount; i++)
        {
            auto dependencyPath = fs::path(shaderModule->getDependencyFilePath(i));
        //    std::ranges::transform(dependencyPath, dependencyPath.begin(), ::tolower);
            compiledData.m_dependencies.push_back(dependencyPath);
        }


        std::vector<Slang::ComPtr<IEntryPoint>> slangEntryPoints;

        for (auto entryPoint : entryPoints)
        {
            Slang::ComPtr<IEntryPoint> e;

            shaderModule->findEntryPointByName(entryPoint, e.writeRef());

            slangEntryPoints.push_back(e);
        }

        std::vector<IComponentType*> components = {};
        components.push_back(coreModule);


        ASSERT(baseModule != nullptr, "Base module is null for {}", path);
        //Base module has global descriptor sets which are not relevant for compute
        if (!compute)
            components.push_back(baseModule);

        components.push_back(shaderModule);

        components.append_range(slangEntryPoints);

        Slang::ComPtr<IComponentType> program;
        session->createCompositeComponentType(components.data(), components.size(), program.writeRef(),
                                              diagnostics.writeRef());

        if (PrintDiagnostics(diagnostics)) return GetFailedCompilationObject();

        slang::ProgramLayout* layout = program->getLayout();

        Slang::ComPtr<IComponentType> linkedProgram;
        program->link(linkedProgram.writeRef(), diagnostics.writeRef());

        if (PrintDiagnostics(diagnostics)) return GetFailedCompilationObject();

        //Global
        ShaderObjectLayoutBuilder layoutBuilder;
        layoutBuilder.m_globalSession = m_globalSession;
        layoutBuilder.ParseShaderProgramLayout(layout, compiledData);

        for (size_t i = 0; i < entryPoints.size(); i++)
        {
            Slang::ComPtr<IBlob> kernel;
            linkedProgram->getEntryPointCode(i, 0, kernel.writeRef(), diagnostics.writeRef());

            ShaderKernel kernelData;
            kernelData.codeSize = kernel->getBufferSize();

            const auto codePtr = malloc(kernelData.codeSize);
            memcpy(codePtr, kernel->getBufferPointer(), kernelData.codeSize);

            kernelData.code = static_cast<const uint32_t*>(codePtr);
            compiledData.AddKernel(entryPoints[i], kernelData);
            if (PrintDiagnostics(diagnostics)) return GetFailedCompilationObject();
        }

        // session->release();
        return compiledData;
    }

    ShaderCompilationObject ShaderCompiler::CompileModule(const fs::path& path)
    {
        ASSERT(!path.empty(), "Cannot compile module. Path is empty");
        Slang::ComPtr<IBlob> diagnostics;
        IModule* module = m_session->loadModule(path.string().c_str(), diagnostics.writeRef());

        PrintDiagnostics(diagnostics);

        std::vector<IComponentType*> components = {};

        components.push_back(module);

        Slang::ComPtr<IComponentType> program;
        m_session->createCompositeComponentType(components.data(), components.size(), program.writeRef(),
                                                diagnostics.writeRef());

        PrintDiagnostics(diagnostics);

        slang::ProgramLayout* layout = program->getLayout();

        Slang::ComPtr<IComponentType> linkedProgram;
        program->link(linkedProgram.writeRef(), diagnostics.writeRef());

        PrintDiagnostics(diagnostics);
        //Global
        ShaderCompilationObject compiledData;
        ShaderObjectLayoutBuilder layoutBuilder;
        layoutBuilder.m_globalSession = m_globalSession;
        layoutBuilder.ParseShaderProgramLayout(layout, compiledData);

        PrintDiagnostics(diagnostics);

        return compiledData;
    }
}
