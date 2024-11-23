#pragma once

#include <sstream>
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>

#include <Resources/ResourcesPool.h>
#include <PuduGraphics.h>
#include <GPUCommands.h>
#include "FrameGraph/RenderPass.h"

namespace Pudu {

	struct RenderFrameData;
	class PuduGraphics;

	enum FrameGraphResourceType {
		FrameGraphResourceType_Invalid = -1,
		FrameGraphResourceType_Buffer = 0,
		FrameGraphResourceType_Texture = 1, //Represents rt to write?
		FrameGraphResourceType_Attachment = 2,
		FrameGraphResourceType_Reference = 3 //If reference, we output the resource and link the next node to this output

	};


	VkAttachmentLoadOp GetVkAttachmentLoadOp(RenderPassOperation op);

	struct FrameGraphResource {

		enum SizeType {
			SCREEN,
			SPECIFIC
		};

		FrameGraphResourceHandle resourceHandle;
		FrameGraphResourceType type;
		std::string name;

		/// <summary>
		/// Resources are laying somewhere in the app, don't need to be produces by a previous node
		/// </summary>
		bool external = false;
		bool allocated = false;


		size_t size;
		VkBufferUsageFlags  flags;
		BufferHandle buffer;
		uint32_t width;
		uint32_t height;
		uint32_t depth;

		VkFormat format;
		RenderPassOperation loadOp;
		TextureType::Enum textureType;

		TextureHandle textureHandle = { k_INVALID_HANDLE };
		SizeType sizeType = SizeType::SPECIFIC;
	};


	struct NodeEdgeHandle {
		uint32_t index;
	};

	/// <summary>
	/// Defines an Input or Output of a Node. Determines the use of the resource for a given Node. 
	/// Are used to define Edges between nodes
	/// </summary>
	struct NodeEdge {
		FrameGraphResourceHandle resource;

		/// <summary>
		/// Stores the parent resource
		/// </summary>
		FrameGraphNodeHandle from;

		/// <summary>
		/// Reference to the node that outputs the resource
		/// This will be used to determine the edges of the graph
		/// </summary>
		FrameGraphNodeHandle to;

		NodeEdgeHandle handle;

		/// <summary>
		/// Used to check whether or not the resource can be aliased, not implemented for now
		/// </summary>
		int32_t RefCount = 0;
		bool allocated = false;

		bool isDepth;
	};


	struct FrameGraphResourceCreateInfo
	{
		FrameGraphResourceHandle resourceHandle;
		FrameGraphResourceType type;
		std::string name;
		bool isDepth;

		/// <summary>
		/// Resources are laying somewhere in the app, don't need to be produces by a previous node
		/// </summary>
		bool external = false;

		size_t size;
		VkBufferUsageFlags  flags;
		BufferHandle buffer;
		uint32_t width;
		uint32_t height;
		uint32_t depth;

		VkFormat format;
		RenderPassOperation loadOp;
		TextureType::Enum textureType;

		TextureHandle textureHandle = { k_INVALID_HANDLE };
		FrameGraphResource::SizeType sizeType = FrameGraphResource::SizeType::SPECIFIC;
	};


	struct FrameGraphNodeCreation
	{
		std::vector<FrameGraphResourceHandle>  inputs;
		std::vector<FrameGraphResourceHandle> outputs;

		RenderPassHandle renderPass;
		RenderPassType renderType;

		bool enabled;
		bool isCompute = false;

		char const* name;
	};

	struct FrameGraphNode {
		int32_t RefCount = 0;

		RenderPassHandle  renderPass;
		FramebufferHandle framebuffer;
		FrameGraphNodeHandle nodeHandle;

		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;

		//Edges represent nodes this node is connected TO
		std::vector<NodeEdgeHandle> outputEdges;
		std::vector<NodeEdgeHandle> inputEdges;
		RenderPassType type;
		bool isCompute;

		bool enabled = true;

		std::string name;
	};

