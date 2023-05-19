/*
MIT License

Copyright (c) 2023 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/* ================ LICENSE END ================ */

#include "connection_thread.hpp"

#include "server.hpp"

using namespace tcpserver;

connection_thread::connection_thread(server& owner) :
	nitki::loop_thread(1),
	owner(owner)
{}

connection_thread::~connection_thread()
{
	this->wait_set.remove(this->connection->socket);
	LOG([](auto& o) {
		o << "connection_thread destroyed" << std::endl;
	})
}

void connection_thread::push(const utki::shared_ref<tcpserver::connection>& conn)
{
	this->push_back([this, c = conn]() mutable {
		ASSERT(!this->connection)
		this->connection = c.to_shared_ptr();

		this->wait_set.add(
			this->connection->socket,
			opros::ready::read, // connection is initially receiving data
			this->connection.get()
		);
	});
}

std::optional<uint32_t> connection_thread::on_loop()
{
	for (auto& t : this->wait_set.get_triggered()) {
		if (!t.user_data) {
			// waitbale is a nitki::queue, skip it
			continue;
		}

		// waitable is a tcp_socket
		// std::cout << "socket ready: " << t.flags << std::endl;

		// check for error condition
		if (t.flags.get(opros::ready::error)) {
			LOG([](auto& o) {
				o << "socket error" << std::endl;
			})

			// TODO: reclaim only connection?
			this->owner.reclaim_thread(*this);
			return {};
		}

		auto c = reinterpret_cast<tcpserver::connection*>(t.user_data);

		if (c->status.get(opros::ready::read) && t.flags.get(opros::ready::read)) {
			// data has arrived to socket and connection is able to receive it

			constexpr const size_t receive_buffer_size = 0x1000; // 4kb
			std::array<uint8_t, receive_buffer_size> buf;

			try {
				bool first_read = true;
				while (size_t num_bytes_received = c->socket.receive(buf)) {
					first_read = false;
					ASSERT(num_bytes_received > 0)

					auto prev_status = c->status;

					c->handle_data_received(utki::make_span(buf.data(), num_bytes_received));

					// write flag can also change, so compare statuses as a whole
					if (c->status != prev_status) {
						// connection status has changed, update waiting flags
						this->wait_set.change(c->socket, c->status, c);
						break;
					}
				}
				if (first_read) {
					// zero bytes received right after the object has become ready to read,
					// this means that the connection is disconnected
					LOG([](auto& o) {
						o << "disconnected (receive), quit thread!" << std::endl;
					})
					// destroy thread
					this->owner.reclaim_thread(*this);
					break;
				}
			} catch (std::exception& e) {
				LOG([&](auto& o) {
					o << "std::exception caught while receiving data from socket: " << e.what() << std::endl;
				})
			}
		}

		// It shouldn't be possible that socket is ready to write, but connection is
		// not ready to write, because then we shouldn't be waiting for writing on the socket.
		ASSERT(!(!c->status.get(opros::ready::write) && t.flags.get(opros::ready::write)))

		if (t.flags.get(opros::ready::write)) {
			bool is_first_send = true;
			while (true) {
				// if we were waiting on socket for writing, then connection is ready to write as well
				ASSERT(c->status.get(opros::ready::write))
				ASSERT(!c->sending_queue.empty())

				// connection has data to send and socket is ready for sending

				ASSERT(c->num_bytes_sent < c->sending_queue.front().size())

				const auto& data_to_send = c->sending_queue.front();

				auto span =
					utki::make_span(data_to_send.data() + c->num_bytes_sent, data_to_send.size() - c->num_bytes_sent);

				size_t num_bytes_sent = c->socket.send(span);

				ASSERT(num_bytes_sent <= span.size())

				if (num_bytes_sent == 0 && is_first_send) {
					// disconnected
					LOG([](auto& o) {
						o << "disconnected (send), quit thread!" << std::endl;
					})
					// destroy thread
					this->owner.reclaim_thread(*this);
					return {};
				}

				is_first_send = false;

				if (num_bytes_sent == span.size()) {
					c->sending_queue.pop_front();
					c->num_bytes_sent = 0;

					if (!c->sending_queue.empty()) {
						// the socket is possibly still ready to write, try to send more data
						continue;
					}

					auto old_status = c->status;

					ASSERT(c->sending_queue.empty())
					c->status.clear(opros::ready::write);
					c->handle_all_data_sent();

					if (c->status.get(opros::ready::write)) {
						ASSERT(c->status.get(opros::ready::write))

						ASSERT(!c->sending_queue.empty())
						ASSERT(!c->sending_queue.front().empty())
						ASSERT(c->num_bytes_sent != c->sending_queue.front().size())
						// the socket is possibly still ready to write, try to send more data
						continue;
					}

					// read flag can also change, so compare statuses as a whole
					if (c->status != old_status) {
						// connection status has changed, update waiting flags
						this->wait_set.change(c->socket, c->status, c);
					}
				} else {
					c->num_bytes_sent += num_bytes_sent;
				}

				break;
			} // ~while(true)
		}
	}

	return {};
}
