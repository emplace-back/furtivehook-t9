#include "dependencies/std_include.hpp"
#include "rendering.hpp"

namespace rendering
{
	utils::hook::vmt swapchain_vmt{ 40 };

	HRESULT __stdcall present(IDXGISwapChain3* thisptr, UINT syncInterval, UINT flags)
	{
		dx::initialize(thisptr);
		dx::on_frame(thisptr);

		return swapchain_vmt.call<HRESULT>(8, thisptr, syncInterval, flags);
	}

	HRESULT __stdcall resize_buffers(IDXGISwapChain3* thisptr, UINT count, UINT width, UINT height, DXGI_FORMAT format, UINT flags)
	{
		if (dx::initialized)
			dx::clean_render_target();

		const auto result = swapchain_vmt.call<HRESULT>(13, thisptr, count, width, height, format, flags);

		if (dx::initialized)
			dx::create_render_target(thisptr);

		return result;
	}
	
	void initialize()
	{
		if (const auto swapchain = *reinterpret_cast<IDXGISwapChain3**>(OFFSET(offsets::swapchain)))
		{
			swapchain_vmt.setup(swapchain);
			swapchain_vmt.hook(8, present);
			swapchain_vmt.hook(13, resize_buffers);
		}
	}
}