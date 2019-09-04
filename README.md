# RenderingX

Core library (with example) for rendering smooth lines and Text with OpenGL.
Vertex displacment distortion correction
SuperSync

**Setup Dependencies**\
2. Forking the repo and including sources manually:
* To your top level settings.gradle file, add
```
include ':VideoCore'
project(':VideoCore').projectDir=new File('..\\Telemetry\\TelemetryCore')
```
and modify the path according to your download file
* To your app level gradle file add
```
implementation project(':TelemetryCore')
```
See [FPV-VR](https://github.com/Consti10/FPV_VR_2018) as an example how to add dependencies.

