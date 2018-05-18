# samp-geoip

[![sampctl](https://shields.southcla.ws/badge/sampctl-samp--geoip-2f2f2f.svg?style=for-the-badge)](https://github.com/Southclaws/samp-geoip)

A simple library that provides information from [IPHub](https://iphub.info/) for
connected players.

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
response has arrived, the functions `GetPlayerCountryCode` and
`GetPlayerCountryName` can be used.

## Testing

(There are no unit tests yet)
