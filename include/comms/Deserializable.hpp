#pragma once

#include <string>

namespace comms {

struct Deserializable {

	virtual void deserialize(unsigned char* data) = 0;

	virtual ~Deserializable() {
	}
};

} // namespace comms