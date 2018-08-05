#include <string>
#include <unordered_map>

std::unordered_map<std::string, int> occludeMap =
{
	{ "material_287",0 }, // plate
	{ "material_033",1 }, // ashtray
	{ "material_034",2 },
	{ "material_034",3 }, // candles
	{ "material_036",4 },
	{ "material_37", 5 }, // group41 wall outside
	{ "material_29", 6 }, // group31 side wall
};

bool isOccluder(const std::string& mtl)
{
	if (occludeMap.find(mtl) != occludeMap.end())
		return true;
	else
		return false;
}