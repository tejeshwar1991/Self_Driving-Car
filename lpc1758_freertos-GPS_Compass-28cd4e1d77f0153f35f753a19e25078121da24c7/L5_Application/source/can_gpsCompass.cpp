/*
 * can_gpsCompass.cpp
 *
 *  Created on: Oct 28, 2015
 *      Author: Abhishek Gurudutt, Tejeshwar
 *
 *  functions to help in can communication.
 */

#include "can_gpsCompass.hpp"
#include "hashDefine.hpp"
#include "stdio.h"
#include "sevenSeg_display.hpp"

static checkPointData_t *nextChkPnt = NULL;
static checkPointData_t *prevChkPnt = NULL;
static checkPointData_t *firstChkPnt = NULL;
static checkPointData_t *giveCheckPoint = NULL;
static uint8_t numberOfChkPnts = 0;

bool addChkPnts(uint8_t latDec, uint32_t latFloat, uint8_t lonDec, uint32_t lonFloat, uint8_t num, bool isFinal)
{
    bool chkPntAdded = false;
    double_t calcLat, calcLong;

    checkPointData_t *traverseChkPnt = NULL;

    // Check for multiple definition, also add the nodes in ascending order.
    traverseChkPnt = firstChkPnt;
    while(NULL != traverseChkPnt)
    {
        if((traverseChkPnt->chkPntNo < num) && (NULL != traverseChkPnt->next))
        {
            traverseChkPnt = traverseChkPnt->next;
            prevChkPnt = traverseChkPnt;
        }
        else if(traverseChkPnt->chkPntNo == num)
        {
            prevChkPnt = traverseChkPnt->prev;
            nextChkPnt = traverseChkPnt->next;
            break;
        }
        else if((traverseChkPnt->chkPntNo > num) && (NULL != traverseChkPnt->prev))
        {
            traverseChkPnt = traverseChkPnt->prev;
            prevChkPnt = traverseChkPnt;
        }
        else
        {
            if((NULL == traverseChkPnt->next) && (traverseChkPnt->chkPntNo < num))
            {
                nextChkPnt = NULL;
                prevChkPnt = traverseChkPnt;
                break;
            }
            else if((NULL == traverseChkPnt->prev) && (traverseChkPnt->chkPntNo > num))
            {
                nextChkPnt = traverseChkPnt;
                prevChkPnt = NULL;
                break;
            }
            else
            {
                nextChkPnt = traverseChkPnt;
                traverseChkPnt = traverseChkPnt->prev;
                prevChkPnt = traverseChkPnt;
                break;
            }
        }
    }

    // if there are no duplicate data, then create a new checkpoint
    checkPointData_t *newChkPnt = new checkPointData_t;
    if (NULL != newChkPnt)
    {
        calcLat = convertLatitudeToDegree(latDec, latFloat);
        calcLong = convertLongitudeToDegree(lonDec, lonFloat);

        // Storing the values in a structure.
        newChkPnt->chkPntLat = calcLat;
        newChkPnt->chkPntLong = calcLong * -1;
        newChkPnt->chkPntNo = num;
        newChkPnt->prev = prevChkPnt;
        newChkPnt->next = nextChkPnt;
        newChkPnt->isFinal = isFinal;
        ++numberOfChkPnts;
        chkPntAdded = true;

            if(NULL == newChkPnt->prev)
            {
                firstChkPnt = newChkPnt;
                giveCheckPoint = firstChkPnt;
            }

            if(NULL == prevChkPnt)
            {
                prevChkPnt = newChkPnt;
            }
            else
            {
                prevChkPnt->next = newChkPnt;
            }

            if(NULL != nextChkPnt)
            {
                nextChkPnt->prev = newChkPnt;
            }
    }
    return chkPntAdded;
}

double_t convertLatitudeToDegree(uint8_t latDec, uint32_t latFloat)
{
    double_t latDegree;

    latDegree = (latFloat / (float_t)TEN_6);
    latDegree = (latDec) + latDegree;

    return latDegree;
}

double_t convertLongitudeToDegree(uint8_t lonDec, uint32_t lonFloat)
{
    double_t longDegree;

    longDegree = (lonFloat / (float_t)TEN_6);
    longDegree = ((float_t)lonDec) + longDegree;

    return longDegree;
}

uint8_t getNumOfChkPnts()
{
    return numberOfChkPnts;
}

uint8_t getNumOfChkPntsFinal()
{
    uint8_t count = 0;

    checkPointData_t *pntToGetNoChk = NULL;
    pntToGetNoChk = firstChkPnt;

    if(numberOfChkPnts == 0)
        return 0;
    else
    {
        while(!pntToGetNoChk->isFinal && pntToGetNoChk != NULL)
		{
            pntToGetNoChk = pntToGetNoChk->next;
            count++;
		}
        return count;
    }

}

bool isFinal()
{
    if(numberOfChkPnts == 0)
        return 0;
    else
        return giveCheckPoint->isFinal;
}

uint8_t getPresentChkPnt()
{
    if(numberOfChkPnts == 0)
        return 0;

    if(giveCheckPoint->chkPntNo == 0)
        updateToNxtChkPnt();

    return giveCheckPoint->chkPntNo;
}

double_t getLongitude(uint8_t longitudeNumber)
{
    if(numberOfChkPnts == 0)
        return 0;

    checkPointData_t *pntToGetLong = NULL;
    pntToGetLong = firstChkPnt;

    while(pntToGetLong != NULL)
    {
        if(pntToGetLong->chkPntNo == longitudeNumber)
            return pntToGetLong->chkPntLong;
        else
            pntToGetLong = pntToGetLong->next;
    }
    return 0;
}

double_t getLatitude(uint8_t latitudeNumber)
{
    if(numberOfChkPnts == 0)
        return 0;

    checkPointData_t *pntToGetLat = NULL;
    pntToGetLat = firstChkPnt;

    while(pntToGetLat != NULL)
    {
        if(pntToGetLat->chkPntNo == latitudeNumber)
            return pntToGetLat->chkPntLat;
        else
            pntToGetLat = pntToGetLat->next;
    }
    return 0;
}

bool updateToNxtChkPnt()
{
    if(giveCheckPoint->next != NULL && (numberOfChkPnts > 0))
    {
        giveCheckPoint = giveCheckPoint->next;
        return true;
    }
    else
        return false;
}

bool updateToPrevChkPnt()
{
    if (giveCheckPoint->prev != NULL && (numberOfChkPnts > 0))
    {
        giveCheckPoint = giveCheckPoint->prev;
        return true;
    }
    else
        return false;
}
