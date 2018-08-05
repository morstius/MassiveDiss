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

void vboIndex(
	const std::vector<ObjInfo>&,
	std::vector<ObjInfo>&
);

#endif // !MODELLOADR_HPP

