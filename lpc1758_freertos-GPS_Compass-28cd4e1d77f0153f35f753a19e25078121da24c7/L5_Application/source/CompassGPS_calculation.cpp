/*
 * CompassGPS_calculation.cpp
 *
 *  Created on: Oct 28, 2015
 *      Author: Abhishek Gurudutt, Tejeshwar
 *
 *  For the project CMPE243: Self drive car
 *  This file performs calculations for the heading direction and total distance.
 */

#include "CompassGPS_calculation.hpp"
#include "can_gpsCompass.hpp"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "io.hpp"
#include "hashDefine.hpp"

float_t calcDistToNxtChkPnt(double_t currentLat, double_t currentLong, double_t chkPntLat, double_t chkPntLong)
{
    // If checkpoints are not defined then return with 0.
    if(!chkPntLat || !chkPntLong)
        return 0.0;

    float_t dist;
    double_t intrmdtCalc;

    // coverting to radians
    double_t phi1 = currentLat * TO_RAD;
    double_t phi2 = chkPntLat * TO_RAD;
    double_t lamda = (chkPntLong - currentLong) * TO_RAD;

    double_t phi = phi2 - phi1;

    // calculation of distance.
    intrmdtCalc = (sin(phi/2) * sin(phi/2)) + (cos(phi1) * cos(phi2) * sin(lamda/2) * sin(lamda/2));
    dist = (float_t) (2 * RADIUS * atan2(sqrt(intrmdtCalc), sqrt(1 - intrmdtCalc)));

    return dist;

}


float_t calcDistToFinalDest(float_t distToChkPnt)
{
    float_t restOfChkPntDist = 0.0, finalDist = 0.0;
    uint8_t chkPnt = getPresentChkPnt();
    uint8_t totalChkPnts = getNumOfChkPntsFinal();
    for (uint8_t i = chkPnt; i <= totalChkPnts; i++)
    {
        restOfChkPntDist += calcDistToNxtChkPnt(getLatitude(i), getLongitude(i),
                getLatitude(i + 1), getLongitude(i + 1));
    }

    // adding the present distance to checkpoint with the rest of the checkpoint distance
    finalDist = (float_t)restOfChkPntDist + distToChkPnt;

    return finalDist;
}


double_t headingdir(double_t latitude1, double_t longitude1, double_t latitude2, double_t longitude2)
{
    if(!latitude2 || !longitude2)
        return 0;

    double_t delta_longitude,firstterm,secondterm,firstproduct,secondproduct,headingdirection;

    // convert to radians
    latitude1 = TO_RAD * (latitude1);
    latitude2 = TO_RAD * (latitude2);

    longitude1 = TO_RAD * (longitude1);
    longitude2 = TO_RAD * (longitude2);

    // heading angle calculation
    delta_longitude = (longitude2 - longitude1);

    firstterm = sin(delta_longitude) * cos(latitude2);
    firstproduct = cos(latitude1) * sin(latitude2);
    secondproduct = (sin(latitude1)*cos(latitude2) * cos(delta_longitude));
    secondterm = firstproduct - secondproduct;

    headingdirection = atan2(firstterm, secondterm) * TO_DEG;

    return fmodf((headingdirection+360),360);
}

int8_t turnDecision(double_t desired, double_t current)
{
    float_t turnAngle = (desired - current) / SCALE;

    uint8_t turnLimit = (FULLCIRCLE / SCALE) / 2;

    if(turnAngle > turnLimit)
        turnAngle = turnAngle - (FULLCIRCLE / SCALE);
    else if(turnAngle < -turnLimit)
        turnAngle = turnAngle + (FULLCIRCLE / SCALE);
    else
    {
        // return turn angle
    }

    return (int8_t)turnAngle;
}

bool checkPntReached(float_t distance)
{
    static bool intermediateChkPnt = true;

    if (distance <= STOP_METERS)
    {
        intermediateChkPnt = !isFinal();
        intermediateChkPnt = updateToNxtChkPnt() && intermediateChkPnt;
    }
    else
        intermediateChkPnt = true;

    if (intermediateChkPnt)
        return false;
    else
        return true;
}

void destReached()
{
    char LEDdisplay[5] = {'F', 'U', 'R', 'Y', ' '};
    static uint8_t selectChar = 0;

    // if final checkpoint reached then display 'FURY'
    LD.setLeftDigit(LEDdisplay[selectChar]);
    LD.setRightDigit(LEDdisplay[selectChar + 1]);
    selectChar = (selectChar + 1) % 4;
}
