#include <string>
#include <unordered_map>

std::unordered_map<std::string, int> occludeMap =
{
	{ "material_287",0 },   // dish
	{ "material_033",1 },   // ashtray
	{ "material_034",2 },
	{ "material_034",3 },   // candles
	{ "material_036",4 },
	{ "material_37", 5 },   // group41 wall outside
	{ "material_29", 6 },   // group31 side wall
	{ "material_041", 7 },  // glass
	{ "material_039", 9 },  // saltshaker
	{ "material_038", 10 }, // silverware
	{ "material_284", 11 }, // table
	{ "material_032", 12 }, // placemat
	{ "material_93", 13 },  // group105 foliage on ground
	{ "material_92", 14 },  // group104 foliage on ground
	{ "material_94", 15 },  // group106 foliage on ground
	{ "material_14", 16 },  // group15 & group50 floor outside in shade
	{ "material_18", 17 },  // group20 second floor floor inside
	{ "material_72", 18 },  // group85 back wall
	{ "material_0", 19  },  // door_arch
	{ "materiald", 20 },	// door
	{ "material_46", 21 },  // group52 beams
	{ "material_47", 22 },  // group55 second floor small floor inside
	{ "material_48", 23 },  // group54 floor
	{ "material_90", 24 }   // beam
};

bool isOccluder(const std::string& mtl)
{
	if (occludeMap.find(mtl) != occludeMap.end())
		return true;
	else
		return false;
}