#include "pch.h"
#include "CppUnitTest.h"
#include <iostream>
#include <exception>
#include <string>
#include <sstream>

#include "../SREESseminarski/systemModel.cpp"  
#include "../SREESseminarski/newtonRaphson.cpp"  

const double eps{ 1e-10 };

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SystemModelTests {
	TEST_CLASS(SystemModelTests) {
		public:

			TEST_METHOD(slackBusTest) {

				SystemModel::Bus bus{SystemModel::TypeOfBus::Slack};


				bus.setVoltageMagnitude(1.0);
				Assert::AreEqual(1.0, bus.getVoltageMagnitude().value(), eps);


				bus.setVoltagePhase(1.1);
				Assert::AreEqual(1.1, bus.getVoltagePhase().value(), eps);


				Assert::ExpectException<std::logic_error>([&bus]() { bus.setActivePower(1.0); });
				Assert::ExpectException<std::logic_error>([&bus]() { bus.getActivePower(); });


				Assert::ExpectException<std::logic_error>([&bus]() { bus.setReactivePower(1.0); });
				Assert::ExpectException<std::logic_error>([&bus]() { bus.getReactivePower(); });
			}

			TEST_METHOD(pvBusTest) {

				SystemModel::Bus bus{ SystemModel::TypeOfBus::PV };


				bus.setVoltageMagnitude(1.0);
				Assert::AreEqual(1.0, bus.getVoltageMagnitude().value(), eps);


				bus.setActivePower(1.1);
				Assert::AreEqual(1.1, bus.getActivePower().value(), eps);


				Assert::ExpectException<std::logic_error>([&bus]() { bus.setVoltagePhase(1.0); });
				Assert::ExpectException<std::logic_error>([&bus]() { bus.getVoltagePhase(); });


				Assert::ExpectException<std::logic_error>([&bus]() { bus.setReactivePower(1.0); });
				Assert::ExpectException<std::logic_error>([&bus]() { bus.getReactivePower(); });
			}

			TEST_METHOD(pqBusTest) {

				SystemModel::Bus bus{ SystemModel::TypeOfBus::PQ };


				bus.setReactivePower(1.0);
				Assert::AreEqual(1.0, bus.getReactivePower().value(), eps);


				bus.setActivePower(1.1);
				Assert::AreEqual(1.1, bus.getActivePower().value(), eps);


				Assert::ExpectException<std::logic_error>([&bus]() { bus.setVoltagePhase(1.0); });
				Assert::ExpectException<std::logic_error>([&bus]() { bus.getVoltagePhase(); });


				Assert::ExpectException<std::logic_error>([&bus]() { bus.setVoltagePhase(1.0); });
				Assert::ExpectException<std::logic_error>([&bus]() { bus.getVoltagePhase(); });
			}

			TEST_METHOD(getBusTest) {
				SystemModel::SystemModel systemModel{ 10 };

				systemModel.addBus(SystemModel::TypeOfBus::PQ);

				systemModel.getBus(1).setActivePower(1.01);
				systemModel.getBus(1).setReactivePower(1.03);

				Assert::AreEqual(int(SystemModel::TypeOfBus::PQ), int(systemModel.getBus(1).getTypeOfBus()));
				Assert::AreEqual(1.01, systemModel.getBus(1).getActivePower().value());
				Assert::AreEqual(1.03, systemModel.getBus(1).getReactivePower().value());

				Assert::ExpectException<std::logic_error>([&systemModel]() { systemModel.getBus(3); });
			}

			TEST_METHOD(addLoadTest) {
				SystemModel::SystemModel systemModel{ 10 };
				
				systemModel.addBus(SystemModel::TypeOfBus::PQ);

				systemModel.addLoad(1, 1.01, 1.03);

				Assert::AreEqual(1.01, systemModel.getBus(1).getActivePower().value(), eps);
				Assert::AreEqual(1.03, systemModel.getBus(1).getReactivePower().value(), eps);
			}

			TEST_METHOD(hasSlackBeenAssignedTest) {
				SystemModel::SystemModel systemModel{ 10 };

				Assert::IsFalse(systemModel.hasSlackBeenAssigned());

				systemModel.addBus(SystemModel::TypeOfBus::Slack);

				Assert::IsTrue(systemModel.hasSlackBeenAssigned());
			}

			TEST_METHOD(systemModelTest1) {
				std::stringstream stringStream;

				SystemModel::SystemModel systemModel{ 10 };

				systemModel.addBus(SystemModel::TypeOfBus::Slack);
				systemModel.getBus(1).setVoltageMagnitude(1);
				systemModel.getBus(1).setVoltagePhase(0);

				systemModel.addBus(SystemModel::TypeOfBus::PQ);
				systemModel.getBus(2).setActivePower(1.01);
				systemModel.getBus(2).setReactivePower(1.03);

				systemModel.addBus(SystemModel::TypeOfBus::PQ);
				systemModel.getBus(3).setActivePower(1.25);
				systemModel.getBus(3).setReactivePower(1.36);

				systemModel.addLine(1, 2, 0.1, 0.2, 3.0);
				systemModel.addTransformer(1, 3, 0.4, 0.5, 10.0, 3.0);

				stringStream << systemModel;

				std::string output;
				
				while (stringStream) {
					if (!output.empty()) {
						output += "\n";
					}
					std::string tmp;
					std::getline(stringStream, tmp);
					output += tmp;
				}

				Assert::AreEqual("Buses:\n\tBus: 1\n\t\tType: Slack\n\t\tVoltage magnitude: 1\n\t\tVoltage phase: 0\n\tBus: 2\n\t\tType: PQ\n\t\tActive power: 1.01\n\t\tReactive power: 1.03\n\tBus: 3\n\t\tType: PQ\n\t\tActive power: 1.25\n\t\tReactive power: 1.36\n\nSystem admittance matrix:\n  (7.97561,-5.21951)               (-2,4)   (-0.97561,1.21951) \n              (-2,4)             (2,-2.5)                (0,0) \n  (-0.97561,1.21951)                (0,0)   (5.97561,-2.71951) \n",
					output.c_str());
			}

			TEST_METHOD(addLineAndEqualsTest) {
				std::stringstream stringStream;

				SystemModel::SystemModel systemModel{ 10 };

				std::complex<double> v1{ 0.8, 0.6 };

				systemModel.addBus(SystemModel::TypeOfBus::Slack);
				systemModel.getBus(1).setVoltageMagnitude(std::abs(v1));
				systemModel.getBus(1).setVoltagePhase(std::arg(v1));

				systemModel.addBus(SystemModel::TypeOfBus::PV);
				systemModel.getBus(2).setActivePower(3);
				systemModel.getBus(2).setVoltageMagnitude(1.02);

				systemModel.addBus(SystemModel::TypeOfBus::PQ);
				systemModel.getBus(3).setActivePower(1.5);
				systemModel.getBus(3).setReactivePower(0.8);

				systemModel.addLine(1, 2, 0.05, 0.1, 0);
				systemModel.addLine(1, 3, 0.025, 0.03, 0);
				systemModel.addLine(2, 3, 0.02, 0.02, 0);

				stringStream << systemModel;

				std::string output;

				while (stringStream) {
					if (!output.empty()) {
						output += "\n";
					}
					std::string tmp;
					std::getline(stringStream, tmp);
					output += tmp;
				}

				Assert::AreEqual("Buses:\n\tBus: 1\n\t\tType: Slack\n\t\tVoltage magnitude: 1\n\t\tVoltage phase: 0.643501\n\tBus: 2\n\t\tType: PV\n\t\tActive power: 3\n\t\tVoltage magnitude: 1.02\n\tBus: 3\n\t\tType: PQ\n\t\tActive power: 1.5\n\t\tReactive power: 0.8\n\nSystem admittance matrix:\n  (20.3934,-27.6721)               (-4,8)   (-16.3934,19.6721) \n              (-4,8)             (29,-33)             (-25,25) \n  (-16.3934,19.6721)             (-25,25)   (41.3934,-44.6721) \n",
					output.c_str());
			}

			TEST_METHOD(getBusFunctionsTest) {
				SystemModel::SystemModel systemModel{ 10 };

				std::complex<double> v1{ 0.8, 0.6 };

				systemModel.addBus(SystemModel::TypeOfBus::Slack);
				systemModel.getBus(1).setVoltageMagnitude(std::abs(v1));
				systemModel.getBus(1).setVoltagePhase(std::arg(v1));

				systemModel.addBus(SystemModel::TypeOfBus::PV);
				systemModel.getBus(2).setActivePower(3);
				systemModel.getBus(2).setVoltageMagnitude(1.02);

				systemModel.addBus(SystemModel::TypeOfBus::PQ);
				systemModel.getBus(3).setActivePower(1.5);
				systemModel.getBus(3).setReactivePower(0.8);

				systemModel.addLine(1, 2, 0.05, 0.1, 0);
				systemModel.addLine(1, 3, 0.025, 0.03, 0);
				systemModel.addLine(2, 3, 0.02, 0.02, 0);

				std::vector<double> x0{ std::arg(v1), std::arg(v1), std::arg(v1), std::abs(v1), 1.02, std::abs(v1) };

				Assert::AreEqual(0, systemModel.getBusFunctions(1).first(x0), eps);
				Assert::AreEqual(0, systemModel.getBusFunctions(1).second(x0), eps);
				Assert::AreEqual(-2.4084, systemModel.getBusFunctions(2).first(x0), eps);
				Assert::AreEqual(0, systemModel.getBusFunctions(2).second(x0), eps);
				Assert::AreEqual(1, systemModel.getBusFunctions(3).first(x0), eps);
				Assert::AreEqual(0.3, systemModel.getBusFunctions(3).second(x0), eps);
			}

			TEST_METHOD(getDerivativesOfBusFunctionsTest2) {
				SystemModel::SystemModel systemModel{ 10 };

				std::complex<double> v1{ 0.8, 0.6 };

				systemModel.addBus(SystemModel::TypeOfBus::Slack);
				systemModel.getBus(1).setVoltageMagnitude(std::abs(v1));
				systemModel.getBus(1).setVoltagePhase(std::arg(v1));

				systemModel.addBus(SystemModel::TypeOfBus::PV);
				systemModel.getBus(2).setActivePower(3);
				systemModel.getBus(2).setVoltageMagnitude(1.02);

				systemModel.addBus(SystemModel::TypeOfBus::PQ);
				systemModel.getBus(3).setActivePower(1.5);
				systemModel.getBus(3).setReactivePower(0.8);

				systemModel.addLine(1, 2, 0.05, 0.1, 0);
				systemModel.addLine(1, 3, 0.025, 0.03, 0);
				systemModel.addLine(2, 3, 0.02, 0.02, 0);

				std::vector<double> x0{ std::arg(v1), std::arg(v1), std::arg(v1), std::abs(v1), 1.02, std::abs(v1) };

				std::vector<std::vector<double>> actualValues{ { 1, 0, 0, 0, 0, 0 },
					{ -8.160000000000000, 33.659999999999997, -25.500000000000000, -4.080000000000000, 30.160000000000000, -25.500000000000000 },
					{ -19.672131147540988, -25.500000000000000, 45.172131147540988, -16.393442622950815, -25.000000000000000, 40.893442622950822 },
					{ 0, 0, 0, 1, 0, 0 },
					{ 0, 0, 0, 0, 1, 0 },
					{ 16.393442622950815, 25.500000000000000, -41.893442622950815, -19.672131147540988, -25.000000000000000, 44.172131147540973 } };

				for (int k : {0, 1}) {
					for (int i{}; i < systemModel.getNumberOfBuses(); i++) {
						for (int j{}; j < 2 * systemModel.getNumberOfBuses(); j++) {
							Assert::AreEqual(actualValues.at(i + k * systemModel.getNumberOfBuses()).at(j),
								((k == 0) ? (systemModel.getDerivativesOfBusFunctions(i + 1).first.at(j)(x0)) : (systemModel.getDerivativesOfBusFunctions(i + 1).second.at(j)(x0))), eps);

						}
					}
				}
			}

			TEST_METHOD(getDerivativesOfBusFunctionsTest) {
				SystemModel::SystemModel systemModel{ 10 };

				std::complex<double> v1{ 0.8, 0.6 };

				systemModel.addBus(SystemModel::TypeOfBus::Slack);
				systemModel.getBus(1).setVoltageMagnitude(std::abs(v1));
				systemModel.getBus(1).setVoltagePhase(std::arg(v1));

				systemModel.addBus(SystemModel::TypeOfBus::PV);
				systemModel.getBus(2).setActivePower(3);
				systemModel.getBus(2).setVoltageMagnitude(1.02);

				systemModel.addBus(SystemModel::TypeOfBus::PQ);
				systemModel.getBus(3).setActivePower(1.5);
				systemModel.getBus(3).setReactivePower(0.8);

				systemModel.addLine(1, 2, 0.05, 0.1, 0);
				systemModel.addLine(1, 3, 0.025, 0.03, 0);
				systemModel.addLine(2, 3, 0.02, 0.02, 0);

				std::vector<double> x1{ 0.643501108793284, 0.737073919144267, 0.689630454314943,1.000000000000000, 1.020000000000000, 0.982939705318109 };

				std::vector<std::vector<double>> actualValues{ { 1, 0, 0, 0, 0, 0 },
					{ -8.505522283881552,  34.731003522701748, -26.225481238820194, -3.299710696166307,  32.544562030819534, -24.261952084517549 },
					{ -20.059003064482091, -23.848036032397765,  43.907039096879856, -15.204959680077126, -25.711256116490386, 39.224992364970198 },
					{ 0, 0, 0, 1, 0, 0 },
					{ 0, 0, 0, 0, 1, 0 },
					{  15.204959680077126,  26.225481238820194, -41.430440918897318, -20.059003064482091, -23.380427482742906,  43.150915622801477 } };

				for (int k : {0, 1}) {
					for (int i{}; i < systemModel.getNumberOfBuses(); i++) {
						for (int j{}; j < 2 * systemModel.getNumberOfBuses(); j++) {
							Assert::AreEqual(actualValues.at(i + k * systemModel.getNumberOfBuses()).at(j),
								((k == 0) ? (systemModel.getDerivativesOfBusFunctions(i + 1).first.at(j)(x1)) : (systemModel.getDerivativesOfBusFunctions(i + 1).second.at(j)(x1))), eps);

						}
					}
				}
			}

			TEST_METHOD(systemModelWithSolverTest) {
				SystemModel::SystemModel systemModel{ 10 };

				std::complex<double> v1{ 0.8, 0.6 };

				systemModel.addBus(SystemModel::TypeOfBus::Slack);
				systemModel.getBus(1).setVoltageMagnitude(std::abs(v1));
				systemModel.getBus(1).setVoltagePhase(std::arg(v1));

				systemModel.addBus(SystemModel::TypeOfBus::PV);
				systemModel.getBus(2).setActivePower(3);
				systemModel.getBus(2).setVoltageMagnitude(1.02);

				systemModel.addBus(SystemModel::TypeOfBus::PQ);
				systemModel.getBus(3).setActivePower(1.5);
				systemModel.getBus(3).setReactivePower(0.8);

				systemModel.addLine(1, 2, 0.05, 0.1, 0);
				systemModel.addLine(1, 3, 0.025, 0.03, 0);
				systemModel.addLine(2, 3, 0.02, 0.02, 0);

				std::vector<double> x0{ 1, 1, 1,1.000000000000000, 1.020000000000000, 1 }, x(6);
				double err;
				int maxNumberOfIter = 50, iter;
				newtonRaphson(systemModel, maxNumberOfIter, eps, x0, x, err, iter);

				for (int i{}; i < x.size() / 2; i++) {
					x.at(i) *= 180 / (PI);
				}

				int i{};
				for (double el : {36.8698976458, 42.0564436745, 39.4010787087, 1.0, 1.02, 0.9816686928}) {
					Assert::AreEqual(el, x.at(i++), eps);
				}
			}

			TEST_METHOD(systemModelWithSolverTest2) {
				SystemModel::SystemModel systemModel{ 10 };

				systemModel.addBus(SystemModel::TypeOfBus::PQ);
				systemModel.getBus(1).setActivePower(1.5);
				systemModel.getBus(1).setReactivePower(0.6);

				systemModel.addBus(SystemModel::TypeOfBus::Slack);
				systemModel.getBus(2).setVoltageMagnitude(1.02);
				systemModel.getBus(2).setVoltagePhase(0);

				systemModel.addBus(SystemModel::TypeOfBus::PV);
				systemModel.getBus(3).setActivePower(1.8);
				systemModel.getBus(3).setVoltageMagnitude(1);

				systemModel.addLine(1, 2, 0.0028, 0.0281, 0.0071);
				systemModel.addLine(1, 3, 0.003, 0.0304, 0.0066);
				systemModel.addLine(2, 3, 0.0011, 0.0108, 0.0185);

				std::vector<double> x0{ 0, 0, 0, 1.02, 1.02, 1 }, x(6);
				double err;
				int maxNumberOfIter = 50, iter;
				newtonRaphson(systemModel, maxNumberOfIter, eps, x0, x, err, iter);

				for (int i{}; i < x.size() / 2; i++) {
					x.at(i) *= 180 / (4 * std::atan(1));
				}

				int i{};
				for (double el : { -0.872622603161, 0.0, 0.676001376542, 0.99931707839, 1.02, 1.0}) {
					Assert::AreEqual(el, x.at(i++), eps);
				}
			}
	};
}
