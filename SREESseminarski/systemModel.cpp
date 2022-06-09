#include "systemModel.h"
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <cmath>



#define pi 4 * std::atan(1.0)

void SystemModel::Bus::setVoltageMagnitude(double voltageMagnitude) {
	if (typeOfBus == TypeOfBus::PQ) {
		throw std::logic_error("Cannot set voltage magnitude for PQ bus.");
	}

	this->voltageMagnitude = voltageMagnitude;
}



void SystemModel::Bus::setVoltagePhase(double voltagePhase) {
	if (typeOfBus != TypeOfBus::Slack) {
		throw std::logic_error("Cannot set voltage phase for PQ and PV buses.");
	}

	this->voltagePhase = voltagePhase;
}



void SystemModel::Bus::setActivePower(double activePower) {
	if (typeOfBus == TypeOfBus::Slack) {
		throw std::logic_error("Cannot set active power for slack bus.");
	}

	this->activePower = activePower;
}



void SystemModel::Bus::setReactivePower(double reactivePower) {
	if (typeOfBus != TypeOfBus::PQ) {
		throw std::logic_error("Cannot set reactive power for PQ and PV buses.");
	}

	this->reactivePower = reactivePower;
}



std::optional<double> SystemModel::Bus::getVoltageMagnitude() const {
	if (typeOfBus == TypeOfBus::PQ) {
		throw std::logic_error("Cannot get voltage magnitude for PQ bus.");
	}

	return voltageMagnitude;
}



std::optional<double> SystemModel::Bus::getVoltagePhase() const {
	if (typeOfBus != TypeOfBus::Slack) {
		throw std::logic_error("Cannot get voltage phase for PQ and PV buses.");
	}

	return voltagePhase;
}



std::optional<double> SystemModel::Bus::getActivePower() const {
	if (typeOfBus == TypeOfBus::Slack) {
		throw std::logic_error("Cannot get active power for slack bus.");
	}

	return activePower;
}



std::optional<double> SystemModel::Bus::getReactivePower() const {
	if (typeOfBus != TypeOfBus::PQ) {
		throw std::logic_error("Cannot get reactive power for PQ and PV buses.");
	}

	return reactivePower;
}



void SystemModel::SystemModel::addBus(TypeOfBus typeOfBus) {
	if (numberOfBuses == maxNumberOfBuses) {
		throw std::length_error("Maximum number of buses reached.");
	}

	if (numberOfBuses + 1 == maxNumberOfBuses) {
		if (typeOfBus != TypeOfBus::Slack) {
			bool slackFound{ false };
			for (Bus bus : buses) {
				if (bus.getTypeOfBus() == TypeOfBus::Slack) {
					slackFound = true;
				}
			}

			if (!slackFound) {
				throw std::length_error("Maximum number of buses will be reached without a slack bus.");
			}
		}
	}
 
	if (typeOfBus == TypeOfBus::Slack && hasSlackBeenAssigned()) {
		throw std::logic_error("There already exists a slack bus.");
	}



	buses.push_back(Bus(typeOfBus));
	numberOfBuses++;
}



void SystemModel::SystemModel::addLoad(uint8_t busNumber, double activePower, double reactivePower) {
	if (busNumber > buses.size() || busNumber == 0) {
		throw std::out_of_range("Invalid bus number.");
	}

	if (buses.at(busNumber - 1).getTypeOfBus() != TypeOfBus::PQ) {
		throw std::logic_error("Given bus is not a PQ bus.");
	}

	if (buses.at(busNumber - 1).getActivePower() || buses.at(busNumber - 1).getReactivePower()) {
		throw std::logic_error("Power already set for this bus.");
	}

	buses.at(busNumber - 1).setActivePower(activePower);
	buses.at(busNumber - 1).setReactivePower(reactivePower);
}



SystemModel::Bus& SystemModel::SystemModel::getBus(uint8_t busNumber) {
	if (busNumber == 0) {
		throw std::logic_error("Invalid bus number.");
	}

	try {
		return buses.at(busNumber - 1);
	} catch (...) {
		throw std::logic_error("Invalid bus number.");
	}
}



