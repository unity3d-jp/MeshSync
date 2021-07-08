# Changelog
All notable changes to the MeshSync package will be documented in this file.

## [0.8.0-preview] - 2021-07-2

### Added
* support plugin auto-installation of Blender 2.92 and 2.93
* plugin: move class macro from MeshSyncDCCPlugins 

### Changed
* better error messages when the auto-installation of  DCC plugin has failed
* use com.unity.film-internal-utilities@0.10.1-preview
* plugin: include <string> in ClientSettings.h 
* plugin: separate msAsyncSceneExporter into three files. 

### Fixed
* warnings when using Timeline 1.6.x 

### Removed
* plugin: remove noncopyable


## [0.7.2-preview] - 2021-03-22

### Changed
* use com.unity.film-internal-utilities@0.8.4-preview

### Fixed
* errors when installing Blender plugin from a project which has space in its path
* stop copying zip files unnecessarily when installing a supported DCC plugin 

## [0.7.1-preview] - 2021-02-03

### Fixed
* error when copying DCC Plugins after installing MeshSyncDCCPlugins 

## [0.7.0-preview] - 2021-02-03

### Added
* support SceneCachePlayer in Timeline 
* AnimationCurve editing of SceneCachePlayableAsset in Timeline 
* add buttons to set the curve of SceneCachePlayableAsset to linear or reset it to the original values

### Changed
* install DCC plugin from MeshSyncDCCPlugin package, instead of from Github
* apply animation tweaks of SceneCachePlayer directly without clicking Apply button 
* replace dependency from com.unity.anime-toolbox to com.unity.film-internal-utilities
* plugin: separate ClientSettings from msClient in the plugin code 
* doc: separate the use of SceneCache in Timeline to its own doc and add curves section 

### Fixed
* install Blender plugin on Mac 

## [0.6.1-preview] - 2020-12-16

### Fixed
* add missing DCC install scripts


## [0.6.0-preview] - 2020-12-15

### Added
* find Blender 2.91 in default locations

### Fixed
* create the asset folder before it's required instead of before updating the scene (#306)
* inaccurate internal path of SceneCachePlayer after copying to StreamingAssets
* include module dependencies in package dependencies 

## [0.5.5-preview] - 2020-12-03

### Changed
* use the SceneCache path specified in ProjectSettings for the path of SceneCachePlayer resources
* change the default generated resources path for SceneCache  
* disable logging by default 

### Fixed
* record property modification for SceneCachePlayer and MeshSyncServer prefabs, and support undo
* store the correct version of installed MeshSyncDCCPlugin


## [0.5.4-preview] - 2020-11-25

### Fixed
* plugin: add MeshSyncConstants implementation to fix undefined symbol errors 

## [0.5.3-preview] - 2020-11-20

### Changed
* plugin: update the source of ISPC library and update to use ISPC 1.14.1

## [0.5.2-preview] - 2020-11-19

### Changed
* plugin: remove "-Wl,--no-undefined" linker flag from MeshUtils plugin library

## [0.5.1-preview] - 2020-11-18

### Changed
* plugin: make plugin project dependencies explicit and remove msEnableZSTD preprocessor directive

## [0.5.0-preview] - 2020-11-17

### Added
* add a button to launch DCC tool in the Preferences page 
* notify users to restart Unity after upgrading MeshSync for Unity 2020.2+

### Changed
* update Preferences doc 
* use com.unity.anime-toolbox@0.2.0-preview

### Fixed
* misleading plugin installation info for multiple DCC Tools which have the same major version
* Blender plugin installation on Mac OSX

## [0.4.0-preview] - 2020-09-29

### Added
* add support for Blender 2.90
* enable the setting of the output path of Scene Cache assets
* select/load a new scene cache file via the inspector of ScenePlayerCache
* reload scene cache file via the inspector of ScenePlayerCache 
* reload/refresh multiple SceneCachePlayer using their original SceneCache file paths 

### Changed
* split ProjectSettings to Server tab and SceneCache tab
* change the layout of the GUI to copy scene cache to StreamingAssets for SceneCachePlayer
* move DCCTool Settings to Preferences (User Settings)
* refactor: refactored the initialization code of SceneCachePlayer
* doc: update doc about Project Settings, Preferences, and SceneCache 

