#ifndef PROFILE_H
#define PROFILE_H

#include <stdio.h>
#include <vector>

#include "common/types/ObjInfo.h"

void generateTreeAndWriteResults(FILE*, std::vector<ObjInfo>);

void profileKdTree(std::vector<ObjInfo>);

#endif // !PROFILE_H
