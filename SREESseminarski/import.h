#pragma once

#include "systemModel.h"
#include <string.h>


int importFromTxt(const char* filename, SystemModel::SystemModel& s,
                   std::vector<std::vector<std::tuple<int, double, double, std::string, double, double, int>>>* buses,
                   std::vector<std::tuple<int, double, double, std::string, double, double, int>>* sortedBuses,
                   std::vector<std::tuple<int, int, double, double, double, std::string, double, double, double, double>>* lineValues,
                   std::vector<std::tuple<int, double, std::string, std::string, double, double>>* batteryValues,
                   std::vector<std::tuple<int, int, double, double, double, double, std::string, double, double>>* transValues);