## [0.3.4-preview] - 2020-09-09

### Fixed
* error when creating SceneCachePlayer 

## [0.3.3-preview] - 2020-09-08

### Fixed
* plugin: insufficient number of mesh refiner attributes (causing crash when an object has 8 UV sets)

## [0.3.2-preview] - 2020-09-07

### Changed
* plugin: cleanup the dependencies of MeshSync plugin library code  

### Fixed
* update the version of the required MeshSyncDCCPlugins 


## [0.3.1-preview] - 2020-09-04

### Changed
* plugin: major plugin code refactoring to reduce dependencies and make used types more explicit 

## [0.3.0-preview] - 2020-09-03

### Added
* core multiple UV support
* Set DetailMap Albedo and Secondary UV properties of StandardMaterial 

### Changed
* plugin: use bit shifting and masking for flag structures instead of using bitfields
* plugin: organize the source code of cmake-built project by folder 
* chore: configure Yamato setting to target 2019.4, and add a job to build docs 
* doc: update package installation steps, esp. for 2020.1 

## [0.2.5-preview] - 2020-08-31

### Fixed
* compile error when targeting Android platform

## [0.2.4-preview] - 2020-07-28

### Fixed
* fix errors when building app build

## [0.2.3-preview] - 2020-07-08

### Fixed
* fix bug in preventing DNS rebinding
* fix broken links in docs and package warnings


## [0.2.2-preview] - 2020-07-03

### Fixed
* fix prevent access using DNS rebinding

## [0.2.1-preview] - 2020-07-02

### Changed
* Update latest known MeshSyncDCCPlugins version to 0.2.0-preview
* various polishes
* doc: Add installation steps for Unity 2020 in the top readme

### Fixed
* fix a bug in ProjectSettings when switching tab
* prevent server root path traversal 
* prevent public access to the server by default 

## [0.2.0-preview] - 2020-06-17

### Added
* add Project Settings to configure MeshSync objects and DCC Tools integrations
* add DCC Tools integration support for Maya 2017-2020
* add DCC Tools integration support for 3ds Max 2017-2020
* add DCC Tools integration support for Blender 2.79, 2.80, 2.81, 2.82, 2.83
* doc: add documentation about Project Settings
* deps: use com.unity.sharp-zip-lib


## [0.1.0-preview] - 2020-04-16

### Added
* add a menu item to download DCC plugins

### Changed
* plugin: change plugin build configuration to Release from MinSizeRel
* doc: update links to MeshSyncDCCPlugins

## [0.0.3-preview] - 2020-03-24

### Added
* docs: add Linux section for building plugins
* docs: add a separate doc page for building zstd

### Changed
* plugin: prebuilding zstd on Mac and Linux
* plugin: update zstd library on Windows to 1.4.4
* plugin: use Unicode for building plugins
* docs: update plugin build steps for Windows and Mac

### Fixed
* fix: compile errors on Linux

## [0.0.2-preview.2] - 2020-03-17

### Fixed
* fix pluginVersion warning

## [0.0.2-preview.1] - 2020-03-16

### Changed
* Removing unused meta files	
* docs: Updating img tag to MD

## [0.0.2-preview] - 2020-03-13

### Changed
* plugin: Add additional Windows dependencies directly in Poco_LIBRARIES	
* docs: Specify the version of Poco and zstd used. Also some other minor updates

### Fixed
* Add missing Tests.meta
* docs: Fix broken links


## [0.0.1-preview.3] - 2020-03-10

### Fixed
* plugin: fix script to build plugin

## [0.0.1-preview.2] - 2020-03-06

### Added
* server manual start 


### Fixed
* fix the deployment of MeshSync's streaming assets 
* plugin: fix the build process on Mac (muEnableISPC, Poco conf)

## [0.0.1-preview.1] - 2020-03-02

### Changed
* Removing unrelevant files from a package

### Fixed
* Minor license fix

## [0.0.1-preview] - 2020-02-28

### Changed
* Converting To Package format

