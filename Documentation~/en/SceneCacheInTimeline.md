# Scene Cache In Timeline

![Menu](../images/SceneCacheInTimeline.png)

[Scene Cache](SceneCache.md) can be controlled via 
[Timeline](https://docs.unity3d.com/Packages/com.unity.timeline@latest) 
by performing the following steps:

1. Open the Timeline window.
1. In the Timeline Window, click **Unity.MeshSync > Scene Cache Track** 
   to add a **SceneCacheTrack**.
1. Drag and drop the SceneCachePlayer GameObject to the newly added track, 
   which will automatically create a [SceneCachePlayableAsset](#scene-cache-playable-asset).

To view animation curves of the clip, click the Curves icon next to the Track name.

## Scene Cache Playable Asset

![SceneCachePlayableAsset](../images/SceneCachePlayableAsset.png)

- **Scene Cache Player:**  
The [SceneCachePlayer](SceneCache.md#scene-cache-player) to be played in Timeline.

- **Curves**  
  - **To Linear:** set the animation curve to a linear curve.
  - **Apply Original:** apply the original animation curve from the Scene Cache (.sc) file.






