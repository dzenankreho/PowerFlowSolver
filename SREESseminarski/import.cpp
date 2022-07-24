#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <tuple>
#include "systemModel.h"


void importFromTxt(const char* filename, SystemModel::SystemModel& systemModel)
{
    try {
        std::ifstream inFile;
        inFile.open(filename);
        if (!inFile)
        {
            std::cerr << "Cannot open file: " << filename << "\n";
            exit(1);
        }

        std::string s;
        int busNum = 1;
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
                    double voltageMagnitude, voltagePhase;
                    inFile >> line >> line >> voltageMagnitude;
                    inFile >> line >> line >> voltagePhase;
                    systemModel.addBus(SystemModel::TypeOfBus::Slack);
                    systemModel.getBus(busNum).setVoltageMagnitude(voltageMagnitude);
                    systemModel.getBus(busNum).setVoltagePhase(voltagePhase);
                    busNum++;
                }
                else if (line == "PV")
                {
                    double activePower, voltageMagnitude;
                    inFile >> line >> line >> activePower;
                    inFile >> line >> line >> voltageMagnitude;
                    systemModel.addBus(SystemModel::TypeOfBus::PV);
                    systemModel.getBus(busNum).setVoltageMagnitude(voltageMagnitude);
                    systemModel.getBus(busNum).setActivePower(activePower);
                    busNum++;
                }

                else if (line == "PQ")
                {
                    double activePower, reactivePower;
                    inFile >> line >> line >> activePower;
                    inFile >> line >> line >> reactivePower;
                    systemModel.addBus(SystemModel::TypeOfBus::PQ);
                    systemModel.getBus(busNum).setReactivePower(reactivePower);
                    systemModel.getBus(busNum).setActivePower(activePower);
                    busNum++;
                }

                inFile >> line;
            }
        }

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
                    double seriesResistance, seriesReactance, shuntConductance, shuntSusceptance;
                    inFile >> line >> line >> seriesResistance;

                    inFile >> line >> line >> seriesReactance;

                    inFile >> line >> line >> shuntConductance;

                    inFile >> line >> line >> shuntSusceptance;

                    systemModel.addTransformer(bus1, bus2, seriesResistance, seriesReactance, shuntConductance, shuntSusceptance);

                }
                else if (line == "Line")
                {
                    double seriesResistance, seriesReactance, shuntSusceptance;
                    inFile >> line >> line >> seriesResistance;

                    inFile >> line >> line >> seriesReactance;

                    inFile >> line >> line >> shuntSusceptance;

                    systemModel.addLine(bus1, bus2, seriesResistance, seriesReactance, shuntSusceptance);

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
                double capacitance;
                inFile >> line >> busNum;
                inFile >> line >> line >> line >> configurationType;
                if (configurationType == "Grounded") inFile >> line;
                inFile >> line >> capacitance;
                if (configurationType == "Delta")
                    systemModel.addCapacitorBank(busNum, capacitance, SystemModel::ThreePhaseLoadConfigurationsType::Delta);
                else if (configurationType == "Grounded")
                    systemModel.addCapacitorBank(busNum, capacitance, SystemModel::ThreePhaseLoadConfigurationsType::GroundedStar);
                else if (configurationType == "Star")
                    systemModel.addCapacitorBank(busNum, capacitance, SystemModel::ThreePhaseLoadConfigurationsType::Star);
                inFile >> line;
            }
        }

    }
    catch (...) {
        throw std::logic_error("Error while reading.");
    }
}