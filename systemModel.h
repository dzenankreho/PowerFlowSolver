#pragma once
#include <vector>
#include <complex>
#include <optional>
#include <functional>
#include <utility>
#include <tuple>



namespace SystemModel {
	using fi = std::pair<std::function<double(std::vector<double>)>, std::function<double(std::vector<double>)>>;



	using dfidx = std::pair<std::vector<std::function<double(std::vector<double>)>>, std::vector<std::function<double(std::vector<double>)>>>;



	using AdmittanceMatrix = std::vector<std::tuple<uint8_t, uint8_t, std::complex<double>>>;



	enum class TypeOfBus { Slack, PV, PQ };



	enum class ThreePhaseLoadConfigurationsType { Star, GroundedStar, Delta };



	class Bus {
		TypeOfBus typeOfBus;
		std::optional<double> voltageMagnitude;
		std::optional<double> voltagePhase;
		std::optional<double> activePower;
		std::optional<double> reactivePower;
	public:
		Bus(TypeOfBus typeOfBus) : typeOfBus{ typeOfBus } {}

		TypeOfBus getTypeOfBus() const {
			return typeOfBus;
		}

		void setVoltageMagnitude(double voltageMagnitude);

		void setVoltagePhase(double voltagePhase);

		void setActivePower(double activePower);

		void setReactivePower(double reactivePower);

		std::optional<double> getVoltageMagnitude() const;

		std::optional<double> getVoltagePhase() const;

		std::optional<double> getActivePower() const;

		std::optional<double> getReactivePower() const;
	};



	class SystemModel {
			AdmittanceMatrix admittanceMatrix;
			uint8_t numberOfBuses{};
			std::vector<Bus> buses;
			const uint8_t maxNumberOfBuses;
			bool checkForConnectionBetweenToBuses(uint8_t busNumber1, uint8_t busNumber2) const;
		public:
			SystemModel(uint8_t maxNumberOfBuses) : maxNumberOfBuses{ maxNumberOfBuses } {}

			AdmittanceMatrix getAdmittanceMatrix() const {
				return admittanceMatrix;
			}

			uint8_t getNumberOfBuses() const {
				return numberOfBuses;
			}

			Bus& getBus(uint8_t busNumber);

			void addBus(TypeOfBus typeOfBus);

			void addLoad(uint8_t busNumber, double activePower, double reactivePower);

			void addLine(uint8_t busNumber1, uint8_t busNumber2, double r, double x, double b);

			friend std::ostream& operator <<(std::ostream& stream, const SystemModel& systemModel);

			void addGenerator(uint8_t busNumber, double voltageMagnitude, double activePower);

			void addSlackGenerator(uint8_t busNumber, double voltageMagnitude, double voltagePhase);

			bool hasSlackBeenAssigned() const;

			void addTransformer(uint8_t busNumber1, uint8_t busNumber2, double r, double x, double g, double b, double n);

			void addCapacitorBank(uint8_t busNumber, double c, ThreePhaseLoadConfigurationsType configurationType);

			fi getBusFunctions(uint8_t busNumber) const;

			dfidx getDerivativesOfBusFunctions(uint8_t busNumber) const;
	};

	

	std::ostream& operator <<(std::ostream& stream, const SystemModel& systemModel);
}