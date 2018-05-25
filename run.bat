"C:\Program Files\CMake\bin\cmake.exe" -DCMAKE_BUILD_TYPE=RELEASE -G "Visual Studio 14 Win64" ..
"C:\Program Files\CMake\bin\cmake.exe" --build . --config Release -- /m:4
.\Release\qrcode.exe test
