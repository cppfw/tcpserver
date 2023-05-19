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

#include "connection.hpp"

using namespace tcpserver;

connection::connection(setka::tcp_socket&& socket) :
	socket(std::move(socket))
{}

void connection::send(std::vector<uint8_t>&& data)
{
	if (this->sending_queue.empty()) {
		this->num_bytes_sent = 0;
	}

	this->sending_queue.push_back(std::move(data));

	this->status.set(opros::ready::write);
}

void connection::set_can_receive_data(bool can_receive)
{
	this->status.set(opros::ready::read, can_receive);
}

void connection::disconnect()
{
	LOG([](auto& o) {
		o << "DISCONNECT" << std::endl;
	})
	this->socket.disconnect();
}
