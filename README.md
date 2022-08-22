# FlowCV NDI Plugin

Adds [NDI](https://www.ndi.tv/) send and receive functionality to FlowCV

---

### Build Instructions

Prerequisites:

* [NDI SDK 5.5](https://www.ndi.tv/sdk/#download) or greater
* Clone [FlowCV](https://github.com/FlowCV-org/FlowCV) repo.
* See System specific build requirements for FlowCV [Building From Source](http://docs.flowcv.org/building_source/build_from_source.html)

cd to repo directory

```shell
mkdir Build
cd Build
```

### Windows

After NDI SDK install it will create the NDI_SDK_DIR environment variable so CMake should find the location and configure the build settings properly.

```shell
cmake .. -DFlowCV_DIR=/path/to/FlowCV/FlowCV_SDK
```

### Linux

You will need to extract the archive and set the location for NDI_DIR for CMake manually.

```shell
cmake .. -DFlowCV_DIR=/path/to/FlowCV/FlowCV_SDK -DNDI_DIR=/path/to/NDI_SDK
```

### MacOS

Install the NDI package which will install the SDK into your /Library folder and CMake should find and configure the build settings properly.

```shell
cmake .. -DFlowCV_DIR=/path/to/FlowCV/FlowCV_SDK
```





