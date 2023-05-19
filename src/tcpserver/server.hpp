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

#pragma once

#include <list>
#include <string>

#include <nitki/loop_thread.hpp>
#include <setka/init_guard.hpp>
#include <setka/tcp_server_socket.hpp>

#include "connection_thread.hpp"

namespace tcpserver {

class server : public nitki::loop_thread
{
	friend class connection_thread;

	std::shared_ptr<utki::destructable> setka_init_guard = setka::get_init_guard_reference();

	setka::tcp_server_socket accept_socket;

	std::list<connection_thread> threads;

	void spawn_thread(setka::tcp_socket&& socket);

	void reclaim_thread(connection_thread& t);

	std::optional<uint32_t> on_loop() override;

public:
	struct configuration {
		uint16_t port = 80;
	};

	server(const configuration& config);
	~server() override;

	virtual utki::shared_ref<connection> spawn_connection(setka::tcp_socket&& socket) const = 0;
};

} // namespace pautina
