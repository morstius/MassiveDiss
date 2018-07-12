#ifndef MODELLOADR_HPP
#define MODELLOADR_HPP

#include <string>

#include "common/types/ObjInfo.h"
#include "common/types/MtlObj.h"

bool loadObj(
	const char *,
	std::vector<ObjInfo>&,
	std::vector<MtlObj>&
);

bool populateMtlLib(
	std::string,
	std::vector<MtlObj>&
);

int findTexIdx(
	std::string,
	std::vector<MtlObj>
);

#endif // !MODELLOADR_HPP

