// built-in include guard removal
// just in case the user has a local dependency with the same file name
#if defined _inc_geoip
	#undef _inc_geoip
#endif
// custom include-guard to ensure we don't duplicate
#if defined _geoip_included
	#endinput
#endif
#define _geoip_included


#include <a_samp>
#include <logger>
#include <requests>
#include <map>
#include <YSI\y_hooks>


#if !defined IPHUB_KEY
    #error Must define `IPHUB_KEY` see iphub.info/api for details
#endif


// E_GEO_DATA matches the iphub response object, minus the IP address itself
enum E_GEO_DATA {
    bool:E_GEO_READY, // false if response hasn't arrived yet
    E_GEO_CODE[4],    // "countryCode": "US",
    E_GEO_NAME[128],  // "countryName": "United States",
    E_GEO_ASN,        // "asn": 15169,
    E_GEO_ISP[128],   // "isp": "GOOGLE - Google Inc.",
    E_GEO_BLOCK,      // "block": 1
}

static GeoData[MAX_PLAYERS][E_GEO_DATA];
static RequestsClient:Client;
static Map:RequestToPlayer;

forward OnGeoResponse(Request:id, E_HTTP_STATUS:status, Node:node);

stock GetPlayerCountryCode(playerid, output[], len) {
    // todo
    return 0;
}

stock GetPlayerCountryName(playerid, output[], len) {
    // todo
    return 0;
}

hook OnScriptInit() {
    Client = RequestsClient("http://v2.api.iphub.info/ip/", RequestHeaders("X-Key", IPHUB_KEY));
}

hook OnPlayerConnect(playerid) {
    new addr[18];
    GetPlayerIp(playerid, addr, sizeof addr);

    new Request:r = RequestJSON(
        Client,
        addr,
        HTTP_METHOD_GET,
        "OnGeoResponse"
    );

    MAP_insert_val_val(RequestToPlayer, _:r, playerid);
}

public OnGeoResponse(Request:id, E_HTTP_STATUS:status, Node:node) {
    if(status != HTTP_STATUS_OK) {
        err("iphub response status code was not OK",
            _i("status", _:status));
        return;
    }

    new playerid = MAP_get_val_val(RequestToPlayer, _:id);
    if(!IsPlayerConnected(playerid)) {
        return;
    }

    // {
    //     "ip": "8.8.8.8",
    //     "countryCode": "US",
    //     "countryName": "United States",
    //     "asn": 15169,
    //     "isp": "GOOGLE - Google Inc.",
    //     "block": 1
    // }

    GeoData[playerid][E_GEO_READY] = true;
    JsonGetString(node, "countryCode", GeoData[playerid][E_GEO_CODE], 4);
    JsonGetString(node, "countryName", GeoData[playerid][E_GEO_NAME], 128);
    JsonGetInt(node, "asn", GeoData[playerid][E_GEO_ASN]);
    JsonGetString(node, "isp", GeoData[playerid][E_GEO_ISP], 128);
    JsonGetInt(node, "block", GeoData[playerid][E_GEO_BLOCK]);
}
