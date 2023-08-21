# ASMSD - Alcatel SMS Daemon

> :warning: :construction: **This is a Proof-of-Concept Project under construction! Use your own risk!** :construction: :warning:

This is a small tool for Alcatel (especially IK41) dongle, intended to somewhat replace `smsd` for CheckMK. The configuration is different, just the input files are similar. Basicly, the program can watch directories for incomming files to send out as SMS via the dongle.

## Build

This is a simple CMake project. For this you need basic build tools, such as `gcc` or `clang`, `make` or `ninja` and `cmake` itself. Only one additional dependency is required from the system side: the `fmt` library.

In Ubuntu, run the following for install:
```cmd
sudo apt install git build-essential ninja-build cmake libfmt-dev
```

To clone, configure, generate then build, run:
```cmd
git clone https://github.com/grixb/asmsd
cd asmsd ; mkdir build
cmake -B build
cmake --build build
```

> Remark: at the moment, install is not supported.

## Usage

The tool can only be configured via cli arguments. See `asmsd --help`:
```cmd
ASMSD - Alcatel SMS Daemon
Usage:
  asmsd [OPTION...]

  -l, --log-level arg     log level (default: info)
  -h, --host arg          hostname or ip address of the device (default: 192.168.1.1)
  -p, --port arg          target port number of the http connection (default: 80)
  -b, --base-path arg     base path of the json rpc endpoint (default: /jrd/webapi)
  -v, --verify-token arg  verification token for the rpc requests (default: 
                          KSDHSDFOGQ5WERYTUIQWERTYUISDFG1HJZXCVCXBN2GDSMNDHKVKFsVBNf)
  -t, --time-out arg      connection time out in seconds (default: 9)
  -k, --keep-alive arg    keep alive timer in seconds (default: 3)
      --help              print usage

 commands options:
      --system-info           show system informations
      --system-status         show RAN connection status
      --connection-state      show WAN connection status
      --sms-storage-state     show SMS storage counters
      --sms-contact-list      show SMS contacts with last message
      --sms-content-list arg  show SMSes of a contact with <id>
  -n, --page arg              specify the page number (default: 1)
  -d, --detailed              show information views in deatiled mode
      --send-sms arg          sending sms to phone numbers
  -c, --content arg           used with send sms for sms text
      --delete-sms arg        wihout sms-content-lits delet by contact id
  -w, --watch [=arg(=.)]      watching directory for file creation
  -m, --move-to [=arg(=.)]    move sended file to this directory
  -r, --reprocess arg         reprocess files which created arg minutes ago (default: 5)
```

### Examples

See system status:
```cmd
> ./asmsd --system-status
RAN: LTE [4G] @ VFHU is connected with signal strength 3, SMS: normal
```

Some command, you can see it in deatiled view:
```cmd
> ./asmsd --system-status --detailed
SYSTEM STATUS [connected]:
	Network Name:     VFHU
	Network Type:     LTE [4G]
	Signal Strength:  3
	SMS State:        normal
	Roaming:          ENABLED
	Connection Error: 0
	Clear Code:       0
```

Also, connection state:
```cmd
> ./asmsd --connection-state --detailed
CONNECTION STATE [connected]:
	Connection Time:  00:02:18
	Connection Error: 0
	Clear Code:       0
	IPv4 address:     100.103.102.201
	IPv6 address:     0::0
	Download Speed:   71
	Upload Speed:     48
	Downloaded Bytes: 15613
	Uploaded Bytes:   6491
```

Sending SMS from the cli (with debug logs):
```cmd
> ./asmsd -l debug --send-sms +36203217654 --content "hello there..."
[2023-06-25 20:48:40.850] [info] log levelt set to: debug
[2023-06-25 20:48:40.850] [debug] using host: 192.168.1.1, port: 80
[2023-06-25 20:48:40.850] [debug] wating for device to be alive...
[2023-06-25 20:48:42.916] [debug] SMS send status: success
```

Watching a directory for newly created or moved in files,
you can use `./asmsd --watch=in_directory`. If you want that
`asmsd` move the sended file to another directory, use `--move-to=destination_directory`.
```cmd
./asmsd -l debug --watch=test_in --move-to=test_out
[2023-06-25 21:05:12.118] [info] log levelt set to: debug
[2023-06-25 21:05:12.118] [debug] using host: 192.168.1.1, port: 80
[2023-06-25 21:05:12.118] [debug] wating for device to be alive...
[2023-06-25 21:05:12.125] [info] reprocessing files since 21:00:12
[2023-06-25 21:05:12.126] [debug] start watching directory for file creation: test_in
[2023-06-25 21:05:12.126] [debug] registring stop signal: press ^C to stop watching...
[2023-06-25 21:05:12.126] [debug] start running notifier on watch thread...
[2023-06-25 21:05:12.126] [debug] start running keepalive thread...
[2023-06-25 21:05:12.126] [debug] wating for keepalive thread to stop...
[2023-06-25 21:06:32.437] [debug] new file created at test_in/something_file
[2023-06-25 21:06:32.437] [debug] sending SMS to: +36203217654
[2023-06-25 21:06:34.483] [debug] moving file something_file to test_out
^C[2023-06-25 21:06:49.801] [debug] got signal 2, stopping keepalive...
[2023-06-25 21:06:52.529] [debug] keepalive stoped running...
[2023-06-25 21:06:52.529] [info] keepalive thread stoped
[2023-06-25 21:06:52.529] [debug] stoping notifier...
[2023-06-25 21:06:52.529] [debug] waiting for watch thread to stop...
[2023-06-25 21:06:52.530] [debug] notifier stoped running...
[2023-06-25 21:06:52.530] [info] watch thread stoped
```

## Remaining tasks

- [ ] documentation: detailed description for cli arguments
- [ ] feature: record time point when device went offline and resends files from that time point.
- [ ] making baseline integration test
- [ ] check with valgrind

## Dependencies

This project could not to be alive without the following amazing open source projects:

- **[CXXOPTS](https://github.com/jarro2783/cxxopts):** for CLI arguments parsing
- **[nlohmann-json](https://github.com/nlohmann/json):** for json parsing
- **[httplib](https://github.com/yhirose/cpp-httplib):** for http client connection
- **[jsonrpcx](https://github.com/jsonrpcx/json-rpc-cxx):** for json-rpc request send and recive
- **[spdlog](https://github.com/gabime/spdlog):** for the beautifull log outputs
- **[inotify-cpp](https://github.com/erikzenker/inotify-cpp):** for directory changes watching

Thanks to all the contributors for creating and maintaining these great projects. :heart: 