void SystemModel::SystemModel::addLine(uint8_t busNumber1, uint8_t busNumber2, double r, double x, double b) {
	if (busNumber1 > buses.size() || busNumber1 == 0) {
		throw std::out_of_range("Invalid bus number 1.");
	}

	if (busNumber2 > buses.size() || busNumber2 == 0) {
		throw std::out_of_range("Invalid bus number 2.");
	}

	if (checkForConnectionBetweenToBuses(busNumber1, busNumber2)) {
		throw std::logic_error("Line or transformer already present between nodes.");
	}

	std::complex<double> z_s{ r, x }, y_sh{0, b};

	admittanceMatrix.rowIndex.push_back(busNumber1);
	admittanceMatrix.columnIndex.push_back(busNumber2);
	admittanceMatrix.values.push_back(- 1.0 / z_s);

	admittanceMatrix.rowIndex.push_back(busNumber2);
	admittanceMatrix.columnIndex.push_back(busNumber1);
	admittanceMatrix.values.push_back(-1.0 / z_s);

	if (!checkForConnectionBetweenToBuses(busNumber1, busNumber1)) {
		admittanceMatrix.rowIndex.push_back(busNumber1);
		admittanceMatrix.columnIndex.push_back(busNumber1);
		admittanceMatrix.values.push_back(y_sh / 2.0 + 1.0 / z_s);
	} else {
		int64_t index{};
		while (!(admittanceMatrix.rowIndex.at(index) == admittanceMatrix.columnIndex.at(index) &&
			admittanceMatrix.rowIndex.at(index) == busNumber1)) {
			index++;
		}
		admittanceMatrix.values.at(index) = admittanceMatrix.values.at(index) + y_sh / 2.0 + 1.0 / z_s;
	}

	if (!checkForConnectionBetweenToBuses(busNumber2, busNumber2)) {
		admittanceMatrix.rowIndex.push_back(busNumber2);
		admittanceMatrix.columnIndex.push_back(busNumber2);
		admittanceMatrix.values.push_back(y_sh / 2.0 + 1.0 / z_s);
	} else {
		int64_t index{};
		while (!(admittanceMatrix.rowIndex.at(index) == admittanceMatrix.columnIndex.at(index) &&
			admittanceMatrix.rowIndex.at(index) == busNumber2)) {
			index++;
		}
		admittanceMatrix.values.at(index) = admittanceMatrix.values.at(index) + y_sh / 2.0 + 1.0 / z_s;
	}
}



void SystemModel::SystemModel::addGenerator(uint8_t busNumber, double voltageMagnitude, double activePower) {
	if (busNumber > buses.size() || busNumber == 0) {
		throw std::out_of_range("Invalid bus number.");
	}

	if (buses.at(busNumber - 1).getTypeOfBus() != TypeOfBus::PV) {
		throw std::logic_error("Given bus is not a PV bus.");
	}

	if (buses.at(busNumber - 1).getActivePower() || buses.at(busNumber - 1).getVoltageMagnitude()) {
		throw std::logic_error("Active power and voltage magnitude already set for this bus.");
	}

	buses.at(busNumber - 1).setActivePower(activePower);
	buses.at(busNumber - 1).setVoltageMagnitude(voltageMagnitude);
}



void SystemModel::SystemModel::addSlackGenerator(uint8_t busNumber, double voltageMagnitude, double voltagePhase) {
	if (busNumber > buses.size() || busNumber == 0) {
		throw std::out_of_range("Invalid bus number.");
	}

	if (buses.at(busNumber - 1).getTypeOfBus() != TypeOfBus::Slack) {
		throw std::logic_error("Given bus is not a PV bus.");
	}

	if (buses.at(busNumber - 1).getVoltagePhase() || buses.at(busNumber - 1).getVoltageMagnitude()) {
		throw std::logic_error("Voltage already set for this bus.");
	}

	buses.at(busNumber - 1).setVoltagePhase(voltagePhase);
	buses.at(busNumber - 1).setVoltageMagnitude(voltageMagnitude);
}



bool SystemModel::SystemModel::hasSlackBeenAssigned() const  {
	for (Bus bus : buses) {
		if (bus.getTypeOfBus() == TypeOfBus::Slack) {
			return true;
		}
	}

	return false;
}



