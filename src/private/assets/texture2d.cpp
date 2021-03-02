
#include "assets/texture2d.h"

Texture2d::Texture2d(Window* context, const AssetRef& asset_reference, const uint8_t* data, size_t width,
                     size_t height, uint8_t channel_count)
	: texture_data(data), texture_width(width), texture_height(height), texture_channels(channel_count), GraphicResource(context, asset_reference)
{
}
