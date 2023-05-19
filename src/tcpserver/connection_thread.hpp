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

#include <nitki/loop_thread.hpp>
#include <nitki/queue.hpp>
#include <opros/wait_set.hpp>
#include <setka/tcp_socket.hpp>
#include <utki/shared_ref.hpp>

#include "connection.hpp"

namespace tcpserver {

class server;

class connection_thread : public nitki::loop_thread
{
	friend class server;

	server& owner;
	std::list<connection_thread>::iterator owner_iter;

	// TODO: handle several connections
	std::shared_ptr<tcpserver::connection> connection;

public:
	connection_thread(server& owner);
	~connection_thread() override;

	std::optional<uint32_t> on_loop() override;

	void push(const utki::shared_ref<tcpserver::connection>& conn);
};

} // namespace pautina