void SystemModel::SystemModel::addTransformer(uint8_t busNumber1, uint8_t busNumber2, double r, double x, double g, double b, double n) {
	if (busNumber1 > buses.size() || busNumber1 == 0) {
		throw std::out_of_range("Invalid bus number 1.");
	}

	if (busNumber2 > buses.size() || busNumber2 == 0) {
		throw std::out_of_range("Invalid bus number 2.");
	}

	if (checkForConnectionBetweenToBuses(busNumber1, busNumber2)) {
		throw std::logic_error("Line or transformer already present between nodes.");
	}

	std::complex<double> z_s{ r, x }, y_sh{ g, -b };

	admittanceMatrix.rowIndex.push_back(busNumber1);
	admittanceMatrix.columnIndex.push_back(busNumber2);
	admittanceMatrix.values.push_back(- n / z_s);

	admittanceMatrix.rowIndex.push_back(busNumber2);
	admittanceMatrix.columnIndex.push_back(busNumber1);
	admittanceMatrix.values.push_back(- n / z_s);


	if (!checkForConnectionBetweenToBuses(busNumber1, busNumber1)) {
		admittanceMatrix.rowIndex.push_back(busNumber1);
		admittanceMatrix.columnIndex.push_back(busNumber1);
		admittanceMatrix.values.push_back(y_sh / 2.0 + 1.0 / z_s);
	} else {
		int64_t index{};
		while (!(admittanceMatrix.rowIndex.at(index) == admittanceMatrix.columnIndex.at(index) &&
			admittanceMatrix.rowIndex.at(index) == busNumber1)) {
			index++;
		}
		admittanceMatrix.values.at(index) = admittanceMatrix.values.at(index) + y_sh / 2.0 + 1.0 / z_s;
	}

	if (!checkForConnectionBetweenToBuses(busNumber2, busNumber2)) {
		admittanceMatrix.rowIndex.push_back(busNumber2);
		admittanceMatrix.columnIndex.push_back(busNumber2);
		admittanceMatrix.values.push_back((y_sh / 2.0 + 1.0 / z_s) * n * n);
	} else {
		int64_t index{};
		while (!(admittanceMatrix.rowIndex.at(index) == admittanceMatrix.columnIndex.at(index) &&
			admittanceMatrix.rowIndex.at(index) == busNumber2)) {
			index++;
		}
		admittanceMatrix.values.at(index) = admittanceMatrix.values.at(index) + (y_sh / 2.0 + 1.0 / z_s) * n * n;
	}
}



bool SystemModel::SystemModel::checkForConnectionBetweenToBuses(uint8_t busNumber1, uint8_t busNumber2) const {
	for (uint8_t i{}; i < admittanceMatrix.columnIndex.size(); i++) {
		if ((admittanceMatrix.columnIndex.at(i) == busNumber1 && admittanceMatrix.rowIndex.at(i) == busNumber2) ||
			(admittanceMatrix.rowIndex.at(i) == busNumber1 && admittanceMatrix.columnIndex.at(i) == busNumber2)) {
			return true;
		}
	}

	return false;
}



std::ostream& SystemModel::operator <<(std::ostream& stream, const SystemModel& systemModel) {
	try {
		const char* busTypeStrings[]{ "Slack", "PV", "PQ" };

		stream << "Buses:" << std::endl;

		for (uint8_t i{}; i < systemModel.buses.size(); i++) {
			stream << "\tBus: " << i + 1 << std::endl;

			stream << "\t\tType: " << busTypeStrings[int(systemModel.buses.at(i).getTypeOfBus())] << std::endl;

			switch (systemModel.buses.at(i).getTypeOfBus()) {
				case TypeOfBus::Slack:
						stream << "\t\tVoltage magnitude: " << systemModel.buses.at(i).getVoltageMagnitude().value() << std::endl;
						stream << "\t\tVoltage phase: " << systemModel.buses.at(i).getVoltagePhase().value() << std::endl;
					break;
				case TypeOfBus::PV:
						stream << "\t\tActive power: " << systemModel.buses.at(i).getActivePower().value() << std::endl;
						stream << "\t\tVoltage magnitude: " << systemModel.buses.at(i).getVoltageMagnitude().value() << std::endl;
					break;
				case TypeOfBus::PQ:
						stream << "\t\tActive power: " << systemModel.buses.at(i).getActivePower().value() << std::endl;
						stream << "\t\tReactive power: " << systemModel.buses.at(i).getReactivePower().value() << std::endl;
					break;
			}
		}


		stream << std::endl << "System admittance matrix:" << std::endl;
		for (uint8_t i{ 1 }; i < systemModel.numberOfBuses + 1; i++) {
			for (uint8_t j{ 1 }; j < systemModel.numberOfBuses + 1; j++) {
				if (systemModel.checkForConnectionBetweenToBuses(i, j)) {
					uint8_t k{};
					while (!(systemModel.admittanceMatrix.rowIndex.at(k) == i && systemModel.admittanceMatrix.columnIndex.at(k) == j)) {
						k++;
					}
					stream << std::setw(20) << systemModel.admittanceMatrix.values.at(k) << " ";
				} else {
					stream << std::setw(20) << std::complex{ 0.0, 0.0 } << " ";
				}
			}
			stream << std::endl;
		}

		return stream;
	} catch (...) {
		throw std::logic_error("System is incomplete.");
	}
}



