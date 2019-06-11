![demo](https://user-images.githubusercontent.com/1488611/39971828-98afa1d8-573d-11e8-9a6f-86263bee8949.gif)
# MeshSync

MeschSync reflects changes to models made in DCC tools in real time in Unity. This allows devs to immediately see how things will look in-game while modelling.  
MeshSync works as a plugin for Unity and DCC tools, and currently supports: [Maya](https://www.autodesk.eu/products/maya/overview), [Maya LT](https://www.autodesk.eu/products/maya-lt/overview), [3ds Max](https://www.autodesk.com/products/3ds-max/overview), [Blender](https://blenderartists.org/), [Metaseq](http://www.metaseq.net/), and [xismo](http://mqdl.jpn.org/).


## Guides
1. [Maya](#maya)
2. [Maya LT](#maya-lt)
3. [3ds Max](#3ds-max)
4. [Motion Builder](#motion-builder)
5. [Blender](#blender)
6. [Modo](#modo)
7. [Metaseq](#Metaseq)
8. [VRED](#vred)
9. [xismo](#xismo)
10. [Unity](#unity)

<img align="right" src="https://user-images.githubusercontent.com/1488611/39971860-7f6d1330-573e-11e8-9a1e-9d95709cbd50.png" height=400>

### Maya
Confirmed functionality with Maya 2015, 2016, 2016.5, 2017, 2018, 2019 + Windows, Mac, and Linux (CentOS 7).
- Installation:
  - Download UnityMeshSync_Maya_*.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases).
  - Windows: If %MAYA_APP_DIR% is already setup, copy the modules directory there, if not copy it to %USERPROFILE%\Documents\maya (← copy paste into the Explorer address bar).
    - For versions prior to 2016, copy to the version name directory (%MAYA_APP_DIR%\2016 etc.)
  - Mac: Copy the UnityMeshSync directory and .mod file to /Users/Shared/Autodesk/modules/maya.
  - Linux: Copy the modules directory to ~/maya/(Maya version).
- Start Maya, then go to Windows -> Settings/Preferences -> Plug-in Manager, and activate the plugin by checking Loaded under MeshSyncClient.
- Now that the UnityMeshSync shelf has been added, click on the gear icon to open the settings menu. 
- While "Auto Sync" is checked, any edits will automatically be reflected in Unity. When Auyo Sync is deactivated, click the  "Manual Sync" button to sync changes.
- Clicking Sync under Animations causes the timer to advance from the first frame to the final frame while baking the animation and sending it to Unity. 

&nbsp;  

- The other buttons correspond to their respective manual sync and animation sync functions. 
- Polygon mesh, camera, and light sync are supported. 
- Polygon mesh will carry skinning/bones (SkinCluster) and BlendShapes over to Unity as is.
  - MeshSync will attempt to apply any additional deformers, but if there is a SkinCluster before or after them they may not apply correctly. 
  - Check "Bake Deformers" to sync the results of applying all deformers. This will mostly sync the Mesh on both the Maya and Unity sides, but this will result in loss of Skinning and BlendShape information.
- Checing "Double Sided" will cause the Mesh to become double-sided on the Unity side
- Be advised that the negative scale is only supported for certain elements. 
  - If XYZ all have negative values, the Mesh will sync properly, however if only one axis has a negative value Unity will treat the Mesh as though every axis has a negative value.
- Non-polygon shape data such as NURBS is not supported.
- Instancing is supported, but instancing for skinned meshes is currently not supported (on the Unity side they all end up in the same position as the original instance). 
- Commands are also registered to MEL, and all features can be accessed through MEL. See [the source code] (https://github.com/unity3d-jp/MeshSync/blob/master/.MeshSync/Plugin/MeshSyncClientMaya/msmayaCommands.cpp) for details.


### Maya LT
Currently, only Windows is supported, and the tool is confirmed to work on Maya LT 2019 + Windows. Maya LT does not natively support outside plugins, so be aware that this may lead to problems. Even small version changes to Maya LT may lead to loss of compatibility.   
This is a separate package, but the process for installation and use is the same as [Non-LT Maya](#maya).


### 3ds Max
Confirmed functionality with 3ds Max 2016, 2017, 2018, 2019, 2020 + Windows.
- Installation: 
  - Donload UnityMeshSync_3dsMax_Windows.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases)
  - Copy MeshSyncClient3dsMax.dlu into the directory for plugin paths.
    - Plugin paths can be added in Max by going to Add under Customize -> Configure User and System Paths -> 3rd Party Plug-Ins
    - The default path (C:\Program Files\Autodesk\3ds Max 2019\Plugins) should be fine, but using a separate path is recommended
- After installing, "UnityMeshSync" will be added to the main menu bar, and the settings window can be opened by clicking "Window". 
  - If you change the menu bar, the "UnityMeshSync" category will be added under Action, where MeshSync features can also be accessed
- While "Auto Sync" is checked, changes to the Mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to sync changes manually.  
- Clicking Sync under Animations will cause the timer to advance from the first frame to the final frame while baking the animation before sending it to Unity.

&nbsp;  

- Polygon mesh, camera, and light sync are supported. 
- Most modifiers are supported, but are unsupported in some cases. Use the following rules.  
  - When there is no Morph or Skin, all modifiers will be applied during sync. 
  - If there is a Morph or Skin, all modifiers before them will be applied during sync.  
    - If there are multiple Morphs / Skins, the one at the bottom will be chosen as the base.
  - Morphs and Skins will sync on the Unity side as Blendshapes / Skins.
    - Unity applies them in order of Blendshape -> Skin, so if the order is reversed in Max, unintentional results may occur.
  - If "Bake Deformers" is checked, the results of applying all deformers will be sent to Unity. This will keep the content of the Mesh mostly consistent between Max and Unity, but will also result in the loss of Skinning and Blendshape information.
- Checking "Double Sided" will make the Mesh double-sided in Unity.
- Be advised that the negative scale is only supported for certain elements.
  - If XYZ all have negative values, the Mesh will sync properly, however if only one axis has a negative value Unity will treat the Mesh as though every axis has a negative value.
- Commands have also been added to the Max script, so all features can be accessed via the Max script.See [the source code] (https://github.com/unity3d-jp/MeshSync/blob/master/.MeshSync/Plugin/MeshSyncClient3dsMax/msmaxEntryPoint.cpp) for details. 


<img align="right" src="https://user-images.githubusercontent.com/1488611/45682175-8a919100-bb7a-11e8-96a1-efe2e28146c3.png" height=200>

### Motion Builder
Confirmed functionality with Motion Builder 2015, 2016, 2017, 2018, 2019 + Windows, Linux (CentOS 7) 
- Installation:
  - Download UnityMeshSync_MotionBuilder_*.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases)
  - Copy MeshSyncClientMotionBuilder.dll to the directory registered as a plugin path 
    - Plugin paths can be added in Motion Builder under Settings -> Preferences -> SDK menu
- After installation an object called UnityMeshSync will be added to the Asset Browser under Templates -> Devices, so add it to the scene  
- The various settings and features can be accessed in the Navigator by selecting Devices -> UnityMeshSync 
- While "Auto Sync" is checked, any changes to the Mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to manually reflect changes  
- Clicking Sync under Animations causes the timer to advance from the first frame to the final frame while baking the animation before sending it to Unity.  

&nbsp;  

- Polygon mesh, camera, and lighting sync are supported.
- The Polygon mesh's skinning/bone and BlendShapes will be carried over to Unity unchanged. 
- Checking "Double Sided" causes the Mesh to become double-sided in Unity. 
- Be advised that the negative scale is only supported for certain elements.
  - If XYZ all have negative values, the Mesh will sync properly, however if only one axis has a negative value Unity will treat the Mesh as though every axis has a negative value.
- Non-polygon shape data such as NURBS is not supported. 


<img align="right" src="https://user-images.githubusercontent.com/1488611/49272332-79d39480-f4b4-11e8-8ca3-0ce0bc90a965.png" height=400>

### Blender
Functionality confirmed with Blender 2.79(a,b), 2.80 beta (2019-4-23) + Windows, Mac, Linux (CentOS 7). Be aware that depending on the implementation, **there is a high possibility that upgrading the Blender version will lead to a loss of compatibility**. Be especially careful when upgrading to the popular 2.8 versions. A supported version will be released when issues become known.
- Installation: 
  - Download UnityMeshSync_Blender_*.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases) 
    - When you decompress the file, there will be another zip file in the UnityMeshSync_Blender_* directory, but this can be left alone 
  - In Blender, go to File -> User Preferences -> Add-ons (2.80 and after: Edit -> User Preferences), click "Install Add-on from file" at the bottom of the screen, and select the plugin zip file. 
  - **If an older version is already installed, it must be deleted beforehand**. Select "Import-Export: Unity Mesh Sync" from the Add-ons menu, **restart Blender after removing the older version** then follow the above steps.
- "Import-Export: Unity Mesh Sync" will be added to the menu, so select it to enable it.
- The MeshSync will also be added, where settings and manual sync can be accessed. 
  - The panel's location can be difficult to find in 2.8 versions. Use the screenshot at the right for reference.
- When "Auto Sync" is selected, changes to the Mesh will automatically be reflected in Unity. If Auyo Sync is disabled, use the "Manual Sync" button to sync changes. 
- Pressing the Animations Sync button will cause the timer to advance from the first frame to the final frame while baking the animation, then send it to Unity. 

&nbsp;  

- Polygon mesh, camera, and lighting sync are supported.
- The polygon mesh's skinning/bone (Armature) and Blendshape will be sent to Unity unchanged. Mirror deformers are also supported. Other deformers will be ignored. 
  - Check "Bake Modifiers" to sync the results of applying all modifiers. This will make the Mesh content mostly consistent between  Blender and Unity, but will also result in the loss of Skinning and Blendshape information.  
- Use "Convert To Mesh" to convert objects such as Nurbs into polygons, if they are able to, then sync. 
- Check the "Double Sided" option to make the Mesh double-sided in Unity.
- Be advised that the negative scale is only supported for certain elements.
  - If XYZ all have negative values, the Mesh will sync properly, however if only one axis has a negative value Unity will treat the Mesh as though every axis has a negative value.


### Modo

  <img src="https://user-images.githubusercontent.com/1488611/55697991-d9135980-59fe-11e9-8e9f-8fcfba1b234f.png" height=300><img src="https://user-images.githubusercontent.com/1488611/55697990-d9135980-59fe-11e9-9312-29c95e20e5b0.png" height=300>

  Functionality confirmed with Modo 10, 12, 13 + Windows, Mac, Linux (CentOS 7).
  - Installation:
    - Download UnityMeshSync_Modo_*.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases)
    - Designate MeshSyncClientModo.fx in Modo under System -> Add Plug-in
  - After installing, View will be added to the menu (Application -> Custom View -> UnityMeshSync), where varous options and settings can be accessed 
  - While "Auto Sync" is checked, changes made to the mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to sync changes
  - Clicking Sync under Animations will cause the timer to advance from the first frame to the final frame while baking the animation and sending it to Unity. 

  &nbsp;

  - Polygon mesh, camera, and light sync are supported. Portions of Mesh Instance and Replicator are also supported.
  - Polygon mesh Skinning/Joints and Morph will carry over to Unity, but be aware of how deformers are handled.
    - MeshSync can only handle Joint + Weight Map skinning, or Morph deformers. Any other deformers will be ignored.
    - "Bake Deformers" をチェックすると、デフォーマを全て適用した結果を送ります。複雑なデフォーマ構成であっても Unity 側の Mesh の内容がほぼ一致するようになりますが、代償としてスキニングや Morph/Blendshape の情報が失われます。
    - Mesh Instance や Replicator のスキニングは正しく Unity 側に反映できません。"Bake Deformers" を使う必要があります。
  - "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
  - 負のスケールは部分的にしかサポートしていないので注意が必要です
    - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます
  - コマンドからも MeshSync の機能にアクセスできます。unity.meshsync.settings で設定の変更、unity.meshsync.export でエクスポートできます

  &nbsp;

Modo は 13 以降 [Mood Bridge for Unity](https://learn.foundry.com/modo/content/help/pages/appendices/modo_bridge.html) という機能が搭載されており、Unity に直接 Mesh や Material を送ることができるようになっています。MeshSync と機能的に近い部分もありますが、以下のような違いがあります。(2019/04 現在)
  - Mood Bridge は Modo <-> Unity の双方向の同期をサポートします。MeshSync は Modo -> Unity の一方向のみをサポートします。
  - MeshSync は Replicator、Mesh の Skinning/Morph、アニメーションを同期できます。Mood Bridge は現状これらはサポートしていません。
  - MeshSync は できるだけ FBX 経由で Unity にデータを持っていった時と近い結果になるように努めています。一方、Modo Bridge では座標系が異なる (Z 方向が反転する)、Mesh のインデックスが展開されている (1000 triangles のモデルは 3000 頂点になっている) などの顕著な違いが出ます。


### メタセコイア
Windows 版 3 系と 4 系 (32bit & 64bit)、Mac 版 (4 系のみ) に対応しています。3 系はたぶん全てのバージョンに対応していますが、4 系は 4.6.4 以上である必要があります。(このバージョン以上でないとボーンの出力がサポートできないため)
また、4.7 系以降用は dll が別になっています。これは 4.7 でボーンの仕様が変わり、プラグインの互換性が失われたためです。4.7 ではモーフの出力にも対応しています。
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_Metasequoia*.zip をダウンロードして展開
  - メタセコイア側で Help -> About Plug-ins を開き、ダイアログ左下の "Install" からプラグインファイルを指定します。ちなみにプラグインのタイプは Station です。
  - **古いバージョンをインストール済みの場合、事前に手動で削除しておく必要があります**。メタセコイアを起動していない状態で該当ファイルを削除してください。
- インストール後 パネル -> Unity Mesh Sync が追加されるのでこれを開き、"Auto Sync" をチェックします。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- "Sync Camera" をチェックすると、VRED 側のカメラを同期します。"Camera Path" が Unity 側のカメラのパスになります。
- "Import Unity Scene" を押すと現在 Unity で開かれているシーンをインポートすることができます。インポートしたシーンの編集もリアルタイムに反映可能です。

&nbsp;

- ミラーリング、スムーシングは Unity にも反映されます。
  - ただし、ミラーリングの "左右を接続した鏡面" は非サポートです。
- メタセコイアで非表示のオブジェクトは Unity でも非表示になります。非表示のオブジェクトはメッシュの内容は送られないので、シーン内にオブジェクトが増えて同期が重くなってきた場合適切に非表示にすることで同期も速くなるはずです。
- マテリアルは Unity には反映されませんが、マテリアル ID に応じて適切にサブメッシュに分割されます。
- サブディビジョンやメタボールはフリーズするまで Unity には反映されません。
- メタセコイア 4 系でサポートされた法線の編集は "Sync Normals" にチェックを入れることで反映できます。
- メタセコイア 4 系でサポートされたボーンは "Sync Bones" にチェックを入れることで反映できます。 "Sync Poses" にチェックを入れると "スキニング" で設定したポーズも反映します。


### VRED
VRED は API hook を用いた特殊な実装になっています。そのため、使い方が特殊であったり、VRED のマイナーバージョンアップでも動作しなくなる可能性があることにご注意ください。
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_VRED_Windows.zip をダウンロードして展開
  - VRED のインストール先がデフォルトであれば、VREDPro_2019.bat などの bat ファイルをダブルクリックで MeshSync が付与された状態で VRED が起動します。
  - VRED のインストール先がデフォルトでない場合、bat ファイルをテキストエディタで開いて "%PROGRAMFILES%\Autodesk\VREDPro-11.0\Bin\WIN64\VREDPro.exe" などの部分をインストール先の VRED の exe ファイルに書き換えてください。(例: "D:\My Install Directory\VREDPro-11.0\Bin\WIN64\VREDPro.exe" など)
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- 部分的ながらテクスチャの同期にも対応しています。シェーダ内のパラメータ "colorTexture" "bumpTexture" に設定されているテクスチャをそれぞれカラーテクスチャ、ノーマルマップ用テクスチャとして同期します。
- "Flip U/V" をチェックするとテクスチャ座標の U/V を反転します。マッピング方法が UV ではない場合でもこれを用いることでつじつまを合わせられるかもしれません。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- "Sync Deleted" をチェックすると、VRED 側で消えたモデルが Unity 側でも消えるようになります。
- "Sync Camera" をチェックすると、VRED 側のカメラを同期します。"Camera Path" が Unity 側のカメラのパスになります。

&nbsp;  
  
- VRED のビューポートに表示されているモデルをそのまま送っているため、モデルは大体同期できるはずです。
  - ただし、VRED はカリングを行うため、カメラの視界外のオブジェクトは同期できないことに注意が必要です。
- 実装上の制限により、オブジェクトの名前やツリー構造は同期できません。Unity 側では全てルートオブジェクトとなり、名前は機械的に変換されたものになります。

### xismo
xismo はプラグインの仕組みを提供していないため (2018/05 現在)、API hook を用いた実装になっています。そのため、使い方が特殊であったり、 xismo のバージョンアップで動作しなくなる可能性が高いことにご注意ください。現行版は xismo 191～199b で動作を確認済しています。
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_xismo_Windows.zip をダウンロード
  - 展開して出てくる 2 つのファイル (MeshSyncClientXismo.exe, MeshSyncClientXismoHook.dll) を xismo がインストールされているディレクトリ (xismo.exe と同じディレクトリ) に置きます
- MeshSyncClientXismo.exe を起動します。これにより MeshSync が付与された状態で xismo が起動します。
- ウィンドウ -> Unity Mesh Sync メニューが追加されており、これで各種設定などを行います。
- "Auto Sync" をチェックすると編集が自動的に Unity 側に反映されるようになります。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- "Sync Camera" をチェックすると、VRED 側のカメラを同期します。"Camera Path" が Unity 側のカメラのパスになります。

&nbsp;  

- xismo のビューポートに表示されているモデルをそのまま送っているため、モデルは大体同期できるはずです。
- モデル以外 (オブジェクト/マテリアルの名前、ボーンなど) は未対応です。これらは xismo 側がプラグイン API を用意しない限り対応が困難であり、現状対応予定はありません。


### Unity
- Unity 5.6 系以上 + Windows (64 bit), Mac, Linux (CentOS 7) で動作を確認しています
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から MeshSync.unitypackage をダウンロードし、プロジェクトにインポートします。
    - Unity 2018.3 以降の場合、このリポジトリを直接インポートすることもできます。プロジェクト内にある Packages/manifest.json をテキストエディタで開き、"dependencies" に以下の行を加えます。
    > "com.utj.meshsync": "https://github.com/unity3d-jp/MeshSync.git",
  
  - 古いバージョンをインストール済みの場合、**パッケージインポート前に一度 Unity を終了し、Assets/UTJ/MeshSync を削除** しておくと確実です。プラグイン dll がロードされていると更新に失敗するためです。
  - インポート後は GameObject -> MeshSync メニューが追加されているはずです
- GameObject -> MeshSync -> Create Server でサーバーオブジェクトを作成します。このサーバーオブジェクトが同期処理を担当します

<img align="right" src="https://user-images.githubusercontent.com/1488611/49274442-c40c4400-f4bb-11e8-99d6-3257cdbe7320.png" height=400>

- Root Object
  - 同期により生成されるオブジェクト群のルートとなるオブジェクトを指定します。未設定の場合、ルートにオブジェクトが生成されていきます。
- Sync Transform など
  - コンポーネント別の同期の有効/無効指定です。Play モードで物理シミュレーションの挙動を確認したい場合などに Transform の同期が邪魔になるので用意されたオプションです。
- Update Mesh Collider
  - これが有効の場合、オブジェクトが MeshCollider を持っていたら Mesh を更新する時に MeshCollider の内容も更新します。
- Track Material Assignment
  - これが有効の場合、SceneView でマテリアルを D&D でアサインした場合などに、その変更を検知して同じマテリアルを持つ他の Mesh に対しても同様にマテリアルを更新します。
- Animation Interpolation
  - アニメーションの補完方法を指定します。多くの場合はデフォルトのスムース補間で問題ないはずですが、映像作品などの場合、DCC 側でアニメーションのサンプル数をターゲットフレームレートに合わせ、補完を無効 (Constant) にした方が望ましい結果になるかもしれません。
- Progressive Display
  - これが有効な場合、受信途中のシーンの更新をリアルタイムで反映していきます。無効な場合はシーン全体のデータの受信が完了するまで待ってから更新を反映します。
  
  &nbsp;

- マテリアルリスト
  - MeshSyncServer はマテリアルのリストを保持しています。このリストにマテリアルを設定すると、対応するオブジェクトに適切にアサインされます。
- アセット化
  - DCC ツール側の編集によって生成された Mesh 郡は、そのままではそのシーン内にしか存在できないオブジェクトです。他のシーンやプロジェクトへ持ち出せるようにするにはアセットファイルとして保存する必要があります。
  MeshSyncServer の "Export Mesh" ボタンを押すとそのアセット化が行われます。("Asset Export Path" で指定されたディレクトリにファイルが生成されます)  


## Tips や注意事項など
- 同期は TCP/IP を介して行われるため、Unity と DCC ツールが別のマシンで動いていても同期させることができます。その場合、クライアントである DCC ツール側は設定項目の Server / Port に Unity 側のマシンを指定してください。


- Unity 上に MeshSyncServer オブジェクトがあるときにサーバーのアドレス:ポートをブラウザで開くと、サーバー側の Unity の GameView をブラウザで見ることができます。 (デフォルトでは [127.0.0.1:8080](http://127.0.0.1:8080))  
  このブラウザの画面のメッセージフォームからメッセージを送ると、Unity の Console にそのメッセージが出るようになっています。Unity 側作業者と DCC 側作業者が別の場合役に立つこともあるかもしれません。


- ポーズ/アニメーションのみを編集中の場合、"Sync Meshes" を切っておくことをおすすめします。メッシュデータを送らなくなるので動作が軽快になるでしょう。
  - プラグイン側でボーンのみの編集の場合メッシュは送らないようにすべきなのですが、判定がやや難しく、現状そうなっていません。


- Unity 2019.1 より前のバージョンでは、頂点あたりの最大影響ボーン数が 4 であることに注意が必要です。
  これが原因でボーンが多いと DCC 側と Unity 側で結果が一致しなくなることがあります。
    - Unity 2019.1 で最大 255 ボーンまで影響できるようになりました (Project Settings -> Quality -> Other の Skin Weights を "Unlimited" に)。
    これにより、フェイシャルなどの多数のボーンを必要とするアニメーションも問題なく持って来れるはずです。


- 本プラグインはその性質上エディタでのみの使用を想定していますが、一応ランタイムでも動作するようにしてあります。**意図せず最終ビルドに残さないようご注意ください**。  
  ランタイムではアニメーションの同期は機能しませんが、モデルの同期は一通り動作します。


##  関連
- [NormalPainter](https://github.com/unity3d-jp/NormalPainter): Unity 上で法線を編集できるようにするツール
- [BlendShapeBuilder](https://github.com/unity3d-jp/BlendShapeBuilder): Unity 上で BlendShape を構築できるようにするツール

## ライセンス
[MIT](LICENSE.txt), ただし Blender プラグインは [GPL3](Plugin/MeshSyncClientBlender/LICENSE.txt) (Blender のソースの一部を使っているため)
