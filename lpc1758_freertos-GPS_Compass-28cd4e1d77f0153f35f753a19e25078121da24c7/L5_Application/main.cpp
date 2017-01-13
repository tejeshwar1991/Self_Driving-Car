/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */

/**
 * @file
 * @brief This is the application entry point.
 * 			FreeRTOS and stdio printf is pre-configured to use uart0_min.h before main() enters.
 * 			@see L0_LowLevel/lpc_sys.h if you wish to override printf/scanf functions.
 *
 */
#include <hashDefine.hpp>
#include "tasks.hpp"
#include "examples/examples.hpp"
#include "utilities.h"
#include "can.h"
#include "io.hpp"
#include "gps.hpp"
#include "inttypes.h"
#include "receive_Canmsg.hpp"
#include "sevenSeg_display.hpp"

#if TESTCODE
#include "compass.hpp"
#include "can_gpsCompass.hpp"
#include "CompassGPS_calculation.hpp"



/*
static QueueHandle_t gpsData_q = scheduler_task::getSharedObject("gps_queue");
gpsData_t gpsData;
*/
float_t latTesting, longTesting;
uint32_t g[2];

void testCode(void *p)
{
    bool valid;
    static uint8_t num = 0;
    while(1)
    {
        num = getPresentChkPnt();

        if(SW.getSwitch(1))
        {
            //g[0] = 0x02f423f7;
            //g[1] = 0x9f423f25;

            float longitude, latitude;
            printf("%f, %f\n", getLatitude(num), getLongitude(num));
            float dist1 = calcDistToNxtChkPnt(getLatitude(num), getLongitude(num), getLatitude(num+1), getLongitude(num+1));
            float dist2 = calcDistToNxtChkPnt(getLatitude(num+1), getLongitude(num+1), getLatitude(num+2), getLongitude(num+2));
            printf("total checkpoints: %d\n", getNumOfChkPnts());
            printf("distance = %f, %f, total dist = %f\n", dist1, dist2, calcDistToFinalDest(dist1));
            float heading = headingdir(getLatitude(num), getLongitude(num), getLatitude(num+1), getLongitude(num+1));
            printf("angle = %f\n", heading);

        }
        if(SW.getSwitch(3))
        {
            valid = updateToNxtChkPnt();
            if (valid)
                printf("updated to next\n");
            else
                printf("its the end\n");
        }
        else if(SW.getSwitch(4))
        {
            valid = updateToPrevChkPnt();
            if (valid)
                printf("update to previous\n");
            else
                printf("its the beginning\n");

        }
        vTaskDelay(500);
    }
}
#endif

/**
 * The main() creates tasks or "threads".  See the documentation of scheduler_task class at scheduler_task.hpp
 * for details.  There is a very simple example towards the beginning of this class's declaration.
 *
 * @warning SPI #1 bus usage notes (interfaced to SD & Flash):
 *      - You can read/write files from multiple tasks because it automatically goes through SPI semaphore.
 *      - If you are going to use the SPI Bus in a FreeRTOS task, you need to use the API at L4_IO/fat/spi_sem.h
 *
 * @warning SPI #0 usage notes (Nordic wireless)
 *      - This bus is more tricky to use because if FreeRTOS is not running, the RIT interrupt may use the bus.
 *      - If FreeRTOS is running, then wireless task may use it.
 *        In either case, you should avoid using this bus or interfacing to external components because
 *        there is no semaphore configured for this bus and it should be used exclusively by nordic wireless.
 */
