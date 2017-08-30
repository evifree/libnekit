// MIT License

// Copyright (c) 2017 Zhuhao Wang

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "nekit/transport/direct_adapter.h"

namespace nekit {
namespace transport {
DirectAdapter::DirectAdapter(
    boost::asio::io_service& io, std::shared_ptr<utils::Session> session,
    std::shared_ptr<ConnectorFactoryInterface> connector_factory)
    : AdapterInterface{io},
      session_{session},
      connector_factory_{connector_factory} {}

const utils::Cancelable& DirectAdapter::Open(EventHandler handler) {
  handler_ = handler;

  connector_ = connector_factory_->Build(session_->endpoint());

  return DoConnect();
}  // namespace transport

const utils::Cancelable& DirectAdapter::DoConnect() {
  connector_cancelable_ =
      connector_->Connect([ this, cancelable{life_time_cancelable_pointer()} ](
          std::unique_ptr<ConnectionInterface> && conn, std::error_code ec) {
        if (cancelable->canceled()) {
          return;
        }

        if (ec) {
          handler_(nullptr, nullptr, ec);
          return;
        }

        handler_(std::move(conn), stream_coder_factory_.Build(session_), ec);
        return;
      });

  return life_time_cancelable();
}

DirectAdapterFactory::DirectAdapterFactory(
    boost::asio::io_service& io,
    std::shared_ptr<ConnectorFactoryInterface> connector_factory)
    : AdapterFactoryInterface{io},
      connector_factory_{std::move(connector_factory)} {}

std::unique_ptr<AdapterInterface> DirectAdapterFactory::Build(
    std::shared_ptr<utils::Session> session) {
  return std::make_unique<DirectAdapter>(io(), session, connector_factory_);
}
}  // namespace transport
}  // namespace nekit
