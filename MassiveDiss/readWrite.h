#ifndef READWRITE_H
#define READWRITE_H

#include <vector>
#include <string>

#include "common/types/ObjInfo.h"
#include "common/types/MtlObj.h"

void writeFile(const std::vector<ObjInfo>&, const std::vector<MtlObj>&, std::string);
void writeBinaryFile(const std::vector<ObjInfo>&, std::string);
void writeMtlFile(const std::vector<MtlObj>&, const std::string&);

void readFile(std::vector<ObjInfo>&, std::vector<MtlObj>&, std::string);
void readBinaryFile(std::vector<ObjInfo>&, const std::string&);
void readMtlFile(std::vector<MtlObj>&, const std::string&);

#endif // !READWRITE_H
