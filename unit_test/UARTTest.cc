/******************************************************************************
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
#include <gtest/gtest.h>

#include <Status.h>
#include <qcc/Util.h>
#include <qcc/UARTStream.h>
#include <qcc/SLAPStream.h>
#define PACKET_SIZE 100
using namespace qcc;

TEST(UARTTest, DISABLED_uart_large_buffer_test)
{
    Timer timer("SLAPtimer", true, 4, false, 10);
    timer.Start();
    UARTFd fd0;
    UARTFd fd1;
    QStatus status = UART("/tmp/COM0", 115200, fd0);
    ASSERT_EQ(status, ER_OK);

    status = UART("/tmp/COM1", 115200, fd1);
    ASSERT_EQ(status, ER_OK);

    UARTStream* s = new UARTStream(fd0);
    UARTStream* s1 = new UARTStream(fd1);
    SLAPStream h(s, timer, PACKET_SIZE);
    SLAPStream h1(s1, timer, PACKET_SIZE);
    h.ScheduleLinkControlPacket();
    h1.ScheduleLinkControlPacket();
    IODispatch iodisp("iodisp", 4);
    iodisp.Start();

    UARTController uc(s, iodisp, &h);
    UARTController uc1(s1, iodisp, &h1);
    uc.Start();
    uc1.Start();
    uint8_t rxBuffer[400];
    memset(&rxBuffer, 'R', sizeof(rxBuffer));

    uint8_t txBuffer[400];
    memset(&txBuffer, 'T', sizeof(txBuffer));

    int blocksize = 20;
    for (int blocks = 0; blocks < 20; blocks++) {
        memset(txBuffer + (blocks * blocksize), 'A' + (uint8_t)blocks, blocksize);
    }

    size_t actual;

    h.PushBytes(txBuffer, 400, actual);
    EXPECT_EQ(actual, 400);


    h1.PullBytes(rxBuffer, 400, actual);
    EXPECT_EQ(actual, 400);

    for (int x = 0; x < 400; x++) {
        EXPECT_EQ(txBuffer[x], rxBuffer[x]);
    }

    timer.Stop();
    uc.Stop();
    uc1.Stop();
    iodisp.Stop();

    timer.Join();
    uc.Join();
    uc1.Join();
    iodisp.Join();

    h.Close();
    h1.Close();
    delete s;
    delete s1;
}

TEST(UARTTest, DISABLED_uart_small_buffer_test)
{
    Timer timer("SLAPtimer", true, 4, false, 10);
    timer.Start();
    UARTFd fd0;
    UARTFd fd1;
    QStatus status = UART("/tmp/COM0", 115200, fd0);
    ASSERT_EQ(status, ER_OK);

    status = UART("/tmp/COM1", 115200, fd1);
    ASSERT_EQ(status, ER_OK);

    UARTStream* s = new UARTStream(fd0);
    UARTStream* s1 = new UARTStream(fd1);
    /* Test different packet size and window size values */
    SLAPStream h(s, timer, 1000);
    SLAPStream h1(s1, timer, PACKET_SIZE, 2);
    h.ScheduleLinkControlPacket();
    h1.ScheduleLinkControlPacket();

    IODispatch iodisp("iodisp", 4);
    iodisp.Start();

    UARTController uc(s, iodisp, &h);
    UARTController uc1(s1, iodisp, &h1);
    uc.Start();
    uc1.Start();

    char buf[16] = "AAAAA";
    char buf1[16] = "BBBBB";
    size_t x;
    char buf2[16] = "CCCCC";
    char buf3[16] = "DDDDD";
    char buf4[16] = "EEEEE";

    status = h.PushBytes(&buf, 5, x);
    EXPECT_EQ(status, ER_OK);
    EXPECT_EQ(x, 5);
    status = h.PushBytes(&buf1, 5, x);
    EXPECT_EQ(status, ER_OK);
    EXPECT_EQ(x, 5);

    status = h.PushBytes(&buf2, 5, x);
    EXPECT_EQ(status, ER_OK);
    EXPECT_EQ(x, 5);

    status = h.PushBytes(&buf3, 5, x);
    EXPECT_EQ(status, ER_OK);
    EXPECT_EQ(x, 5);

    size_t act;

    status = h1.PullBytes(&buf, 12, act);
    EXPECT_EQ(status, ER_OK);
    EXPECT_EQ(act, 12);

    status = h1.PullBytes(&buf, 8, act);
    EXPECT_EQ(status, ER_OK);
    EXPECT_EQ(act, 8);


    status = h.PushBytes(&buf4, 5, x);
    EXPECT_EQ(status, ER_OK);
    EXPECT_EQ(x, 5);

    status = h.PushBytes(&buf, 5, x);
    EXPECT_EQ(status, ER_OK);
    EXPECT_EQ(x, 5);

    status = h.PushBytes(&buf1, 5, x);
    EXPECT_EQ(status, ER_OK);
    EXPECT_EQ(x, 5);


    status = h1.PullBytes(&buf, 15, act);
    EXPECT_EQ(status, ER_OK);
    EXPECT_EQ(act, 15);

    timer.Stop();
    uc.Stop();
    uc1.Stop();
    iodisp.Stop();

    timer.Join();
    uc.Join();
    uc1.Join();
    iodisp.Join();

    h.Close();
    h1.Close();
    delete s;
    delete s1;
}
/* The following two tests are written using the Gtest framework but arent really unit tests.
 * The tests are to be run side by side and both ends send and receive data.
 */
