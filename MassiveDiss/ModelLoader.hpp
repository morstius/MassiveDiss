#ifndef MODELLOADR_HPP
#define MODELLOADR_HPP

#include "common/types/ObjInfo.h"
#include "common/types/MtlObj.h"

bool loadObj(
	const char *,
	std::vector<ObjInfo>&,
	std::vector<MtlObj>&
);

bool populateMtlLib(
	const char *,
	std::vector<MtlObj>&
);

int findTexIdx(
	const char *,
	std::vector<MtlObj>
);

#endif // !MODELLOADR_HPP

