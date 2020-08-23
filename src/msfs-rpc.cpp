<<<<<<< HEAD
//Copyright (c) hazz.club.  All rights reserved. 
//------------------------------------------------------------------------------
//
//  MSFS RPC Client
// 
//    Description:
//                When a flight loads, it will show the user aircraft in the RPC client.
//------------------------------------------------------------------------------

//Normal windows stuff
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <iostream>
#include <thread>
#include <functional>
#include <string>

//Discord SDK Stuff.
#include "DiscordSDK/include/discord_rpc.h"
#include "DiscordSDK/include/discord_register.h"

//SimConnect SDK
#include "SimConnect.h"


int quit = 0;
HANDLE  hSimConnect = NULL;

struct Struct1
{
    char    title[256];
    double  kohlsmann;
    double  altitude;
    double  latitude;
    double  longitude;
};

static enum EVENT_ID {
    EVENT_SIM_START,
};

static enum DATA_DEFINE_ID {
    DEFINITION_1,
};

static enum DATA_REQUEST_ID {
    REQUEST_1,
};

static bool gInit, gRPC = true;

void SetupDiscord()
{
    if (gRPC)
    {
        DiscordEventHandlers handlers;
        memset(&handlers, 0, sizeof(handlers));
        Discord_Initialize("746083840132645025", &handlers, 1, "0");
    }
}

const char* simConnect()
{
    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Open and Close", NULL, 0, 0, 0)))
    {
        printf("Successful Connection to SimConnect!\r\n");
        std::string str = "Connected to SimConnect";
        return "Connected to SimConnect";
        //response = true; //response is true so return a 0
    }
    else
        printf("Failed Connection!\r\n");
    std::string str = "Failed to Connect";
    return "Failed to Connect";
    //response = false; //response is false so return a 1
}
static int64_t StartTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

void baseDiscordUpdate()
{
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    if (simConnect() == "Connected to SimConnect")
    {
        discordPresence.state = "Connected to SimConnect";
    }
    else
        discordPresence.state = "Failed to Connect";
    //discordPresence.state = "Solo";

    const char* mainStr = "Flying nothing yet";
    discordPresence.details = mainStr;
    discordPresence.startTimestamp = StartTime;
    discordPresence.largeImageKey = "logo";
    Discord_UpdatePresence(&discordPresence);
}

void CALLBACK MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
    HRESULT hr;

    switch (pData->dwID)
    {
    case SIMCONNECT_RECV_ID_EVENT:
    {
        SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;

        switch (evt->uEventID)
        {
        case EVENT_SIM_START:

            // Now the sim is running, request information on the user aircraft
            hr = SimConnect_RequestDataOnSimObjectType(hSimConnect, REQUEST_1, DEFINITION_1, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);

            break;

        default:
            break;
        }
        break;
    }

    case   SIMCONNECT_RECV_ID_OPEN: 
    {
        SIMCONNECT_RECV_OPEN* openData = (SIMCONNECT_RECV_OPEN*)pData;
        printf("MSFS Opened!");
        break;
    }


    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
    {
        SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;

        switch (pObjData->dwRequestID)
        {
        case REQUEST_1:
        {
            DWORD ObjectID = pObjData->dwObjectID;
            Struct1* pS = (Struct1*)&pObjData->dwData;
            if (SUCCEEDED(StringCbLengthA(&pS->title[0], sizeof(pS->title), NULL))) // security check
            {
                DiscordRichPresence discordPresence;
                memset(&discordPresence, 0, sizeof(discordPresence));
                if (simConnect() == "Connected to SimConnect")
                {
                    discordPresence.state = "Connected to SimConnect";
                }
                else
                    discordPresence.state = "Failed to Connect";
                //discordPresence.state = "Solo";

                //const char* mainStr = "Flying the ";
                //const char* title = pS->title;
                discordPresence.details = ("Flying the %s", pS->title);
                discordPresence.startTimestamp = StartTime;
                discordPresence.largeImageKey = "logo";
                Discord_UpdatePresence(&discordPresence);

                printf("\nObjectID=%d  title=\"%s\"\nLat=%f  Lon=%f  Alt=%f  Kohlsman=%.2f", ObjectID, pS->title, pS->latitude, pS->longitude, pS->altitude, pS->kohlsmann);
            }
            break;
        }

        default:
            break;
        }
        break;
    }


    case SIMCONNECT_RECV_ID_QUIT:
    {
        std::cout << "MSFS Program Exit\r\nCode: 1";
        DiscordRichPresence discordPresence;
        memset(&discordPresence, 0, sizeof(discordPresence));
        if (simConnect() == "Connected to SimConnect")
        {
            discordPresence.state = "Connected to SimConnect";
        }
        else
            discordPresence.state = "Failed to Connect";
        //discordPresence.state = "Solo";

        //const char* mainStr = "Flying the ";
        //const char* title = pS->title;
        discordPresence.details = ("Flying nothing.");
        discordPresence.startTimestamp = StartTime;
        discordPresence.largeImageKey = "logo";
        Discord_UpdatePresence(&discordPresence);

        break;
    }

    default:
        printf("\nReceived:%d", pData->dwID);
        break;
    }
}

void testDataRequest()
{
    HRESULT hr;

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Request Data", NULL, 0, 0, 0)))
    {
        printf("Connected to MSFS!\r\n");

        // Set up the data definition, but do not yet do anything with it
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "title", NULL, SIMCONNECT_DATATYPE_STRING256);
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Kohlsman setting hg", "inHg");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Plane Altitude", "feet");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Plane Latitude", "degrees");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Plane Longitude", "degrees");

        // Request an event when the simulation starts
        hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart");

        while (0 == quit)
        {
            SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, NULL);
            Sleep(1);
        }

        hr = SimConnect_Close(hSimConnect);
    }
}


void timer_start(std::function<void(void)> func, unsigned int interval)
{
    std::thread([func, interval]()
        {
            while (true)
            {
                auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
                func();
                std::this_thread::sleep_until(x);
            }
        }).detach();
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{

    //simConnect();
    printf("MSFS RPC Client\r\nMade by Hazz#3894\r\n");
    SetupDiscord();
    baseDiscordUpdate(); //update discord rpc
    testDataRequest();

    return 0;
}
=======

>>>>>>> d733b67bb6bc8849be2377f4133d7a049144a617
