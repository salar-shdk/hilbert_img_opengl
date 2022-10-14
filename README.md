# hilbert_img

This project creates a hilbert curve and sets the colors according to the given image. For example consider we are using this picture.

![img](https://user-images.githubusercontent.com/33146532/195764634-1e96fb2c-d020-4ac5-a460-ab45b88e7239.jpg)

The result would be the following images:

![screenshot_20221013-184646_1920x1080](https://user-images.githubusercontent.com/33146532/195764974-753ab858-443c-4cad-aed7-fa2bf9904baf.png)

![screenshot_20221013-184643_1920x1080](https://user-images.githubusercontent.com/33146532/195764983-9df60068-c584-4d6c-a567-63300c42d8fe.png)

And because its a space-filling curve, if you rise the iteration enough, you will get a full image.

![screenshot_20221013-184733_1920x1080](https://user-images.githubusercontent.com/33146532/195765063-650582d2-e4e2-4a90-bb0c-3f6824d2a128.png)

# Build

to build the project run:
```bash
./build -f
``` 

to clean build files run:
```bash
./build -c
```

# Usage

After building, you may find executable and build files in 'Release' directory. To run it go into Release Directory and:

```bash
./hilbert {image_file_path}

# Example:
./hilbert ./image.png
```

you can zoom in and zoom out arrows up and down, and change iteration with left and right arrows.

# Dependencies

[opencv](https://opencv.org/) | OpenCV is a library of programming functions mainly aimed at real-time computer vision.

[opengl](https://www.opengl.org/) | OpenGL is a cross-language, cross-platform application programming interface for rendering 2D and 3D vector graphics.

**Other dependencies:** (you don't need to install them, they are already in the thirdparty directory)

```
argh
fmt
glew
glfw
glm
vivid
```
