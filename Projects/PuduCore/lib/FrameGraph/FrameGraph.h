#pragma once

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

	struct FrameGraphResourceInfo {
		/// <summary>
		/// Resources are laying somewhere in the app, don't need to be produces by a previous node
		/// </summary>
		bool external = false;

		union {
			struct
			{
				size_t size;
				VkBufferUsageFlags  flags;
				BufferHandle buffer;
			} buffer;

			struct {
				uint32_t width;
				uint32_t height;
				uint32_t depth;

				VkFormat format;
				VkImageUsageFlags flags;
				RenderPassOperation loadOp;
				TextureType::Enum textureType;

				TextureHandle handle;
			} texture;
		};
	};



	/// <summary>
	/// Defines an Input or Output of a Node. Determines the use of the resource for a given Node. 
	/// Are used to define Edges between nodes
	/// </summary>
	struct FrameGraphResource {
		FrameGraphResourceType type;
		FrameGraphResourceInfo resourceInfo;

		/// <summary>
		/// Reference to the node that outputs the resource
		/// This will be used to determine the edges of the graph
		/// </summary>
		FrameGraphNodeHandle producer;

		/// <summary>
		/// Stores the parent resource
		/// </summary>
		FrameGraphResourceHandle outputHandle;

		/// <summary>
		/// Used to check whether or not the resource can be aliased, not implemented for now
		/// </summary>
		int32_t RefCount = 0;
		bool allocated = false;

		std::string name;
	};

	struct FrameGraphResourceInputCreation {
		FrameGraphResourceType                  type;
		FrameGraphResourceInfo                  resource_info;

		std::string name;
	};

	struct FrameGraphResourceOutputCreation {
		FrameGraphResourceType                  type;
		FrameGraphResourceInfo                  resourceInfo;

		std::string name;
	};

	struct FrameGraphNodeCreation
	{
		std::vector<FrameGraphResourceInputCreation>  inputs;
		std::vector<FrameGraphResourceOutputCreation> outputs;

		RenderPassType renderType;
		bool enabled;
		bool isCompute = false;

		char const* name;
	};

	struct FrameGraphNode {
		int32_t RefCount = 0;

		RenderPassHandle  renderPass;
		FramebufferHandle framebuffer;

		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;

		//Edges represent nodes this node is connected TO
		std::vector<FrameGraphNodeHandle> edges;
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
	};

	struct FrameGraphNodeCache {
		void Init(PuduGraphics* device);
		void Shutdown();

		PuduGraphics* device;

		std::unordered_map<std::string, uint32_t> nodeMap;
		ResourcePool<FrameGraphNode> nodes;
	};

	struct FrameGraphBuilder {
		void Init(PuduGraphics* device);
		void Shutdown();

		FrameGraphResourceHandle CreateNodeOutputResource(const FrameGraphResourceOutputCreation& creation, FrameGraphNodeHandle producer);
		FrameGraphResourceHandle CreateNodeInput(const FrameGraphResourceInputCreation& creation);
		FrameGraphNodeHandle CreateNode(const FrameGraphNodeCreation& creation);

		FrameGraphNode* GetNode(std::string name);
		FrameGraphNode* GetNode(FrameGraphNodeHandle handle);

		FrameGraphResource* GetResource(FrameGraphResourceHandle handle);
		FrameGraphResource* GetOutputResource(std::string name);

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
		void Shutdown();

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
		void EnableRenderPass(char* renderPassName);
		void DisableRenderPass(char* renderPassName);
		void OnResize(PuduGraphics& gpu, uint32_t width, uint32_t height);

		void AttachRenderPass(RenderPass renderPass, RenderPassType type);

		FrameGraphNode* GetNode(char* name);
		FrameGraphNode* GetNode(FrameGraphNodeHandle handle);

		FrameGraphResource* GetOutputResource(char const* name);
		FrameGraphResource* GetResource(FrameGraphResourceHandle handle);

		void AddNode(FrameGraphNodeCreation& node);

		std::vector<FrameGraphNodeHandle> nodes;

		FrameGraphBuilder* builder;

		const char* name = nullptr;
	};
}
