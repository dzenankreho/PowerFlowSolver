#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <tuple>
#include "systemModel.h"
#include <string.h>


/// <summary>
///  Imports SystemModel from the .txt file
/// </summary>
/// <param name="const char*">Name of the file</param>
/// <param name="SystemModel::SystemModel">System model</param>
/// <returns></returns>
int importFromTxt(const char* filename, SystemModel::SystemModel& systemModel,
                   std::vector<std::vector<std::tuple<int, double, double, std::string, double, double, int>>>* buses,
                   std::vector<std::tuple<int, double, double, std::string, double, double, int>>* sortedBuses,
                   std::vector<std::tuple<int, int, double, double, double, std::string, double, double, double, double>>* lineValues,
                   std::vector<std::tuple<int, double, std::string,std::string, double, double>>* batteryValues,
                   std::vector<std::tuple<int, int, double, double, double, double, std::string, double, double>>* transValues)
{
    try
    {
        std::vector<std::tuple<int, double, double, std::string, double, double, int>> slackValues, pvValues, pqValues;
        std::ifstream inFile;
        inFile.open(filename);
        if (!inFile)
        {
            std::cerr << "Cannot open file: " << filename << "\n";
            return 0;
        }
        int i = 0, j = 0, k = 0;
        std::string s;
        std::string line; 
        std::getline(inFile, line);
        if (line == "Buses:")
        {
            inFile >> line;
            while (line == "Bus:")
            {
                int busNum;
                inFile >> busNum;
                inFile >> line >> line;
                if (line == "Slack")
                { 
                    double voltageMagnitude, voltagePhase, x, y;
                    std::string color;
                    inFile >> line >> line >> voltageMagnitude;
                    inFile >> line >> line >> voltagePhase;
                    inFile >> line >> color;
                    inFile >> line >> x;
                    inFile >> line >> y;
                    systemModel.addBus(SystemModel::TypeOfBus::Slack);
                    systemModel.getBus(busNum).setVoltageMagnitude(voltageMagnitude);
                    systemModel.getBus(busNum).setVoltagePhase(voltagePhase);
                    slackValues.push_back(std::make_tuple(busNum, voltageMagnitude, voltagePhase, color, x, y, 1));
                    sortedBuses->push_back(slackValues[i]);
//                    std::strcpy((std::string), color);
                    i++;
                    busNum++;
                }
                else if (line == "PV")
                {
                    double activePower, voltageMagnitude, x, y;
                    std::string color;
                    inFile >> line >> line >> activePower;
                    inFile >> line >> line >> voltageMagnitude;
                    inFile >> line >> color;
                    inFile >> line >> x;
                    inFile >> line >> y;
                    systemModel.addBus(SystemModel::TypeOfBus::PV);
                    systemModel.getBus(busNum).setVoltageMagnitude(voltageMagnitude);
                    systemModel.getBus(busNum).setActivePower(activePower);
                    pvValues.push_back(std::make_tuple(busNum, voltageMagnitude, activePower, color, x, y, 2));
                    sortedBuses->push_back(pvValues[j]);
                    j++;
                    busNum++;
                }

                else if (line == "PQ")
                {
                    double activePower, reactivePower, x, y;
                    std::string color;
                    inFile >> line >> line >> activePower;
                    inFile >> line >> line >> reactivePower;
                    inFile >> line >> color;
                    inFile >> line >> x;
                    inFile >> line >> y;
                    systemModel.addBus(SystemModel::TypeOfBus::PQ);
                    systemModel.getBus(busNum).setReactivePower(reactivePower);
                    systemModel.getBus(busNum).setActivePower(activePower);
                    pqValues.push_back(std::make_tuple(busNum, activePower, reactivePower, color, x, y, 3));
                    sortedBuses->push_back(pqValues[k]);
                    k++;
                    busNum++;
                }

                inFile >> line;
            }
        }
        buses->push_back(slackValues);
        buses->push_back(pvValues);
        buses->push_back(pqValues);

        if (line == "Branches:")
        {
            inFile >> line;
            while (line == "From")
            {
                int bus1, bus2;
                inFile >> line >> bus1 >> line >>line >> bus2;

                inFile >> line >> line;
                    
                if (line == "Transformer")
                {
                    double seriesResistance, seriesReactance, shuntConductance, shuntSusceptance, x, y;
                    std::string color;
                    
                    inFile >> line >> line >> seriesResistance;

                    inFile >> line >> line >> seriesReactance;

                    inFile >> line >> line >> shuntConductance;

                    inFile >> line >> line >> shuntSusceptance;
                    inFile >> line >> color;
                    inFile >> line >> x;
                    inFile >> line >> y;

                    systemModel.addTransformer(bus1, bus2, seriesResistance, seriesReactance, shuntConductance, shuntSusceptance);
                    transValues->push_back(std::make_tuple(bus1, bus2, seriesResistance,seriesReactance,shuntConductance,shuntSusceptance, color, x, y));

                }
                else if (line == "Line")
                {
                    double seriesResistance, seriesReactance, shuntSusceptance, x1, y1, x2, y2;
                    std::string color;
                    inFile >> line >> line >> seriesResistance;

                    inFile >> line >> line >> seriesReactance;

                    inFile >> line >> line >> shuntSusceptance;
                    inFile >> line >> color;
                    inFile >> line >> x1;
                    inFile >> line >> y1;
                    inFile >> line >> x2;
                    inFile >> line >> y2;

                    systemModel.addLine(bus1, bus2, seriesResistance, seriesReactance, shuntSusceptance);
                    lineValues->push_back(std::make_tuple(bus1, bus2, seriesResistance,seriesReactance,shuntSusceptance, color, x1, y1 ,x2, y2));

                }

                inFile >> line;
            }
        }
        if (line == "Capacitor")
        {
            inFile >> line;
            inFile >> line;
            while (line == "At")
            {
                int busNum;
                std::string configurationType;
                double capacitance, x, y;
                std::string color, config;
                inFile >> line >> busNum;
                inFile >> line >> line >> line >> configurationType;
                if (configurationType == "Grounded") inFile >> line;
                inFile >> line >> capacitance;
                inFile >> line >> color;
                inFile >> line >> x;
                inFile >> line >> y;
                if (configurationType == "Delta")
                    systemModel.addCapacitorBank(busNum, capacitance, SystemModel::ThreePhaseLoadConfigurationsType::Delta);
                else if (configurationType == "Grounded")
                {
                    systemModel.addCapacitorBank(busNum, capacitance, SystemModel::ThreePhaseLoadConfigurationsType::GroundedStar);
                    configurationType = "Grounded Star";
                }
                else if (configurationType == "Star")
                    systemModel.addCapacitorBank(busNum, capacitance, SystemModel::ThreePhaseLoadConfigurationsType::Star);
                inFile >> line;
                for (int x = 0; x < (configurationType.length() + 1); x++)
                        config[x] = configurationType[x];
                batteryValues->push_back(std::make_tuple(busNum, capacitance, config, color, x, y));
            }
        }
        return 1;
    }
    catch (...) {
        throw std::logic_error("Error while reading.");
    }
}