void SystemModel::SystemModel::addCapacitorBank(uint8_t busNumber, double c, ThreePhaseLoadConfigurationsType configurationType) {
	if (busNumber > buses.size() || busNumber == 0) {
		throw std::out_of_range("Invalid bus number.");
	}

	if (buses.at(busNumber - 1).getTypeOfBus() == TypeOfBus::Slack) {
		throw std::logic_error("Cannot add capacitor bank to slack bus.");
	}

	std::complex<double> admittance{0, 0};

	switch (configurationType) {
		case ThreePhaseLoadConfigurationsType::Delta:
			admittance = {0, 3 * 2 * pi * 50 * c};
			break;
		case ThreePhaseLoadConfigurationsType::Star:
			admittance = { 0, 2 * pi * 50 * c };
			break;
		case ThreePhaseLoadConfigurationsType::GroundedStar:
			admittance = { 0, 2 * pi * 50 * c };
			break;
	}

	int64_t index{};
	while (admittanceMatrix.rowIndex.at(index) != admittanceMatrix.columnIndex.at(index)) {
		index++;
	}
	admittanceMatrix.values.at(index) = admittanceMatrix.values.at(index) + admittance;
}



SystemModel::fi SystemModel::SystemModel::getBusFunctions(uint8_t busNumber) const {
	if (busNumber > buses.size() || busNumber == 0) {
		throw std::out_of_range("Invalid bus number.");
	}

	std::function<double(std::vector<double>)> fip, fiq;

	const Bus& bus{ buses.at(busNumber - 1) };
	uint8_t numberOfBuses{ this->numberOfBuses };

	switch (bus.getTypeOfBus()) {
		case TypeOfBus::Slack:
			fip = [bus, busNumber](std::vector<double> v) -> double {
				return v.at(busNumber - 1) - bus.getVoltagePhase().value();
			};
			fiq = [bus, busNumber, numberOfBuses](std::vector<double> v) -> double {
				return v.at(busNumber - 1 + numberOfBuses) - bus.getVoltageMagnitude().value();
			};
			break;
		case TypeOfBus::PV:
			fip = [bus, busNumber, this, numberOfBuses](std::vector<double> v) -> double {
				double sum{};
				for (uint8_t i{}; i < numberOfBuses; i++) {
					uint8_t k{};
					AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };
					while (!(admittanceMatrix.columnIndex.at(k) == i + 1 && admittanceMatrix.rowIndex.at(k) == busNumber)) {
						k++;
					}
					sum += v.at(i + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
						* std::cos(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(i));;
				}

				return v.at(busNumber - 1 + numberOfBuses) * sum - bus.getActivePower().value();
			};
			fiq = [bus, busNumber, numberOfBuses](std::vector<double> v) -> double {
				return v.at(busNumber - 1 + numberOfBuses) - bus.getVoltageMagnitude().value();
			};
			break;
		case TypeOfBus::PQ:
			fip = [bus, busNumber, this, numberOfBuses](std::vector<double> v) -> double {
				double sum{};
				for (uint8_t i{}; i < numberOfBuses; i++) {
					uint8_t k{};
					AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };
					while (!(admittanceMatrix.columnIndex.at(k) == i + 1 && admittanceMatrix.rowIndex.at(k) == busNumber)) {
						k++;
					}

					sum += v.at(i + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
						* std::cos(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(i));
				}

				return v.at(busNumber - 1 + numberOfBuses) * sum + bus.getActivePower().value();
			};
			fiq = [bus, busNumber, this, numberOfBuses](std::vector<double> v) -> double {
				double sum{};
				for (uint8_t i{}; i < numberOfBuses; i++) {
					uint8_t k{};
					AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };
					while (!(admittanceMatrix.columnIndex.at(k) == i + 1 && admittanceMatrix.rowIndex.at(k) == busNumber)) {
						k++;
					}

					sum += v.at(i + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
						* std::sin(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(i));
				}

				return v.at(busNumber - 1 + numberOfBuses) * sum + bus.getReactivePower().value();
			};
			break;
	}

	return { fip, fiq };
}



