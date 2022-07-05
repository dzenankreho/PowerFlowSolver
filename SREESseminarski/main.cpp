#include <iostream>
#include "systemModel.h"
#include "newtonRaphson.h"

const double eps{ 1e-10 };

int main() {
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

	std::vector<double> x0{ 1, 1, 1,1.000000000000000, 1.020000000000000, 1 },x(6);
	double err;
	int maxNumberOfIter = 50,iter;
	newtonRaphson(systemModel, maxNumberOfIter, eps, x0, x, err, iter);
	return 0;
}