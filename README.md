![DELQA](https://raw.githubusercontent.com/cwhitney/DelqaTools/master/DELQA_logo.png)
##### Open source tools from the installation 
##### 7/18/2015
---
Visit [http://delqa.is](http://delqa.is) for documentation and a full list of collaborators.

This is an open source tool we developed for the interactive sound installation, DELQA.  It was installed at the New Museum's NEW INC space from August 6-9th in 2015.  The installation utilized 8 Kinect 2 sensors that tracked visitors interacting with and deforming the environment.  This tool is specifically built to help track soft-surface deformation and touches.  We used it to track visitors pushing and pulling fabric meshes inside of a our purpose built architecture.

The code utilizes several open source projects including:

Cinder 0.9.0dev - Barbarian Group - https://github.com/cinder/Cinder

Cinder OpenCV https://github.com/cinder/Cinder-OpenCV

KCB2 - Wieden and Kennedy - https://github.com/wieden-kennedy/Cinder-KCB2

Pretzel Gui - Charlie Whitney - https://github.com/cwhitney/PretzelGui


#####Setting up your dev environment 
---
This application was built on Windows 8.1 using Visual Studio 2013 Community edition.

Visual Studio [[Download](https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx)]

Kinect for Windows SDK [[Download](http://www.microsoft.com/en-us/download/details.aspx?id=44561)]

At the time of writing, Cinder 0.9.0 was in development and not officially released yet.  The SHA1 of the version used is 71502f739fb7adbeea8e85b29a363f9015df4b35.  It can be cloned from github with the following commands
```
git clone https://github.com/cinder/Cinder.git
git checkout 71502f739fb7adbeea8e85b29a363f9015df4b35
```

Instructions on how to build Cinder can additionally be found here: http://libcinder.org/docs/welcome/GitSetup.html

Clone this repository into a directory inside of cinder, two directories deep.  For example if your Cinder directory is at:
```
C:\Users\You\Document\Cinder
```
You would want to clone this in a folder at:
```
C:\Users\You\Document\Cinder\apps\delqa
```

#####Anatomy of the application
---
|File | Description|
|-----|-----------|
| DelqaTrackApp | The main application file. Notably contains the GUI setup. |
| KinectManager | Handles getting data from the Kinect into a depth channel |
| OscManager | Broadcasts our information once collected |
| BlobTracker | The work horse. Uses openCv to detect touches in our depth image |
