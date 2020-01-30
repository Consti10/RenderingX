# RenderingX

[![](https://jitci.com/gh/Consti10/RenderingX/svg)](https://jitci.com/gh/Consti10/RenderingX)

Core library (with example) for rendering smooth lines and Text with OpenGL.
Vertex displacment distortion correction
SuperSync

| Main screen | Smooth text |
| :---:  | :---: |
| <img src="Screenshots/main.png"> | <img src="Screenshots/smoothText.png"> |

| Smooth icons | Smooth lines | Colored geometry
| :---:  | :---: | :---: |
| <img src="Screenshots/smoothIcons.png"> | <img src="Screenshots/smoothLines.png"> | <img src="Screenshots/geometry.png"> |

| Example Vertex Displacement | Example 360 image |
| :---:  | :---: |
| <img src="Screenshots/example_distortion_vertex_displacement.png"> | <img src="Screenshots/example_distortion_360.png"> |


**Setup Dependencies** \
**Forking the repo and including sources manually:** \
* To your top level settings.gradle file, add
```
include ':RenderingXCore'
project(':RenderingXCore').projectDir=new File('..\\RenderingX\\RenderingXCore')
```
and modify the path according to your download file
* To your app level gradle file add
```
implementation project(':RenderingXCore')
```
See [FPV-VR](https://github.com/Consti10/FPV_VR_2018) as an example how to add dependencies.

