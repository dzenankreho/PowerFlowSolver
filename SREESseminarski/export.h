#pragma once
#include <fstream>
#include <iostream>
#include "systemModel.h"

void exportToLatex(SystemModel::SystemModel s);

void exportToHTML(SystemModel::SystemModel s);

void exportToTxt(const char* filename, SystemModel::SystemModel s);