#pragma once
#include <string>

class AssetId;

namespace std
{
	template <>
	struct hash<AssetId>
	{
		std::size_t operator()(const AssetId& other) const noexcept;
	};
}
class AssetId final
{
public:
	AssetId(const size_t id_val);
	AssetId(const AssetId& other);
	AssetId(const AssetId&& other) noexcept;
	explicit AssetId(const std::string& name);
	AssetId(const char* name) : AssetId(std::string(name)) {}

	size_t operator()() const { return id; }

	AssetId& operator=(const AssetId& other) { id = other.id; return *this; }
	bool operator==(const AssetId& other) const { return other.id == id; }

	[[nodiscard]] std::string to_string() const;

private:

	friend class AssetBase;
	AssetId() = default;
	
	friend std::size_t std::hash<AssetId>::operator()(const AssetId& other) const noexcept;
	
	size_t id;
#ifdef _DEBUG
	std::string asset_name;
#endif
};
