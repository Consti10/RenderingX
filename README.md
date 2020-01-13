# RenderingX

[![](https://jitci.com/gh/Consti10/RenderingX/svg)](https://jitci.com/gh/Consti10/RenderingX)

Core library (with example) for rendering smooth lines and Text with OpenGL.
Vertex displacment distortion correction
SuperSync

<img src="https://github.com/Consti10/RenderingX/blob/master/Screenshots/main.png" alt="Example" width="240"/>
<img src="https://github.com/Consti10/RenderingX/blob/master/Screenshots/smoothText.png" width="240"/> <img src="https://github.com/Consti10/RenderingX/blob/master/Screenshots/smoothIcons.png" width="240"/>
<img src="https://github.com/Consti10/RenderingX/blob/master/Screenshots/smoothLines.png" width="240"/> <img src="https://github.com/Consti10/RenderingX/blob/master/Screenshots/geometry.png" width="240"/> \

<img src="https://github.com/Consti10/RenderingX/blob/master/Screenshots/example_distortion_vertex_displacement.png.png" width="240"/>
<img src="https://github.com/Consti10/RenderingX/blob/master/Screenshots/example_distortion_360.png" width="240"/>


**Setup Dependencies** \
**Forking the repo and including sources manually:** \
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

