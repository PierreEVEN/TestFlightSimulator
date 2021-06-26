
#include "assets/assetId.h"

std::size_t std::hash<AssetId>::operator()(const AssetId& other) const noexcept
{
    using std::hash;
    using std::size_t;
    using std::string;

    return hash<size_t>()(other.id);
}

AssetId::AssetId(const size_t id_val) : id(id_val) {}

AssetId::AssetId(const AssetId& other) : AssetId(other.id)
{
#ifdef _DEBUG
    asset_name = other.asset_name;
#endif
}

AssetId::AssetId(const AssetId&& other) noexcept : AssetId(other.id)
{
#ifdef _DEBUG
    asset_name = other.asset_name;
#endif
}
AssetId::AssetId(const std::string& name) : AssetId(std::hash<std::string>{}(name))
{
#ifdef _DEBUG
    asset_name = name;
#endif
}

std::string AssetId::to_string() const
{
#ifdef _DEBUG
    return asset_name;
#else
    return std::to_string(id);
#endif
}