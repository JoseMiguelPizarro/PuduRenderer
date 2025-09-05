//Windows defines a min max func that messes up std funcs :')
#define NOMINMAX

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <ranges>
#include <algorithm>
#include <limits>

#include "PuduGraphics.h"
#include "PuduGlobals.h"

#include <stdexcept>
#include <Logger.h>
#include <set>
#include "UniformBufferObject.h"

#include "DrawCall.h"

#include "Frame.h"
#include "FrameGraph/FrameGraph.h"

#include "VulkanUtils.h"
#include "Pipeline.h"
#include "Lighting/LightBuffer.h"
#include "Texture.h"
#include "Texture2D.h"
#include "TextureCube.h"

#include <ktx.h>
#include "FileManager.h"
#include "RenderFrameData.h"
#include <vk_mem_alloc.h>
#include <FileWatch.hpp>

#include "Profiling/PuduGPUProfiler.h"
#include <memory>


namespace Pudu
{
    const char* SHADER_ENTRY_POINT = "main";

    std::unordered_map<fs::path, std::vector<GPUResourceHandle<Shader>>> LoadedShaders;
    std::unordered_map<fs::path, std::unique_ptr<filewatch::FileWatch<std::string>>> watchedFiles;

    class FrameGraph;

    PuduGraphics* PuduGraphics::s_instance = nullptr;

    PuduGraphics* PuduGraphics::Instance()
    {
        return s_instance;
    }

    void PuduGraphics::Init(PuduGraphicsSettings& settings)
    {
        PuduGraphics::s_instance = this;
        LOG("Graphics Init");
        m_settings = settings;
        WindowWidth = settings.resolution.x;
        WindowHeight = settings.resolution.y;

        m_resources.Init(this);

        m_shaderCompiler.Init();

        InitWindow();
        InitVulkan();
        InitDefaultResources();

        m_initialized = true;
    }

    void PuduGraphics::InitVulkan()
    {
        CreateFrames();
        CreateVulkanInstance();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        InitDebugUtilsObjectName();
        InitVMA();
        CreateSwapChain(m_currentSwapchain);
        CreateSwapchainImageViews(m_currentSwapchain);

        CreateBindlessDescriptorPool();

        CreateUniformBuffers();

        m_commandPool = GetCommandPool(QueueFamily::Graphics);
        CreateFramesCommandBuffer();
        CreateSwapChainSyncObjects();

        pfn_vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(
            m_device, "vkCmdPushDescriptorSetKHR");
        if (!pfn_vkCmdPushDescriptorSetKHR)
        {
            LOG("Could not get a valid function pointer for vkCmdPushDescriptorSetKHR");
        }

        InitProfiler();
    }

    void PuduGraphics::InitVMA()
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = m_physicalDevice;
        allocatorInfo.device = m_device;
        allocatorInfo.instance = m_vkInstance;

