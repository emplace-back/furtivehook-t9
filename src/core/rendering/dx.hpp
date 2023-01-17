#pragma once
#include "dependencies/stdafx.hpp"

namespace rendering::dx
{
    void create_render_target(IDXGISwapChain3* chain);
    void clean_render_target();
    void on_frame(IDXGISwapChain3* chain);
    void initialize(IDXGISwapChain3* chain);

    extern bool initialized;
}