int main(void)
{
/*    while(1)
    {
        printf("compassBearing_inDeg:%f\n",compassBearing_inDeg());
        delay_ms(1000);
    }*/
    /**
     * A few basic tasks for this bare-bone system :
     *      1.  Terminal task provides gateway to interact with the board through UART terminal.
     *      2.  Remote task allows you to use remote control to interact with the board.
     *      3.  Wireless task responsible to receive, retry, and handle mesh network.
     *
     * Disable remote task if you are not using it.  Also, it needs SYS_CFG_ENABLE_TLM
     * such that it can save remote control codes to non-volatile memory.  IR remote
     * control codes can be learned by typing the "learn" terminal command.
     */


    /* Consumes very little CPU, but need highest priority to handle mesh network ACKs */
    scheduler_add_task(new wirelessTask(PRIORITY_CRITICAL));

    /* Used to calculate the present location. connect the GPS module to UART2 */
    #if GPSMODULE
        scheduler_add_task(new gps_data(PRIORITY_MEDIUM));
    #endif

    /* Change "#if 0" to "#if 1" to run period tasks; @see period_callbacks.cpp */
    #if 1
       scheduler_add_task(new periodicSchedulerTask());

    #endif
       scheduler_add_task(new terminalTask(PRIORITY_MEDIUM));


#if TESTCODE
    /*
     * Testing purpose
     */


    /*
    addChkPnts(37, 335382, 121, 881242, 1, 0); // latitude: 37.33422, longitude: 121.8834
    addChkPnts(37, 336042, 121, 881841, 2, 0); // latitude: 3720.06544, longitude: 12152.98048
    addChkPnts(37, 337318, 121, 882753, 3, 1); // latitude: 3720.07426, longitude: 12152.9776
    //addChkPnts(37, 334804, 121, 882389, 4); // latitude: 3720.08884, longitude: 12152.94292
    //addChkPnts(37, 335100, 121, 881664, 5); // latitude: 3720.10654, longitude: 12152.89942

    xTaskCreate(testCode, "Test code", 2048, NULL, 1, NULL);
*/

    /*addChkPnts(37, 334429, 121, 883402, 1); // latitude: 37.33422, longitude: 121.8834
    addChkPnts(37, 334453, 121, 883290, 2); // latitude: 3720.06544, longitude: 12152.98048
    addChkPnts(37, 334642, 121, 882854, 3); // latitude: 3720.07426, longitude: 12152.9776
    addChkPnts(37, 334804, 121, 882389, 4); // latitude: 3720.08884, longitude: 12152.94292
    addChkPnts(37, 335100, 121, 881664, 5); // latitude: 3720.10654, longitude: 12152.89942

    xTaskCreate(testCode, "Test code", 2048, NULL, 1, NULL);*/

#endif
    /* The task for the IR receiver */
    // scheduler_add_task(new remoteTask  (PRIORITY_LOW));

    /* Your tasks should probably used PRIORITY_MEDIUM or PRIORITY_LOW because you want the terminal
     * task to always be responsive so you can poke around in case something goes wrong.
     */

    /**
     * This is a the board demonstration task that can be used to test the board.
     * This also shows you how to send a wireless packets to other boards.
     */
    #if 0
        scheduler_add_task(new example_io_demo());
    #endif

    /**
     * Change "#if 0" to "#if 1" to enable examples.
     * Try these examples one at a time.
     */
    #if 0
        scheduler_add_task(new example_task());
        scheduler_add_task(new example_alarm());
        scheduler_add_task(new example_logger_qset());
        scheduler_add_task(new example_nv_vars());
    #endif

    /**
	 * Try the rx / tx tasks together to see how they queue data to each other.
	 */
    #if 0
        scheduler_add_task(new queue_tx());
        scheduler_add_task(new queue_rx());
    #endif

    /**
     * Another example of shared handles and producer/consumer using a queue.
     * In this example, producer will produce as fast as the consumer can consume.
     */
    #if 0
        scheduler_add_task(new producer());
        scheduler_add_task(new consumer());
    #endif

    /**
     * If you have RN-XV on your board, you can connect to Wifi using this task.
     * This does two things for us:
     *   1.  The task allows us to perform HTTP web requests (@see wifiTask)
     *   2.  Terminal task can accept commands from TCP/IP through Wifly module.
     *
     * To add terminal command channel, add this at terminal.cpp :: taskEntry() function:
     * @code
     *     // Assuming Wifly is on Uart3
     *     addCommandChannel(Uart3::getInstance(), false);
     * @endcode
     */
    #if 0
        Uart3 &u3 = Uart3::getInstance();
        u3.init(WIFI_BAUD_RATE, WIFI_RXQ_SIZE, WIFI_TXQ_SIZE);
        scheduler_add_task(new wifiTask(Uart3::getInstance(), PRIORITY_LOW));
    #endif

    scheduler_start(); ///< This shouldn't return
    return -1;
}
