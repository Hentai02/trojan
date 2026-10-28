# Trojan AGENTS.md

Trojan is an unidentifiable proxy mechanism that helps bypass GFW. It's a C++11 project built with CMake, using Boost.Asio for async I/O and OpenSSL for TLS.

## Dev Environment

**Dependencies** (Linux):
- CMake ≥ 3.7.2
- Boost ≥ 1.66.0 (system, program_options)
- OpenSSL ≥ 1.1.0
- libpq (PostgreSQL client library, optional, enabled by default)

Debian: `sudo apt -y install build-essential cmake libboost-system-dev libboost-program-options-dev libssl-dev libpq-dev`

**Build Type**: Defaults to Release if `CMAKE_BUILD_TYPE` not set.

## Build & Test

```bash
# Standard build
mkdir build && cd build
cmake ..
make
ctest                    # runs LinuxSmokeTest (requires curl, netcat, openssl, python3)
sudo make install        # installs binary, config examples, man page, systemd service
```

**CMake Options**:
- `-DENABLE_POSTGRESQL=ON|OFF` (default ON)
- `-DENABLE_NAT=ON|OFF` (Linux only, default ON)
- `-DENABLE_REUSE_PORT=ON|OFF` (Linux only, default ON)
- `-DENABLE_SSL_KEYLOG=ON|OFF` (OpenSSL ≥ 1.1.1, default ON)
- `-DENABLE_TLS13_CIPHERSUITES=ON|OFF` (OpenSSL ≥ 1.1.1, default ON)
- `-DFORCE_TCP_FASTOPEN=ON|OFF` (default OFF)
- `-DSYSTEMD_SERVICE=AUTO|ON|OFF` (Linux only)
- `-DDEFAULT_CONFIG=/path/to/config.json`

**CI Builds** (from `azure-pipelines.yml`):
- Linux: static Boost, libpq, FORCE_TCP_FASTOPEN=ON, strip binary
- macOS: Homebrew boost/openssl@1.1, static libs, PostgreSQL disabled
- Windows: vcpkg Boost, OpenSSL static, PostgreSQL disabled, Release config

## Conventions

**Source Layout**:
```
src/
  main.cpp                    # entry point, CLI parsing, signal handling
  core/                       # config, logging, version, service, authenticator
  session/                    # client/server/forward/nat/udp sessions
  ssl/                        # SSL session handling, defaults
  proto/                      # protocol: trojan request, socks5, udp packet
```

**Naming**:
- Classes: `PascalCase` (e.g., `ClientSession`, `Config`, `Service`)
- Methods: `snake_case` (e.g., `load_config`, `run_service`)
- Members: `snake_case` (e.g., `local_addr`, `run_type`)
- Files: lowercase with underscore (e.g., `clientsession.h`, `trojanrequest.cpp`)

**Config**: JSON with `run_type` (client/server/forward/nat), passwords as SHA-224 hashes. Example at `examples/client.json-example`.

**Logging**: `Log::log(msg, level)` with levels `OFF`, `FATAL`, `ERROR`, `WARN`, `INFO`, `DEBUG`. Use `Log::log_with_date_time()` for timestamped messages.

**Error Handling**: Exceptions caught in `main()`, logged via `Log::log_with_date_time("fatal: " + e.what(), Log::FATAL)`, then `exit(EXIT_FAILURE)`.

**Signals**: SIGINT/SIGTERM stop service; SIGHUP restarts; SIGUSR1 reloads cert (Linux/macOS only).

## Pitfalls

- **Tests require runtime deps**: `curl`, `netcat`, `openssl`, `python3` must be in PATH for `ctest` to pass.
- **Generated files**: `version.h` is generated at build time — don't hand-edit.
- **Default config path**: On Linux install, defaults to `/etc/trojan/config.json` (set via `DEFAULT_CONFIG` cmake option or env).
- **SIP003 plugin mode**: Activated via `SS_PLUGIN_OPTIONS` env var — expects specific env vars (`SS_REMOTE_HOST`, `SS_REMOTE_PORT`, etc.).
- **OpenSSL version mismatch**: Version info printed with `-v` shows compile-time vs runtime OpenSSL.
- **PostgreSQL optional but default ON**: Disable with `-DENABLE_POSTGRESQL=OFF` if libpq unavailable.
- **Windows builds**: Link `wsock32`, `ws2_32`, `crypt32`; no systemd/service install.
- **Static linking**: CI uses `-DBoost_USE_STATIC_LIBS=ON` and `-DOPENSSL_USE_STATIC_LIBS=ON` for portable binaries.