        vmaCreateAllocator(&allocatorInfo, &m_VmaAllocator);
    }

    void PuduGraphics::InitWindow()
    {
        Print("Graphics Init Window");
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        WindowPtr = glfwCreateWindow(WindowWidth, WindowHeight, RENDERER_NAME, nullptr, nullptr);
        glfwSetWindowUserPointer(WindowPtr, this);
        glfwSetFramebufferSizeCallback(WindowPtr, FramebufferResizeCallback);
    }


    bool PuduGraphics::CheckValidationLayerSupport()
    {
        u32 layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    void PuduGraphics::UpdateBindlessTexture(GPUResourceHandle<Texture> handle)
    {
        ResourceUpdate resourceToUpdate{};
        resourceToUpdate.handle = {handle.Index()};

        m_bindlessResourcesToUpdate.push_back(resourceToUpdate);
    }

    void PuduGraphics::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        createInfo.pfnUserCallback = VkDebugCallback;
        createInfo.pUserData = nullptr;
    }

    void PuduGraphics::SetupDebugMessenger()
    {
        if (!enableValidationLayers)
        {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, m_allocatorPtr, &m_debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    VkSurfaceFormatKHR PuduGraphics::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == m_settings.surfaceFormat && availableFormat.colorSpace ==
                m_settings.surfaceColorSpace)
            {
                return availableFormat;
            }
        }

        LOG_ERROR("Failed to find suitable surface format!");

        return availableFormats[0];
    }

    VkPresentModeKHR PuduGraphics::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == ToVk(m_settings.presentMode))
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }


    VkExtent2D PuduGraphics::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        int width, height;
        glfwGetFramebufferSize(WindowPtr, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<u32>(width),
            static_cast<u32>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }

    VmaAllocation PuduGraphics::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                             VmaAllocationCreateFlags properties,
                                             VkBuffer& buffer, const char* name)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocCreateInfo.flags = properties;

        VmaAllocation alloc;
        VmaAllocationInfo allocInfo;


        //Create buffer, allocate the memory and binds memory to buffer
        VKCheck(
            vmaCreateBuffer(m_VmaAllocator, &bufferInfo, &allocCreateInfo, &buffer, &alloc, &allocInfo
            ),
            "Failed creating buffer");
        if (name != nullptr)
        {
            SetResourceName(VK_OBJECT_TYPE_BUFFER, (u64)buffer, name);
            SetResourceName(VK_OBJECT_TYPE_DEVICE_MEMORY, (u64)allocInfo.deviceMemory, name);
        }

        return alloc;
    }

    void PuduGraphics::UpdateDescriptorSet(const uint16_t count, const VkWriteDescriptorSet* write,
                                           const uint16_t copyCount,
                                           const VkCopyDescriptorSet* copy)
    {
        vkUpdateDescriptorSets(m_device, count, write, copyCount, copy);
    }

    void PuduGraphics::UploadBufferData(GraphicsBuffer* buffer, const void* data, const Size size, const Size offset)
    {
        memcpy(static_cast<byte*>(buffer->GetMappedData()) + offset, data, size);
    }

    std::vector<ResourceUpdate>* PuduGraphics::GetBindlessResourcesToUpdate()
    {
        return &m_bindlessResourcesToUpdate;
    }

    void PuduGraphics::CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size)
    {
        auto commandBuffer = BeginSingleTimeCommands();

        commandBuffer.CopyBuffer(srcBuffer, dstBuffer, size);

        EndSingleTimeCommands(commandBuffer);
    }

    u32 PuduGraphics::FindMemoryType(const u32 typeFilter, const VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

        for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if (typeFilter & (1 << i) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        LOG_ERROR("Failed to find suitable memory type!");

        return -1;
    }

    void PuduGraphics::DestroyBuffer(const SPtr<GraphicsBuffer>& buffer) const
    {
        if (!buffer->IsDestroyed())
        {
            vmaDestroyBuffer(m_VmaAllocator, buffer->vkHandle, buffer->allocation);
            buffer->Destroy();
        }
    }

    std::vector<const char*> PuduGraphics::GetInstanceExtensions() const
    {
        u32 glfwExtensionsCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

        if (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        return extensions;
    }

    void PuduGraphics::InitProfiler()
    {
        pfn_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)
            vkGetInstanceProcAddr(m_vkInstance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");

        pfn_vkGetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)vkGetDeviceProcAddr(
            m_device, "vkGetCalibratedTimestampsEXT");

        std::vector<VkTimeDomainEXT> timeDomains;

        u32 timeDomainsCount = 0;
        pfn_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(
            m_physicalDevice, &timeDomainsCount, nullptr);

        timeDomains.resize(timeDomainsCount);
        pfn_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(m_physicalDevice, &timeDomainsCount, timeDomains.data());


        const bool hasHostQuery =
            [&timeDomains]() -> bool
            {
                for (const VkTimeDomainEXT domain : timeDomains)
                {
                    if (domain == VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT ||
                        domain == VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT)
                        return true;
                }
                return false;
            }();

        if (hasHostQuery)
        {
            m_gpuProfiler = new GPUProfiler();
            auto pfn_rqp = reinterpret_cast<PFN_vkResetQueryPool>(vkGetInstanceProcAddr(
                m_vkInstance, "vkResetQueryPool"));
            m_gpuProfiler->Init(m_physicalDevice, m_device, pfn_rqp, pfn_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT,
                                pfn_vkGetCalibratedTimestampsEXT);
        }
    }

    void PuduGraphics::DrawFrame(RenderFrameData& frameData)
    {
        PROFILER_ZONE("Draw Frame", 0xffffff);
        auto frameGraph = frameData.frameGraph;

        frameData.frameIndex = m_currentFrameIndex;
        Frame frame = m_Frames[m_currentFrameIndex];

        //don't wait the first frames
        if (m_absoluteFrame >= MAX_FRAMES_IN_FLIGHT)
        {
            u64 graphicsTimelineValue = m_absoluteFrame;
            u64 computeTimelineValue = m_computeTimelineSemaphore->TimelineValue();

            u64 timelineValues[]{graphicsTimelineValue, computeTimelineValue};
            VkSemaphore semaphores[]{m_graphicsTimelineSemaphore->vkHandle, m_computeTimelineSemaphore->vkHandle};

            VkSemaphoreWaitInfo waitInfo{VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO};
            waitInfo.semaphoreCount = 1; //for now just wait for the graphics
            waitInfo.pSemaphores = semaphores;
            waitInfo.pValues = timelineValues;

            vkWaitSemaphores(m_device, &waitInfo, ~0ull); //wait infinite
        }
        //If any shader needs to be reloaded, must be done after the GPU finish its current workload, hence the semaphore
        ReloadPendingShaders();

        ////Fences are used to ensure that the GPU has stopped using resources for a given frame. This force the CPU to wait for the GPU to finish using the resources
        //vkWaitForFences(m_device, 1, &frame.InFlightFence, VK_TRUE, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(m_device, m_currentSwapchain.swapchainHandle, UINT64_MAX,
                                                *frame.PresentSemaphore,
                                                VK_NULL_HANDLE, &frameData.swapChainIndex);

        frameData.currentSwapChain = m_currentSwapchain.textures[frameData.swapChainIndex];

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LOG_ERROR("failed to acquire swap chain image!");
        }

        vkResetFences(m_device, 1, &frame.InFlightFence);

        frame.CommandBuffer->Reset();
        frame.ComputeCommandBuffer->Reset();

        PROFILER_ZONE("Render Frame", 0xffffff);
        frame.CommandBuffer->BeginCommands();


        frame.ComputeCommandBuffer->BeginCommands();

        frameData.frame = &m_Frames[m_currentFrameIndex];
        frameData.currentCommand = frame.CommandBuffer;
        frameData.graphics = this;

        frameData.commandsToSubmit.push_back(frame.CommandBuffer->vkHandle);

        frameGraph->RenderFrame(frameData);

        //PRESENT LOGIC

        frameData.currentCommand->TransitionTextureLayout(frameData.activeRenderTarget,
                                                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        frameData.currentCommand->TransitionTextureLayout(m_currentSwapchain.textures[frameData.frameIndex],
                                                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        frameData.currentCommand->Blit(frameData.activeRenderTarget,
                                       m_currentSwapchain.textures[frameData.frameIndex],
                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        frameData.currentCommand->TransitionTextureLayout(m_currentSwapchain.textures[frameData.frameIndex],
                                                          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        //TODO: Ideally we shouldn't have to set the texture usage manually, a solution would be to move the transition image layout logic directly
        frameGraph->SetTextureUsage(frameData.activeRenderTarget->Handle(), COPY_SOURCE);

        frame.CommandBuffer->EndCommands();

        frame.ComputeCommandBuffer->EndCommands();

        auto computeCommands = frame.ComputeCommandBuffer;
        frameData.computeCommandsToSubmit.push_back(computeCommands.get());


        PROFILER_ZONE_END()
        PROFILE_GPU_COLLECT(m_gpuProfiler, frame.CommandBuffer->vkHandle);
        SubmitComputeWork(frameData);
        SubmitFrame(frameData);

        EndDrawFrame();
        PROFILER_ZONE_END()
    }

    SPtr<Texture> PuduGraphics::GetMultisampledColorTexture()
    {
        return m_multisampledColorTexture;
    }

    SPtr<Texture> PuduGraphics::GetMultisampledDepthTexture()
    {
        return m_multisampledDepthTexture;
    }

    void PuduGraphics::SubmitComputeWork(RenderFrameData& frameData)
    {
        bool hasWaitSemaphore = m_computeTimelineSemaphore->TimelineValue() > 0;

        VkSemaphoreSubmitInfo waitSemaphores[]{
            {
                VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, m_computeTimelineSemaphore->vkHandle,
                m_computeTimelineSemaphore->TimelineValue(), VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, 0
            },
        };


        VkSemaphoreSubmitInfo signalSemaphores[]{
            {
                VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, m_computeTimelineSemaphore->vkHandle,
                m_computeTimelineSemaphore->Signal(), VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, 0
            }
        };

        std::vector<VkCommandBufferSubmitInfo> commandSubmitInfos;
        for (auto command : frameData.computeCommandsToSubmit)
        {
            if (!command->HasRecordedCommand())
            {
                continue;
            }

            VkCommandBufferSubmitInfo commandInfo{};
            commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            commandInfo.commandBuffer = command->vkHandle;

            commandSubmitInfos.push_back(commandInfo);
        }

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = hasWaitSemaphore ? 1 : 0;
        submitInfo.pWaitSemaphoreInfos = waitSemaphores;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos = signalSemaphores;
        submitInfo.commandBufferInfoCount = commandSubmitInfos.size();
        submitInfo.pCommandBufferInfos = commandSubmitInfos.data();

        vkQueueSubmit2(m_computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
    }

    void PuduGraphics::InitDefaultCube()
    {
             // Create default cube mesh
            MeshCreationData defaultCubeMeshData;
            defaultCubeMeshData.Indices = {
                0, 1, 2, 2, 3, 0, // front
                4, 5, 6, 6, 7, 4, // back
                8, 9, 10, 10, 11, 8, // right
                12, 13, 14, 14, 15, 12, // left
                16, 17, 18, 18, 19, 16, // top
                20, 21, 22, 22, 23, 20 // bottom
            };

            // Get vertex count
            const uint32_t vertexCount = 24;

            auto cubeAttributes = MeshAttributes::Create();

            vec3 cubePositions[] = {
                {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, // Front
                {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, // Back
                {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, // Right
                {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}, // Left
                {-0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f}, // Top
                {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f} // Bottom
            };

            vec3 cubeColors[vertexCount];
            for (int i = 0; i < vertexCount; i++)
            {
                cubeColors[i] = {1, 1, 1};
            }

            vec2 cubeUVs[] = {
                {0, 0}, {1, 0}, {1, 1}, {0, 1}, // Front
                {1, 0}, {1, 1}, {0, 1}, {0, 0}, // Back
                {0, 0}, {0, 1}, {1, 1}, {1, 0}, // Right
                {1, 0}, {0, 0}, {0, 1}, {1, 1}, // Left
                {0, 1}, {0, 0}, {1, 0}, {1, 1}, // Top
                {0, 0}, {1, 0}, {1, 1}, {0, 1} // Bottom
            };

            vec3 cubeNormals[] = {
                {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, // Front
                {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, // Back
                {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, // Right
                {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, // Left
                {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, // Top
                {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0} // Bottom
            };

            vec4 cubeTangents[] = {
                {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, // Front
                {-1, 0, 0, 1}, {-1, 0, 0, 1}, {-1, 0, 0, 1}, {-1, 0, 0, 1}, // Back
                {0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, // Right
                {0, 0, -1, 1}, {0, 0, -1, 1}, {0, 0, -1, 1}, {0, 0, -1, 1}, // Left
                {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, // Top
                {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1} // Bottom
            };

            cubeAttributes->CreateAttribute({VertexAttributeType::POSITION, ChannelFormat::R32G32B32_SFLOAT},
                                            vertexCount);
            cubeAttributes->SetAttributeData(VertexAttributeType::POSITION, cubePositions);

            cubeAttributes->CreateAttribute({VertexAttributeType::COLOR, ChannelFormat::R32G32B32_SFLOAT}, vertexCount);
            cubeAttributes->SetAttributeData(VertexAttributeType::COLOR, cubeColors);

            cubeAttributes->CreateAttribute({VertexAttributeType::TEXCOORD0, ChannelFormat::R32G32_SFLOAT},
                                            vertexCount);
            cubeAttributes->SetAttributeData(VertexAttributeType::TEXCOORD0, cubeUVs);

            cubeAttributes->CreateAttribute({VertexAttributeType::NORMAL, ChannelFormat::R32G32B32_SFLOAT},
                                            vertexCount);
            cubeAttributes->SetAttributeData(VertexAttributeType::NORMAL, cubeNormals);

            cubeAttributes->CreateAttribute({VertexAttributeType::TANGENT, ChannelFormat::R32G32B32A32_SFLOAT},
                                            vertexCount);
            cubeAttributes->SetAttributeData(VertexAttributeType::TANGENT, cubeTangents);


            defaultCubeMeshData.MeshAttributes = cubeAttributes;
            defaultCubeMeshData.Name = "DefaultCube";
            m_defaultCube = CreateMesh(defaultCubeMeshData);
    }

    void PuduGraphics::InitDefaultQuad()
    {
        MeshCreationData defaultQuadMeshData;
        defaultQuadMeshData.Indices = {0, 2, 1, 1, 2, 3};

        vec3 positions[] = {
            {-.5f, 0, -0.5f},
            {0.5f, 0, -0.5f},
            {-.5f, 0, 0.5f},
            {0.5f, 0, 0.5f}
        };
        vec3 colors[] = {
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 1}
        };
        vec2 uvs[] = {
            {0, 0},
            {1, 0},
            {0, 1},
            {1, 1}
        };

        vec3 normals[] = {
            {0, 1, 0},
            {0, 1, 0},
            {0, 1, 0},
            {0, 1, 0}
        };

        auto quadAttributes = MeshAttributes::Create();
        quadAttributes->CreateAttribute({VertexAttributeType::POSITION, ChannelFormat::R32G32B32_SFLOAT}, 4);
        quadAttributes->SetAttributeData(VertexAttributeType::POSITION, positions);

        quadAttributes->CreateAttribute({VertexAttributeType::COLOR, ChannelFormat::R32G32B32_SFLOAT}, 4);
        quadAttributes->SetAttributeData(VertexAttributeType::COLOR, colors);

        quadAttributes->CreateAttribute({VertexAttributeType::TEXCOORD0, ChannelFormat::R32G32_SFLOAT}, 4);
        quadAttributes->SetAttributeData(VertexAttributeType::TEXCOORD0, uvs);

        quadAttributes->CreateAttribute({VertexAttributeType::NORMAL, ChannelFormat::R32G32B32_SFLOAT}, 4);
        quadAttributes->SetAttributeData(VertexAttributeType::NORMAL, normals);

        defaultQuadMeshData.MeshAttributes = quadAttributes;
        defaultQuadMeshData.Name = "DefaultQuad";

        m_defaultQuad = CreateMesh(defaultQuadMeshData);
    }

    void PuduGraphics::InitDefaultSphere()
    {
        // Create default sphere mesh
        MeshCreationData defaultSphereMeshData;
        const float sphereRings = 32.;
        const float sphereSegments = 32.;
        const float sphereRadius = 0.5f;

        constexpr u32 sphereVertexCount = (sphereRings + 1) * (sphereSegments + 1);

        auto sphereAttributes = MeshAttributes::Create();
        sphereAttributes->CreateAttribute({VertexAttributeType::POSITION, ChannelFormat::R32G32B32_SFLOAT},
                                          sphereVertexCount);
        sphereAttributes->CreateAttribute({VertexAttributeType::COLOR, ChannelFormat::R32G32B32_SFLOAT},
                                          sphereVertexCount);
        sphereAttributes->CreateAttribute({VertexAttributeType::TEXCOORD0, ChannelFormat::R32G32_SFLOAT},
                                          sphereVertexCount);
        sphereAttributes->CreateAttribute({VertexAttributeType::NORMAL, ChannelFormat::R32G32B32_SFLOAT},
                                          sphereVertexCount);
        sphereAttributes->CreateAttribute({VertexAttributeType::TANGENT, ChannelFormat::R32G32B32A32_SFLOAT},
                                          sphereVertexCount);

        vec3 spherePositions[sphereVertexCount];
        vec3 sphereColors[sphereVertexCount];
        vec2 sphereUVs[sphereVertexCount];
        vec3 sphereNormals[sphereVertexCount];
        vec4 sphereTangents[sphereVertexCount];

        u32 sphereVertexIndex = 0;
        for (int ring = 0; ring <= sphereRings; ring++)
        {
            float theta = glm::pi<float>() * ring / sphereRings;
            for (int segment = 0; segment <= sphereSegments; segment++)
            {
                float phi = 2.0f * glm::pi<float>() * segment / sphereSegments;

                float x = sphereRadius * sin(theta) * cos(phi);
                float y = sphereRadius * cos(theta);
                float z = sphereRadius * sin(theta) * sin(phi);

                spherePositions[sphereVertexIndex] = vec3(x, y, z);
                sphereColors[sphereVertexIndex] = vec3(1, 1, 1);
                sphereUVs[sphereVertexIndex] = vec2(segment / sphereSegments, ring / sphereRings);
                sphereNormals[sphereVertexIndex] = normalize(vec3(x, y, z));
                sphereTangents[sphereVertexIndex] = vec4(cos(phi), 0, -sin(phi), 1);

                sphereVertexIndex++;
            }
        }

        sphereAttributes->SetAttributeData(VertexAttributeType::POSITION, spherePositions);
        sphereAttributes->SetAttributeData(VertexAttributeType::COLOR, sphereColors);
        sphereAttributes->SetAttributeData(VertexAttributeType::TEXCOORD0, sphereUVs);
        sphereAttributes->SetAttributeData(VertexAttributeType::NORMAL, sphereNormals);
        sphereAttributes->SetAttributeData(VertexAttributeType::TANGENT, sphereTangents);


        // Generate indices
        for (int ring = 0; ring < sphereRings; ring++)
        {
            for (int segment = 0; segment < sphereSegments; segment++)
            {
                int current = ring * (sphereSegments + 1) + segment;
                int next = current + sphereSegments + 1;

                defaultSphereMeshData.Indices.push_back(current);
                defaultSphereMeshData.Indices.push_back(current + 1);
                defaultSphereMeshData.Indices.push_back(next);

                defaultSphereMeshData.Indices.push_back(current + 1);
                defaultSphereMeshData.Indices.push_back(next + 1);
                defaultSphereMeshData.Indices.push_back(next);
            }
        }

        defaultSphereMeshData.Name = "DefaultSphere";
        defaultSphereMeshData.MeshAttributes = sphereAttributes;

        m_defaultSphere = CreateMesh(defaultSphereMeshData);
    }

    void PuduGraphics::InitDefaultResources()
    {
        //Default Meshes
        {
            InitDefaultCube();

            InitDefaultQuad();

            InitDefaultSphere();
        }

        InitDefaultTextures();

        //Default Shaders

        {
            m_defaultStandardShader = CreateShader("standardSurface.shader.slang", "Default StandardSurface");
            m_defaultOverlayShader = CreateShader("quadOverlay.shader.slang", "QuadOverlay");
            m_defaultErrorShader = CreateShader(k_DEFAULT_ERROR_SHADER_PATH, "ErrorShader");

            m_defaultOverlayTextureArrayShader = CreateShader("quadTextureArrayOverlay.shader.slang",
                                                              "QuadArrayOverlay");

            ComputeShaderCreationData envToCubemapCS_Data{
                "Compute/horizonMapToCubeMap.compute.slang", "horizonToCubemap"
            };
            m_horizonToCubeCompute = CreateComputeShader(envToCubemapCS_Data);
            auto horizonToCubemapMat = CreateMaterial();
            horizonToCubemapMat->SetShader(m_horizonToCubeCompute);

            m_horizonToCubemapCSRenderer.SetShader(m_horizonToCubeCompute);
            m_horizonToCubemapCSRenderer.SetMaterial(horizonToCubemapMat);
        }

        //Default materials
        m_defaultStandardMaterial = CreateMaterial();
        m_defaultStandardMaterial->name = "DefaultStandardMaterial";
        m_defaultStandardMaterial->SetShader(m_defaultStandardShader);
        m_defaultStandardMaterial->SetProperty("material.normalTex", m_defaultNormalmapTexture);
        m_defaultStandardMaterial->SetProperty("material.albedoTex", m_defaultWhiteTexture);
        m_defaultStandardMaterial->SetProperty("material.metallicRoughnessTex", m_defaultMetallicRoughnessTexture);
        m_defaultStandardMaterial->SetProperty("material.heightTex", m_defaultBlackTexture);
        m_defaultStandardMaterial->SetProperty("material.F90", {0.04f, 0.04f, 0.04f});
        m_defaultStandardMaterial->SetProperty("material.F90",  {0.5f, 0.5f, 0.5f});
    }

    void PuduGraphics::InitDefaultTextures()
    {
        TextureCreationData textureCreationData;
        textureCreationData.height = 2;
        textureCreationData.width = 2;
        textureCreationData.format = VK_FORMAT_R8G8B8A8_SRGB;
        textureCreationData.depth = 1;
        textureCreationData.layers = 1;
        textureCreationData.bindless = true;
        textureCreationData.name = "PuduWhite";
        textureCreationData.flags = TextureFlags::Sample;

        SamplerCreationData samplerCreationData{true};

        using b = std::byte;

        std::byte data[] = {
            b{255}, b{255}, b{255}, b{255},
            b{255}, b{255}, b{255}, b{255},
            b{255}, b{255}, b{255}, b{255},
            b{255}, b{255}, b{255}, b{255}
        };

        textureCreationData.pixels = data;
        textureCreationData.dataSize = sizeof(data);
        textureCreationData.samplerData = &samplerCreationData;

        m_defaultWhiteTexture = CreateTexture(textureCreationData);

        std::byte metallicRoughnessData[] = {
            b{0}, b{128}, b{0}, b{128}, // R = 0, G = 0.5 (128 in byte value), A = 0
            b{0}, b{128}, b{0}, b{128}, // G
        };

        std::byte normalData[] = {
            b{128}, b{128}, b{255}, b{255},
            b{128}, b{128}, b{255}, b{255},
            b{128}, b{128}, b{255}, b{255},
            b{128}, b{128}, b{255}, b{255}
        };

        std::byte blackData[] = {
            b{0}, b{0}, b{0}, b{255},
            b{0}, b{0}, b{0}, b{255},
            b{0}, b{0}, b{0}, b{255},
            b{0}, b{0}, b{0}, b{255}
        };

        TextureCreationData blackTextureData;
        blackTextureData.height = 2;
        blackTextureData.width = 2;
        blackTextureData.format = VK_FORMAT_R8G8B8A8_UNORM; // Format for RGBA
        blackTextureData.depth = 1;
        blackTextureData.layers = 1;
        blackTextureData.bindless = true;
        blackTextureData.name = "DefaultBlack";
        blackTextureData.flags = TextureFlags::Sample;
        blackTextureData.pixels = blackData;
        blackTextureData.dataSize = sizeof(blackData);

        SamplerCreationData blackSamplerData{true};
        blackTextureData.samplerData = &blackSamplerData;

        m_defaultBlackTexture = CreateTexture(blackTextureData);

        TextureCreationData normalTextureData;
        normalTextureData.height = 2;
        normalTextureData.width = 2;
        normalTextureData.format = VK_FORMAT_R8G8B8A8_UNORM; // Four channels, R, G, B, A
        normalTextureData.depth = 1;
        normalTextureData.layers = 1;
        normalTextureData.bindless = true;
        normalTextureData.name = "DefaultNormal";
        normalTextureData.flags = TextureFlags::Sample;
        normalTextureData.pixels = normalData;
        normalTextureData.dataSize = sizeof(normalData);

        SamplerCreationData normalSamplerData{true};
        normalTextureData.samplerData = &normalSamplerData;

        m_defaultNormalmapTexture = CreateTexture(normalTextureData);

        TextureCreationData metallicRoughnessTextureData;
        metallicRoughnessTextureData.height = 2;
        metallicRoughnessTextureData.width = 2;
        metallicRoughnessTextureData.format = VK_FORMAT_R8G8_UNORM; // Two channels: R and G
        metallicRoughnessTextureData.depth = 1;
        metallicRoughnessTextureData.layers = 1;
        metallicRoughnessTextureData.bindless = true;
        metallicRoughnessTextureData.name = "DefaultMetallicRoughness";
        metallicRoughnessTextureData.flags = TextureFlags::Sample;
        metallicRoughnessTextureData.pixels = metallicRoughnessData;
        metallicRoughnessTextureData.dataSize = sizeof(metallicRoughnessData);

        SamplerCreationData metallicRoughnessSamplerData{true};
        metallicRoughnessTextureData.samplerData = &metallicRoughnessSamplerData;

        m_defaultMetallicRoughnessTexture = CreateTexture(metallicRoughnessTextureData);


        TextureCreationData defaultCubemapData{};
        defaultCubemapData.width = 1;
        defaultCubemapData.height = 1;
        defaultCubemapData.format = VK_FORMAT_R8G8B8A8_SRGB;
        defaultCubemapData.textureType = TextureType::Texture_Cube;
        defaultCubemapData.name = "DefaultCubemap";
        defaultCubemapData.flags = TextureFlags::Sample;
        defaultCubemapData.layers = 6;
        defaultCubemapData.mipmaps = 1;

        std::vector<float> cubemapPixels(24, 1.0f); // 6 faces * RGBA
        defaultCubemapData.pixels = cubemapPixels.data();
        defaultCubemapData.dataSize = cubemapPixels.size() * sizeof(float);

        SamplerCreationData cubemapSamplerData{true};
        defaultCubemapData.samplerData = &cubemapSamplerData;

        m_defaultCubemap = CreateTexture(defaultCubemapData);

        SamplerCreationData samplerData{true, 1, VK_FILTER_NEAREST};
        TextureLoadSettings pudutextureSettings;
        pudutextureSettings.mipLevels = 1;
        pudutextureSettings.name = "PuduLogo";
        pudutextureSettings.samplerData = samplerData;

        m_defaultPuduTexture = LoadTexture2D("textures/PuduLogo.png", pudutextureSettings)->Handle();
    }

    void PuduGraphics::CreateVKGraphicsPipeline(Pipeline* pipeline, PipelineCreationData& creationData)
    {
        auto renderPass = creationData.renderPassHandle.Get();


        auto shaderStateHandle = CreateShaderState(creationData.shadersStateCreationData);
        pipeline->shaderState = shaderStateHandle;
        auto shaderState = shaderStateHandle.Get();

        auto& renderPassOutput = renderPass->attachments;
        auto outputCount = renderPassOutput.colorAttachmentVkCount;

        pipeline->shaderState = shaderStateHandle;
        pipeline->depthStencilFormat = renderPassOutput.depthStencilFormat;
        pipeline->pipelineType = PipelineType::Graphics;

        VkPushConstantRange pushConstant{};
        pushConstant.offset = 0;
        pushConstant.size = sizeof(UniformBufferObject);
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        //Push contants support
        VkPushConstantRange constants[1]{pushConstant};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = creationData.activeLayouts;
        pipelineLayoutInfo.pSetLayouts = creationData.vkDescriptorSetLayout;
        pipelineLayoutInfo.pPushConstantRanges = constants;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

        VkPipelineLayout pipelineLayout;
        vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, m_allocatorPtr, &pipelineLayout);

        pipeline->vkPipelineLayoutHandle = pipelineLayout;

        if (shaderState->graphicsPipeline)
        {
            VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
            graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

            graphicsPipelineInfo.pStages = shaderState->shaderStageInfo;
            graphicsPipelineInfo.stageCount = shaderState->activeShaders;

            graphicsPipelineInfo.layout = pipelineLayout;

            //Vertex input
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            //Vertex attributes
            std::vector<VkVertexInputAttributeDescription> vertexAttributes(
                creationData.vertexInput.GetVertexLayout()->GetAttributeCount());

            u32 vertexAttribsCount = creationData.vertexInput.GetVertexLayout()->GetAttributeCount();

            for (size_t i = 0; i < vertexAttribsCount; i++)
            {
                VkVertexInputAttributeDescription& vertexAttribute = vertexAttributes[i];
                vertexAttribute = {};

                auto inputVertexAttrib = creationData.vertexInput.GetVertexLayout()->GetAttribute(i);
                vertexAttribute.location = inputVertexAttrib.location;
                vertexAttribute.binding = inputVertexAttrib.binding;
                vertexAttribute.offset = inputVertexAttrib.offset;
                vertexAttribute.format = ToVk(inputVertexAttrib.format);
            }

            vertexInputInfo.vertexAttributeDescriptionCount = vertexAttribsCount;
            vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();

            u32 vertexStreamsCount = creationData.vertexInput.GetStreamCount();


            //Vertex Bindings
            std::vector<VkVertexInputBindingDescription> vertexBindings(vertexStreamsCount);
            if (vertexStreamsCount)
            {
                for (size_t i = 0; i < vertexStreamsCount; i++)
                {
                    VertexStream const& vertexStream = creationData.vertexInput.GetVertexStream(i);
                    VkVertexInputBindingDescription binding{};
                    binding.binding = vertexStream.binding;
                    binding.inputRate = vertexStream.GetVkInputRate();
                    binding.stride = vertexStream.stride;

                    vertexBindings[i] = binding;
                }

                vertexInputInfo.vertexBindingDescriptionCount = vertexBindings.size();
                vertexInputInfo.pVertexBindingDescriptions = vertexBindings.data();
            }

            graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;

            //Input assembly
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
            inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyInfo.topology = creationData.topology;
            inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

            graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyInfo;


            //Blending
            size_t blendCount = creationData.blendState.activeStatesCount;
            std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

            if (blendCount)
            {
                colorBlendAttachments.resize(blendCount);
                for (size_t i = 0; i < blendCount; i++)
                {
                    auto& attachment = colorBlendAttachments[i];

                    auto blendState = creationData.blendState.blendStates[i];

                    attachment.blendEnable = blendState.blendEnabled;
                    attachment.colorWriteMask = blendState.colorMask;
                    attachment.srcColorBlendFactor = blendState.sourceColorFactor;
                    attachment.dstColorBlendFactor = blendState.destinationColorFactor;
                    attachment.colorBlendOp = blendState.colorBlendOperation;
                    attachment.srcAlphaBlendFactor = blendState.sourceAlphaFactor;
                    attachment.dstAlphaBlendFactor = blendState.destinationAlphaFactor;
                }
            }
            else
            {
                colorBlendAttachments.resize(outputCount);
                for (u32 i = 0; i < outputCount; ++i)
                {
                    colorBlendAttachments[i] = {};
                    colorBlendAttachments[i].blendEnable = VK_FALSE;
                    colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                }
            }

            VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
            colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendingInfo.logicOpEnable = VK_FALSE;
            colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
            colorBlendingInfo.attachmentCount = blendCount ? (u32)blendCount : outputCount;
            colorBlendingInfo.pAttachments = colorBlendAttachments.data();

            graphicsPipelineInfo.pColorBlendState = &colorBlendingInfo;

            //Depth Stencil

            VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
            auto const& depthStencilState = creationData.depthStencil;

            depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilInfo.depthWriteEnable = depthStencilState.isDepthWriteEnable;
            depthStencilInfo.stencilTestEnable = false;
            depthStencilInfo.depthTestEnable = depthStencilState.isDepthEnabled;
            depthStencilInfo.depthCompareOp = depthStencilState.depthComparison;
            depthStencilInfo.front = depthStencilState.GetVkFront();
            depthStencilInfo.back = depthStencilState.GetVkBack();

            graphicsPipelineInfo.pDepthStencilState = &depthStencilInfo;

            //Multisample
            VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
            multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisamplingInfo.rasterizationSamples = creationData.multiSampled
                                                         ? static_cast<VkSampleCountFlagBits>(m_antialiasingSettings.
                                                             sampleCount)
                                                         : VK_SAMPLE_COUNT_1_BIT;
            multisamplingInfo.sampleShadingEnable = VK_FALSE;
            multisamplingInfo.minSampleShading = 1.0f; // Optional
            multisamplingInfo.pSampleMask = nullptr; // Optional
            multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
            multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional

            graphicsPipelineInfo.pMultisampleState = &multisamplingInfo;

            //Rasterizer
            VkPipelineRasterizationStateCreateInfo rasterizer{
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
            };
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = creationData.rasterization.cullMode;
            rasterizer.frontFace = creationData.rasterization.front;
            rasterizer.depthBiasEnable = VK_TRUE; //TODO: This should be enabled only on shadowmapping pass
            //rasterizer.depthBiasConstantFactor = 0.0f; // Optional
            //rasterizer.depthClampEnable = VK_FALSE;
            //rasterizer.depthBiasClamp = 0.0f; // Optional
            //rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

            graphicsPipelineInfo.pRasterizationState = &rasterizer;

            //// Viewport state
            VkViewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)m_swapChainExtent.width;
            viewport.height = (float)m_swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor = {};
            scissor.offset = {0, 0};
            scissor.extent = {m_swapChainExtent.width, m_swapChainExtent.height};

            VkPipelineViewportStateCreateInfo viewportStateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
            viewportStateInfo.viewportCount = 1;
            viewportStateInfo.pViewports = &viewport;
            viewportStateInfo.scissorCount = 1;
            viewportStateInfo.pScissors = &scissor;

            graphicsPipelineInfo.pViewportState = &viewportStateInfo;

            //RenderPass
            VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo{};
            pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
            pipelineRenderingInfo.colorAttachmentCount = renderPass->attachments.colorAttachmentVkCount;
            pipelineRenderingInfo.pColorAttachmentFormats = renderPass->attachments.GetColorAttachmentsFormat();
            pipelineRenderingInfo.depthAttachmentFormat = renderPass->attachments.depthStencilFormat;
            pipelineRenderingInfo.stencilAttachmentFormat = renderPass->attachments.GetStencilFormat();

            graphicsPipelineInfo.renderPass = nullptr;
            graphicsPipelineInfo.pNext = &pipelineRenderingInfo;
            // Dynamic states
            VkDynamicState dynamicStates[] = {
                VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS
            };
            VkPipelineDynamicStateCreateInfo dynamic_state{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
            dynamic_state.dynamicStateCount = 3;
            dynamic_state.pDynamicStates = dynamicStates;

            graphicsPipelineInfo.pDynamicState = &dynamic_state;

            vkCreateGraphicsPipelines(m_device, nullptr, 1, &graphicsPipelineInfo, m_allocatorPtr, &pipeline->vkHandle);
            pipeline->vkPipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            //TODO: ADD SUPPORT FOR COMPUTE
            pipeline->bindlessUpdated = false;
        }

        if (creationData.activeLayouts > 0)
        {
            //Create pipeline descriptor set, only handling bindless for now
            CreateDescriptorSets(m_bindlessDescriptorPool, pipeline->vkDescriptorSets,
                                 creationData.activeLayouts, creationData.vkDescriptorSetLayout);

            pipeline->numActiveLayouts = creationData.activeLayouts;
        }
    }

    void PuduGraphics::SubmitFrame(RenderFrameData& frameData)
    {
        auto frame = frameData.frame;


        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

        u64 graphicsWaitValue = 0;
        if (m_absoluteFrame > MAX_FRAMES_IN_FLIGHT)
        {
            graphicsWaitValue = m_absoluteFrame - MAX_FRAMES_IN_FLIGHT - 1;
        }

        VkSemaphoreSubmitInfo waitSemaphores[]{
            {
                VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, m_computeTimelineSemaphore->vkHandle,
                m_computeTimelineSemaphore->TimelineValue(), VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT, 0
            },
            {
                VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, m_graphicsTimelineSemaphore->vkHandle,
                graphicsWaitValue, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0
            },
            {
                VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, *frame->PresentSemaphore, 0,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0
            },
        };

        VkSemaphoreSubmitInfo signalSemaphores[]{
            {
                VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, *m_currentSwapchain.renderCompleteSemaphores[frameData.swapChainIndex], 0,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0
            },
            {
                VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, m_graphicsTimelineSemaphore->vkHandle,
                m_absoluteFrame + 1, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
            }
        };

        VkCommandBufferSubmitInfo commandSubmitInfo;

        std::vector<VkCommandBufferSubmitInfo> commandSubmitInfos;
        for (auto command : frameData.commandsToSubmit)
        {
            VkCommandBufferSubmitInfo commandInfo{};
            commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            commandInfo.commandBuffer = command;

            commandSubmitInfos.push_back(commandInfo);
        }


        submitInfo.waitSemaphoreInfoCount = 3;
        submitInfo.pWaitSemaphoreInfos = waitSemaphores;

        submitInfo.signalSemaphoreInfoCount = 2;
        submitInfo.pSignalSemaphoreInfos = signalSemaphores;

        submitInfo.commandBufferInfoCount = frameData.commandsToSubmit.size();
        submitInfo.pCommandBufferInfos = commandSubmitInfos.data();


        VkResult submitResult = vkQueueSubmit2(m_graphicsQueue, 1, &submitInfo, frame->InFlightFence);
        if (submitResult != VK_SUCCESS)
        {
            LOG("VkERROR: {}", (u32)submitResult);
            throw std::runtime_error("failed to submit draw command buffer!");
        }


        VkSemaphore presentWaitSemaphores[]{
            *m_currentSwapchain.renderCompleteSemaphores[frameData.swapChainIndex]
        };

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = presentWaitSemaphores;


        VkSwapchainKHR swapChains[] = {m_currentSwapchain.swapchainHandle};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &frameData.swapChainIndex;
        presentInfo.pResults = nullptr; // Optional


        auto result = vkQueuePresentKHR(m_presentationQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || FramebufferResized)
        {
            FramebufferResized = false;
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swap chain image!");
        }
    }

    //Do a one-shot compute dispatch. WARNING: This will stall the CPU till the computation is done. For for sequential CS executions prefer to use a compute renderpass
    void PuduGraphics::DispatchCompute(ComputeShaderRenderer* computeShaderRenderer, u32 groupCountX, u32 groupCountY,
                                       u32 groupCountZ)
    {
        auto cmd = BeginSingleTimeCommands();
        auto pipeline = m_resources.GetPipeline(computeShaderRenderer->GetShader()->GetPipelineHandle());

        cmd.BindPipeline(pipeline.get());

        auto material = computeShaderRenderer->GetMaterial();

        material->ApplyProperties(&cmd);
        cmd.BindDescriptorSetCompute(pipeline->vkPipelineLayoutHandle, material->GetDescriptorSets(),
                                     material->GetDescriptorSetsCount());
        cmd.Dispatch(groupCountX, groupCountY, groupCountZ);

        EndSingleTimeComputeCommands(cmd);
    }


    void PuduGraphics::AdvanceFrame()
    {
        //When `MAX_FRAMES_IN_FLIGHT` is a power of 2 you can update the current frame without modulo division
        m_currentFrameIndex = (m_currentFrameIndex + 1) & (MAX_FRAMES_IN_FLIGHT - 1);
        m_absoluteFrame++;
    }

    void PuduGraphics::EndDrawFrame()
    {
        AdvanceFrame();
    }

    void PuduGraphics::CreateVulkanInstance()
    {
        if (enableValidationLayers && !CheckValidationLayerSupport())
        {
            throw std::runtime_error("Validation layers requested, but not available :(");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Pudu Renderer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        u32 glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        //Add validation layers
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        auto extensions = GetInstanceExtensions();

        createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();


        //Enable debug info, this is necessary for spotting problems during create VKCreateInstance and VKDestroyInstance
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, m_allocatorPtr, &m_vkInstance) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create instance!");
        }
        else
        {
            printf("Vulkan instance created successfully\n");
        }
    }

    SPtr<Framebuffer> PuduGraphics::CreateFramebuffer(FramebufferCreationData const& creationData)
    {
        SPtr<Framebuffer> framebuffer = m_resources.AllocateFrameBuffer();

        framebuffer->numColorAttachments = creationData.numRenderTargets;

        for (u32 i = 0; i < creationData.numRenderTargets; i++)
        {
            framebuffer->colorAttachmentHandles[i] = creationData.outputTexturesHandle[i];
        }

        framebuffer->depthStencilAttachmentHandle = creationData.depthStencilTextureHandle;
        framebuffer->width = creationData.width;
        framebuffer->height = creationData.height;
        framebuffer->scaleX = creationData.scaleX;
        framebuffer->scaleY = creationData.scaleY;
        framebuffer->resize = creationData.resize;
        framebuffer->name = creationData.name;
        framebuffer->renderPassHandle = creationData.renderPassHandle;

        auto renderPass = m_resources.GetRenderPass(framebuffer->renderPassHandle);

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass->vkHandle;
        framebufferInfo.width = framebuffer->width;
        framebufferInfo.height = framebuffer->height;
        framebufferInfo.layers = 1;

        VkImageView framebufferAttachments[K_MAX_IMAGE_OUTPUTS + 1]{};
        u32 activeAttachments = 0;
        for (; activeAttachments < framebuffer->numColorAttachments; activeAttachments++)
        {
            auto texture = m_resources.GetTexture<
                RenderTexture>(framebuffer->colorAttachmentHandles[activeAttachments]);
            framebufferAttachments[activeAttachments] = texture->vkImageViewHandle;
        }

        if (framebuffer->depthStencilAttachmentHandle.Index() != k_INVALID_HANDLE)
        {
            auto depthTexture = m_resources.GetTexture<RenderTexture>(framebuffer->depthStencilAttachmentHandle);
            framebufferAttachments[activeAttachments++] = depthTexture->vkImageViewHandle;
        }

        framebufferInfo.attachmentCount = activeAttachments;
        framebufferInfo.pAttachments = framebufferAttachments;

        vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &framebuffer->vkHandle);

        //TODO: SET RESOURCE NAME

        return framebuffer;
    }

    SPtr<GraphicsBuffer> PuduGraphics::CreateGraphicsBuffer(u64 size, void* bufferData, VkBufferUsageFlags usage,
                                                            VkMemoryPropertyFlags flags, const char* name)
    {
        VkDeviceSize bufferSize = size;
        VkBuffer vkBuffer = {};
        VkDeviceMemory vkdeviceMemory = {};
        VmaAllocation bufferAlloc;

        if (bufferData != nullptr)
        {
            VkBuffer stagingBuffer;
            auto stagingAllocation = CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                  VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingBuffer);

            VmaAllocationInfo allocInfo;
            vmaGetAllocationInfo(m_VmaAllocator, stagingAllocation, &allocInfo);
            memcpy(allocInfo.pMappedData, bufferData, (size_t)bufferSize);
            vmaFlushAllocation(m_VmaAllocator, stagingAllocation, 0, VK_WHOLE_SIZE);

            bufferAlloc = CreateBuffer(bufferSize, usage, flags, vkBuffer,
                                       name);


            CopyBuffer(stagingBuffer, vkBuffer, bufferSize);

            vmaDestroyBuffer(m_VmaAllocator, stagingBuffer, stagingAllocation);
        }
        else
        {
            bufferAlloc = CreateBuffer(bufferSize, usage, flags, vkBuffer, name);
        }

        auto graphicsBuffer = m_resources.AllocateGraphicsBuffer();
        graphicsBuffer->vkHandle = vkBuffer;

        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(m_VmaAllocator, bufferAlloc, &allocInfo);
        graphicsBuffer->allocation = bufferAlloc;
        graphicsBuffer->allocationInfo = allocInfo;
        if (name != nullptr)
        {
            graphicsBuffer->name.append(name);
        }

        return graphicsBuffer;
    }

    VkImageView PuduGraphics::CreateImageView(ImageViewCreateData data)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = data.image;
        viewInfo.viewType = data.imageView;
        viewInfo.format = data.format;
        viewInfo.subresourceRange.aspectMask = data.aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = data.baseMipLevel;
        viewInfo.subresourceRange.levelCount = data.levelCount;
        viewInfo.subresourceRange.baseArrayLayer = data.baseArrayLayer;
        viewInfo.subresourceRange.layerCount = data.layerCount;

        VkImageView imageView;
        VKCheck(vkCreateImageView(m_device, &viewInfo, nullptr, &imageView), "failed to create texture image view!");

        if (data.name != nullptr)
        {
            SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE_VIEW, (u64)imageView, data.name);
        }

        return imageView;
    }

    void PuduGraphics::PickPhysicalDevice()
    {
        LOG("PickPhysicalDevice");
        u32 deviceCount = 0;
        vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            LOG_ERROR("Failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            if (IsDeviceSuitable(device))
            {
                m_physicalDevice = device;
                //For now just pick the first suitable device, later we can pick the most fancy one
                VkPhysicalDeviceProperties2 deviceProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
                VkPhysicalDeviceFeatures2 deviceFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

                deviceFeatures.features.vertexPipelineStoresAndAtomics = VK_TRUE;
                deviceFeatures.features.robustBufferAccess = VK_TRUE;

                VkPhysicalDeviceRobustness2FeaturesEXT robustnessFeatures{
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT
                };
                robustnessFeatures.nullDescriptor = VK_TRUE;
                robustnessFeatures.robustBufferAccess2 = VK_TRUE;

                VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
                indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

                VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeature{
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES
                };

                VkPhysicalDeviceSynchronization2Features synchronization2Feature{
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES
                };
                synchronization2Feature.synchronization2 = VK_TRUE;

                void* currentPnext = &indexingFeatures;

                synchronization2Feature.pNext = currentPnext;
                currentPnext = &synchronization2Feature;

                timelineSemaphoreFeature.pNext = currentPnext;
                currentPnext = &timelineSemaphoreFeature;

                robustnessFeatures.pNext = currentPnext;
                currentPnext = &robustnessFeatures;

                deviceFeatures.pNext = currentPnext;

                vkGetPhysicalDeviceProperties2(device, &deviceProperties);
                vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);

                Print("Picked device name %s", deviceProperties.properties.deviceName);
                Print("Device type %i", deviceProperties.properties.deviceType);

                break;
            }
        }

        if (m_physicalDevice == VK_NULL_HANDLE)
        {
            LOG_ERROR("Failed to find a suitable GPU!");
        }
    }

    void PuduGraphics::CreateLogicalDevice()
    {
        LOG("CreateLogicalDevice");
        QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<u32> uniqueQueueFamilies = {
            indices.graphicsFamily.value(),
            indices.presentFamily.value(),
            indices.computeFamily.value(),
            indices.transferFamily.value()
        };

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;

        float queuePriority = 1.0f;

        for (auto queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceVulkan11Features featuresVulkan11{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
        featuresVulkan11.shaderDrawParameters = VK_TRUE;

        VkPhysicalDeviceVulkan12Features featuresVulkan12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
        featuresVulkan12.timelineSemaphore = VK_TRUE;
        featuresVulkan12.descriptorIndexing = VK_TRUE;
        featuresVulkan12.descriptorBindingPartiallyBound = VK_TRUE;
        featuresVulkan12.runtimeDescriptorArray = VK_TRUE;
        featuresVulkan12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        featuresVulkan12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
        featuresVulkan12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
        featuresVulkan12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        featuresVulkan12.separateDepthStencilLayouts = VK_TRUE;
        featuresVulkan12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
        featuresVulkan12.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
        featuresVulkan12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
        featuresVulkan12.hostQueryReset = VK_TRUE;
        featuresVulkan12.scalarBlockLayout = VK_TRUE;

        VkPhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures.features.samplerAnisotropy = VK_TRUE;
        deviceFeatures.features.multiDrawIndirect = VK_TRUE;
        deviceFeatures.features.vertexPipelineStoresAndAtomics = VK_TRUE;

        VkPhysicalDeviceSynchronization2Features syncFeatures{};
        syncFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        syncFeatures.synchronization2 = VK_TRUE;

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES
        };
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;


        deviceFeatures.pNext = &syncFeatures;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        void* currentPNext = nullptr;

        syncFeatures.pNext = currentPNext;
        currentPNext = &syncFeatures;

        dynamicRenderingFeatures.pNext = currentPNext;
        currentPNext = &dynamicRenderingFeatures;

        featuresVulkan12.pNext = currentPNext;
        currentPNext = &featuresVulkan12;

        featuresVulkan11.pNext = currentPNext;
        currentPNext = &featuresVulkan11;

        createInfo.enabledExtensionCount = static_cast<u32>(DeviceExtensionNames.size());
        createInfo.ppEnabledExtensionNames = DeviceExtensionNames.data();

        VkPhysicalDevicePipelineCreationCacheControlFeatures cacheFeatures{};
        cacheFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES;
        cacheFeatures.pipelineCreationCacheControl = true;

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        if (m_physicalDeviceData.SupportsBindless)
        {
            /*m_physicalDeviceData.IndexingFeatures.pNext = currentPNext;

            currentPNext = &m_physicalDeviceData.IndexingFeatures;*/
        }

        deviceFeatures.pNext = currentPNext;
        currentPNext = &deviceFeatures;

        createInfo.pNext = currentPNext;

        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        VkDeviceQueueInfo2 computeInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2};
        computeInfo.queueFamilyIndex = indices.computeFamily.value();
        computeInfo.queueIndex = 0;

        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentationQueue);
        vkGetDeviceQueue2(m_device, &computeInfo, &m_computeQueue);
    }

    void PuduGraphics::CreateSurface()
    {
        if (glfwCreateWindowSurface(m_vkInstance, WindowPtr, m_allocatorPtr, &m_surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface");
        }
    }


    void PuduGraphics::CreateSwapChain(Swapchain& swapchain)
    {
        LOG("CreateSwapChain");

        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);
        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
        u32 queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VKCheck(vkCreateSwapchainKHR(m_device, &createInfo, m_allocatorPtr, &m_currentSwapchain.swapchainHandle),
                "failed to create swap chain!");

        vkGetSwapchainImagesKHR(m_device, m_currentSwapchain.swapchainHandle, &imageCount, nullptr);
        vkGetSwapchainImagesKHR(m_device, m_currentSwapchain.swapchainHandle, &imageCount, swapchain.images);

        m_imageCount = imageCount;
        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

        swapchain.imageCount = imageCount;

        for (u32 i = 0; i < imageCount; i++)
        {
            SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE, (u64)swapchain.images[i],
                            std::format("Swapchain Image {}", i).c_str());
        }

        LOG("Swap chain images created!");
    }

    void PuduGraphics::CreateSwapchainImageViews(Swapchain& swapchain)
    {
        LOG("Create Swapchain Images Views");

        for (size_t i = 0; i < swapchain.imageCount; i++)
        {
            ImageViewCreateData createData;
            createData.image = swapchain.images[i];
            createData.format = m_swapChainImageFormat;
            createData.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
            createData.name = std::format("Swapchain Image View {}", i).c_str();

            auto imageView = CreateImageView(createData);

            swapchain.imageViews[i] = imageView;

            SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE_VIEW, (u64)swapchain.imageViews[i],
                            std::format("Swapchain Image View {}", i).c_str());

            auto handle = m_resources.AllocateRenderTexture();
            auto texture = m_resources.GetTexture<RenderTexture>(handle->Handle());

            texture->vkImageViewHandle = imageView;
            texture->vkImageHandle = swapchain.images[i];
            texture->format = m_swapChainImageFormat;
            texture->width = m_swapChainExtent.width;
            texture->height = m_swapChainExtent.height;
            texture->isSwapChain = true;

            swapchain.textures[i] = texture;
        }

        LOG("Swapchain images created");
    }

    void PuduGraphics::SetResourceName(VkObjectType type, u64 handle, const char* name)
    {
        const VkDebugUtilsObjectNameInfoEXT resourceNameInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = NULL,
            .objectType = type,
            .objectHandle = (u64)handle,
            .pObjectName = name,
        };

        pfn_SetDebugUtilsObjectNameEXT(m_device, &resourceNameInfo);
    }

    void PuduGraphics::InitDebugUtilsObjectName()
    {
        pfn_SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(
            m_device, "vkSetDebugUtilsObjectNameEXT");

        if (pfn_SetDebugUtilsObjectNameEXT == nullptr)
        {
            LOG_ERROR("Debug utils object function not found");
        }
    }


    void PuduGraphics::CreateRenderPass(RenderPass* renderPass)
    {
        auto output = renderPass->attachments;

        VkAttachmentDescription2 colorAttachments[8] = {};
        VkAttachmentReference2 colorAttachmentsRef[8] = {};

        VkAttachmentLoadOp depthLoadOp, stencilLoadOp;
        VkAttachmentStoreOp depthStoreOp = renderPass->writeDepth
                                               ? VK_ATTACHMENT_STORE_OP_STORE
                                               : VK_ATTACHMENT_STORE_OP_NONE;
        VkImageLayout depthInitialLayout;

        switch (output.depthOperation)
        {
        case Load:
            depthLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthInitialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            break;
        case Clear:
            depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        default:
            depthLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        }


        switch (output.stencilOperation)
        {
        case Load:
            stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case Clear:
            stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        default:
            stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
        }

        //Color attachments
        u32 colorAttachmentsCount = 0;
        auto colorRenderPassAttachments = output.GetColorRenderPassAttachments();


        for (; colorAttachmentsCount < colorRenderPassAttachments->size(); colorAttachmentsCount++)
        {
            auto attachment = colorRenderPassAttachments->at(colorAttachmentsCount);

            VkAttachmentLoadOp colorLoadOp;
            VkImageLayout colorInitialLayout;

            colorLoadOp = attachment.loadOperation;
            colorInitialLayout = attachment.layout;

            VkAttachmentDescription2& colorAttachment = colorAttachments[colorAttachmentsCount];
            colorAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
            colorAttachment.format = attachment.resource->format;
            colorAttachment.samples = static_cast<VkSampleCountFlagBits>(attachment.sampleCount);
            colorAttachment.loadOp = colorLoadOp;
            colorAttachment.storeOp = attachment.storeOp;
            colorAttachment.stencilLoadOp = stencilLoadOp;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = colorInitialLayout;
            colorAttachment.finalLayout = attachment.layout;

            VkAttachmentReference2& colorAttachmentRef = colorAttachmentsRef[colorAttachmentsCount];
            colorAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
            colorAttachmentRef.attachment = colorAttachmentsCount;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }


        //Depth attachment
        VkAttachmentDescription2 depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;

        VkAttachmentReference2 depthAttachmentRef{};
        depthAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;

        if (output.depthStencilFormat != VK_FORMAT_UNDEFINED)
        {
            depthAttachment.format = output.depthStencilFormat;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = depthLoadOp;
            depthAttachment.storeOp = depthStoreOp;
            depthAttachment.stencilLoadOp = stencilLoadOp;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = depthInitialLayout;
            depthAttachment.finalLayout = output.depthStencilFinalLayout;

            depthAttachmentRef.attachment = colorAttachmentsCount;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        //Create subpass
        VkSubpassDescription2 subpass{};
        subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        VkAttachmentDescription2 attachments[K_MAX_IMAGE_OUTPUTS + 1]{};

        for (u32 activeAttachmentIndex = 0; activeAttachmentIndex < colorAttachmentsCount; activeAttachmentIndex
             ++)
        {
            attachments[activeAttachmentIndex] = colorAttachments[activeAttachmentIndex];
        }

        subpass.colorAttachmentCount = colorAttachmentsCount;
        subpass.pColorAttachments = colorAttachmentsRef;
        subpass.pDepthStencilAttachment = nullptr;

        u32 depthStencilCount = 0;
        if (output.depthStencilFormat != VK_FORMAT_UNDEFINED)
        {
            attachments[subpass.colorAttachmentCount] = depthAttachment;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            depthStencilCount++;
        }

        //TODO: WE ARE USING DYNAMIC RENDERING SO NO VK RENDERPASSES!
        // VkRenderPassCreateInfo2 renderPassInfo{};
        // renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
        // renderPassInfo.attachmentCount = colorAttachmentsCount + depthStencilCount;
        // renderPassInfo.pAttachments = attachments;
        // renderPassInfo.subpassCount = 1;
        // renderPassInfo.pSubpasses = &subpass;
        //
        // VKCheck(vkCreateRenderPass2(m_device, &renderPassInfo, m_allocatorPtr, &renderPass->vkHandle),
        //         "Failed to create renderpass");

        //  SetResourceName(VK_OBJECT_TYPE_RENDER_PASS, (u64)renderPass->vkHandle, renderPass->name.c_str());
    }

    GPUResourceHandle<ShaderState> PuduGraphics::CreateShaderState(ShaderStateCreationData const& creation)
    {
        auto shaderState = m_resources.AllocateShaderState();
        shaderState->graphicsPipeline = true;
        shaderState->activeShaders = creation.stageCount;
        shaderState->name = creation.name;

        CreateVKShaderState(shaderState.get(), creation);

        return shaderState->Handle();
    }

    void PuduGraphics::
    CreateVKShaderState(ShaderState* shaderState,
                        ShaderStateCreationData const& creation)
    {
        u32 compiledShaders = 0;

        shaderState->graphicsPipeline = true;
        shaderState->activeShaders = creation.stageCount;
        shaderState->name = creation.name;

        for (; compiledShaders < creation.stageCount; compiledShaders++)
        {
            auto stage = creation.stages[compiledShaders];

            VkPipelineShaderStageCreateInfo& shaderStageInfo = shaderState->shaderStageInfo[compiledShaders];
            shaderStageInfo = {};
            shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageInfo.stage = stage.type;
            shaderStageInfo.pName = stage.entryPointName;
            shaderStageInfo.module = CreateShaderModule(stage.code, stage.codeSize, shaderState->name.c_str());
        }
    }

    void PuduGraphics::DestroyTexture(SPtr<Texture> texture)
    {
        if (!texture->IsDestroyed())
        {
            if (texture->isSwapChain)
            {
                return; //Swapchain textures are released by destroyswapchain
            }

            vkDestroyImage(m_device, texture->vkImageHandle, m_allocatorPtr);
            vkDestroyImageView(m_device, texture->vkImageViewHandle, m_allocatorPtr);
            vkDestroySampler(m_device, texture->Sampler.vkHandle, m_allocatorPtr);

            vmaFreeMemory(m_VmaAllocator, texture->vmaAllocation);

            //vkFreeMemory(m_device, texture->vkMemoryHandle, m_allocatorPtr);

            texture->Destroy();
        }
    }

    void PuduGraphics::CreateUniformBuffers()
    {
        m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkDeviceSize const bufferSize = sizeof(UniformBufferObject);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_uniformBuffers[i] = CreateGraphicsBuffer(bufferSize, nullptr, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                       VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                       VMA_ALLOCATION_CREATE_MAPPED_BIT, "UniformBufer");
        }
    }


    VertexAttributeStream GenerateTangents(VertexAttributeStream positionStream, VertexAttributeStream uvStream,
                                           VertexAttributeStream normalStream, std::vector<u32>& indices)
    {
        ASSERT(indices.size() % 3 == 0, "Indices must be a multiple of 3");


        glm::vec3* tan1 = new glm::vec3[positionStream.Count * 2];
        glm::vec3* tan2 = tan1 + positionStream.Count;

        memset(tan1, 0, sizeof(glm::vec3) * positionStream.Count * 2);
        auto positions = positionStream.GetData<vec3>();
        auto uvs = uvStream.GetData<vec2>();
        auto normals = normalStream.GetData<vec3>();

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            uint i0 = indices[i];
            uint i1 = indices[i + 1];
            uint i2 = indices[i + 2];

            vec3 p0 = positions[i0];
            vec3 p1 = positions[i1];
            vec3 p2 = positions[i2];

            vec2 uv0 = uvs[i0];
            vec2 uv1 = uvs[i1];
            vec2 uv2 = uvs[i2];

            float x1 = p1.x - p0.x;
            float x2 = p2.x - p0.x;
            float y1 = p1.y - p0.y;
            float y2 = p2.y - p0.y;
            float z1 = p1.z - p0.z;
            float z2 = p2.z - p0.z;

            float s1 = uv1.x - uv0.x;
            float s2 = uv2.x - uv0.x;
            float t1 = uv1.y - uv0.y;
            float t2 = uv2.y - uv0.y;

            float r = 1.0F / (s1 * t2 - s2 * t1);

            glm::vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                           (t2 * z1 - t1 * z2) * r);
            glm::vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                           (s1 * z2 - s2 * z1) * r);

            tan1[i0] += sdir;
            tan1[i1] += sdir;
            tan1[i2] += sdir;

            tan2[i0] += tdir;
            tan2[i1] += tdir;
            tan2[i2] += tdir;
        }

        // Normalize the tangents for each vertex
        VertexAttributeStream tangentsStream;
        tangentsStream.Count = positionStream.Count;
        tangentsStream.Stride = sizeof(vec4);
        tangentsStream.Attribute.type = VertexAttributeType::TANGENT;
        tangentsStream.Attribute.format = ChannelFormat::R32G32B32A32_SFLOAT;

        auto tangentsData = new vec4[tangentsStream.Count];
        for (size_t i = 0; i < positionStream.Count; i++)
        {
            glm::vec3 t = tan1[i];

            // Gram-Schmidt orthogonalize
            glm::vec3 tangent = normalize(
                t - normals[i] * dot(
                    normals[i], glm::vec3(t.x, t.y, t.z)));

            tangentsData[i].x = tangent.x;
            tangentsData[i].y = tangent.y;
            tangentsData[i].z = tangent.z;

            // Calculate handedness
            tangentsData[i].w = (dot(cross(normals[i], t), tan2[i]) < 0.0F) ? -1.0F : 1.0F;
        }

        delete[] tan1;

        tangentsStream.Data = reinterpret_cast<byte*>(tangentsData);
        return tangentsStream;
    }

    SPtr<Mesh> PuduGraphics::CreateMesh(MeshCreationData const& data)
    {
        auto indices = data.Indices;
        auto mesh = m_resources.AllocateMesh();
        mesh->m_attributes = data.MeshAttributes;
        mesh->m_indices = indices;
        mesh->name = data.Name;
        mesh->m_hasTangents = data.HasTangents;

        return mesh;
    }

    void PuduGraphics::AllocateMeshGPUData(Mesh* mesh)
    {
        SPtr<GraphicsBuffer> indexBuffer = CreateGraphicsBuffer(sizeof(mesh->m_indices[0]) * mesh->m_indices.size(),
                                                                mesh->m_indices.data(),
                                                                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                                VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

        mesh->m_indexBuffer = indexBuffer;

        std::vector<SPtr<GraphicsBuffer>> vertexAttributeStreamBuffers;
        auto meshAttributes = mesh->GetVertexAttributeStreams();
        for (Size i = 0; i < meshAttributes->GetAttributeCount(); i++)
        {
            auto& stream = meshAttributes->GetAttributeStream(i);

            ASSERT(stream.Data != nullptr, "Vertex attribute stream data is null {} {}", mesh->GetName()->c_str(),
                   ToString(stream.Attribute.type));

            SPtr<GraphicsBuffer> streamBuffer = CreateGraphicsBuffer(stream.Stride * stream.Count, stream.Data,
                                                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                                     VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

            vertexAttributeStreamBuffers.push_back(streamBuffer);
        }

        mesh->m_vertexAttributeStreamBuffers = vertexAttributeStreamBuffers;

        if (!mesh->HasTangents())
        {
            auto positionStream = meshAttributes->FindVertexAttributeStream(VertexAttributeType::POSITION);

            auto uvStream = meshAttributes->FindVertexAttributeStream(VertexAttributeType::TEXCOORD0);

            auto normalStream =meshAttributes->FindVertexAttributeStream(VertexAttributeType::NORMAL);

            if (positionStream.has_value() &&
                uvStream.has_value() &&
                normalStream.has_value())
            {
                auto tangentStream = GenerateTangents(*positionStream.value(), *uvStream.value(), *normalStream.value(), mesh->m_indices);

                SPtr<GraphicsBuffer> tangentStreamBuffer = CreateGraphicsBuffer(
                    tangentStream.Stride * tangentStream.Count, tangentStream.Data,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

               meshAttributes->PushVertexAttributeStream(tangentStream);

                mesh->m_vertexAttributeStreamBuffers.push_back(tangentStreamBuffer);

                mesh->m_hasTangents = true;
            }
        }
    }

    void PuduGraphics::CreateBindlessDescriptorPool()
    {
        LOG("Creating desciptor Pool");

        const u32 poolsSizesCount = 4;
        std::array<VkDescriptorPoolSize, poolsSizesCount> poolSizesBindless =
        {
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, k_MAX_BINDLESS_RESOURCES},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, k_MAX_BINDLESS_RESOURCES},
            VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, k_MAX_BINDLESS_RESOURCES),
            VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, k_MAX_BINDLESS_RESOURCES),
        };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT; //support bindless
        poolInfo.maxSets = k_MAX_BINDLESS_RESOURCES * (u32)poolSizesBindless.size();
        poolInfo.poolSizeCount = (u32)poolSizesBindless.size();
        poolInfo.pPoolSizes = poolSizesBindless.data();

        m_physicalDeviceData.PoolSizesCount = poolsSizesCount;

        if (vkCreateDescriptorPool(m_device, &poolInfo, m_allocatorPtr, &m_bindlessDescriptorPool) != VK_SUCCESS)
        {
            LOG_ERROR("Failed to create descriptor pool!");
        }
    }

    //A descriptor set layout is the template of the resources that are needed for a given render pipeline
    GPUResourceHandle<DescriptorSetLayout> PuduGraphics::CreateBindlessDescriptorSetLayout(
        DescriptorSetLayoutInfo& creationData)
    {
        ASSERT(!creationData.Bindings.empty(), "Trying to create descriptorsetlayout with 0 bindings name: {}",
               creationData.name.c_str());

        LOG("CreateDescriptorSetLayout");
        VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

        std::vector<VkDescriptorBindingFlags> bindingFlags;
        bindingFlags.resize(creationData.Bindings.size());

        for (u32 i = 0; i < creationData.Bindings.size(); ++i)
        {
            bindingFlags[i] = bindlessFlags;
        }

        for (auto& binding : creationData.Bindings)
        {
            binding.descriptorCount = k_MAX_BINDLESS_RESOURCES;
        }

        auto descriptorSetLayout = m_resources.AllocateDescriptorSetLayout();

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{};
        extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        extendedInfo.bindingCount = (u32)creationData.Bindings.size();
        extendedInfo.pBindingFlags = bindingFlags.data();

        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = creationData.CreateInfo.bindingCount;
        createInfo.flags = creationData.CreateInfo.flags | VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        createInfo.pBindings = creationData.Bindings.data();
        createInfo.pNext = &extendedInfo;

        VkDescriptorSetLayout layout{};

        VKCheck(vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr, &layout),
                "failed to create descriptor set layout!");

        descriptorSetLayout->vkHandle = layout;

        LOG("CreateDescriptorSetLayout End");

        return descriptorSetLayout->Handle();
    }

    GPUResourceHandle<DescriptorSetLayout> PuduGraphics::CreateDescriptorSetLayout(DescriptorSetLayoutInfo& data)
    {
        auto descriptorSetLayout = m_resources.AllocateDescriptorSetLayout();

        for (auto& binding : data.Bindings)
        {
            binding.descriptorCount = 1;
        }

        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = data.CreateInfo.bindingCount;
        createInfo.pBindings = data.Bindings.data();

        VkDescriptorSetLayout layout{};

        VKCheck(vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr, &layout),
                "failed to create descriptor set layout!");

        descriptorSetLayout->vkHandle = layout;
        descriptorSetLayout->scope = data.scope;
        descriptorSetLayout->name = data.name;
        descriptorSetLayout->setIndex = data.SetNumber;

        return descriptorSetLayout->Handle();
    }

    std::vector<SPtr<GPUCommands>> PuduGraphics::CreateCommandBuffers(GPUCommands::CreationData creationData,
                                                                      const char* name)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool->vkHandle;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (u32)MAX_FRAMES_IN_FLIGHT * 2;

        std::vector<VkCommandBuffer> buffers;
        buffers.resize(MAX_FRAMES_IN_FLIGHT * 2); //Multiply by 2 for compute queue buffer

        if (vkAllocateCommandBuffers(m_device, &allocInfo, buffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffer!");
        }

        std::vector<SPtr<GPUCommands>> commands;
        for (auto buffer : buffers)
        {
            auto gpubuffer = m_resources.AllocateCommandBuffer();
            gpubuffer->vkHandle = buffer;
            gpubuffer->m_graphics = this;

            commands.push_back(gpubuffer);
        }

        if (name != nullptr)
        {
            for (size_t i = 0; i < commands.size(); i++)
            {
                SetResourceName(VK_OBJECT_TYPE_COMMAND_BUFFER, (u64)commands[i]->vkHandle,
                                std::format("{} {}", name, i).c_str());
            }
        }

        return commands;
    }

    void PuduGraphics::CreateDescriptorSets(const VkDescriptorPool pool, VkDescriptorSet* descriptorSet,
                                            const uint16_t setsCount,
                                            const VkDescriptorSetLayout* layouts) const
    {
        LOG("Creating descriptor set");

        if (setsCount == 0)
            return;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.pSetLayouts = layouts;
        allocInfo.descriptorSetCount = setsCount;

        VKCheck(vkAllocateDescriptorSets(m_device, &allocInfo, descriptorSet), "Failed creating descriptor set");

        LOG("Creating descriptor set end");
    }

    void PuduGraphics::CreateDescriptorSets(VkDescriptorSet* descriptorSet, const u16 setsCount,
                                            const VkDescriptorSetLayout* layouts) const
    {
        CreateDescriptorSets(m_bindlessDescriptorPool, descriptorSet, setsCount, layouts);
    }

    PipelineCreationData PuduGraphics::GetPipelineCreationData(Shader* shader, RenderPass* renderPass)
    {
        PipelineCreationData creationData;
        creationData.vertexShaderData = shader->m_vertexData;
        creationData.fragmentShaderData = shader->m_fragmentData;
        creationData.name = renderPass->name.c_str();

        BlendStateCreation blendStateCreation;

        if (shader->HasFragmentData())
        {
            auto renderPassBlendState = shader->OverridesPipelineState()
                                            ? shader->GetBlendState()
                                            : *renderPass->GetBlendState();


            blendStateCreation.AddBlendState()
                              .SetAlphaBlending(renderPassBlendState.sourceAlphaFactor,
                                                renderPassBlendState.destinationAlphaFactor,
                                                renderPassBlendState.alphaBlendOperation)
                              .SetColorBlending(renderPassBlendState.sourceColorFactor,
                                                renderPassBlendState.destinationColorFactor,
                                                renderPassBlendState.colorBlendOperation)
                              .SetColorWriteMask(renderPassBlendState.colorMask);
        }

        auto cullMode = shader->OverridesPipelineState() ? shader->GetCullMode() : renderPass->GetCullMode();

        RasterizationCreation rasterizationCreation;
        rasterizationCreation.cullMode = ToVk(cullMode);
        rasterizationCreation.fill = FillMode::Solid;
        rasterizationCreation.front = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        DepthStencilCreation depthStencilCreation;
        depthStencilCreation.SetDepth(renderPass->writeDepth, VK_COMPARE_OP_LESS_OR_EQUAL);

        auto vertexLayout = shader->m_compilationObject.GetVertexLayout();
        VertexInputCreation vertexInputCreation(vertexLayout);

        for (Size i = 0; i < vertexLayout->GetAttributeCount(); i++)
        {
            auto attribute = vertexLayout->GetAttribute(i);

            VertexStream vertexStream;
            vertexStream.binding = i; //TODO: SUPPORT DYNAMIC STREAM BINDING
            vertexStream.inputRate = VertexInputRate::PerVertex;
            vertexStream.stride = attribute.GetStride();
            vertexInputCreation.PushVertexStream(vertexStream);
        }

        ShaderStateCreationData shaderData;
        shaderData.SetName(shader->name.c_str());

        creationData.pushConstants = shader->m_compilationObject.GetPushConstantsInfo();

        if (shader->HasFragmentData())
        {
            shaderData.AddStage(shader->m_fragmentData, "fragmentMain", shader->m_fragmentDataSize,
                                VK_SHADER_STAGE_FRAGMENT_BIT);
        }

        if (shader->HasVertexData())
        {
            shaderData.AddStage(shader->m_vertexData, "vertexMain", shader->m_vertexDataSize,
                                VK_SHADER_STAGE_VERTEX_BIT);
        }

        creationData.blendState = blendStateCreation;
        creationData.rasterization = rasterizationCreation;
        creationData.depthStencil = depthStencilCreation;
        creationData.vertexInput = vertexInputCreation;
        creationData.shadersStateCreationData = shaderData;

        creationData.renderPassHandle = renderPass->Handle();

        creationData.descriptorSetLayouts = &shader->descriptorSetLayouts;
        creationData.activeLayouts = shader->numActiveLayouts;
        creationData.vkDescriptorSetLayout = shader->GetVkDescriptorSetLayouts();
        creationData.multiSampled = renderPass->IsMultisampled();

        return creationData;
    }

    SPtr<ComputeShader> PuduGraphics::GetHorizonMapToCubeComputeShader()
    {
        return m_horizonToCubeCompute;
    }

    SPtr<Texture> PuduGraphics::GetDefaultWhiteTexture()
    {
        return m_resources.GetTexture<Texture>(m_defaultWhiteTexture);
    }

    SPtr<Texture> PuduGraphics::GetDefaultBlackTexture()
    {
        return m_resources.GetTexture<Texture>(m_defaultBlackTexture);
    }

    SPtr<Texture> PuduGraphics::GetDefaultNormalMapTexture()
    {
        return m_resources.GetTexture<Texture>(m_defaultNormalmapTexture);
    }

    SPtr<Texture> PuduGraphics::GetDefaultMetallicRoughnessTexture()
    {
        return m_resources.GetTexture<Texture>(m_defaultMetallicRoughnessTexture);
    }

    SPtr<Texture> PuduGraphics::GetDefaultCubemapTexture()
    {
        return m_defaultCubemap.Get();
    }

    SPtr<Texture> PuduGraphics::GetPuduTexture()
    {
        return m_defaultPuduTexture.Get();
    }

    SPtr<Shader> PuduGraphics::GetDefaultOverlayShader()
    {
        return m_defaultOverlayShader;
    }

    SPtr<Shader> PuduGraphics::GetDefaultOverlayTextureArrayShader()
    {
        return m_defaultOverlayTextureArrayShader;
    }

    SPtr<Shader> PuduGraphics::GetDefaultStandardShader()
    {
        return m_defaultStandardShader;
    }

    SPtr<Material> PuduGraphics::GetDefaultStandardMaterial()
    {
        return m_defaultStandardMaterial;
    }

    SPtr<Mesh> PuduGraphics::GetDefaultQuad()
    {
        return m_defaultQuad;
    }

    SPtr<Mesh> PuduGraphics::GetDefaultCube()
    {
        return m_defaultCube;
    }

    SPtr<Mesh> PuduGraphics::GetDefaultSphere()
    {
        return m_defaultSphere;
    }

    GPUProfiler* PuduGraphics::GetGPUProfiler()
    {
        ASSERT(m_gpuProfiler->m_context != nullptr, "GPUProfiler not initialized");
        return m_gpuProfiler;
    }

    void PuduGraphics::DestroySemaphore(SPtr<Semaphore> semaphore)
    {
        vkDestroySemaphore(m_device, semaphore->vkHandle, m_allocatorPtr);
    }

    void PuduGraphics::DestroyShader(SPtr<Shader> shader)
    {
        vkDestroyShaderModule(m_device, shader->GetModule(), m_allocatorPtr);
    }


    SPtr<RenderTexture> PuduGraphics::GetRenderTexture()
    {
        auto rt = m_resources.AllocateRenderTexture();
        TextureFlags::Enum v;
        rt->m_flags = static_cast<TextureFlags::Enum>(TextureFlags::RenderTargetMask | TextureFlags::Sample |
            TextureFlags::UnorderedAccess);

        rt->bindless = true;

        return rt;
    }

    void PuduGraphics::DestroyShaderModule(VkShaderModule& state)
    {
        vkDestroyShaderModule(m_device, state, m_allocatorPtr);
    }

    /// <summary>
    /// In Vulkan NOn-Bindless resources shouldn't use a Bindless descriptor so we need to check what kind of descriptor we should create
    /// </summary>
    /// <param name="layoutData"></param>
    /// <param name="out"></param>
    void PuduGraphics::CreateDescriptorsLayouts(std::vector<DescriptorSetLayoutInfo>& layoutsData,
                                                std::vector<GPUResourceHandle<DescriptorSetLayout>>& out)
    {
        for (auto& layoutData : layoutsData)
        {
            if (layoutData.bindless) //
            {
                out.emplace_back(CreateBindlessDescriptorSetLayout(layoutData));
            }
            else
            {
                out.emplace_back(CreateDescriptorSetLayout(layoutData));
            }
        }
    }


    GPUResourceHandle<Pipeline> PuduGraphics::CreateGraphicsPipeline(PipelineCreationData& creationData)
    {
        LOG("CreateGraphicsPipeline Renderpass: {} Shader: {}",
            creationData.renderPassHandle.Get()->name.c_str(),
            creationData.shadersStateCreationData.name.c_str());

        auto pipeline = m_resources.AllocatePipeline();
        auto renderPass = creationData.renderPassHandle.Get();
        pipeline->name = std::format("Pipeline {} {}", renderPass->name, creationData.shadersStateCreationData.name);
        pipeline->m_renderPass = renderPass->Handle();

        CreateVKGraphicsPipeline(pipeline.get(), creationData);

        return pipeline->Handle();
    }

    GPUResourceHandle<Pipeline> PuduGraphics::CreateComputePipeline(ComputePipelineCreationData& creationData)
    {
        auto computeShader = m_resources.GetComputeShader(creationData.computeShaderHandle);
        VkPipelineShaderStageCreateInfo computeShaderStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeShaderStageInfo.module = computeShader->GetModule();
        computeShaderStageInfo.pName = creationData.kernel;

        auto pipeline = m_resources.AllocatePipeline();
        pipeline->vkPipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        pipeline->pipelineType = PipelineType::Compute;

        if (creationData.name != nullptr)
        {
            pipeline->name.append(creationData.name);
        }

        pipeline->numActiveLayouts = creationData.activeLayouts;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutInfo.setLayoutCount = creationData.descriptorSetLayouts->size();
        pipelineLayoutInfo.pSetLayouts = creationData.vkDescriptorSetLayout;

        VKCheck(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pipeline->vkPipelineLayoutHandle),
                "Failed to create compute pipeline layout");

        VkComputePipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        pipelineInfo.layout = pipeline->vkPipelineLayoutHandle;
        pipelineInfo.stage = computeShaderStageInfo;

        if (creationData.activeLayouts > 0)
        {
            CreateDescriptorSets(m_bindlessDescriptorPool, pipeline->vkDescriptorSets,
                                 creationData.activeLayouts, creationData.vkDescriptorSetLayout);
            pipeline->numDescriptorSets = creationData.activeLayouts;
        }

        VKCheck(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline->vkHandle),
                "Failed to create compute pipeline");

        SetResourceName(VK_OBJECT_TYPE_PIPELINE, (u64)pipeline->vkHandle, pipeline->name.c_str());

        return pipeline->Handle();
    }

    void PuduGraphics::CreateFrames()
    {
        m_Frames.resize(MAX_FRAMES_IN_FLIGHT);
    }

    void PuduGraphics::CreateCommandPool(VkCommandPool* cmdPool, u32 familyIndex)
    {
        LOG("CreateCommandPool");

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = familyIndex;

        VKCheck(vkCreateCommandPool(m_device, &poolInfo, nullptr, cmdPool), "Failed to create command pool");
    }


    GPUResourceHandle<Texture> PuduGraphics::CreateTexture(TextureCreationData const& creationData)
    {
        SPtr<Texture> texture = nullptr;
        switch (creationData.textureType)
        {
        case TextureType::Texture2D:
            texture = m_resources.AllocateTexture2D();
            break;
        case TextureType::Texture_Cube:
            texture = m_resources.AllocateTextureCube();
            break;
        case TextureType::Texture_2D_Array:
            texture = m_resources.AllocateTexture2DArray();

        default:
            //LOG_ERROR("Texture Type not supported {}", creationData.textureType);
            break;
        }

        texture->name.append(creationData.name);
        texture->width = creationData.width;
        texture->height = creationData.height;
        texture->format = creationData.format;
        texture->depth = creationData.depth;
        texture->layers = creationData.layers;
        texture->mipLevels = creationData.mipmaps;
        texture->pixels = creationData.pixels;
        texture->sourceData = creationData.sourceData;
        texture->m_flags = creationData.flags;
        texture->bindless = creationData.bindless;
        texture->exposeMipViews = creationData.exposeMipViews;
        texture->useAutoGeneratedMipMaps = creationData.generateMipmaps;

        u32 dataSize = creationData.dataSize;

        if (creationData.dataSize == -1)
        {
            dataSize = creationData.width * creationData.height * 4; //Assume RGBA8
        }

        texture->dataSize = dataSize;
        SamplerCreationData* samplerData = creationData.samplerData;
        samplerData->maxLOD = texture->mipLevels - 1;
        texture->samplerData = samplerData;

        texture->Create(this);

        return texture->Handle();
    }

    GPUResourceHandle<TextureSampler> PuduGraphics::CreateSampler(const SamplerCreationData& creationData)
    {
        auto sampler = m_resources.AllocateSampler();

        CreateVKTextureSampler(creationData, sampler->vkHandle);

        return sampler->Handle();
    }

    void PuduGraphics::CreateVKTexture(Texture* texture)
    {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.flags = ToVkImageFlags(texture->GetTextureType());
        imageCreateInfo.imageType = ToVkImageType(texture->GetTextureType());
        imageCreateInfo.extent.width = texture->width;
        imageCreateInfo.extent.height = texture->height;
        imageCreateInfo.extent.depth = texture->depth;
        imageCreateInfo.mipLevels = texture->mipLevels;
        imageCreateInfo.arrayLayers = texture->GetTextureType() == TextureType::Texture_Cube ? 6 : texture->layers;
        imageCreateInfo.samples = static_cast<VkSampleCountFlagBits>(texture->GetSampleCount());
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.format = texture->format;
        imageCreateInfo.initialLayout = texture->GetImageLayout();

        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        const bool isRenderTarget = (texture->GetFlags() & TextureFlags::RenderTargetMask) ==
            TextureFlags::RenderTargetMask;
        const bool isComputeUsed = texture->GetFlags() & TextureFlags::UnorderedAccess;

        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (texture->useAutoGeneratedMipMaps)
            usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        imageCreateInfo.usage = usage;
        imageCreateInfo.usage |= isComputeUsed ? VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT : 0;

        imageCreateInfo.usage ^= texture->IsMultisampled() ? VK_IMAGE_USAGE_STORAGE_BIT : 0;

        if (TextureFormat::HasDepthOrStencil(texture->format))
        {
            imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT; //TODO: ONLY ENABLE TRANSFER IF NEEDED
            if (isRenderTarget)
            {
                imageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            }
        }
        else
        {
            imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; //Read&write but it might but just for read
            imageCreateInfo.usage |= isRenderTarget ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
        }

        /*if (isRenderTarget)
        {
            imageCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }*/

        if (texture->GetFlags() & TextureFlags::Sample || texture->GetFlags() & TextureFlags::Default)
        {
            imageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        if (texture->GetTextureType() == TextureType::Texture_Cube)
        {
            imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }

        VmaAllocationCreateInfo memoryInfo{};
        memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = texture->mipLevels;
        subresourceRange.layerCount = texture->layers;

        //Allocate image
        vmaCreateImage(m_VmaAllocator, &imageCreateInfo, &memoryInfo, &texture->vkImageHandle, &texture->vmaAllocation,
                       nullptr);

        u32 dataSize = texture->dataSize;

        if (texture->dataSize == -1)
        {
            VmaAllocationInfo2 allocationInfo{};
            vmaGetAllocationInfo2(m_VmaAllocator, texture->vmaAllocation, &allocationInfo);
            dataSize = texture->width * texture->height * 4; //Assume RGBA8
            //dataSize = allocationInfo.allocationInfo.size;
        }

        texture->dataSize = dataSize;

        //Allocate cubemap faces
        std::vector<VkBufferImageCopy2> bufferCopyRegions;
        if (texture->GetTextureType() == TextureType::Texture_Cube)
        {
            subresourceRange.levelCount = texture->mipLevels;
            subresourceRange.layerCount = 6;

            if (texture->sourceData == nullptr)
                goto outCubemap;

            u32 offset = 0;
            for (u32 face = 0; face < 6; face++)
            {
                for (u32 layer = 0; layer < texture->layers; layer++)
                {
                    for (size_t level = 0; level < texture->mipLevels; level++)
                    {
                        //HERE WE ARE ASSUMING KTX FORMAT
                        ktx_size_t offset;
                        KTX_error_code code = ktxTexture_GetImageOffset((ktxTexture*)texture->sourceData, level,
                                                                        layer, face, &offset);
                        assert(code == KTX_SUCCESS);
                        VkBufferImageCopy2 bufferCopyRegion = {VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2};
                        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        bufferCopyRegion.imageSubresource.mipLevel = level;
                        bufferCopyRegion.imageSubresource.baseArrayLayer = face;
                        bufferCopyRegion.imageSubresource.layerCount = 1;
                        bufferCopyRegion.imageExtent.width = texture->width >> level;
                        bufferCopyRegion.imageExtent.height = texture->height >> level;
                        bufferCopyRegion.imageExtent.depth = 1;
                        bufferCopyRegion.bufferOffset = offset;
                        bufferCopyRegions.push_back(bufferCopyRegion);
                    }
                }
            }
        }
    outCubemap:


        //Image view
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = texture->vkImageHandle;
        imageViewInfo.viewType = ToVkImageViewType(texture->GetTextureType());
        imageViewInfo.format = texture->format;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        if (TextureFormat::HasDepthOrStencil(texture->format))
        {
            aspectMask = TextureFormat::HasDepth(texture->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
        }

        subresourceRange.aspectMask = aspectMask;

        imageViewInfo.subresourceRange = subresourceRange;

        vkCreateImageView(m_device, &imageViewInfo, m_allocatorPtr, &texture->vkImageViewHandle);

        SetResourceName(VK_OBJECT_TYPE_IMAGE, (u64)texture->vkImageHandle, texture->name.c_str());

        if (texture->bindless)
        {
            UpdateBindlessTexture(texture->Handle());
        }

        if (texture->pixels != nullptr)
        {
            UploadTextureData(texture, texture->pixels, subresourceRange, &bufferCopyRegions);
        }

        //Exposed mipmap views
        if (texture->exposeMipViews)
        {
            texture->m_mipImageViews[0] = texture->vkImageViewHandle;
            for (int i = 1; i < texture->mipLevels; i++)
            {
                subresourceRange.baseMipLevel = i;
                subresourceRange.levelCount = texture->mipLevels - i;
                imageViewInfo.subresourceRange = subresourceRange;
                vkCreateImageView(m_device, &imageViewInfo, m_allocatorPtr, &texture->m_mipImageViews[i]);
            }
        }

        texture->m_aspectMask = aspectMask;

        LOG("Created Texture {} id: {}", texture->name, texture->Handle().Index());
    }


    void PuduGraphics::UploadTextureData(Texture* texture, void* pixels, VkImageSubresourceRange& range,
                                         std::vector<VkBufferImageCopy2>* regions)
    {
        //TODO: COMPUTE TEXTURE SIZE
        auto imageSize = texture->dataSize;
        SPtr<GraphicsBuffer> stagingBuffer = CreateGraphicsBuffer(imageSize, nullptr, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                                                  | VMA_ALLOCATION_CREATE_MAPPED_BIT);

        VmaAllocationCreateInfo memoryInfo{};
        memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        memcpy(stagingBuffer->GetMappedData(), pixels, static_cast<size_t>(imageSize));
        vmaFlushAllocation(m_VmaAllocator, stagingBuffer->allocation, 0, VK_WHOLE_SIZE);

        auto cmd = BeginSingleTimeCommands();

        cmd.TransitionTextureLayout(texture,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &range);

        cmd.CopyBufferToImage(stagingBuffer->vkHandle, texture->vkImageHandle, static_cast<u32>(texture->width),
                              static_cast<u32>(texture->height), regions);

        if (texture->useAutoGeneratedMipMaps)
            GenerateTextureMipMaps(texture, &cmd);

        cmd.TransitionTextureLayout(texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &range);
        EndSingleTimeCommands(cmd);

        DestroyBuffer(stagingBuffer);
    }

    void PuduGraphics::GenerateTextureMipMaps(Texture* texture, GPUCommands* cmd)
    {
        LOG("Generate MipMaps for {}", texture->name);
        if (texture->mipLevels == 0)
            return;

        u32 mipWidth = texture->width;
        u32 mipHeight = texture->height;

        VkImageMemoryBarrier2 barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        barrier.image = texture->vkImageHandle;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = texture->layers;
        barrier.subresourceRange.levelCount = 1;

        for (u32 i = 1; i < texture->mipLevels; i++)
        {
            u32 srcMip = i - 1;
            u32 dstMip = i;
            barrier.subresourceRange.baseMipLevel = srcMip; //PUT BARRIER ON THE SOURCE IMAGE
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

            cmd->ImageBarrier(&barrier);

            barrier.subresourceRange.baseMipLevel = dstMip; //PUT BARRIER ON THE DST
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

            cmd->ImageBarrier(&barrier);

            VkImageBlit2 blit{VK_STRUCTURE_TYPE_IMAGE_BLIT_2};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {static_cast<i32>(mipWidth), static_cast<i32>(mipHeight), 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = texture->layers;
            blit.srcSubresource.mipLevel = srcMip;

            mipWidth = std::max(mipWidth / 2, 1u);
            mipHeight = std::max(mipHeight / 2, 1u);

            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {static_cast<i32>(mipWidth), static_cast<i32>(mipHeight), 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = texture->layers;
            blit.dstSubresource.mipLevel = dstMip;

            cmd->Blit(texture, texture, VK_FILTER_LINEAR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &blit);
        }

        barrier.subresourceRange.baseMipLevel = texture->mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        //We need to align the mipmap layout with the rest of the levels so we can transition it as a whole later
        cmd->ImageBarrier(&barrier);

        texture->SetImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        //We set it at TRANSFER_SRC during the loop so we need to update its layout
        LOG("End Generating mipmaps");
    }

    //TODO: HANDLE CASE WHEN ANTIALIASING HAS ALREADY BEEN SET
    void PuduGraphics::SetAntiAliasing(const AntialiasingSettings& settings)
    {
        auto resolution = GetResolution();
        m_antialiasingSettings = settings;
        m_multisampledColorTexture = GetRenderTexture();
        m_multisampledColorTexture->m_sampleCount = settings.sampleCount;
        m_multisampledColorTexture->width = resolution.x;
        m_multisampledColorTexture->height = resolution.y;
        m_multisampledColorTexture->name = "Multisampled Texture";
        m_multisampledColorTexture->format = VK_FORMAT_R8G8B8A8_UNORM;
        //TODO: GET A CLEVER WAY OF GETTING THIS FORMAT SINCE MUST MATCH WHATEVER IS GOING TO BE USED ON THE RENDERPASS, SWAPCHAIN FORMAT IS A GOOD CANDIDATE
        m_multisampledColorTexture->SetUsage(ResourceUsage::RENDER_TARGET);
        m_multisampledColorTexture->Create(this);

        m_multisampledDepthTexture = GetRenderTexture();
        m_multisampledDepthTexture->m_sampleCount = settings.sampleCount;
        m_multisampledDepthTexture->width = resolution.x;
        m_multisampledDepthTexture->height = resolution.y;
        m_multisampledDepthTexture->name = "Multisampled depth Texture";
        m_multisampledDepthTexture->format = m_depthFormat;
        m_multisampledDepthTexture->SetUsage(static_cast<ResourceUsage>(DEPTH_WRITE | DEPTH_READ));
        m_multisampledDepthTexture->Create(this);
    }

    void PuduGraphics::CreateTextureImageView(Texture2d& texture2d)
    {
        ImageViewCreateData createData;
        createData.image = texture2d.vkImageHandle;
        createData.format = texture2d.format;
        createData.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

        texture2d.vkImageViewHandle = CreateImageView(createData);
    }

    void PuduGraphics::CreateVKTextureSampler(const SamplerCreationData& data, VkSampler& sampler)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = data.filter;
        samplerInfo.minFilter = data.filter;

        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        if (!data.wrap)
        {
            addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }

        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 1.0f;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = data.maxLOD;

        if (vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            LOG_ERROR("failed to create texture sampler!");
        }
    }

    void PuduGraphics::CreateFramesCommandBuffer()
    {
        GPUCommands::CreationData commandsData;
        commandsData.pool = m_commandPool->vkHandle;
        commandsData.count = (u32)MAX_FRAMES_IN_FLIGHT * 2;

        auto buffers = CreateCommandBuffers(commandsData);

        //We might need to fix the layout
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            Frame& frame = m_Frames[i];

            frame.CommandBuffer = buffers[i * 2];
            frame.ComputeCommandBuffer = buffers[i * 2 + 1];

            SetResourceName(VK_OBJECT_TYPE_COMMAND_BUFFER, (u64)frame.CommandBuffer->vkHandle,
                            std::format("Frame {} cmd: graphics queue", i).c_str());
            SetResourceName(VK_OBJECT_TYPE_COMMAND_BUFFER, (u64)frame.ComputeCommandBuffer->vkHandle,
                            std::format("Frame {} cmd: compute queue", i).c_str());
        }

        LOG("Created command buffer");
    }

    void PuduGraphics::CreateSwapChainSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_Frames[i].PresentSemaphore = CreateSemaphoreSPtr(std::format("Image available {}", i).c_str());
            m_currentSwapchain.renderCompleteSemaphores[i] = (CreateSemaphoreSPtr(std::format("Render Finished {}", i).c_str()));

            vkCreateFence(m_device, &fenceInfo, nullptr, &m_Frames[i].InFlightFence);
        }

        m_graphicsTimelineSemaphore = CreateTimelineSemaphore("GraphicsTimeline");
        m_computeTimelineSemaphore = CreateTimelineSemaphore("ComputeTimeline");
    }

    void PuduGraphics::RecreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(WindowPtr, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwWaitEvents();
            glfwGetFramebufferSize(WindowPtr, &width, &height);

            if (glfwWindowShouldClose(WindowPtr))
            {
                return;
            }
        }

        vkDeviceWaitIdle(m_device);

        CleanupSwapChain(m_currentSwapchain);

        CreateSwapChain(m_currentSwapchain);
        CreateSwapchainImageViews(m_currentSwapchain);
    }

    void PuduGraphics::UpdateUniformBuffer(u32 currentImage)
    {
        //NOT USED ANYMORE SINCE WE ARE PUSHING CONSTANTS FOR NOW
        /*UniformBufferObject ubo = GetUniformBufferObject();
        memcpy(m_uniformBuffers[currentImage].MappedMemory, &ubo, sizeof(ubo));*/
    }

    UniformBufferObject PuduGraphics::GetUniformBufferObject(DrawCall& drawCall)
    {
        UniformBufferObject ubo{};

        ubo.modelMatrix = drawCall.GetModelMatrix();
        return ubo;
    }

    DescriptorSetLayoutsCollection PuduGraphics::CreateDescriptorSetLayoutsFromModule(
        const fs::path& modulePath)
    {
        ShaderCompilationObject compiledModule = m_shaderCompiler.CompileModule(modulePath);

        for (auto& descriptorSetLayoutInfo : compiledModule.descriptorsData.setLayoutInfos)
        {
            auto descriptorSetLayoutHandle = CreateDescriptorSetLayout(descriptorSetLayoutInfo);

            auto descriptorSetLayout = m_resources.GetDescriptorSetLayout(descriptorSetLayoutHandle);

            compiledModule.descriptorsData.m_setLayouts.push_back(descriptorSetLayout);
            compiledModule.descriptorsData.m_vkSetLayouts.push_back(descriptorSetLayout->vkHandle);
        }

        compiledModule.descriptorsData.m_descriptorSetLayoutsCreated = true;
        return compiledModule.descriptorsData;
    }

    DescriptorSetLayoutsCollection PuduGraphics::CreateDescriptorSetLayoutsFromModules(
        const std::vector<fs::path>& modulePaths)
    {
        ShaderCompilationObject compiledModule = m_shaderCompiler.CompileModules(modulePaths);

        for (auto& descriptorSetLayoutInfo : compiledModule.descriptorsData.setLayoutInfos)
        {
            auto descriptorSetLayoutHandle = CreateDescriptorSetLayout(descriptorSetLayoutInfo);

            auto descriptorSetLayout = m_resources.GetDescriptorSetLayout(descriptorSetLayoutHandle);

            compiledModule.descriptorsData.m_setLayouts.push_back(descriptorSetLayout);
            compiledModule.descriptorsData.m_vkSetLayouts.push_back(descriptorSetLayout->vkHandle);
        }

        compiledModule.descriptorsData.m_descriptorSetLayoutsCreated = true;
        return compiledModule.descriptorsData;
    }

    /*SPtr<Shader> PuduGraphics::CreateShader(fs::path fragmentPath, fs::path vertexPath, const char* name)
    {
        auto shader = m_resources.AllocateShader();

        auto m_fragmentData = fragmentPath.empty() ? std::vector<char>() : FileManager::LoadShader(fragmentPath);
        auto m_vertexData = vertexPath.empty() ? std::vector<char>() : FileManager::LoadShader(vertexPath);

        shader->LoadFragmentData(m_fragmentData, "fragmentMain");
        shader->LoadVertexData(m_vertexData, "vertexMain");
        shader->SetName(name);

        SPIRVParser::GetDescriptorSetLayout(shader.get(), shader->m_descriptors);

        return shader;
    }*/

    SPtr<Shader> PuduGraphics::CreateShader(const fs::path& shaderPath, const char* name)
    {
        auto shader = m_resources.AllocateShader();
        shader->SetName(name);
        shader->m_shaderPath = shaderPath;

        const char* fragmentEntryPoint = "fragmentMain";
        const char* vertexEntryPoint = "vertexMain";

        const std::vector<const char*> entryPoints = {fragmentEntryPoint, vertexEntryPoint};
        auto compileData = m_shaderCompiler.Compile(shaderPath.string().c_str(), entryPoints);

        if (compileData.result == ShaderCompilationResult::Failed)
        {
            compileData = m_shaderCompiler.Compile(k_DEFAULT_ERROR_SHADER_PATH.string().c_str(), entryPoints);
        }

        auto fragmentKernel = compileData.GetKernel(fragmentEntryPoint);
        auto vertexKernel = compileData.GetKernel(vertexEntryPoint);
        shader->LoadFragmentData(fragmentKernel->code, fragmentKernel->codeSize, fragmentEntryPoint);
        shader->LoadVertexData(vertexKernel->code, vertexKernel->codeSize, vertexEntryPoint);
        shader->m_compilationObject = compileData;

        CreateDescriptorsLayouts(shader->GetDescriptorSetLayoutsData()->setLayoutInfos,
                                 shader->m_descriptorSetLayoutHandles);

        std::vector<SPtr<DescriptorSetLayout>> layouts;
        for (u32 i = 0; i < shader->m_descriptorSetLayoutHandles.size(); i++)
        {
            layouts.push_back(m_resources.GetDescriptorSetLayout(
                shader->m_descriptorSetLayoutHandles[i]));
        }

        shader->SetDescriptorSetLayouts(layouts);

        shader->numActiveLayouts = layouts.size();

        auto absoluteShaderPath = fs::absolute("Shaders" / shaderPath);
        std::string absoluteShaderPathString = absoluteShaderPath.string();
        std::ranges::transform(absoluteShaderPathString, absoluteShaderPathString.begin(), ::tolower);

        absoluteShaderPath = absoluteShaderPathString;


        auto dependencies = shader->GetDependencies();
        for (Size i = 0; i < dependencies->size(); i++)
        {
            fs::path dependency = fs::path(dependencies->at(i));

            auto absolutePath = fs::absolute("Shaders" / dependency);
            std::string absolutePathString = absolutePath.string();
            std::ranges::transform(absolutePathString, absolutePathString.begin(), ::tolower);
            //    absolutePath = absolutePathString;

            if (LoadedShaders.find(absolutePath) == LoadedShaders.end())
                LoadedShaders[absolutePath] = {};

            if (watchedFiles.find(absolutePath) == watchedFiles.end())
            {
                watchedFiles[absolutePath] = std::make_unique<filewatch::FileWatch<std::string>>(
                    absolutePath.string().c_str(), [this](const filewatch::CallbackInformation<std::string>& info)
                    {
                        if (info.event == filewatch::Event::modified)
                        {
                            LOG("Shader {} has been modified", info.fullPath);

                            auto shadersToReload = LoadedShaders[info.fullPath];
                            for (auto& shaderToReload : shadersToReload)
                            {
                                ReloadShader(shaderToReload.Get().get());
                            }
                        }
                    });
            }
            LoadedShaders[absolutePath].push_back(shader->Handle());
        }

        return shader;
    }

    void PuduGraphics::ReloadShader(Shader* shader)
    {
        m_shadersToReload.push_back(shader->m_handle);
    }

    SPtr<ComputeShader> PuduGraphics::CreateComputeShader(ComputeShaderCreationData& creationData)
    {
        LOG("Creating Compute Shader {}:", creationData.name);
        auto shader = m_resources.AllocateComputeShader();

        auto compiledShader = m_shaderCompiler.Compile(creationData.shaderPath.string().c_str(),
                                                       {creationData.kernel.c_str()}, true);

        const char* kernelName = creationData.kernel.c_str();
        auto kernel = compiledShader.GetKernel(kernelName);
        shader->m_module = CreateShaderModule(kernel->code, kernel->codeSize, creationData.name.c_str());
        shader->m_compilationObject = compiledShader;
        shader->SetKernel(kernelName);

        CreateDescriptorsLayouts(shader->GetDescriptorSetLayoutsData()->setLayoutInfos,
                                 shader->m_descriptorSetLayoutHandles);

        std::vector<SPtr<DescriptorSetLayout>> layouts;
        for (u32 i = 0; i < shader->m_descriptorSetLayoutHandles.size(); i++)
        {
            layouts.push_back(m_resources.GetDescriptorSetLayout(
                shader->m_descriptorSetLayoutHandles[i]));
        }

        shader->SetDescriptorSetLayouts(layouts);

        shader->numActiveLayouts = layouts.size();

        shader->CreatePipeline(this, nullptr);

        return shader;
    }

    void PuduGraphics::UpdateBindlessResources(VkDescriptorSet set, u32 binding)
    {
        VkWriteDescriptorSet bindlessDescriptorWrites[k_MAX_BINDLESS_RESOURCES];
        VkDescriptorImageInfo bindlessImageInfos[k_MAX_BINDLESS_RESOURCES];
        u32 currentWriteIndex = 0;


        for (int i = 0; i < m_bindlessResourcesToUpdate.size(); i++)
        {
            ResourceUpdate& textureToUpdate = m_bindlessResourcesToUpdate[i];

            auto texture = m_resources.GetTexture<Texture2d>({textureToUpdate.handle});
            VkWriteDescriptorSet& descriptorWrite = bindlessDescriptorWrites[currentWriteIndex];
            descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.dstArrayElement = textureToUpdate.handle;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.dstSet = set;

            descriptorWrite.dstBinding = binding;

            auto textureSampler = texture->Sampler;

            VkDescriptorImageInfo& descriptorImageInfo = bindlessImageInfos[currentWriteIndex];
            descriptorImageInfo.sampler = textureSampler.vkHandle;
            descriptorImageInfo.imageView = texture->vkImageViewHandle;
            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            descriptorWrite.pImageInfo = &descriptorImageInfo;

            currentWriteIndex++;
        }

        if (currentWriteIndex)
        {
            vkUpdateDescriptorSets(m_device, currentWriteIndex, bindlessDescriptorWrites, 0, nullptr);
        }
    }

    SPtr<CommandPool> PuduGraphics::GetCommandPool(QueueFamily type)
    {
        auto commandPool = m_resources.AllocateCommandPool();

        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

        u32 familyIndex = 0;

        switch (type)
        {
        case QueueFamily::Graphics:
            familyIndex = queueFamilyIndices.graphicsFamily.value();
            break;
        case QueueFamily::Compute:
            familyIndex = queueFamilyIndices.computeFamily.value();
            break;
        case QueueFamily::Transfer:
            familyIndex = queueFamilyIndices.transferFamily.value();
            break;
        case QueueFamily::Present:
            familyIndex = queueFamilyIndices.presentFamily.value();
            break;
        default:
            throw std::exception("Error: Invalid family type");
            break;
        }

        CreateCommandPool(&commandPool->vkHandle, familyIndex);

        return commandPool;
    }

    SPtr<DescriptorPool> PuduGraphics::GetDescriptorPool(DescriptorPoolCreationData& creationData)
    {
        auto descriptorPool = m_resources.AllocateDescriptorPool();

        VkDescriptorPoolCreateFlags flags = {};


        if (creationData.bindless)
        {
            flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
        }
        else
        {
            flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        }

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = flags;
        pool_info.maxSets = creationData.poolSizes.size();
        pool_info.poolSizeCount = creationData.poolSizes.size();
        pool_info.pPoolSizes = creationData.poolSizes.data();

        VKCheck(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &descriptorPool->vkHandle),
                "Error creation descriptor pool");

        return descriptorPool;
    }

    void PuduGraphics::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                             VkImageLayout newLayout)
    {
        auto commandBuffer = BeginSingleTimeCommands();

        commandBuffer.TransitionImageLayout(image, format, oldLayout, newLayout);

        EndSingleTimeCommands(commandBuffer);
    }

    SPtr<Texture2d> PuduGraphics::LoadTexture2D(fs::path filePath, TextureLoadSettings& settings)
    {
        settings.textureType = TextureType::Texture2D;

        return std::dynamic_pointer_cast<Texture2d>(LoadAndCreateTexture(filePath, settings));
    }

    SPtr<TextureCube> PuduGraphics::LoadTextureCube(fs::path filePath, TextureLoadSettings& settings)
    {
        settings.textureType = TextureType::Texture_Cube;

        return std::dynamic_pointer_cast<TextureCube>(LoadAndCreateTexture(filePath, settings));
    }

    SPtr<TextureCube> PuduGraphics::LoadTextureHorizonAsCube(fs::path filePath, TextureLoadSettings& creationSettings)
    {
        ASSERT(fs::exists(filePath), "Trying to load a non-existent texture {}. Did you check the path is correct?",
               filePath.string());

        creationSettings.textureType = TextureType::Texture2D;
        auto horizonTexture = LoadAndCreateTexture(filePath, creationSettings);

        u32 cubemapResolution = horizonTexture->width / 4;
        SamplerCreationData samplerData;
        TextureCreationData cubemapRTCreationData{};
        cubemapRTCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        cubemapRTCreationData.width = cubemapResolution;
        cubemapRTCreationData.height = cubemapResolution;
        cubemapRTCreationData.textureType = TextureType::Texture_2D_Array;
        cubemapRTCreationData.generateMipmaps = false;
        cubemapRTCreationData.name = "EnvCubeRT";
        cubemapRTCreationData.flags = TextureFlags::UnorderedAccess;
        cubemapRTCreationData.layers = 6;

        cubemapRTCreationData.samplerData = &samplerData;

        auto envCubemapRTHandle = CreateTexture(cubemapRTCreationData);
        auto cubemapRT = Resources()->GetTexture<Texture>(envCubemapRTHandle);

        m_horizonToCubemapCSRenderer.GetMaterial()->SetProperty("material.input", horizonTexture);
        m_horizonToCubemapCSRenderer.GetMaterial()->SetProperty("material.output", cubemapRT);

        DispatchCompute(&m_horizonToCubemapCSRenderer, cubemapResolution / 32, cubemapResolution / 32, 6);

        TextureCreationData cubemapCreationData{};
        cubemapCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        cubemapCreationData.width = cubemapResolution;
        cubemapCreationData.height = cubemapResolution;
        cubemapCreationData.generateMipmaps = false;
        cubemapCreationData.textureType = TextureType::Texture_Cube;
        cubemapCreationData.name = horizonTexture->name.c_str();
        cubemapCreationData.layers = 6;
        cubemapCreationData.exposeMipViews = true;
        cubemapCreationData.samplerData = &samplerData;
        cubemapCreationData.mipmaps = creationSettings.generateMipmaps
                                          ? Texture::CalculateMipLevels(cubemapResolution, cubemapResolution)
                                          : 1;

        auto cubemapHandle = CreateTexture(cubemapCreationData);
        auto cubeMap = Resources()->GetTexture<TextureCube>(cubemapHandle);
        auto cmd = BeginSingleTimeCommands();

        cmd.Blit(cubemapRT, cubeMap);
        if (creationSettings.generateMipmaps)
        {
            cubeMap->useAutoGeneratedMipMaps = true;
            GenerateTextureMipMaps(cubeMap.get(), &cmd);
        }

        EndSingleTimeCommands(cmd);


        DestroyTexture(horizonTexture);
        DestroyTexture(cubemapRT);

        return cubeMap;
    }


    void PuduGraphics::CopyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height,
                                         std::vector<VkBufferImageCopy2>* regions)
    {
        auto commandBuffer = BeginSingleTimeCommands();

        commandBuffer.CopyBufferToImage(buffer, image, width, height, regions);

        EndSingleTimeCommands(commandBuffer);
    }

    SPtr<Texture> PuduGraphics::LoadAndCreateTexture(fs::path path, TextureLoadSettings& settings)
    {
        if (m_loadedTexturesMap.contains(path))
        {
            return m_loadedTexturesMap[path];
        }

        ASSERT(fs::exists(path), "Trying to load a non-existent texture {}. Did you check the path is correct?",
               path.string());
        ASSERT(settings.name != nullptr, "Trying to load a texture without a name! Be sure to set settings.name");

        bool isKTX = path.extension() == ".ktx" || path.extension() == ".ktx2";
        void* pixelsData = nullptr;
        void* sourceData = nullptr;

        int texWidth, texHeight, texChannels = 0;
        int depth = 1;
        u32 dataSize = -1;
        u32 layers = 1;
        u32 levels = 1;

        TextureType::Enum textureType = settings.textureType;


        ktxTexture* ktxTexture;
        if (isKTX)
        {
            auto result = ktxTexture_CreateFromNamedFile(FileManager::GetAssetPath(path).string().c_str(),
                                                         KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

            sourceData = ktxTexture;

            pixelsData = ktxTexture->pData;

            texWidth = ktxTexture->baseWidth;
            texHeight = ktxTexture->baseHeight;
            depth = ktxTexture->baseDepth;
            layers = ktxTexture->numLayers;
            levels = ktxTexture->numLevels;
            dataSize = ktxTexture->dataSize;

            if (ktxTexture->isCubemap)
            {
                textureType = TextureType::Texture_Cube;
            }
        }
        else
        {
            pixelsData = stbi_load(FileManager::GetAssetPath(path).string().c_str(), &texWidth, &texHeight,
                                   &texChannels, STBI_rgb_alpha);

            dataSize = texWidth * texHeight * 4; //TODO: Assume 32bit Compute with mipmap
        }

        if (settings.generateMipmaps)
        {
            levels = settings.mipLevels > 0
                         ? settings.mipLevels
                         : Texture::CalculateMipLevels(texWidth, texHeight);
        }

        TextureCreationData creationData;
        creationData.bindless = settings.bindless;
        creationData.depth = depth;
        creationData.width = texWidth;
        creationData.height = texHeight;
        creationData.name = settings.name;
        creationData.format = settings.format;
        creationData.generateMipmaps = settings.generateMipmaps;
        creationData.mipmaps = levels;
        creationData.flags = TextureFlags::Sample;
        creationData.textureType = textureType;
        creationData.dataSize = dataSize;
        creationData.layers = layers;
        creationData.pixels = pixelsData;
        creationData.sourceData = sourceData;
        creationData.samplerData = &settings.samplerData;

        auto textureHandle = CreateTexture(creationData);

        if (isKTX)
        {
            ktxTexture_Destroy(ktxTexture);
        }
        else
        {
            stbi_image_free(pixelsData);
        }


        auto texture = m_resources.GetTexture<Texture>(textureHandle);
        m_loadedTexturesMap[path] = texture;

        return texture;
    }

    int2 PuduGraphics::GetResolution() const
    {
        return int2(WindowWidth, WindowHeight);
    }

    QueueFamilyIndices PuduGraphics::GetQueueFamiliesIndex()
    {
        return FindQueueFamilies(m_physicalDevice);
    }

    void PuduGraphics::DestroyRenderPass(SPtr<RenderPass> handle)
    {
        vkDestroyRenderPass(m_device, handle->vkHandle, nullptr);
    }

    void PuduGraphics::DestroyFrameBuffer(SPtr<Framebuffer> handle)
    {
        vkDestroyFramebuffer(m_device, handle->vkHandle, nullptr);
    }

    Model PuduGraphics::CreateModel(std::shared_ptr<Mesh> mesh, const SPtr<Material> material)
    {
        Model model;

        std::vector<std::shared_ptr<Mesh>> meshes{mesh};
        std::vector<SPtr<Material>> materials{material};
        model.Meshes = meshes;
        model.Materials = materials;

        return model;
    }

    Model PuduGraphics::CreateModel(MeshCreationData const& data)
    {
        auto mesh = CreateMesh(data);

        auto material = CreateMaterial();
        material->SetShader(m_defaultStandardShader);
        material->name = data.Name;
        const std::filesystem::path path = data.Material.BaseTexturePath;
        if (data.Material.hasBaseTexture)
        {
            TextureLoadSettings settings{};
            settings.bindless = true;
            settings.name = "Albedo";
            settings.samplerData.wrap = true;

            material->SetProperty("material.albedoTex", LoadTexture2D(path, settings));
        }
        else
        {
            material->SetProperty("material.albedoTex", GetDefaultWhiteTexture());
        }

        if (data.Material.hasNormalMap)
        {
            int texWidth, texHeight, texChannels;

            TextureLoadSettings normalCreation;
            normalCreation.bindless = true;
            normalCreation.name = "Normal";
            normalCreation.format = VK_FORMAT_R8G8B8A8_UNORM;

            material->SetProperty("material.normalTex", LoadTexture2D(data.Material.NormalMapPath, normalCreation));
        }
        else
        {
            material->SetProperty("material.normalTex", GetDefaultNormalMapTexture());
        }
        if (data.Material.hasMetallicRoughness)
        {
            TextureLoadSettings metallicRoughnessCreation;
            metallicRoughnessCreation.bindless = true;
            metallicRoughnessCreation.name = "Roughness";
            metallicRoughnessCreation.format = VK_FORMAT_R8G8B8A8_UNORM;

            material->SetProperty("material.metallicRoughnessTex",
                                  LoadTexture2D(data.Material.MetallicRoughnessPath, metallicRoughnessCreation));
        }
        else
        {
            material->SetProperty("material.metallicRoughnessTex", GetDefaultMetallicRoughnessTexture());
        }

        if (data.Material.hasEmissiveTexture)
        {
            TextureLoadSettings emissiveCreation;
            emissiveCreation.bindless = true;
            emissiveCreation.name = "Emissive";
            emissiveCreation.format = VK_FORMAT_R8G8B8A8_UNORM;

            material->SetProperty("material.emissiveTex",
                                  LoadTexture2D(data.Material.EmissivePath, emissiveCreation));
        }
        else
        {
            material->SetProperty("material.emissiveTex", GetDefaultBlackTexture());
        }

        material->SetProperty("material.heightTex", GetDefaultBlackTexture());

        auto m = CreateModel(mesh, material);
        m.Name = data.Name;
        return m;
    }

    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

    VkFormat PuduGraphics::FindDepthFormat()
    {
        return FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }


    VkFormat PuduGraphics::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                               VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        LOG_ERROR("Failed to find supported format!");

        return VK_FORMAT_UNDEFINED;
    }

    void PuduGraphics::ReloadPendingShaders()
    {
        for (auto shaderHandle : m_shadersToReload)
        {
            auto shader = shaderHandle.Get();

            const char* fragmentEntryPoint = "fragmentMain";
            const char* vertexEntryPoint = "vertexMain";

            const std::vector entryPoints = {fragmentEntryPoint, vertexEntryPoint};
            auto compileData = m_shaderCompiler.Compile(shader->m_shaderPath.string().c_str(), entryPoints);

            if (compileData.result == ShaderCompilationResult::Failed)
            {
                compileData = m_shaderCompiler.Compile(k_DEFAULT_ERROR_SHADER_PATH.string().c_str(), entryPoints);
            }

            auto fragmentKernel = compileData.GetKernel(fragmentEntryPoint);
            auto vertexKernel = compileData.GetKernel(vertexEntryPoint);

            shader->LoadFragmentData(fragmentKernel->code, fragmentKernel->codeSize, fragmentEntryPoint);
            shader->LoadVertexData(vertexKernel->code, vertexKernel->codeSize, vertexEntryPoint);
            shader->m_compilationObject = compileData;

            for (auto& pipelineHandle : shader->m_pipelines)
            {
                auto pipeline = pipelineHandle.Get();
                auto renderPass = pipeline->m_renderPass.Get();

                auto shaderState = pipeline->shaderState.Get();

                for (Size i = 0; i < shaderState->activeShaders; i++)
                {
                    vkDestroyShaderModule(m_device, shaderState->shaderStageInfo[i].module, nullptr);
                }

                vkDestroyPipeline(m_device, pipeline->vkHandle, nullptr);

                auto newPipelineData = GetPipelineCreationData(shader.get(), renderPass.get());
                CreateVKGraphicsPipeline(pipeline.get(), newPipelineData);
            }
        }

        m_shadersToReload.clear();
    }

    GPUCommands PuduGraphics::BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool->vkHandle;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return GPUCommands(commandBuffer, this);
    }

    SPtr<Semaphore> PuduGraphics::CreateSemaphoreSPtr(const char* name)
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        auto semaphore = m_resources.AllocateSemaphore();

        vkCreateSemaphore(m_device, &semaphoreInfo, m_allocatorPtr, &semaphore->vkHandle);

        if (name != nullptr)
        {
            SetResourceName(VK_OBJECT_TYPE_SEMAPHORE, (u64)semaphore->vkHandle, name);
        }

        return semaphore;
    }

    SPtr<Semaphore> PuduGraphics::CreateTimelineSemaphore(const char* name)
    {
        VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VkSemaphoreTypeCreateInfo semaphoreTypeInfo{VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO};
        semaphoreTypeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

        semaphoreInfo.pNext = &semaphoreTypeInfo;

        auto semaphore = m_resources.AllocateSemaphore();

        vkCreateSemaphore(m_device, &semaphoreInfo, m_allocatorPtr, &semaphore->vkHandle);

        if (name != nullptr)
        {
            SetResourceName(VK_OBJECT_TYPE_SEMAPHORE, (u64)semaphore->vkHandle, name);
        }

        return semaphore;
    }

    void PuduGraphics::EndSingleTimeCommands(GPUCommands commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer.vkHandle);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer.vkHandle;

        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_graphicsQueue);

        vkFreeCommandBuffers(m_device, m_commandPool->vkHandle, 1, &commandBuffer.vkHandle);
    }

    void PuduGraphics::EndSingleTimeComputeCommands(GPUCommands commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer.vkHandle);
        RenderFrameData frameData;
        frameData.computeCommandsToSubmit.push_back(&commandBuffer);
        SubmitComputeWork(frameData);

        //Wait for compute work to complete
        u64 computeTimelineValue = m_computeTimelineSemaphore->TimelineValue();
        VkSemaphore semaphores[]{m_computeTimelineSemaphore->vkHandle};
        u64 timelineValues[]{computeTimelineValue};

        VkSemaphoreWaitInfo waitInfo{VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO};
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = semaphores;
        waitInfo.pValues = timelineValues;

        vkWaitSemaphores(m_device, &waitInfo, UINT64_MAX);
        vkFreeCommandBuffers(m_device, m_commandPool->vkHandle, 1, &commandBuffer.vkHandle);
    }

    VkShaderModule PuduGraphics::CreateShaderModule(const u32* code, Size size, const char* name)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = size;
        createInfo.pCode = code;

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module!");
        }

        if (name != nullptr)
        {
            SetResourceName(VK_OBJECT_TYPE_SHADER_MODULE, (u64)shaderModule, name);
        }

        return shaderModule;
    }

    SwapChainSupportDetails PuduGraphics::QuerySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

        u32 formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

        if (formatCount > 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
        }

        u32 presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

        if (presentModeCount > 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount,
                                                      details.presentModes.data());
        }

        return details;
    }

    bool PuduGraphics::IsDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties2 deviceProperties{};
        deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

        void* currentPNext = nullptr;

        VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

        currentPNext = &indexingFeatures;

        VkPhysicalDeviceMultiDrawFeaturesEXT multiDrawFeatures;

        //This feature is to simplify barriers and semaphore
        VkPhysicalDeviceSynchronization2Features synchronization2Features{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES
        };
        synchronization2Features.synchronization2 = VK_TRUE;

        VkPhysicalDeviceTimelineSemaphoreFeatures semaphoreFeatures{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES
        };


        synchronization2Features.pNext = currentPNext;
        currentPNext = &synchronization2Features;
        semaphoreFeatures.pNext = currentPNext;
        currentPNext = &semaphoreFeatures;


        VkPhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures.pNext = currentPNext;

        vkGetPhysicalDeviceProperties2(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);

        QueueFamilyIndices indices = FindQueueFamilies(device);

        bool bindlessSupported =
            indexingFeatures.descriptorBindingPartiallyBound &&
            indexingFeatures.runtimeDescriptorArray;

        if (deviceProperties.properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            return false; //Force not using integrated for now
        }

        bool extensionsSupported = CheckDeviceExtensionSupport(device);
        bool swapChainAdequate = false;

        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        // These are required to support the 4 descriptor binding flags we use in this sample.
        indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        // Enables use of runtimeDescriptorArrays in SPIR-V shaders.
        indexingFeatures.runtimeDescriptorArray = VK_TRUE;

        LOG("Bindless suported {}", bindlessSupported);
        m_physicalDeviceData.IndexingFeatures = indexingFeatures;
        m_physicalDeviceData.SupportsBindless = bindlessSupported;
        m_physicalDeviceData.features = deviceFeatures;

        LOG("Multidraw indirect supported {}", deviceFeatures.features.multiDrawIndirect);

        return indices.IsComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.features.
            samplerAnisotropy && bindlessSupported;
    }

    bool PuduGraphics::CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        u32 extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(DeviceExtensionNames.begin(), DeviceExtensionNames.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices PuduGraphics::FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            if (queueFamily.queueFlags * VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                indices.computeFamily = i;
            }
            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                indices.transferFamily = i;
            }

            if (indices.IsComplete())
                break;

            i++;
        }

        return indices;
    }

    void PuduGraphics::DestroyDescriptorSetLayout(DescriptorSetLayout& descriptorset)
    {
        vkDestroyDescriptorSetLayout(m_device, descriptorset.vkHandle, m_allocatorPtr);
    }

    void PuduGraphics::DestroyCommandPool(CommandPool* commandPool)
    {
        vkDestroyCommandPool(m_device, commandPool->vkHandle, m_allocatorPtr);
    }


    void PuduGraphics::Cleanup()
    {
        if (!m_initialized)
        {
            return;
        }
        CleanupSwapChain(m_currentSwapchain);

        m_resources.DestroyAllResources(this);

        vkDestroyDescriptorPool(m_device, m_bindlessDescriptorPool, m_allocatorPtr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyFence(m_device, m_Frames[i].InFlightFence, m_allocatorPtr);
        }

        if (enableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
        }


        vkDestroySurfaceKHR(m_vkInstance, m_surface, m_allocatorPtr); //Be sure to destroy surface before instance

        vkDestroyDevice(m_device, m_allocatorPtr);

        vkDestroyInstance(m_vkInstance, nullptr);

        glfwDestroyWindow(WindowPtr);

        glfwTerminate();

        m_initialized = false;

        delete m_gpuProfiler;
    }

    void PuduGraphics::CleanupSwapChain(Swapchain& swapchain)
    {
        vkDestroySwapchainKHR(m_device, swapchain.swapchainHandle, nullptr);

        for (Size i = 0; i < swapchain.imageCount; i++)
        {
            vkDestroyImageView(m_device, swapchain.imageViews[i], m_allocatorPtr);
        }

        swapchain.imageCount = 0;
    }

    void PuduGraphics::DestroyMesh(SPtr<Mesh> mesh)
    {
        if (!mesh->IsDisposed())
        {
            DestroyBuffer(mesh->GetIndexBuffer());
            mesh->Destroy();
        }
    }

    void PuduGraphics::WaitIdle()
    {
        vkDeviceWaitIdle(m_device);
    }
}
