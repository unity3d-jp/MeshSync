![demo](https://user-images.githubusercontent.com/1488611/39971828-98afa1d8-573d-11e8-9a6f-86263bee8949.gif)
# MeshSync
[English](https://translate.google.com/translate?sl=ja&tl=en&u=https://github.com/unity3d-jp/MeshSync)

DCC ツール上のモデルの編集をリアルタイムに Unity に反映させるツールです。ゲーム上でどう見えるかをその場で確認しながらモデリングすることを可能にします。  
Unity と DCC ツール両方のプラグインとして機能し、現在 [Maya](https://www.autodesk.eu/products/maya/overview), [Maya LT](https://www.autodesk.eu/products/maya-lt/overview), [3ds Max](https://www.autodesk.com/products/3ds-max/overview), [Blender](https://blenderartists.org/), [メタセコイア](http://www.metaseq.net/), [xismo](http://mqdl.jpn.org/) をサポートしています。


## 使い方
1. [Maya](#maya)
2. [Maya LT](#maya-lt)
3. [3ds Max](#3ds-max)
4. [Motion Builder](#motion-builder)
5. [Blender](#blender)
6. [Modo](#modo)
7. [メタセコイア](#メタセコイア)
8. [Unity](#unity)

<img align="right" src="https://user-images.githubusercontent.com/1488611/39971860-7f6d1330-573e-11e8-9a1e-9d95709cbd50.png" height=400>

### Maya
Maya 2016, 2016.5, 2017, 2018, 2019 + Windows, Mac, Linux (CentOS 7) で動作を確認しています。
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_Maya_*.zip をダウンロード。
  - Windows: %MAYA_APP_DIR% が設定されている場合はそこに、ない場合は %USERPROFILE%\Documents\maya (←を Explorer のアドレスバーへコピペで直行) に modules ディレクトリをそのままコピー。
    - 2016 以前の場合はバージョン名のディレクトリへコピーします。(%MAYA_APP_DIR%\2016 など)
  - Mac: /Users/Shared/Autodesk/modules/maya に UnityMeshSync ディレクトリと .mod ファイルをコピー。
  - Linux: ~/maya/(Maya のバージョン) に modules ディレクトリをそのままコピー。
- Maya を起動し、Windows -> Settings/Preferences -> Plug-in Manager を開き、MeshSyncClient の Loaded にチェックを入れてプラグインを有効化します。
- UnityMeshSync シェルフが追加されているので、それの歯車アイコンで設定メニューを開きます。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。
- "Export Cache" で全フレームのデータをファイルにエクスポートできます。エクスポートしたファイルは Unity で再生できます。より詳しくは [Scene Cache](Documentation~/SceneCache.md) を参照ください。

&nbsp;  

- 歯車アイコン以外のボタンはそれぞれ手動同期、アニメーション同期相当のボタンになっています。
- ポリゴンメッシュ、カメラ、ライトの同期に対応しています。
- ポリゴンメッシュはスキニング/ボーン (SkinCluster) と BlendShape もそのまま Unity へ持ってこれるようになっています。
  - これら以外のデフォーマも適用を試みますが、前後に SkinCluster があった場合などに正しく適用されない可能性があります。
  - "Bake Deformers" をチェックすると、デフォーマを全て適用した結果を同期します。Maya 側と Unity 側で Mesh の内容がほぼ一致するようになりますが、代償として Skinning や Blendshape の情報が失われます。
  - "Bake Transform" をチェックすると、位置/回転/スケールを Mesh の頂点に適用し、Unity 側の Transform は初期値になります。pivot が絡む複雑な Transform は Unity では再現できないことがありますが、そのような場合でもこのオプションを使うと Mesh の見た目は一致するようになります。このオプションは "Bake Deformers" が有効なときのみ有効です。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- 負のスケールは部分的にしかサポートしていないので注意が必要です。
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます。"Bake Transform" オプションを使うとそのようなケースでも Mesh は一致するようになりますが、デフォーマの情報が失われます。
- NURBS などポリゴン以外の形状データは対応していません。
- インスタンシングは対応していますが、スキニングされたメッシュのインスタンスは現在未対応です (Unity 側では全て元インスタンスと同じ位置になっていまいます)。
- MEL にもコマンドが登録されており、全ての機能に MEL 経由でアクセスできるようになっています。こちらの詳細は[ソースコードを参照ください](https://github.com/unity3d-jp/MeshSync/blob/master/Plugin~/MeshSyncClientMaya/msmayaCommands.cpp)。


### Maya LT
現在 Windows のみ対応で、Maya LT 2019 + Windows で動作を確認しています。Maya LT は本来外部プラグインをサポートしないため、問題が起きる可能性が高いことに留意ください。Maya LT 側のマイナーバージョンアップでも互換性が失われる可能性が考えられます。  
パッケージは別になっているものの、インストールや使い方は [非 LT の Maya](#maya) と同じです。


### 3ds Max
3ds Max 2016, 2017, 2018, 2019, 2020 + Windows で動作を確認しています。
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_3dsMax_Windows.zip をダウンロード
  - MeshSyncClient3dsMax.dlu をプラグイン用パスとして登録されているディレクトリにコピー
    - プラグイン用パスは max 内の Customize -> Configure User and System Paths -> 3rd Party Plug-Ins の Add で追加できます
    - デフォルトで用意されているパス (C:\Program Files\Autodesk\3ds Max 2019\Plugins など) でもおそらく機能しますが、デフォルトとそれ以外で別のパスを用意しておくことをおすすめします
- インストール後、メインメニューバーに "UnityMeshSync" が追加されているので、それの "Window" から設定ウィンドウを開けます。
  - メニューバーを編集する場合、Action に "UnityMeshSync" カテゴリが追加されているので、そちらから MeshSync の機能にアクセスできます
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。
- "Export Cache" で全フレームのデータをファイルにエクスポートできます。エクスポートしたファイルは Unity で再生できます。より詳しくは [Scene Cache](Documentation~/SceneCache.md) を参照ください。

&nbsp;  

- ポリゴンメッシュ、カメラ、ライトの同期に対応しています。
- モディファイアは大体対応していますが、対応できないケースもあります。以下のルールに従います。
  - Morph も Skin もない場合、全てのモディファイアを適用した状態で同期します。
  - Morph か Skin がある場合、その一つ前までのモディファイアを適用した状態で同期します。
    - Morph / Skin が複数ある場合、一番下のものが基準として選ばれます。
  - Morh と Skin は Unity 側にそのまま Blendshape / Skin として同期します。
    - Unity 側では常に Blendshape -> Skin の順番で適用されるため、Max 側で順番が逆だと意図しない結果になる可能性があります。
  - "Bake Modifiers" をチェックすると、モディファイアを適用した結果を送ります。Max 側と Unity 側で Mesh の内容がほぼ一致するようになりますが、代償として Skinning や Blendshape の情報が失われます。
  - "Bake Transform" をチェックすると、位置/回転/スケールを Mesh の頂点に適用し、Unity 側の Transform は初期値になります。pivot が絡む複雑な Transform は Unity では再現できないことがありますが、そのような場合でもこのオプションを使うと Mesh の見た目は一致するようになります。このオプションは "Bake Modifiers" が有効なときのみ有効です。
  - "Use Render Meshes" をチェックすると、レンダリング用の Mesh からデータを抽出します。例えば Turbo Smooth は viewport 用とレンダリング用で別の Iteration を指定できますが、レンダリング用の設定が Unity 側に反映されるようになります。また、Fluid などのレンダリング時にしか現れない Mesh や、Space Warps なども正しく反映されるようになります。
- "Ignore Non-Rebderable" をチェックすると、renderable ではない Mesh を無視します。例えばボーンの viewport の表示の四角錐のような形状などが renderable ではない Mesh に該当します。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- 負のスケールは部分的にしかサポートしていないので注意が必要です。
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます。"Bake Transform" オプションを使うとそのようなケースでも Mesh は一致するようになりますが、モディファイアの情報が失われます。
- Max script にもコマンドが追加されており、全ての機能に Max script 経由でアクセスできるようになっています。こちらの詳細は[ソースコードを参照ください](https://github.com/unity3d-jp/MeshSync/blob/master/Plugin~/MeshSyncClient3dsMax/msmaxEntryPoint.cpp)


<img align="right" src="https://user-images.githubusercontent.com/1488611/45682175-8a919100-bb7a-11e8-96a1-efe2e28146c3.png" height=200>

### Motion Builder
Motion Builder 2016, 2017, 2018, 2019 + Windows, Linux (CentOS 7) で動作を確認しています
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_MotionBuilder_*.zip をダウンロード
  - MeshSyncClientMotionBuilder.dll をプラグイン用パスとして登録されているディレクトリにコピー
    - プラグイン用パスは Motion Builder 内の Settings -> Preferences -> SDK メニューから追加できます
- インストール後、Asset Browser 内の Templates -> Devices に UnityMeshSync というオブジェクトが追加されているので、それをシーンに追加します
- Navigator 内の Devices -> UnityMeshSync を選択することで各種設定や機能にアクセスできます 
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。

&nbsp;  

- ポリゴンメッシュ、カメラ、ライトの同期に対応しています。
- ポリゴンメッシュはスキニング/ボーンと BlendShape もそのまま Unity へ持ってこれるようになっています。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- 負のスケールは部分的にしかサポートしていないので注意が必要です
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます
- NURBS などポリゴン以外の形状データは対応していません


<img align="right" src="https://user-images.githubusercontent.com/1488611/49272332-79d39480-f4b4-11e8-8ca3-0ce0bc90a965.png" height=400>

### Blender
Blender 2.79b, 2.80 + Windows, Mac, Linux (CentOS 7) で動作を確認しています。[開発版の Blender](https://builder.blender.org/download/) はサポート外で、ほぼ動作しません。  
実装の都合上、**Blender のバージョンが上がると互換性が失われる可能性が高い** ことにご注意ください。[Blender version issue](https://github.com/unity3d-jp/MeshSync/issues/89) で最新の状況や hot fix を提供予定です。
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_Blender_*.zip をダウンロードして展開
    - 展開して出てくる UnityMeshSync_Blender_* ディレクトリの中にも zip が入っていますが、これらはそのままで大丈夫です
  - Blender 側で File -> User Preferences -> Add-ons (2.80 以降では Edit -> User Preferences) を開き、画面下部の "Install Add-on from file" を押し、プラグインの zip ファイルを指定します。
  - **古いバージョンをインストール済みの場合、事前に削除しておく必要があります**。Add-ons メニューから "Import-Export: Unity Mesh Sync" を選択して  **Remove した後、blender を再起動** してから上記手順を踏んでください。
- "Import-Export: Unity Mesh Sync" が追加されるので、チェックを入れて有効化します。
- MeshSync パネルが追加されるので、そちらから設定や手動の同期を行います。
  - 2.8 系ではパネルの場所がややわかりにくい場所になっています。右スクリーンショットを参照ください。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。
- "Export Cache" で全フレームのデータをファイルにエクスポートできます。エクスポートしたファイルは Unity で再生できます。より詳しくは [Scene Cache](Documentation~/SceneCache.md) を参照ください。

&nbsp;  

- ポリゴンメッシュ、カメラ、ライトの同期に対応しています。
- ポリゴンメッシュはスキニング/ボーン (Armature) と BlendShape もそのまま Unity へ持ってこれるようになっています。Mirror デフォーマも対応しています。これら以外のモディファイアは無視されます。
  - "Bake Modifiers" をチェックすると、モディファイアを全て適用した結果を同期します。Blender 側と Unity 側で Mesh の内容がほぼ一致するようになりますが、代償として Skinning や Blendshape の情報が失われます。
  - "Bake Transform" をチェックすると、位置/回転/スケールを Mesh の頂点に適用し、Unity 側の Transform は初期値になります。pivot が絡む複雑な Transform は Unity では再現できないことがありますが、そのような場合でもこのオプションを使うと Mesh の見た目は一致するようになります。このオプションは "Bake Modifiers" が有効なときのみ有効です。
- "Curves as Mesh" をチェックすると、Curve や Text などポリゴンに変換可能なオブジェクトを変換して同期します。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- 負のスケールは部分的にしかサポートしていないので注意が必要です
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます。"Bake Transform" オプションを使うとそのようなケースでも Mesh は一致するようになりますが、モディファイアの情報が失われます。


### Modo

<img src="https://user-images.githubusercontent.com/1488611/55697991-d9135980-59fe-11e9-8e9f-8fcfba1b234f.png" height=300><img src="https://user-images.githubusercontent.com/1488611/55697990-d9135980-59fe-11e9-9312-29c95e20e5b0.png" height=300>

Modo 12, 13 + Windows, Mac, Linux (CentOS 7) で動作を確認しています。
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_Modo_*.zip をダウンロードして展開
  - Modo 内の System -> Add Plug-in で MeshSyncClientModo.fx を指定
  - **古いバージョンをインストール済みの場合、古いプラグインをロードしていない状態で再度上記手順を踏む必要があります**。プラグインがロードされるタイミングは主に下記 view (Application -> Custom View -> UnityMeshSync) を表示したタイミングなので、view を出していない状態で一度 modo を終了、再度起動、プラグインをインストール、とすると確実です。
- インストール後は新たな View が追加されており、ここから各種設定や機能にアクセスできます (Application -> Custom View -> UnityMeshSync)
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。

&nbsp;

- ポリゴンメッシュ、カメラ、ライトの同期に対応しています。Mesh Instance や Replicator も部分的にサポートしています。
- ポリゴンメッシュはスキニング / Joint と Morph も Unity へ持ってこれるようになっていますが、デフォーマの扱いには注意が必要です。
  - MeshSync が解釈できるデフォーマは Joint + Weight Map 方式のスキニング、および Morph のみです。それ以外のデフォーマは無視されます。
  - "Bake Deformers" をチェックすると、デフォーマを全て適用した結果を送ります。複雑なデフォーマ構成であっても Unity 側の Mesh の内容がほぼ一致するようになりますが、代償としてスキニングや Morph/Blendshape の情報が失われます。
  - "Bake Transform" をチェックすると、位置/回転/スケールを Mesh の頂点に適用し、Unity 側の Transform は初期値になります。pivot が絡む複雑な Transform は Unity では再現できないことがありますが、そのような場合でもこのオプションを使うと Mesh の見た目は一致するようになります。このオプションは "Bake Deformers" が有効なときのみ有効です。
  - Mesh Instance や Replicator のスキニングは正しく Unity 側に反映できません。"Bake Deformers" を使う必要があります。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- 負のスケールは部分的にしかサポートしていないので注意が必要です
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます。"Bake Transform" オプションを使うとそのようなケースでも Mesh は一致するようになりますが、デフォーマの情報が失われます。
- コマンドからも MeshSync の機能にアクセスできます。unity.meshsync.settings で設定の変更、unity.meshsync.export でエクスポートできます
- "Export Cache" で全フレームのデータをファイルにエクスポートできます。エクスポートしたファイルは Unity で再生できます。より詳しくは [Scene Cache](Documentation~/SceneCache.md) を参照ください。

&nbsp;

Modo は 13 以降 [Modo Bridge for Unity](https://learn.foundry.com/modo/content/help/pages/appendices/modo_bridge.html) という機能が搭載されており、Unity に直接 Mesh や Material を送ることができるようになっています。MeshSync と機能的に近い部分もありますが、以下のような違いがあります。(2019/04 現在)
- Modo Bridge は Modo <-> Unity の双方向の同期をサポートします。MeshSync は Modo -> Unity の一方向のみをサポートします。
- MeshSync は Replicator、Mesh の Skinning/Morph、アニメーションを同期できます。Mood Bridge は現状これらはサポートしていません。
- MeshSync は できるだけ FBX 経由で Unity にデータを持っていった時と近い結果になるように努めています。一方、Modo Bridge では座標系が異なる (Z 方向が反転する)、Mesh のインデックスが展開されている (1000 triangles のモデルは 3000 頂点になっている) などの顕著な違いが出ます。


### メタセコイア
Windows 版 3 系と 4 系 (32bit & 64bit)、Mac 版 (4 系のみ) に対応しています。3 系はたぶん全てのバージョンに対応していますが、4 系は 4.6.4 以上である必要があります。(このバージョン以上でないとボーンの出力がサポートできないため)
また、4.7 系以降用は dll が別になっています。これは 4.7 でボーンの仕様が変わり、プラグインの互換性が失われたためです。4.7 ではモーフの出力にも対応しています。
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_Metasequoia*.zip をダウンロードして展開
  - メタセコイア側で Help -> About Plug-ins を開き、ダイアログ左下の "Install" からプラグインファイルを指定します。ちなみにプラグインのタイプは Station です。
  - **古いバージョンをインストール済みの場合、事前に手動で削除しておく必要があります**。メタセコイアを起動していない状態で該当ファイルを削除、または直接新しい dll で置き換えてください。
- インストール後 パネル -> Unity Mesh Sync が追加されるのでこれを開き、"Auto Sync" をチェックします。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- "Sync Camera" をチェックすると、パースペクティブビューのカメラを同期します。"Camera Path" が Unity 側のカメラのパスになります。
- "Import Unity Scene" を押すと現在 Unity で開かれているシーンをインポートすることができます。インポートしたシーンの編集もリアルタイムに反映可能です。

&nbsp;

- ミラーリング、スムーシングは Unity にも反映されます。
  - ただし、ミラーリングの "左右を接続した鏡面" は非サポートです。
- メタセコイアで非表示のオブジェクトは Unity でも非表示になります。非表示のオブジェクトはメッシュの内容は送られないので、シーン内にオブジェクトが増えて同期が重くなってきた場合適切に非表示にすることで同期も速くなるはずです。
- マテリアルは Unity には反映されませんが、マテリアル ID に応じて適切にサブメッシュに分割されます。
- サブディビジョンやメタボールはフリーズするまで Unity には反映されません。
- メタセコイア 4 系でサポートされた法線の編集は "Sync Normals" にチェックを入れることで反映できます。
- メタセコイア 4 系でサポートされたボーンは "Sync Bones" にチェックを入れることで反映できます。 "Sync Poses" にチェックを入れると "スキニング" で設定したポーズも反映します。


### Unity
- Unity 2017.4 系以上 + Windows (64 bit), Mac, Linux (CentOS 7) で動作を確認しています
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から MeshSync.unitypackage をダウンロードし、プロジェクトにインポートします。
    - Unity 2018.3 以降の場合、このリポジトリを直接インポートすることもできます。プロジェクト内にある Packages/manifest.json をテキストエディタで開き、"dependencies" に以下の行を加えます。
    > "com.unity.meshsync": "https://github.com/unity3d-jp/MeshSync.git",

  - 古いバージョンをインストール済みの場合、**パッケージインポート前に一度 Unity を終了し、Assets/UTJ/MeshSync を削除** しておくと確実です。プラグイン dll がロードされていると更新に失敗するためです。
  - インポート後は GameObject -> MeshSync メニューが追加されているはずです
- GameObject -> MeshSync -> Create Server でサーバーオブジェクトを作成します。このサーバーオブジェクトが同期処理を担当します
- GameObject -> MeshSync -> Create Cache Player で事前にエクスポートしておいたキャッシュを再生するプレイヤーを作成します。現在 Maya, 3ds Max, Modo, Blender 用のプラグインがキャッシュのエクスポートをサポートしています。

<img align="right" src="https://user-images.githubusercontent.com/1488611/64145334-5b3f6700-ce53-11e9-8bc1-93f48b865e0f.png" height=500>

- Root Object
  - 同期により生成されるオブジェクト群のルートとなるオブジェクトを指定します。未設定の場合、ルートにオブジェクトが生成されていきます。
- Sync Transform など
  - コンポーネント別の同期の有効/無効指定です。Play モードで物理シミュレーションの挙動を確認したい場合などに Transform の同期が邪魔になるので用意されたオプションです。
  - "Update Mesh Collider" が有効の場合、オブジェクトが MeshCollider を持っていたら Mesh を更新する時に MeshCollider の内容も更新します。
- Animation Interpolation
  - アニメーションの補完方法を指定します。多くの場合デフォルトのスムース補間で問題ないと思われますが、フレーム単位で一致させたい場合などは補間が無効 (Constant) の方が望ましいと思われます。
- Keyframe Reduction
  - 有効な場合、アニメーションをインポートする際にキーフレームリダクションを行います。"Threshold" は誤差の許容量で、大きいほどキーの数は減りますが、元のアニメーションとの誤差が大きくなります。"Erase Flat Curves" が有効な場合、最初から最後まで変化がないカーブを削除します。
- Z-Up Correction
  - 座標系が Z-Up である 3ds max と Blender のみ関係がある設定で、Z-Up を Y-Up に変換する方法を指定します。
  "Flip YZ" は全 Transform と Mesh の頂点を Y-up に直します。"Rotate X" はルートとなるオブジェクトの Transform に -90 の X 回転をかけることで Y-up に直すもので、Mesh は Z-up のままになります。
  多くの場合 "Flip YZ" の方が望ましいと思われますが、Unity 標準の fbx Importer が "Rotate X" 相当の処理を行っているため、選べるようにしてあります。

&nbsp;

- Material List
  - MeshSyncServer や SceneCachePlayer はマテリアルのリストを保持しています。このリストのマテリアルを変更すると、対応するオブジェクトにも変更が反映されます。
  - "Sync Material List" が有効な場合、オブジェクトのマテリアルを変更した際にそれをマテリアルリストにも反映し、同じマテリアルを持つ他のオブジェクトにも変更を伝播します。
  - "Import List" "Export List" でリストの保存と読み込みができます。キャッシュファイルを更新する場合、これを用いることでマテリアルを引き継ぐことができます。

- Animation Tweak
基本的なアニメーションの調整がここで可能です。
  - "Override Frame Rate" でフレームレートを変更できます。Unity 標準の "Set Sample Rate" と異なり、変更するのは純粋にフレームレートだけで、キーの時間やアニメーションの長さは変えません。補間がない 24 FPS のアニメーションを 60 FPS の環境で再生するとガタガタになってしまうので、そのような場合に無理矢理アニメーションを 120 FPS などに変えて緩和する、というような用途を想定しています。
  - "Time Scale" で時間のスケーリングを行います。例えば 0.5 を適用すると倍速になります。"Offset" は指定秒分オフセットを加えます。例えば 5 秒のアニメーションに対して スケール -1、オフセット -5 を適用すると逆再生になります。
  - "Drop Keyframe" はフレームの間引きを行います。30 個のキーが打たれているアニメーションに Step=2 で適用すると奇数フレームを間引き、15 フレームのアニメーションになります。同様に Step=3 だと 10 フレームになります。

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
