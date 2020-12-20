# Connected Texture Project

Program *attempts* to generate seamless connected tile textures using two provided tile textures
<br /><br /><br />

## Description
---
Fourteen connected textures are created based on two images.  Generation of each texture is run on their own thread for increased efficiency.  Using normal distributions and walking algorithms, samples are randomly generated for calculating a gradient.  The algorithm only accepts a sample array that begins and ends with samples at most 1 unit away from each other.  This constraint helps with seamless connection when repeating the same tile or another adjacent tile.  The samples are then used to create a gradient with matching width and height of the input images.  Values closer to 1.0 make the pixel biased towards the top image while values closer to 0.0 make the pixel biased towards the bottom image.  The samples provide the "halfway" line for when the gradient value is 0.5.  Finally, per-pixel blending is calculated and applied to the output image.  All threads are then closed and the program exits.

If the input images both have alpha/gamma channels then the program will blend them as well.  Otherwise the image with the least amount of channels determines the output channel count.

A default settings file will be created in the current working directory if one isn't specified on the command line. **NOTE:** If a settings file is specified but does not have all the settings defined, a default value will be set.

Not every output image looks perfect...**yet**. Keep repeating the command until you find one that looks good.
<br /><br />
Required command options:

* -t *png-path*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;path to top image
* -b *png-path*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;path to bottom image
* -o *png-path*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;path to output image

Optional command options:

* -s *file-path*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;path to settings file
<br /><br /><br />

## Getting Started
---
### Limitations
* Input files must be supported by stb_image
* Input images must have the same width and height
* Single-connection textures are **not** generated
* This program assumes you have already made the base tiles seamless

### Dependencies

* premake5 for creating solution or makefile
* glm as git submodule

### Building
* Creating project files requires premake5. Install premake if not installed already.
* Install glm as a git submodule by running `git submodule init` and then `git submodule update`
* Either use generate scripts or use premake on command line to create project files
* Build the executable using either make or visual studio

### Executing program

1. Configure settings in settings file
2. Run the program on the command line:
```
./ct -t top-img-path -b bottom-img-path -o output-img-path [-t settings-path]
```
<br /><br /><br />

## Example
---
### Top Image
![Top Image](https://github.com/docter60/connected-textures/blob/master/demo/grass.png)
### Bottom Image
![Bottom Image](https://github.com/docter60/connected-textures/blob/master/demo/dirt.png)
### Result
![Output Image](https://github.com/docter60/connected-textures/blob/master/demo/out.png)
### Settings file
You can view an example settings file in the [demo](https://github.com/docter60/connected-textures/blob/master/demo/settings.txt) folder.
<br /><br /><br />

## Testing
---
Test images are provided in the test folder. From the repository directory, run this command on Windows:
```
.\bin\Debug\ct.exe -t test\white128.png -b test\black128.png -o test\out128.png
```
Or this command on linux:
```
./bin/Debug/ct -t test/white128.png -b test/black128.png -o test/out128.png
```
<br /><br /><br />

## Acknowledgments
---
* Uses [stb_image](https://github.com/nothings/stb) and [stb_image_write](https://github.com/nothings/stb)
* Uses [glm](https://github.com/g-truc/glm)