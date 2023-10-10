#pragma once

#include "comms/Serializable.hpp"
#include "comms/api.hpp"
#include "pros/apix.h"

namespace comms {

template <class PacketT> class SendComms {
	static_assert(std::is_base_of<Serializable, PacketT>::value,
	              "Packet type must derive from comms::Serializable");

private:
	pros::Serial ser;
	unsigned char start;

public:
	SendComms(int port, int baud, unsigned char start)
	    : ser(port, baud), start(start) {
		this->ser.flush();
	}

	void send_data(PacketT packet) {
		unsigned char buffer[MAX_BUFFER_SIZE];
		int buffer_len = packet.serialize(buffer);

		this->ser.flush();

		this->ser.write_byte(0);
		this->ser.write_byte(0);
		this->ser.write_byte(0);
		this->ser.write_byte(this->start);

		for (int i = 0; i < sizeof(buffer_len); i++) {
			unsigned char c = (buffer_len & (0xFF << (i * 8))) >> (i * 8);
			this->ser.write_byte(c);
		}

		for (int i = 0; i < buffer_len; i++) {
			unsigned char c = buffer[i];
			this->ser.write_byte(c);
		}
	}
};

} // namespace comms