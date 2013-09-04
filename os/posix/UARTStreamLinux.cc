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
#if !defined(QCC_OS_DARWIN)
#include <qcc/UARTStream.h>
#include <fcntl.h>
#include <errno.h>

#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>

#define QCC_MODULE "UART"

using namespace qcc;
using namespace std;
namespace qcc {
QStatus UART(qcc::String devName, uint32_t speed, UARTFd& fd)
{
    int ret = open(devName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (ret == -1) {
        fd = -1;
        QCC_LogError(ER_OS_ERROR, ("failed to open serial device %s. ret = %d, %d - %s", devName.c_str(), ret, errno, strerror(errno)));
        return ER_OS_ERROR;
    }
    fd = ret;

    /* Lock this FD, to ensure exclusive access to this serial port. */
    ret = flock(fd, LOCK_EX | LOCK_NB);
    if (ret) {
        fd = -1;
        QCC_LogError(ER_OS_ERROR, ("Lock fd %d failed with '%s'", fd, strerror(errno)));
        return ER_OS_ERROR;
    }

    QCC_DbgPrintf(("opened serial device %s successfully. ret = %d", devName.c_str(), ret));

    speed_t baudrate;
    /**
     * Set input and output baudrate
     */
    switch (speed) {
    case 2400:
        baudrate = B2400;
        break;

    case 9600:
        baudrate = B9600;
        break;

    case 19200:
        baudrate = B19200;
        break;

    case 38400:
        baudrate = B38400;
        break;

    case 57600:
        baudrate = B57600;
        break;

    case 115200:
        baudrate = B115200;
        break;

    case 230400:
        baudrate = B230400;
        break;

    case 460800:
        baudrate = B460800;
        break;

    case 921600:
        baudrate = B921600;
        break;

    case 1000000:
        baudrate = B1000000;
        break;

    case 1152000:
        baudrate = B1152000;
        break;

    case 1500000:
        baudrate = B1500000;
        break;

    case 2000000:
        baudrate = B2000000;
        break;

    case 2500000:
        baudrate = B2500000;
        break;

    case 3000000:
        baudrate = B3000000;
        break;

    case 3500000:
        baudrate = B3500000;
        break;

    case 4000000:
        baudrate = B4000000;
        break;

    default:
        assert(false);

    }

    struct termios ttySettings;
    memset(&ttySettings, 0, sizeof(ttySettings));

    cfsetospeed(&ttySettings, baudrate);
    cfsetispeed(&ttySettings, baudrate);

    /* 115200 - 8 - E - 1 */
    ttySettings.c_iflag |= INPCK;
    ttySettings.c_cflag |= CS8 | CLOCAL | CREAD | PARENB;

    tcflush(fd, TCIOFLUSH);

    /**
     * Set the new options on the port
     */
    tcsetattr(fd, TCSANOW, &ttySettings);

    tcflush(fd, TCIOFLUSH);

    return ER_OK;
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
/* This frame size is chosen so that most of the SLAP packets fit into one frame.
 * If the packet doesnt fit within this, it will be read using two calls to read().
 */
#define RX_BUFSIZE  640
static uint8_t RxBuffer[RX_BUFSIZE];
QStatus UARTStream::PullBytes(void* buf, size_t numBytes, size_t& actualBytes, uint32_t timeout) {
    QStatus status = ER_OK;
    int ret = read(fd, buf, numBytes);
    if (ret == -1) {
        if (errno == EAGAIN) {
            status = ER_WOULDBLOCK;
        } else {
            status = ER_OS_ERROR;
            QCC_DbgHLPrintf(("UARTStream::PullBytes (fd = %u): %d - %s", fd, errno, strerror(errno)));
        }
    } else {
        actualBytes = ret;
    }
    return status;
}
void UARTStream::Close() {
    QCC_DbgPrintf(("Uart::close()"));
    if (fd != -1) {
        /* Release the lock on this FD */
        flock(fd, LOCK_UN);
        close(fd);
        fd = -1;
    }
}
QStatus UARTStream::PushBytes(const void* buf, size_t numBytes, size_t& actualBytes) {
    QStatus status = ER_OK;
    int ret = write(fd, buf, numBytes);
    if (ret == -1) {
        if (errno == EAGAIN) {
            status = ER_WOULDBLOCK;
        } else {
            status = ER_OS_ERROR;
            QCC_DbgHLPrintf(("UARTStream::PushBytes (fd = %u): %d - %s", fd, errno, strerror(errno)));
        }
    } else {
        actualBytes = ret;
    }
    return status;
}

UARTController::UARTController(UARTStream* uartStream, IODispatch& iodispatch, UARTReadListener* readListener) :
    m_uartStream(uartStream), m_iodispatch(iodispatch), m_readListener(readListener), exitCount(0)
{
}

QStatus UARTController::Start()
{
    return m_iodispatch.StartStream(m_uartStream, this, NULL, this, true, false);
}

QStatus UARTController::Stop()
{
    return m_iodispatch.StopStream(m_uartStream);
}

QStatus UARTController::Join()
{
    while (!exitCount) {
        qcc::Sleep(100);
    }
    return ER_OK;
}

QStatus UARTController::ReadCallback(Source& source, bool isTimedOut)
{
    size_t actual;
    QStatus status = m_uartStream->PullBytes(RxBuffer, RX_BUFSIZE, actual);
    assert(status == ER_OK);
    m_readListener->ReadEventTriggered(RxBuffer, actual);
    m_iodispatch.EnableReadCallback(m_uartStream);
    return status;
}

void UARTController::ExitCallback()
{
    m_uartStream->Close();
    exitCount = 1;
}
#endif
