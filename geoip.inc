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


#if !defined MAX_GEOIP_COUNTRY_CODE
    #define MAX_GEOIP_COUNTRY_CODE (4)
#endif

#if !defined MAX_GEOIP_COUNTRY_NAME
    #define MAX_GEOIP_COUNTRY_NAME (128)
#endif

#if !defined MAX_GEOIP_ISP_NAME
    #define MAX_GEOIP_ISP_NAME (128)
#endif


// E_GEO_DATA matches the iphub response object, minus the IP address itself
enum E_GEO_DATA {
    bool:E_GEO_READY,                   // false if response hasn't arrived yet
    E_GEO_CODE[MAX_GEOIP_COUNTRY_CODE], // "countryCode": "US",
    E_GEO_NAME[MAX_GEOIP_COUNTRY_NAME], // "countryName": "United States",
    E_GEO_ASN,                          // "asn": 15169,
    E_GEO_ISP[MAX_GEOIP_ISP_NAME],      // "isp": "GOOGLE - Google Inc.",
    E_GEO_BLOCK,                        // "block": 1
}

static GeoData[MAX_PLAYERS][E_GEO_DATA];
static RequestsClient:Client;
static Map:RequestToPlayer;

forward OnGeoResponse(Request:id, E_HTTP_STATUS:status, Node:node);
forward OnGeoDataReady(playerid);


// -
// API
// -


// IsGeoDataReady checks if geo IP data for a player is ready to read yet.
stock bool:IsGeoDataReady(playerid) {
    if(!IsPlayerConnected(playerid)) {
        return false;
    }
    return GeoData[playerid][E_GEO_READY];
}

stock GetPlayerCountryCode(playerid, output[], len = sizeof output) {
    if(!IsPlayerConnected(playerid)) {
        return 1;
    }

    if(!GeoData[playerid][E_GEO_READY]) {
        return 2;
    }

    strcat(output, GeoData[playerid][E_GEO_CODE], len);
    return 0;
}

stock GetPlayerCountryName(playerid, output[], len = sizeof output) {
    if(!IsPlayerConnected(playerid)) {
        return 1;
    }

    if(!GeoData[playerid][E_GEO_READY]) {
        return 2;
    }

    strcat(output, GeoData[playerid][E_GEO_NAME], len);
    return 0;
}

stock GetPlayerASN(playerid, &asn) {
    if(!IsPlayerConnected(playerid)) {
        return 1;
    }

    if(!GeoData[playerid][E_GEO_READY]) {
        return 2;
    }

    asn = GeoData[playerid][E_GEO_ASN];
    return 0;
}

stock GetPlayerISP(playerid, output[], len = sizeof output) {
    if(!IsPlayerConnected(playerid)) {
        return 1;
    }

    if(!GeoData[playerid][E_GEO_READY]) {
        return 2;
    }

    strcat(output, GeoData[playerid][E_GEO_ISP], len);
    return 0;
}

stock GetPlayerIPBlock(playerid, &block) {
    if(!IsPlayerConnected(playerid)) {
        return 1;
    }

    if(!GeoData[playerid][E_GEO_READY]) {
        return 2;
    }

    block = GeoData[playerid][E_GEO_BLOCK];
    return 0;
}


// -
// Internal
// -


hook OnScriptInit() {
    new key[] = IPHUB_KEY;
    if(strlen(key) == 0) {
        fatal("IPHUB_KEY must be set for geoip library");
    }

    Client = RequestsClient("http://v2.api.iphub.info/ip/", RequestHeaders("X-Key", key));
}

hook OnPlayerConnect(playerid) {
    new addr[16];
    GetPlayerIp(playerid, addr, sizeof addr);
    _geoip_doRequest(addr, playerid);
}

hook OnPlayerDisconnect(playerid, reason) {
    GeoData[playerid][E_GEO_READY] = false;
    GeoData[playerid][E_GEO_CODE][0] = EOS;
    GeoData[playerid][E_GEO_NAME][0] = EOS;
    GeoData[playerid][E_GEO_ASN] = 0;
    GeoData[playerid][E_GEO_ISP][0] = EOS;
    GeoData[playerid][E_GEO_BLOCK] = 0;
}

_geoip_doRequest(addr[16], playerid) {
    dbg("geoip", "requesting ip data",
        _s("addr", addr),
        _i("playerid", playerid));

    new Request:r = RequestJSON(
        Client,
        addr,
        HTTP_METHOD_GET,
        "OnGeoResponse"
    );

    MAP_insert_val_val(RequestToPlayer, _:r, playerid);
}

public OnGeoResponse(Request:id, E_HTTP_STATUS:status, Node:node) {
    dbg("geoip", "received ip data",
        _i("request", _:id),
        _i("status", _:status));

    if(status != HTTP_STATUS_OK) {
        err("iphub response status code was not OK",
            _i("status", _:status));
        return;
    }

    new playerid = MAP_get_val_val(RequestToPlayer, _:id);
    if(!IsPlayerConnected(playerid)) {
        return;
    }

    dbg("geoip", "determined target player for ip data",
        _i("playerid", playerid));

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

    dbg("geoip", "extracted ip data for player",
        _s("code", GeoData[playerid][E_GEO_CODE]),
        _s("name", GeoData[playerid][E_GEO_NAME]),
        _i("asn", GeoData[playerid][E_GEO_ASN]),
        _s("isp", GeoData[playerid][E_GEO_ISP]),
        _i("block", GeoData[playerid][E_GEO_BLOCK])
    );
    
    CallLocalFunction("OnGeoDataReady", "%d", playerid);
}