SystemModel::dfidx SystemModel::SystemModel::getDerivativesOfBusFunctions(uint8_t busNumber) const {
	if (busNumber > buses.size() || busNumber == 0) {
		throw std::out_of_range("Invalid bus number.");
	}

	std::vector<std::function<double(std::vector<double>)>> dfipdx, dfiqdx;

	const Bus& bus{ buses.at(busNumber - 1) };
	uint8_t numberOfBuses{ this->numberOfBuses };

	switch (bus.getTypeOfBus()) {
		case TypeOfBus::Slack:
			for (uint8_t i{1}; i < 2 * numberOfBuses + 1; i++) {
				dfipdx.push_back([i, busNumber](std::vector<double> v) -> double {
					return i == busNumber;
				});
			}
			for (uint8_t i{ 1 }; i < 2 * numberOfBuses + 1; i++) {
				dfiqdx.push_back([i, busNumber, numberOfBuses](std::vector<double> v) -> double {
					return i == busNumber + numberOfBuses;
				});
			}
			break;
		case TypeOfBus::PV:
			for (uint8_t i{ 1 }; i < 2 * numberOfBuses + 1; i++) {
				if (i == busNumber) {
					// dfip/dfi
					dfipdx.push_back([busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						double sum{};
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						for (uint8_t j{}; j < numberOfBuses; j++) {
							if (j + 1 == busNumber) {
								continue;
							}

							uint8_t k{};
							while (!(admittanceMatrix.columnIndex.at(k) == j + 1 && admittanceMatrix.rowIndex.at(k) == busNumber)) {
								k++;
							}

							sum += v.at(j + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
								* std::sin(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(j));
						}

						return - v.at(busNumber - 1 + numberOfBuses) * sum;
					});
				} else if (i <= numberOfBuses) {
					// dfip/dfj
					dfipdx.push_back([i, busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						uint8_t k{};
						while (!(admittanceMatrix.columnIndex.at(k) == i && admittanceMatrix.rowIndex.at(k) == busNumber)) {
							k++;
						}

						return v.at(busNumber - 1 + numberOfBuses) * v.at(i - 1 + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
							* std::sin(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(i - 1));
					});
				} else if (i == busNumber + numberOfBuses) {
					// dfip/dvi
					dfipdx.push_back([busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						double sum{};
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						for (uint8_t j{}; j < numberOfBuses; j++) {
							uint8_t k{};
							while (!(admittanceMatrix.columnIndex.at(k) == j + 1 && admittanceMatrix.rowIndex.at(k) == busNumber)) {
								k++;
							}
							if (j + 1 == busNumber) {
								sum += 2 * std::abs(admittanceMatrix.values.at(k)) * v.at(busNumber - 1 + numberOfBuses)
									* std::cos(std::arg(admittanceMatrix.values.at(k)));
							} else {
								sum += std::abs(admittanceMatrix.values.at(k)) * v.at(j + numberOfBuses)
									* std::cos(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(j));
							}
						}

						return sum;
					});
				} else {
					// dfip/dvj
					dfipdx.push_back([i, busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						uint8_t k{};
						while (!(admittanceMatrix.columnIndex.at(k) == ((i <= numberOfBuses) ? (i) : (i - numberOfBuses)) && admittanceMatrix.rowIndex.at(k) == busNumber)) {
							k++;
						}

						return v.at(busNumber - 1 + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
							* std::cos(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(((i <= numberOfBuses) ? (i) : (i - numberOfBuses)) - 1));
					});
				}
			}
			for (uint8_t i{ 1 }; i < 2 * numberOfBuses + 1; i++) {
				dfiqdx.push_back([i, busNumber, numberOfBuses](std::vector<double> v) -> double {
					return i == busNumber + numberOfBuses;
				});
			}
			break;
		case TypeOfBus::PQ:
			for (uint8_t i{ 1 }; i < 2 * numberOfBuses + 1; i++) {
				if (i == busNumber) {
					// dfip/dfi
					dfipdx.push_back([busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						double sum{};
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						for (uint8_t j{}; j < numberOfBuses; j++) {
							if (j + 1 == busNumber) {
								continue;
							}

							uint8_t k{};
							while (!(admittanceMatrix.columnIndex.at(k) == j + 1 && admittanceMatrix.rowIndex.at(k) == busNumber)) {
								k++;
							}

							sum += v.at(j + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
								* std::sin(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(j));
						}

						return - v.at(busNumber - 1 + numberOfBuses) * sum;
					});
				} else if (i <= numberOfBuses) {
					// dfip/dfj
					dfipdx.push_back([i, busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						uint8_t k{};
						while (!(admittanceMatrix.columnIndex.at(k) == i && admittanceMatrix.rowIndex.at(k) == busNumber)) {
							k++;
						}

						return v.at(busNumber - 1 + numberOfBuses) * v.at(i - 1 + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
							* std::sin(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(i - 1));
					});
				} else if (i == busNumber + numberOfBuses) {
					// dfip/dvi
					dfipdx.push_back([busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						double sum{};
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						for (uint8_t j{}; j < numberOfBuses; j++) {
							uint8_t k{};
							while (!(admittanceMatrix.columnIndex.at(k) == j + 1 && admittanceMatrix.rowIndex.at(k) == busNumber)) {
								k++;
							}
							if (j + 1 == busNumber) {
								sum += 2 * std::abs(admittanceMatrix.values.at(k)) * v.at(busNumber - 1 + numberOfBuses)
									* std::cos(std::arg(admittanceMatrix.values.at(k)));
							} else {
								sum += std::abs(admittanceMatrix.values.at(k)) * v.at(j + numberOfBuses)
									* std::cos(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(j));
							}
						}

						return sum;
					});
				} else {
					// dfip/dvj
					dfipdx.push_back([i, busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						uint8_t k{};
						while (!(admittanceMatrix.columnIndex.at(k) == ((i <= numberOfBuses) ? (i) : (i - numberOfBuses)) && admittanceMatrix.rowIndex.at(k) == busNumber)) {
							k++;
						}

						return v.at(busNumber - 1 + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
							* std::cos(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(((i <= numberOfBuses) ? (i) : (i - numberOfBuses)) - 1));
					});
				}
			}
			for (uint8_t i{ 1 }; i < 2 * numberOfBuses + 1; i++) {
				if (i == busNumber) {
					// dfiq/dfi
					dfiqdx.push_back([busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						double sum{};
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						for (uint8_t j{}; j < numberOfBuses; j++) {
							if (j + 1 == busNumber) {
								continue;
							}

							uint8_t k{};
							while (!(admittanceMatrix.columnIndex.at(k) == j + 1 && admittanceMatrix.rowIndex.at(k) == busNumber)) {
								k++;
							}

							sum += v.at(j + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
								* std::cos(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(j));
						}

						return v.at(busNumber - 1 + numberOfBuses) * sum;
					});
				} else if (i <= numberOfBuses) {
					// dfiq/dfj
					dfiqdx.push_back([i, busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						uint8_t k{};
						while (!(admittanceMatrix.columnIndex.at(k) == i && admittanceMatrix.rowIndex.at(k) == busNumber)) {
							k++;
						}

						return - v.at(busNumber - 1 + numberOfBuses) * v.at(i - 1 + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
							* std::cos(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(i - 1));
					});
				} else if (i == busNumber + numberOfBuses) {
					// dfiq/dvi
					dfiqdx.push_back([busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						double sum{};
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						for (uint8_t j{}; j < numberOfBuses; j++) {
							uint8_t k{};
							while (!(admittanceMatrix.columnIndex.at(k) == j + 1 && admittanceMatrix.rowIndex.at(k) == busNumber)) {
								k++;
							}
							if (j + 1 == busNumber) {
								sum -= 2 * std::abs(admittanceMatrix.values.at(k)) * v.at(busNumber - 1 + numberOfBuses)
									* std::sin(std::arg(admittanceMatrix.values.at(k)));
							} else {
								sum += std::abs(admittanceMatrix.values.at(k)) * v.at(j + numberOfBuses)
									* std::sin(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(j));
							}
						}

						return sum;
					});
				} else {
					// dfiq/dvj
					dfiqdx.push_back([i, busNumber, numberOfBuses, this](std::vector<double> v) -> double {
						AdmittanceMatrix admittanceMatrix{ this->getAdmittanceMatrix() };

						uint8_t k{};
						while (!(admittanceMatrix.columnIndex.at(k) == ((i <= numberOfBuses) ? (i) : (i - numberOfBuses)) && admittanceMatrix.rowIndex.at(k) == busNumber)) {
							k++;
						}

						return v.at(busNumber - 1 + numberOfBuses) * std::abs(admittanceMatrix.values.at(k))
							* std::sin(v.at(busNumber - 1) - std::arg(admittanceMatrix.values.at(k)) - v.at(((i <= numberOfBuses) ? (i) : (i - numberOfBuses)) - 1));
					});
				}
			}
			break;
	}

	return { dfipdx, dfiqdx };
}