TEST(UARTTest, DISABLED_serial_testrecv) {
    Timer timer("SLAPtimer", true, 4, false, 10);
    timer.Start();
    uint8_t rxBuffer[1600];
    memset(&rxBuffer, '\0', sizeof(rxBuffer));
    uint8_t txBuffer[1600];
    memset(&txBuffer, 'T', sizeof(txBuffer));

    int blocksize = 100;
    for (int blocks = 0; blocks < 16; blocks++) {
        memset(txBuffer + (blocks * blocksize), 0x41 + (uint8_t)blocks, blocksize);
    }
    UARTFd fd0;
    QStatus status = UART("/tmp/COM0", 115200, fd0);
    ASSERT_EQ(status, ER_OK);

    UARTStream* s = new UARTStream(fd0);
    SLAPStream h(s, timer, PACKET_SIZE);
    h.ScheduleLinkControlPacket();

    IODispatch iodisp("iodisp", 4);
    iodisp.Start();

    UARTController uc(s, iodisp, &h);
    uc.Start();
    size_t act;
    for (int iter = 0; iter < 10; iter++) {
        printf("iteration %d", iter);
        h.PullBytes(rxBuffer, 200, act);
        EXPECT_EQ(act, 200);
        qcc::Sleep(500);
        printf(".");

        h.PullBytes(rxBuffer + 200, 200, act);
        EXPECT_EQ(act, 200);
        qcc::Sleep(500);
        printf(".");

        h.PullBytes(rxBuffer + 400, 200, act);
        EXPECT_EQ(act, 200);
        qcc::Sleep(500);
        printf(".");

        h.PullBytes(rxBuffer + 600, 200, act);
        EXPECT_EQ(act, 200);
        qcc::Sleep(500);
        printf(".");

        h.PullBytes(rxBuffer + 800, 200, act);
        EXPECT_EQ(act, 200);
        qcc::Sleep(500);
        printf(".");

        h.PullBytes(rxBuffer + 1000, 200, act);
        EXPECT_EQ(act, 200);
        qcc::Sleep(500);
        printf(".");

        h.PullBytes(rxBuffer + 1200, 200, act);
        EXPECT_EQ(act, 200);
        qcc::Sleep(500);
        printf(".");

        h.PullBytes(rxBuffer + 1400, 200, act);
        EXPECT_EQ(act, 200);
        for (int i = 0; i < 1600; i++) {
            EXPECT_EQ(txBuffer[i], rxBuffer[i]);
        }
        h.PushBytes(txBuffer, sizeof(txBuffer), act);
        EXPECT_EQ(act, 1600);
        printf("\n");

    }

    /* Wait for retransmission to finish */
    qcc::Sleep(4000);

    timer.Stop();
    uc.Stop();
    iodisp.Stop();

    timer.Join();
    uc.Join();
    iodisp.Join();

    h.Close();
    delete s;
}

