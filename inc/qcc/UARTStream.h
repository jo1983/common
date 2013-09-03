/**
 * @file
 *
 * This file defines a UART based physical link for communication.
 */

/******************************************************************************
 *
 *
 * Copyright 2013, Qualcomm Innovation Center, Inc.
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

#ifndef _QCC_UARTSTREAM_H
#define _QCC_UARTSTREAM_H

#include <qcc/platform.h>
#include <qcc/Stream.h>
#include <qcc/IODispatch.h>
#include <qcc/Thread.h>

namespace qcc {
/**
 * Opens a serial device and returns the file descriptor.
 * @param devName       name of the device to open
 * @param speed         baud rate to set for the device.
 * @param fd[out]	The file descriptor value.
 * @return ER_OK - port opened sucessfully, error otherwise.
 *
 */
QStatus UART(qcc::String devName, uint32_t speed, qcc::UARTFd& fd);

class UARTStream : public NonBlockingStream {
  public:

    UARTStream(UARTFd fd);

    ~UARTStream();

    /* Close the fd */
    void Close();

    /**
     * Pull bytes from the stream.
     * The source is exhausted when ER_NONE is returned.
     *
     * @param buf          Buffer to store pulled bytes
     * @param reqBytes     Number of bytes requested to be pulled from source.
     * @param actualBytes  Actual number of bytes retrieved from source.
     * @param timeout      Time to wait to pull the requested bytes.
     * Note: Since this is a non-blocking stream, this parameter is ignored.
     * @return   ER_OK if successful. ER_NONE if source is exhausted. Otherwise an error.
     */
    QStatus PullBytes(void* buf, size_t numBytes, size_t& actualBytes, uint32_t timeout = 0);

    /**
     * Push zero or more bytes into the sink with infinite ttl.
     *
     * @param buf          Buffer to store pulled bytes
     * @param numBytes     Number of bytes from buf to send to sink.
     * @param numSent      Number of bytes actually consumed by sink.
     * @return   ER_OK if successful.
     */
    QStatus PushBytes(const void* buf, size_t numBytes, size_t& actualBytes);

    /**
     * Get the Event indicating that data is available.
     *
     * @return Event that is set when data is available.
     */
    Event& GetSourceEvent() { return *sourceEvent; }

    /**
     * Get the Event indicating that sink can accept data.
     *
     * @return Event set when socket can accept more data via PushBytes
     */
    Event& GetSinkEvent() { return *sinkEvent; }
    UARTFd GetFD() { return fd; }
  private:

    /** Private default constructor - does nothing */
    UARTStream();

    /**
     * Private Copy-constructor - does nothing
     *
     * @param other  UARTStream to copy from.
     */
    UARTStream(const UARTStream& other);

    /**
     * Private Assignment operator - does nothing.
     *
     * @param other  UARTStream to assign from.
     */
    UARTStream operator=(const UARTStream& other) { return *this; };

    int fd;             /**< File descriptor associated with the device */
    Event* sourceEvent; /**< Event signaled when data is available */
    Event* sinkEvent;   /**< Event signaled when sink can accept data */
};
class UARTReadListener {
  public:
    virtual ~UARTReadListener() { };
    virtual void ReadEventTriggered(uint8_t* buf, size_t numBytes) = 0;
};

class UARTController : public IOReadListener, public IOExitListener {
  public:

    UARTController(UARTStream* uartStream, IODispatch& iodispatch, UARTReadListener* readListener);
    ~UARTController() { };
    QStatus Start();
    QStatus Stop();
    QStatus Join();
/**
 * Read callback for the stream.
 * @param source             The stream that this entry is associated with.
 * @param isTimedOut         false - if the source event has fired.
 *                           true - if no source event has fired in the specified timeout.
 * @return  ER_OK if successful.
 */
    virtual QStatus ReadCallback(Source& source, bool isTimedOut);
    /**
     * Write callback for the stream.
     * Indicates that the stream needs to shutdown.
     */
    virtual void ExitCallback();
    UARTStream* m_uartStream;           /**< The UART stream that this controller reads from */
    IODispatch& m_iodispatch;           /**< The IODispatch used to trigger read callbacks */
    UARTReadListener* m_readListener;   /**< The Read listener to call back after reading data */
    int exitCount;                      /**< Count indicating whether the uart stream has exited successfully. */
};
}
#endif
