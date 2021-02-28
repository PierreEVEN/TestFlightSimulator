#pragma once
#include <filesystem>
#include <cstdint>

#include "GraphicResource.h"


class ITexture2dBase : public GraphicResource
{
	
};

struct PixelWB
{
	uint8_t r;
};

struct PixelRGB
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct PixelRGBA
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

template <typename ChannelType>
class Texture2d : public ITexture2dBase
{
public:
	Texture2d(const AssetRef& asset_ref, const std::filesystem::path& source_path) {}

	

private:

	ChannelType* data = nullptr;	
};

typedef Texture2d<PixelWB> Texture2dWB;
typedef Texture2d<PixelRGB> Texture2dRGB;
typedef Texture2d<PixelRGBA> Texture2dRGBA;