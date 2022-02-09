# RenderingX
![git_ci_build](https://github.com/Consti10/RenderingX/actions/workflows/Android-CI.yml/badge.svg)
[![circle_ci_build](https://circleci.com/gh/Consti10/RenderingX.svg?style=svg)](https://circleci.com/gh/Consti10/RenderingX)

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


**Building** \
This project depends on both JAVA and CPP code that is included in [LiveVideo10ms](https://github.com/Consti10/LiveVideo10ms). \
To build this as a standalone project, clone this repository with submodules included and (possibly) modify the settings.gradle file
(See comments in settings.gradle). Make sure to select the main repository folder (RenderingX) when opening the project in Android Studio.

**Include as Submodule** \
If you want to use RenderingXCore as a module in your own project,see [FPV-VR](https://github.com/Consti10/FPV_VR_OS) as an example how to add dependencies.

