#pragma once

#include "pros/apix.h"

#include <string>

namespace comms {

struct Serializable {

	virtual int serialize(unsigned char* buffer) = 0;

	virtual ~Serializable() {
	}
};

} // namespace comms