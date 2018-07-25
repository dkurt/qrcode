## QR codes detector

This project is a part of practice course at Intel's Summer Internship program 2018


### Getting started
* Clone this repository to your computer
  ```
  git clone https://github.com/dkurt/qrcode.git
  cd qrcode
  ```

* Initialize submodules (useful 3rdparty libraries from different repositories)
  ```
  git submodule init
  git submodule update
  ```

* Build a project (make sure if `cmake` installed)
  ```
  mkdir build && cd build
  ```

  * Linux
    ```
    cmake -DCMAKE_BUILD_TYPE=Release .. && make -j4
    ```

  * Microsoft Windows
    ```
    "C:\Program Files\CMake\bin\cmake.exe" -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 14 Win64" ..
    "C:\Program Files\CMake\bin\cmake.exe" --build . --config Release
    ```