TEST(UARTTest, DISABLED_serial_testsend) {
    Timer timer("SLAPtimer", true, 4, false, 10);
    timer.Start();
    uint8_t rxBuffer[1600];
    memset(&rxBuffer, 'R', sizeof(rxBuffer));

    uint8_t txBuffer[1600];
    memset(&txBuffer, 'T', sizeof(txBuffer));

    int blocksize = 100;
    for (int blocks = 0; blocks < 16; blocks++) {
        memset(txBuffer + (blocks * blocksize), 0x41 + (uint8_t)blocks, blocksize);
    }
    size_t x;
    UARTFd fd1;
    QStatus status = UART("/tmp/COM1", 115200, fd1);
    ASSERT_EQ(status, ER_OK);

    UARTStream* s1 = new UARTStream(fd1);
    SLAPStream h1(s1, timer, PACKET_SIZE);
    h1.ScheduleLinkControlPacket();

    IODispatch iodisp("iodisp", 4);
    iodisp.Start();

    UARTController uc(s1, iodisp, &h1);
    uc.Start();

    for (int iter = 0; iter < 10; iter++) {
        printf("iteration %d", iter);
        h1.PushBytes(txBuffer, sizeof(txBuffer), x);
        EXPECT_EQ(x, 1600);
        printf(".");
        h1.PullBytes(rxBuffer, 1600, x);
        EXPECT_EQ(x, 1600);
        printf(".");
        for (int i = 0; i < 1600; i++) {
            EXPECT_EQ(txBuffer[i], rxBuffer[i]);
        }
        printf("\n");

    }
    /* Wait for retransmission to finish */
    qcc::Sleep(4000);

    timer.Stop();
    uc.Stop();
    iodisp.Stop();

    timer.Join();
    uc.Join();
    iodisp.Join();

    h1.Close();
    delete s1;
}

TEST(UARTTest, DISABLED_serial_testrecv_ajtcl) {

    Timer timer("SLAPtimer", true, 4, false, 10);
    timer.Start();

    uint8_t rxBuffer[1600];
    memset(&rxBuffer, '\0', sizeof(rxBuffer));

    uint8_t txBuffer[1600];
    memset(&txBuffer, 'T', sizeof(txBuffer));

    int blocksize = 100;

    for (int blocks = 0; blocks < 16; blocks++) {
        memset(txBuffer + (blocks * blocksize), 0x41 + (uint8_t)blocks, blocksize);
    }

    UARTFd fd0;
    QStatus status = UART("/tmp/COM0", 115200, fd0);
    ASSERT_EQ(status, ER_OK);

    UARTStream* s = new UARTStream(fd0);
    SLAPStream h(s, timer, PACKET_SIZE);
    h.ScheduleLinkControlPacket();

    IODispatch iodisp("iodisp", 4);
    iodisp.Start();

    UARTController uc(s, iodisp, &h);
    uc.Start();


    size_t act;

    h.PullBytes(rxBuffer, 1600, act);
    EXPECT_EQ(act, 1600);

    for (int i = 0; i < 1600; i++) {
        EXPECT_EQ(txBuffer[i], rxBuffer[i]);
    }
    printf("\n");

    timer.Stop();
    uc.Stop();
    iodisp.Stop();

    timer.Join();
    uc.Join();
    iodisp.Join();

    h.Close();
    delete s;
}

TEST(UARTTest, DISABLED_serial_testsend_ajtcl)
{

    Timer timer("SLAPtimer", true, 4, false, 10);
    timer.Start();

    uint8_t rxBuffer[1600];
    memset(&rxBuffer, 'R', sizeof(rxBuffer));

    uint8_t txBuffer[1600];
    memset(&txBuffer, 'T', sizeof(txBuffer));

    int blocksize = 100;

    for (int blocks = 0; blocks < 16; blocks++) {
        memset(txBuffer + (blocks * blocksize), 0x41 + (uint8_t)blocks, blocksize);
    }

    size_t x;

    UARTFd fd1;
    QStatus status = UART("/tmp/COM1", 115200, fd1);
    ASSERT_EQ(status, ER_OK);

    UARTStream* s1 = new UARTStream(fd1);
    SLAPStream h1(s1, timer, PACKET_SIZE);
    h1.ScheduleLinkControlPacket();
    IODispatch iodisp("iodisp", 4);
    iodisp.Start();

    UARTController uc1(s1, iodisp, &h1);
    uc1.Start();


    h1.PushBytes(txBuffer, sizeof(txBuffer), x);
    EXPECT_EQ(x, 1600);

    /* Wait for retransmission to finish */
    qcc::Sleep(4000);
    timer.Stop();
    uc1.Stop();
    iodisp.Stop();

    timer.Join();
    uc1.Join();
    iodisp.Join();

    h1.Close();
    delete s1;
}
