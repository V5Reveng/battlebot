#include <array>
#include <pros/misc.hpp>
#include <pros/motors.hpp>
#include <pros/rtos.hpp>

#include "main.hpp"

void initialize() {}
void disabled() {}
void competition_initialize() {}
void autonomous() {}

namespace motors {
static pros::Motor left_front{ 10 };
static pros::Motor left_back{ 15 };
static pros::Motor right_front{ 20, true };
static pros::Motor right_back{ 19, true };
}  // namespace motors

static pros::Controller controller{ pros::E_CONTROLLER_MASTER };

static constexpr double dampening = 1.0 / 3.0;

#if 0
static constexpr std::array<int32_t, 200> make_lookup_table() {
	std::array<int32_t, 200> ret{};
	for (int32_t i = -100; i < 100; ++i) {
		auto const input = static_cast<double>(i) / 100.0;
		ret[static_cast<size_t>(i + 100)] = static_cast<int32_t>(input * input * input * 127.0 * dampening);
	}
	return ret;
}

static constinit auto const lookup_table = make_lookup_table();

#define AXIS(AXIS_NAME) (lookup_table[static_cast<size_t>(controller.get_analog(pros::E_CONTROLLER_ANALOG_##AXIS_NAME) + 100)])
#else
constexpr int32_t axis_calc(int32_t raw_input) {
	double input = static_cast<double>(raw_input) / 100.0;
	return static_cast<int32_t>(input * input * input * 127.0 * dampening);
}
#define AXIS(AXIS_NAME) (axis_calc(controller.get_analog(pros::E_CONTROLLER_ANALOG_##AXIS_NAME)))
#endif

/*
There are three control axes that map to four motors.

The first axis is the forward axis. This one is the most simple: all the motors go forward when the forward axis is positive, and backward when the forward axis is negative. Therefore we can directly add it to all the motor speeds.

The second axis is the strafe axis. This one is a bit more complicated. Our wheels are diagonal omniwheels, in an "X" configuration, with the subwheels facing like so:

Front
\ /
/ \
Back

This means that, to strafe to the right, the left front and right back wheels must turn forward, and the right front and left back wheels must turn backward. Therefore we add the strafe axis to the left front and right back, and subtract it from the right front and left back.

Finally, the third axis is rotation. This works exactly like a regular tank turn: to turn clockwise, the wheels on the left side go forward, and the wheels on the right side go backward. Thus we add the rotation axis to the left motors, and subtract it from the right motors.
*/

void opcontrol() {
	while (true) {
		auto const rotate_axis = AXIS(RIGHT_X);
		auto const strafe_axis = AXIS(LEFT_X);
		auto const forward_axis = AXIS(LEFT_Y);
		using namespace motors;
		left_front.move(forward_axis + strafe_axis + rotate_axis);
		right_front.move(forward_axis - strafe_axis - rotate_axis);
		left_back.move(forward_axis - strafe_axis + rotate_axis);
		right_back.move(forward_axis + strafe_axis - rotate_axis);
		pros::c::delay(10);
	}
}
