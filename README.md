# samp-geoip

[![sampctl](https://shields.southcla.ws/badge/sampctl-samp--geoip-2f2f2f.svg?style=for-the-badge)](https://github.com/Southclaws/samp-geoip)

A simple library that provides information from [IPHub](https://iphub.info/) for
connected players.

This package is built using the Requests package which is used to perform a web
request to IPHub's API when a player connects.

Currently the results are not cached so you may encounter rate limits if your
server has many players connecting per minute. This may be addressed in future
by using Redis as a cache.

## Installation

Simply install to your project:

```bash
sampctl package install Southclaws/samp-geoip
```

Include in your code and begin using the library:

```pawn
#include <geoip>
```

## Usage

When a player connects, their details are automatically requested. Once the
response has arrived, the API can be used.

`bool:IsGeoDataReady(playerid)` Checks if the data is ready, requests should
only take milliseconds but this ensures your code doesn't attempt to request
data that isn't ready yet.

The rest of the API corresponds directly to the
[IPHub documentation](https://docs.iphub.info/documentation/json-keys/):

*   `GetPlayerCountryCode(playerid, output[], len = sizeof output)`
*   `GetPlayerCountryName(playerid, output[], len = sizeof output)`
*   `GetPlayerASN(playerid, &asn)`
*   `GetPlayerISP(playerid, output[], len = sizeof output)`
*   `GetPlayerIPBlock(playerid, &block)`

## Testing

To test, first you must build build the package with the file `iphub_key.inc`
which should consist of:

```pawn
#define IPHUB_KEY your_api_key
```

You can get a free API key from [this page](https://iphub.info/pricing) by
clicking `Looking for the free plan (2000 req/day)? Here!`

Then just run as normal and connect to `localhost:7777`:

```bash
sampctl package run
```

If you connect locally, you'll see no useful data because you'll be sending the
loopback interface address, but a response will still appear:

```bash
text="requesting ip data" addr="127.0.0.1" playerid=0
text="received ip data" request=0 status=200
text="determined target player for ip data" playerid=0
text="extracted ip data for player" code="ZZ" name="Unknown" asn=0 isp="Private/local IP" block=0
```

You can also type `/ip` in-game to see a message containing the data.
