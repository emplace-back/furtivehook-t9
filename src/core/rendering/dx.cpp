#include "dependencies/stdafx.hpp"
#include "dx.hpp"

namespace rendering::dx
{
	struct frame_context
	{
		ID3D12CommandAllocator* allocator;
		ID3D12Resource* resource;
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
	};

	constexpr auto NUM_FRAMES{ 2 };
	
	bool initialized{ false };
	frame_context frame_context_[NUM_FRAMES]{};
	ID3D12DescriptorHeap* src_desc_heap{ nullptr };
	ID3D12Device* device{ nullptr };
	ID3D12DescriptorHeap* rtv_desc_heap{ nullptr };
	ID3D12GraphicsCommandList* command_list{ nullptr };

	void create_render_target(IDXGISwapChain3* chain)
	{
		if (!chain)
			return;

		for (size_t i = 0; i < NUM_FRAMES; ++i)
		{
			ID3D12Resource* buffer{ nullptr };
			chain->GetBuffer(i, IID_PPV_ARGS(&buffer));
			auto& ctx = frame_context_[i];
			device->CreateRenderTargetView(buffer, nullptr, ctx.handle);
			ctx.resource = buffer;
		}
	}

	void clean_render_target()
	{
		for (size_t i = 0; i < NUM_FRAMES; ++i)
		{
			if (auto resource = frame_context_[i].resource; resource)
			{
				resource->Release();
				resource = nullptr;
			}
		}
	}

	bool create(IDXGISwapChain3* chain)
	{
		if (!chain)
			return false;

		if (chain->GetDevice(__uuidof(device), reinterpret_cast<void**>(&device)))
			return false;

		D3D12_DESCRIPTOR_HEAP_DESC desc_back{};
		desc_back.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc_back.NumDescriptors = NUM_FRAMES;
		desc_back.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc_back.NodeMask = 1;

		if (device->CreateDescriptorHeap(&desc_back, IID_PPV_ARGS(&rtv_desc_heap)))
			return false;

		const auto desc_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		auto handle = rtv_desc_heap->GetCPUDescriptorHandleForHeapStart();

		for (size_t i = 0; i < NUM_FRAMES; ++i)
		{
			frame_context_[i].handle = handle;
			handle.ptr += desc_size;
		}

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;

		if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&src_desc_heap)))
			return false;

		for (size_t i = 0; i < NUM_FRAMES; ++i)
		{
			if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame_context_[i].allocator)))
				return false;
		}

		if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame_context_[0].allocator, NULL, IID_PPV_ARGS(&command_list)))
			return false;

		if (command_list->Close())
			return false;

		create_render_target(chain);
		return true;
	}

	void on_frame(IDXGISwapChain3* chain)
	{
		if (dx::initialized)
		{
			ImGui::GetIO().MouseDrawCursor = menu::open; 
			
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0, 100, 200, 255).Value);
			utils::toast::draw_toast();
			ImGui::PopStyleColor(); 
			
			menu::draw();

			auto& ctx = frame_context_[chain->GetCurrentBackBufferIndex()];
			ctx.allocator->Reset();

			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = ctx.resource;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			command_list->Reset(ctx.allocator, nullptr);
			command_list->ResourceBarrier(1, &barrier);
			command_list->OMSetRenderTargets(1, &ctx.handle, FALSE, nullptr);
			command_list->SetDescriptorHeaps(1, &src_desc_heap);

			ImGui::Render(); 
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list);

			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			command_list->ResourceBarrier(1, &barrier);
			command_list->Close();

			if (const auto command_queue = *reinterpret_cast<ID3D12CommandQueue**>(uintptr_t(chain) + 0x118); command_queue)
			{
				command_queue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&command_list));
			}
		}
	}

	void initialize(IDXGISwapChain3* chain)
	{
		if (!dx::initialized)
		{
			dx::initialized = true;
			
			const auto created = dx::create(chain);
			if (!created)
				return;

			DXGI_SWAP_CHAIN_DESC sd{};
			if (chain->GetDesc(&sd))
				return;

			if (const auto hwnd = sd.OutputWindow; hwnd)
			{
				ImGui::CreateContext();
				ImGui_ImplWin32_Init(hwnd);
				ImGui_ImplDX12_Init(device, NUM_FRAMES,
					DXGI_FORMAT_R8G8B8A8_UNORM,
					src_desc_heap,
					src_desc_heap->GetCPUDescriptorHandleForHeapStart(),
					src_desc_heap->GetGPUDescriptorHandleForHeapStart());

				ImGui::GetIO().IniFilename = nullptr;

				menu::set_style();
				input::hwnd_ = hwnd;
			}
		}
	}
}