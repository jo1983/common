/**
 * @file
 *
 * This file implements a UART based physical link for communication.
 */

/******************************************************************************
 * Copyright 2009-2011, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/
#if defined(QCC_OS_DARWIN)
#include <qcc/UARTStream.h>


#define QCC_MODULE "UART"

using namespace qcc;
using namespace std;
namespace qcc {
QStatus UART(qcc::String devName, uint32_t speed, UARTFd& fd)
{
    return ER_NOT_IMPLEMENTED;
}
}
UARTStream::UARTStream(UARTFd fd) :
    fd(fd),
    sourceEvent(new Event(fd, Event::IO_READ, false)),
    sinkEvent(new Event(*sourceEvent, Event::IO_WRITE, false))
{

}

UARTStream::~UARTStream()
{
    delete sourceEvent;
    delete sinkEvent;
}
#define RX_BUFSIZE  640
static uint8_t RxBuffer[RX_BUFSIZE];
QStatus UARTStream::PullBytes(void* buf, size_t numBytes, size_t& actualBytes, uint32_t timeout) {
    return ER_NOT_IMPLEMENTED;
}
void UARTStream::Close() {

}
QStatus UARTStream::PushBytes(const void* buf, size_t numBytes, size_t& actualBytes) {
    return ER_NOT_IMPLEMENTED;
}

UARTController::UARTController(UARTStream* uartStream, IODispatch& iodispatch, UARTReadListener* readListener) :
    m_uartStream(uartStream), m_iodispatch(iodispatch), m_readListener(readListener), exitCount(0)
{
}

QStatus UARTController::Start()
{
    return ER_NOT_IMPLEMENTED;
}

QStatus UARTController::Stop()
{
    return ER_NOT_IMPLEMENTED;
}

QStatus UARTController::Join()
{
    return ER_NOT_IMPLEMENTED;
}

QStatus UARTController::ReadCallback(Source& source, bool isTimedOut)
{
    return ER_NOT_IMPLEMENTED;

}

void UARTController::ExitCallback()
{

}
#endif
