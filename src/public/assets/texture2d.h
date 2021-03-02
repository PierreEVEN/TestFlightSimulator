#pragma once

#include <cstdint>

#include "GraphicResource.h"


class Texture2d : public GraphicResource
{
public:
	Texture2d(Window* context, const AssetRef& asset_reference, const uint8_t* data, size_t width, size_t height, uint8_t channel_count);

private:
	const uint8_t* texture_data;
	const size_t texture_width;
	const size_t texture_height;
	const size_t texture_channels;
};

