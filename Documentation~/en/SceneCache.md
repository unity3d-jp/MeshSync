# Scene Cache

Scene Cache is a feature to playback all frames of an *.sc* file that 
was exported using [MeshSyncDCCPlugins](https://github.com/Unity-Technologies/MeshSyncDCCPlugins)
installed in a DCC Tool. 
This functionality is very similar to [AlembicForUnity](https://docs.unity3d.com/Packages/com.unity.formats.alembic@latest/index.html),
but it has the following differences:

1. Scene Cache is designed to playback frames precisely with high performance.
2. Scene Cache supports material export/import
3. Unlike Alembic, *.sc* files are only playable in Unity.

## How to use

From the menu, select **Game Object > MeshSync > Create Cache Player**, 
and then select the *.sc* file exported by the DCC tool.  
This will automatically create a GameObject with SceneCachePlayer component, 
which will be played automatically in playmode.

![Menu](../images/MenuCreateCachePlayer.png)

### Playback via Timeline

![Menu](../images/SceneCacheInTimeline.png)

We can also control the playback of Scene Cache via 
[Timeline](https://docs.unity3d.com/Packages/com.unity.timeline@latest) 
by performing the following steps:


1. Open the Timeline window.
1. Add a **SceneCacheTrack** in the Timeline Window.
1. Right click in the newly added track, and click **Add Scene Cache Playable Asset**.
1. Drag and drop the GameObject with SceneCachePlayer component to the *Scene Cache Playable Ref* 
   property of **SceneCachePlayableAsset**


## Scene Cache Player

![SceneCachePlayer](../images/SceneCachePlayer.png)

This component handles the playback. 
There are many settings which are in common with [MeshSyncServer](MeshSyncServer.md).

When "Create Cache Player" on the menu is selected, an AnimationClip, which also handles the animation Playback, will be created and tied to the SceneCachePlayer.  
We can place this clip on an AnimationTrack to play it in Timeline.

- **Cache File Path**  
Initially, the path to the *.sc* file is absolute, and therefore the animation can only be played on that PC.  
Copying the cache file to StreamingAssets is recommended, and can be done by simply clicking the "Copy" button.  
This will allow us to play the clips on the same project copied to another PC, or in a runtime executable.

- **Time**  
This is the playback time. We can play the animation by moving this parameter.
Usually this will be controlled by the AnimationClip.

- **Interpolation**  
Smooths animations by interpolating meshes and transforms from previous and subsequent frames.   
Mesh is only interpolated if the topologies match (the index remains unchanged).

