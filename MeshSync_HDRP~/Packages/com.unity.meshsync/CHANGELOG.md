# Changelog
All notable changes to the MeshSync package will be documented in this file.

## [0.10.1-preview] - 2021-11-17

### Added
* doc: add a section about SceneCache import settings

### Changed
* doc: update the documentation of MeshSync components (MeshSyncServer, SceneCachePlayer)
* doc: update ProjectSettings documentation

### Fixed
* fix: validate the asset folder of MeshSyncServer component
* fix: null check when changing an object's material in the Scene view

## [0.10.0-preview] - 2021-11-10

### Added
* feat: search materials based on MaterialSearchMode settings for MeshSyncServer and SceneCachePlayer
* feat: add a custom inspector for SceneCache files under Assets folder to set material creation settings 
* feat: override material creation settings for SceneCachePlayer when the assigned SceneCache file is under Assets folder
* feat: allow enabling/disabling the creation and update of Lights and Cameras objects for MeshSyncServer and SceneCachePlayer
* feat: add a checkbox in ProjectSettings to set the default material creation settings
* feat: add options in ProjectSettings to set the default setting to use physical camera params 
* feat: add a button to reset MeshSync configs in ProjectSettings 
* api: open MeshSyncPlayer class to public
* api: add a callback in MeshSyncServer that is called after receiving data 
* api: open SceneCachePlayer to public 
* api: add a public API to force update SceneCachePlayer

### Changed
* deps: update dependency to com.unity.film-internal-utilities@0.12.2-preview
* ui: indent "Update Mesh Colliders" setting in ProjectSettings

### Fixed
* fix: try to use existing animationController asset for SceneCache if possible

### Removed
* removed plugin installation support for Maya 2017 and 3ds Max 2017

## [0.9.3-preview] - 2021-10-20

### Changed
* deps: update to use com.unity.sharp-zip-lib@1.2.2-preview.2
* deps: update dependency to com.unity.film-internal-utilities@0.11.1-preview 
* doc: update SceneCachePlayableAsset documentation

