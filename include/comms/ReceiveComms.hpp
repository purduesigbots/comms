#pragma once

#include "comms/Deserializable.hpp"
#include "comms/api.hpp"

#include "pros/apix.h"
#include "pros/rtos.hpp"

#include <memory>
#include <queue>

namespace comms {

template <class PacketT> class ReceiveComms {
	static_assert(std::is_base_of<Deserializable, PacketT>::value,
	              "Packet type must derive from comms::Deserializable");

private:
	pros::Serial ser;
	pros::Mutex queue_mtx;
	std::queue<PacketT> packet_q;
	unsigned char start;

	std::shared_ptr<pros::rtos::Task> serial_t;

	void receive_task() {
		while (true) {
			pros::delay(3);

			if (this->ser.get_read_avail() > 0) {
				int buffer_len = 0;

				bool started = false;
				int finding_len = sizeof(int);
				unsigned char buffer[MAX_BUFFER_SIZE];
				int ptr = 0;

				while (this->ser.get_read_avail() > 0) {
					unsigned char in = this->ser.read_byte();

					if (in == this->start && !started) {
						started = true;
						continue;
					}

					if (started) {
						if (finding_len > 0) {
							finding_len--;
							buffer_len += in << ((3 - finding_len) * 8);
						} else if (buffer_len > 0) {
							buffer[ptr] = in;
							buffer_len--;
							ptr++;
						}

						if (finding_len == 0 && buffer_len == 0) {
							PacketT new_packet;
							new_packet.deserialize(buffer);

							this->queue_mtx.take();
							this->packet_q.push(new_packet);
							this->queue_mtx.give();

							started = false;
							finding_len = sizeof(int);
							ptr = 0;
						}
					}
				}
			}
		}
	}

public:
	ReceiveComms(int port, int baud, unsigned char start)
	    : ser(port, baud), start(start) {
		this->ser.flush();
		this->serial_t = nullptr;
	}

	void begin() {
		if (this->serial_t == nullptr) {
			this->serial_t =
			    std::make_shared<pros::Task>([this] { this->receive_task(); });
		} else {
			this->serial_t->resume();
		}
	}

	void pause() {
		if (this->serial_t != nullptr) {
			this->serial_t->suspend();
		}
	}

	bool has_data() {
		this->queue_mtx.take();
		bool is_empty = this->packet_q.empty();
		this->queue_mtx.give();

		return !is_empty;
	}

	PacketT get_data() {
		this->queue_mtx.take();
		PacketT ret = this->packet_q.front();
		this->packet_q.pop();
		this->queue_mtx.give();

		return ret;
	}
};

} // namespace comms