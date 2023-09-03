# elker - A Counter Strike Tradeup Searcher
![elker](resources/branding/elker-logo.png)


[![ci](https://github.com/fpetm/elker/actions/workflows/ci.yml/badge.svg)](https://github.com/fpetm/elker/actions/workflows/ci.yml)

## Build instructions

0. Clone repository: `git clone https://github.com/fpetm/elker.git`

1. Update git submodules: `git submodule update --init --recursive`

2. Generate build configuration: `cmake -Bbuild .`

3. Configure build configuration: `ccmake build` or `cmake-gui build`

4. Build project: `cmake --build build`  
The elker executable is located in build/elker/elker

## Updating skin data
0. Create virtual environment and clone requirements
```shell
python -m venv ./venv
source venv/bin/activate # on unix
./venv/Scripts/activate # on windows
python -m pip install -r requirements
```

1. Get skin data from the CS:GO game files
```shell
python script/getskins.py
```

2. Get skin prices (You must have the environment variable `STEAMAPIS_API_KEY` set an API key from steamapis.com with `market/item` permission)
```shell
python script/updateprices.py
```

## Usage
After you made sure that the prices of skins has been updated simply run the executable.  
Configuration of search parameters can only be done by modifing the source code and rebuilding it.
