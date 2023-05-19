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

#include "server.hpp"

using namespace tcpserver;

server::server(const configuration& config) :
	nitki::loop_thread(1),
	accept_socket(config.port)
{
	this->wait_set.add(this->accept_socket, opros::ready::read, &this->accept_socket);
}

server::~server()
{
	this->wait_set.remove(this->accept_socket);
}

std::optional<uint32_t> server::on_loop()
{
	for (const auto& t : this->wait_set.get_triggered()) {
		if (t.user_data == &this->accept_socket) {
			auto socket = this->accept_socket.accept();

			LOG([](auto& o) {
				o << "connection accepted" << std::endl;
			})
			this->spawn_thread(std::move(socket));
		}
	}

	return {};
}

void server::spawn_thread(setka::tcp_socket&& socket)
{
	auto& thread = this->threads.emplace_back(*this);
	thread.owner_iter = std::prev(this->threads.end());
	thread.start();

	thread.push(this->spawn_connection(std::move(socket)));
}

void server::reclaim_thread(connection_thread& t)
{
	t.quit();
	// TODO: push to killer thread
	this->push_back([i = t.owner_iter, this]() {
		i->join();
		this->threads.erase(i);
	});
}
