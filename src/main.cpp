#include "main.h"
#include "comms/api.hpp"

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {
}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
}

struct Packet : comms::Serializable, comms::Deserializable {
	int data_int;

	int serialize(unsigned char* buffer) {
		for (int i = 0; i < sizeof(data_int); i++) {
			buffer[i] = (data_int & (0xFF << (i * 8))) >> (i * 8);
		}

		return sizeof(data_int);
	}

	void deserialize(unsigned char* data) {
		this->data_int = 0;
		for (int i = 0; i < sizeof(data_int); i++) {
			data_int += (data[i] << (i * 8));
		}
	}
};

/**
 * Runs the operator control code. This function will be started in
 * its own task with the default priority and stack size whenever the
 * robot is enabled via the Field Management System or the VEX
 * Competition Switch in the operator control mode.
 *
 * If no competition control is connected, this function will run
 * immediately following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will
 * restart the task, not resume it from where it left off.
 */
void opcontrol() {
	Packet outbound;
	outbound.data_int = -1102938091;

	comms::ReceiveComms<Packet> rx(9, 9600, '!');
	comms::SendComms<Packet> tx(19, 9600, '!');

	rx.begin();

	bool sending = false;

	while (true) {

		if (!sending) {
			if (rx.has_data()) {
				auto inbound = rx.get_data();
				pros::lcd::clear_line(1);
				pros::lcd::print(1, "inbound data_int: %d", inbound.data_int);
			} else {
				pros::lcd::clear_line(1);
				pros::lcd::print(1, "no data");
			}
		} else {
			pros::lcd::clear_line(1);
			pros::lcd::print(1, "outbound data_int: %d", outbound.data_int);
			tx.send_data(outbound);
		}

		pros::delay(10);
	}
}