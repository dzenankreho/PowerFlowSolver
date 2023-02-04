#pragma once
#include <fstream>
#include <iostream>
#include "systemModel.h"
#include <string.h>


void exportToLatex(const char* filename, SystemModel::SystemModel s);

void exportToHTML(const char* filename, SystemModel::SystemModel s);

void exportToTxt(const char* filename, SystemModel::SystemModel s,
                 std::vector<std::vector<std::tuple<int, double, double, std::string, double, double, int>>> buses,
                 std::vector<std::tuple<int, int, double, double, double,std::string, double, double, double, double>> lineValues,
                 std::vector<std::tuple<int, double, std::string, std::string, double, double>> batteryValues,
                 std::vector<std::tuple<int, int, double, double, double, double,std::string, double, double>> transValues);
