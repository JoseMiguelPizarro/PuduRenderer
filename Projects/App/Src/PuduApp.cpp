#include "PuduApp.h"
#include <PuduGlobals.h>
#include <stdexcept>


void PuduApp::Run()
{
	InitWindow();
	InitVulkan();
	MainLoop();
	Cleanup();
}

void PuduApp::InitWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_windowPtr = glfwCreateWindow(WindowWidth, WindowHeight, RENDERER_NAME, nullptr, nullptr);
}

bool PuduApp::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
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

void PuduApp::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	
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

void PuduApp::SetupDebugMessenger()
{
	if (!enableValidationLayers)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_vkInstance,&createInfo, m_allocatorPtr, &m_debugMessenger)!= VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug messenger!");
	}
}

std::vector<const char*> PuduApp::GetRequiredExtensions()
{
	uint32_t glfwExtensionsCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void PuduApp::InitVulkan()
{
	CreateVulkanInstance();
}

void PuduApp::CreateVulkanInstance() {

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
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	//Add validation layers
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	auto extensions = GetRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();


	//Enable debug info, this is necessary for spotting problems during create VKCreateInstance and VKDestroyInstance
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t> (validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)& debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, m_allocatorPtr, &m_vkInstance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create instance!");
	}
	else
	{
		printf("Vulkan instance created successfully\n");
	}
}

void PuduApp::MainLoop()
{
	while (!glfwWindowShouldClose(m_windowPtr))
	{
		glfwPollEvents();
	}
}

void PuduApp::Cleanup()
{
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, m_allocatorPtr);
	}

	glfwDestroyWindow(m_windowPtr);

	vkDestroyInstance(m_vkInstance, m_allocatorPtr);

	glfwTerminate();
}

