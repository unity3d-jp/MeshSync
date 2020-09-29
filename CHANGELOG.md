# Changelog
All notable changes to the MeshSync package will be documented in this file.

## [0.4.0-preview] - 2020-09-29
* feat: add support for Blender 2.90
* feat: split ProjectSettings to Server tab and SceneCache tab
* feat: enable the setting of the output path of Scene Cache assets
* feat: select/load a new scene cache file via the inspector of ScenePlayerCache
* feat: reload scene cache file via the inspector of ScenePlayerCache 
* feat: reload/refresh multiple SceneCachePlayer using their original SceneCache file paths 
* ui: change the layout of the GUI to copy scene cache to StreamingAssets for SceneCachePlayer
* chore: move DCCTool Settings to Preferences (User Settings)
* refactor: refactored the initialization code of SceneCachePlayer
* doc: update doc about Project Settings, Preferences, and SceneCache 

## [0.3.4-preview] - 2020-09-09
* fix: error when creating SceneCachePlayer 

## [0.3.3-preview] - 2020-09-08
* fix: insufficient number of mesh refiner attributes (causing crash when an object has 8 UV sets)

## [0.3.2-preview] - 2020-09-07
* chore: cleanup the dependencies of MeshSync plugin library code  
* fix: update the version of the required MeshSyncDCCPlugins 


## [0.3.1-preview] - 2020-09-04
* refactor: major plugin code refactoring to reduce dependencies and make used types more explicit 

## [0.3.0-preview] - 2020-09-03
* feat: core multiple UV support
* feat: Set DetailMap Albedo and Secondary UV properties of StandardMaterial 
* refactor: use bit shifting and masking for flag structures instead of using bitfields
* chore: organize the source code of cmake-built project by folder 
* chore: configure Yamato setting to target 2019.4, and add a job to build docs 
* doc: update package installation steps, esp. for 2020.1 

## [0.2.5-preview] - 2020-08-31
* fix: compile error when targeting Android platform

## [0.2.4-preview] - 2020-07-28
* fix: fix errors when building app build

## [0.2.3-preview] - 2020-07-08
* fix: bug in preventing DNS rebinding
* fix: broken links in docs and package warnings


## [0.2.2-preview] - 2020-07-03
* fix: prevent access using DNS rebinding

## [0.2.1-preview] - 2020-07-02

* fix: fix a bug in ProjectSettings when switching tab
* fix: prevent server root path traversal 
* fix: prevent public access to the server by default 
* chore: Update latest known MeshSyncDCCPlugins version to 0.2.0-preview
* chore: various polishes
* doc: Add installation steps for Unity 2020 in the top readme

## [0.2.0-preview] - 2020-06-17
* feat: add Project Settings to configure MeshSync objects and DCC Tools integrations
* feat: add DCC Tools integration support for Maya 2017-2020
* feat: add DCC Tools integration support for 3ds Max 2017-2020
* feat: add DCC Tools integration support for Blender 2.79, 2.80, 2.81, 2.82, 2.83
* doc: add documentation about Project Settings
* chore: use com.unity.sharp-zip-lib


## [0.1.0-preview] - 2020-04-16
* feat: add a menu item to download DCC plugins
* chore: change plugin build configuration to Release from MinSizeRel
* docs: update links to MeshSyncDCCPlugins

## [0.0.3-preview] - 2020-03-24
* fix: compile errors on Linux
* chore: prebuilding zstd on Mac and Linux
* chore: update zstd library on Windows to 1.4.4
* chore: use Unicode for building plugins
* docs: update plugin build steps for Windows and Mac
* docs: add Linux section for building plugins
* docs: add a separate doc page for building zstd


## [0.0.2-preview.2] - 2020-03-17
* fix: pluginVersion warning

## [0.0.2-preview.1] - 2020-03-16
* chore: Removing unused meta files	
* docs: Updating img tag to MD

## [0.0.2-preview] - 2020-03-13
* chore: Add additional Windows dependencies directly in Poco_LIBRARIES	
* chore: Add missing Tests.meta
* docs: Fix broken links
* docs: Specify the version of Poco and zstd used. Also some other minor updates


## [0.0.1-preview.3] - 2020-03-10
* chore: fix script to build plugin

## [0.0.1-preview.2] - 2020-03-06

* chore: fix the build process on Mac (muEnableISPC, Poco conf)
* feat: server manual start 
* fix: the deployment of MeshSync's streaming assets 

## [0.0.1-preview.1] - 2020-03-02

* Removing unrelevant files from a package
* Minor license fix

## [0.0.1-preview] - 2020-02-28

* Converting To Package format

