# fat32emulation
The project was designed for educational purposes. Here are implemented a simple, user-space, shell-like utility that is capable of reading and writing to FAT32 file system image.

## Get the code
```
git clone https://github.com/MaksymKuzminchuk/fat32emulation.git
```

## Requirement

fat32emulation is not depend on other libraries

## Build

For Linux users:

`make`
## Usage

Run `./fat32emulation <your_img>`. If the file doesn't exist then it will be created with 20Mb size. 

## Available commands

`ls` - list of files in current directory
`cd` - change current directory (use absolute path with starting `/`)
`mkdir <name>` - create new directory
`touch <name>` - create new file
`format` - formatting your image to fat32
`exit` - exit the app
