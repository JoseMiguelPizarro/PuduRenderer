#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <Resources/ResourcesManager.h>
#include <PuduGraphics.h>
#include "FrameGraphRenderPass.h"


namespace Pudu {

	struct FrameGraph;
	class PuduGraphics;


	enum FrameGraphResourceType {
		FrameGraphResourceType_Invalid = -1,
		FrameGraphResourceType_Buffer = 0,
		FrameGraphResourceType_Texture = 1,
		FrameGraphResourceType_Attachment = 2,
		FrameGraphResourceType_Reference = 3

	};


	struct FrameGraphResourceInfo {
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

				RenderPassOperation load_op;

				TextureHandle texture;
			} texture;
		};
	};

	// NOTE(marco): an input could be used as a texture or as an attachment.
	// If it's an attachment we want to control whether to discard previous
	// content - for instance the first time we use it - or to load the data
	// from a previous pass
	// NOTE(marco): an output always implies an attachment and a store op
	struct FrameGraphResource {
		FrameGraphResourceType type;
		FrameGraphResourceInfo resourceInfo;

		FrameGraphNodeHandle producer;
		FrameGraphResourceHandle outputHandle;

		int32_t RefCount = 0;

		const char* name = nullptr;
	};

	struct FrameGraphResourceInputCreation {
		FrameGraphResourceType                  type;
		FrameGraphResourceInfo                  resource_info;

		const char* name;
	};

	struct FrameGraphResourceOutputCreation {
		FrameGraphResourceType                  type;
		FrameGraphResourceInfo                  resource_info;

		const char* name;
	};


	struct FrameGraphNodeCreation
	{
		std::vector<FrameGraphResourceInputCreation>  inputs;
		std::vector<FrameGraphResourceOutputCreation> outputs;

		bool enabled;

		const char* name;
	};


	struct FrameGraphNode {
		int32_t RefCount = 0;

		RenderPassHandle  renderPass;
		FramebufferHandle framebuffer;

		FrameGraphRenderPass* graphRenderPass;

		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;

		//Edges represent nodes this node is connected TO
		std::vector<FrameGraphNodeHandle> edges;

		bool enabled = true;

		const char* name = nullptr;
	};

	struct FrameGraphRenderPassCache {
		void Init();
		void Shutdown();

		std::unordered_map<uint64_t, FrameGraphRenderPass*> renderPassMap;
	};

	struct FrameGraphResourceCache {
		void Init(PuduGraphics* device);
		void Shutdown();

		PuduGraphics* device;

		std::unordered_map<uint64_t, uint32_t> resource_map;
		ResourcePool<FrameGraphResource> resources;
	};

	struct FrameGraphNodeCache {
		void Init(PuduGraphics* device);
		void Shutdown();

		PuduGraphics* device;

		std::unordered_map<uint64_t, uint32_t> nodeMap;
		ResourcePool<FrameGraphNode> nodes;
	};
	//
	//
	struct FrameGraphBuilder {
		void Init(PuduGraphics* device);
		void Shutdown();

		void RegisterRenderPass(char* name, FrameGraphRenderPass* renderPass);

		FrameGraphResourceHandle CreateNodeOutput(const FrameGraphResourceOutputCreation& creation, FrameGraphNodeHandle producer);
		FrameGraphResourceHandle CreateNodeInput(const FrameGraphResourceInputCreation& creation);
		FrameGraphNodeHandle CreateNode(const FrameGraphNodeCreation& creation);

		FrameGraphNode* GetNode(char const* name);
		FrameGraphNode* GetNode(FrameGraphNodeHandle handle);

		FrameGraphResource* GetResource(FrameGraphResourceHandle handle);
		FrameGraphResource* GetResource(char const * name);

		FrameGraphResourceCache resourceCache;
		FrameGraphNodeCache nodeCache;
		FrameGraphRenderPassCache renderPassCache;

		ResourcesManager resourcesManager;

		PuduGraphics* graphics;

		static constexpr uint32_t            k_max_render_pass_count = 256;
		static constexpr uint32_t            k_max_resources_count = 1024;
		static constexpr uint32_t            k_max_nodes_count = 1024;
	};

	//
	//
	struct FrameGraph {
		void Init(FrameGraphBuilder* builder);
		void Shutdown();

		void Parse(char* file_path);

		// NOTE(marco): each frame we rebuild the graph so that we can enable only
		// the nodes we are interested in
		void Reset();
		void EnableRenderPass(char* renderPassName);
		void DisableRenderPass(char* renderPassName);
		void Compile();
		void Render(VkCommandBuffer* commands, Scene* renderScene);
		void OnResize(PuduGraphics& gpu, uint32_t width, uint32_t height);

		FrameGraphNode* GetNode(char* name);
		FrameGraphNode* AccessNode(FrameGraphNodeHandle handle);

		FrameGraphResource* GetResource(char const * name);
		FrameGraphResource* GetResource(FrameGraphResourceHandle handle);

		// TODO(marco): in case we want to add a pass in code
		void AddNode(FrameGraphNodeCreation& node);

		// NOTE(marco): nodes sorted in topological order
		std::vector<FrameGraphNodeHandle> nodes;

		FrameGraphBuilder* builder;

		const char* name = nullptr;
	};

} // namespace raptor