### Fixed
* fix: prioritize files under "Library/PackageCache/*" for installing DCC Plugins (#444)
* fix: retain previous GameObjects if applicable when reloading SceneCache
* fix: previously manually added children should not be saved along as prefab when reloading SceneCache
* fix: choose the correct shader for default materials on URP projects 
* fix: set light values correctly on HDRP projects
* fix errors when dragging clips with SceneCachePlayableAsset to another track
* fix the incorrect extra addition of curve key when dragging a folder to SceneCacheTrack

## [0.9.2-preview] - 2021-10-07

### Fixed
* fix: make reloading SceneCache work properly again 
* fix: properly normalize the paths inside SceneCachePlayer 

## [0.9.1-preview] - 2021-09-15

### Fixed
* fix: automatic plugin installation 

## [0.9.0-preview] - 2021-09-14

### Added
* feat: support automatic installation for Maya 2022 and 3ds Max 2021 
* feat: add an option in SceneCachePlayableAsset to snap to nearest frame
* feat: add an option in ProjectSettings to set the default SnapToFrame behaviour 

### Changed
* deps: update dependencies to com.unity.film-internal-utilities@0.11.0-preview 
* opt: record instance modification if there is an actual change for SceneCachePlayer
* opt: disable serialization temporarily when creating SceneCache for the first time 

### Fixed
* fix: ensure SceneCache prefab modifications are saved after creating 

## [0.8.4-preview] - 2021-09-02

### Changed
* deps: update dependencies to com.unity.film-internal-utilities@0.10.2-preview 
* opt: optimize SceneCache inspector by doing BeginChangeCheck() per property

## [0.8.3-preview] - 2021-07-21

### Fixed
* SceneCache not playing properly when started from non-zero time
* Ensure error and warning logs appear for SceneCachePlayer 

## [0.8.2-preview] - 2021-07-14

### Fixed
* doc: various documentation fixes

## [0.8.1-preview] - 2021-07-12

### Changed
* convert changelog format to semantics versioning 
* doc: update all English documentations

### Removed
* remove plugin auto-installation for Blender 2.79 
* doc: remove Japanese translation inside the package. (Moved to the doc official site)
* doc: remove Japanese readme page 

## [0.8.0-preview] - 2021-07-02

### Added
* support plugin auto-installation of Blender 2.92 and 2.93
* plugin: move class macro from MeshSyncDCCPlugins 

### Changed
* better error messages when the auto-installation of  DCC plugin has failed
* use com.unity.film-internal-utilities@0.10.1-preview
* plugin: include &lt;string&gt; in ClientSettings.h 
* plugin: separate msAsyncSceneExporter into three files. 

### Fixed
* fix warnings when using Timeline 1.6.x 

### Removed
* plugin: remove noncopyable


## [0.7.2-preview] - 2021-03-22

### Changed
* deps: use com.unity.film-internal-utilities@0.8.4-preview

### Fixed
* fix errors when installing Blender plugin from a project which has space in its path
* stop copying zip files unnecessarily when installing a supported DCC plugin 

## [0.7.1-preview] - 2021-02-03

### Fixed
* fix errors when copying DCC Plugins after installing MeshSyncDCCPlugins 

## [0.7.0-preview] - 2021-02-03

### Added
* support SceneCachePlayer in Timeline 
* AnimationCurve editing of SceneCachePlayableAsset in Timeline 
* add buttons to set the curve of SceneCachePlayableAsset to linear or reset it to the original values

### Changed
* install DCC plugin from MeshSyncDCCPlugin package, instead of from Github
* apply animation tweaks of SceneCachePlayer directly without clicking Apply button 
* deps: replace dependency from com.unity.anime-toolbox to com.unity.film-internal-utilities
* plugin: separate ClientSettings from msClient in the plugin code 
* doc: separate the use of SceneCache in Timeline to its own doc and add curves section 

### Fixed
* fix the installation of Blender plugin on Mac 

## [0.6.1-preview] - 2020-12-16

### Fixed
* add missing DCC install scripts


## [0.6.0-preview] - 2020-12-15

### Added
* find Blender 2.91 in default locations

### Changed
* include module dependencies in package dependencies 

### Fixed
* create the asset folder before it's required instead of before updating the scene (#306)
* fix inaccurate internal path of SceneCachePlayer after copying to StreamingAssets

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
* deps: use com.unity.anime-toolbox@0.2.0-preview

### Fixed
* fix misleading plugin installation info for multiple DCC Tools which have the same major version
* fix Blender plugin installation on Mac OSX

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
* fix errors when creating SceneCachePlayer 

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
* fix compile error when targeting Android platform

## [0.2.4-preview] - 2020-07-28

### Fixed
* fix errors when building app build

## [0.2.3-preview] - 2020-07-08

### Fixed
* fix bug in preventing DNS rebinding
* fix broken links in docs and package warnings


## [0.2.2-preview] - 2020-07-03

### Fixed
* fix the prevention of access using DNS rebinding

## [0.2.1-preview] - 2020-07-02

### Changed
* Update latest known MeshSyncDCCPlugins version to 0.2.0-preview
* various polishes
* doc: Add installation steps for Unity 2020 in the top readme

### Fixed
* fix a bug in ProjectSettings when switching tab
* fix the prevention of server root path traversal 
* fix the prevention of public access to the server by default 

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
* doc: add Linux section for building plugins
* doc: add a separate doc page for building zstd

### Changed
* plugin: prebuilding zstd on Mac and Linux
* plugin: update zstd library on Windows to 1.4.4
* plugin: use Unicode for building plugins
* doc: update plugin build steps for Windows and Mac

### Fixed
* fix: compile errors on Linux

## [0.0.2-preview.2] - 2020-03-17

### Fixed
* fix pluginVersion warning

## [0.0.2-preview.1] - 2020-03-16

### Changed
* Removing unused meta files	
* doc: Updating img tag to MD

## [0.0.2-preview] - 2020-03-13

### Changed
* plugin: Add additional Windows dependencies directly in Poco_LIBRARIES	
* doc: Specify the version of Poco and zstd used. Also some other minor updates

### Fixed
* Add missing Tests.meta
* doc: Fix broken links


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

