#ifndef READWRITE_H
#define READWRITE_H

#include <vector>
#include <string>

#include "common/types/ObjInfo.h"

void writeFile(const std::vector<ObjInfo>&, const std::vector<MtlObj>&, std::string);

void readFile(std::vector<ObjInfo>&, std::vector<MtlObj>&, std::string);

#endif // !READWRITE_H