	struct FrameGraphRenderPassCache {
		void Init();
		void Shutdown();

		std::unordered_map<uint64_t, RenderPass*> renderPassMap;
	};

	struct FrameGraphResourceCache {
		void Init(PuduGraphics* device);
		void Shutdown();

		PuduGraphics* device;

		/// <summary>
		/// First: resource name hash
		/// Second: resource handle
		/// ONLY output resources are handled by this map during <CreateNodeOutput>
		/// </summary>
		std::unordered_map<std::string, uint32_t> resourcesMap;
		ResourcePool<FrameGraphResource> resources;

		bool AddResourceToCache(const char* name, FrameGraphResourceHandle resourceHandle);
	};

	struct FrameGraphNodeCache {
		void Init(PuduGraphics* device);
		void Shutdown();

		PuduGraphics* device;

		std::unordered_map<std::string, uint32_t> nodeMap;
		ResourcePool<FrameGraphNode> nodes;
		ResourcePool<NodeEdge> nodeEdges;
	};

	struct FrameGraphBuilder {
		void Init(PuduGraphics* device);
		void Shutdown();

		FrameGraphResourceHandle CreateOrGetFrameGraphResource(const FrameGraphResourceCreateInfo& creation);
		FrameGraphNodeHandle CreateNode(const FrameGraphNodeCreation& creation);

		FrameGraphNode* GetNode(std::string name);
		FrameGraphNode* GetNode(FrameGraphNodeHandle nodeHandle);
		NodeEdge* GetNodeEdge(NodeEdgeHandle textureHandle);
		NodeEdgeHandle CreateNodeEdge(FrameGraphNodeHandle from, FrameGraphNodeHandle to, FrameGraphResourceHandle resourceHandle);

		FrameGraphResource* GetResource(FrameGraphResourceHandle textureHandle);
		FrameGraphResource* GetResource(std::string name);

		FrameGraphResourceCache resourceCache;
		FrameGraphNodeCache nodeCache;
		FrameGraphRenderPassCache renderPassCache;

		PuduGraphics* graphics;

		static constexpr uint32_t            K_MAX_RENDER_PASS_COUNT = 256;
		static constexpr uint32_t            K_MAX_RESOURCES_COUNT = 1024;
		static constexpr uint32_t            K_MAX_NODES_COUNT = 1024;
	};

	class FrameGraph {
	public:
		void Init(FrameGraphBuilder* builder);

		void RenderFrame(RenderFrameData& renderData);
		/// <summary>
		/// Load file from filePath and create frame graph nodes
		/// </summary>
		/// <param name="file_path"></param>
		void Parse(std::filesystem::path path);

		/// <summary>
		/// Sorts nodes topologically based on their dependencies. Input/Output. Allocate resources handles
		/// </summary>
		void Compile();


		void Reset();
		void AllocateRequiredResources();
		void AllocateResource(FrameGraphResourceHandle handle);
		void EnableRenderPass(char* renderPassName);
		void DisableRenderPass(char* renderPassName);
		void OnResize(PuduGraphics& gpu, uint32_t width, uint32_t height);
		std::string ToString();

		void AttachRenderPass(RenderPass renderPass, RenderPassType type);

		FrameGraphNode* GetNode(char* name);
		FrameGraphNode* GetNode(FrameGraphNodeHandle nodeHandle);

		NodeEdge* GetNodeEdge(NodeEdgeHandle textureHandle);
		FrameGraphResource* GetResource(FrameGraphResourceHandle textureHandle);
		FrameGraphResourceHandle AddResource(FrameGraphResourceCreateInfo createInfo);

		///TODO: WE SHOULD BE ABLE TO CREATE THE NODES BY ADDING THE SPECIFYING THE INPUT/OUTPUT RESOURCES
		FrameGraphNodeHandle CreateNode(FrameGraphNodeCreation& creationData);

		std::vector<FrameGraphNodeHandle> nodes;

		FrameGraphBuilder* builder;

		const char* name = nullptr;
	};
}
