![demo](https://user-images.githubusercontent.com/1488611/39971828-98afa1d8-573d-11e8-9a6f-86263bee8949.gif)
# MeshSync
[English](https://translate.google.com/translate?sl=ja&tl=en&u=https://github.com/unity3d-jp/MeshSync)

DCC ツール上のモデルの編集をリアルタイムに Unity に反映させるツールです。ゲーム上でどう見えるかをその場で確認しながらモデリングすることを可能にします。  
Unity と DCC ツール両方のプラグインとして機能し、現在 [Maya](https://www.autodesk.eu/products/maya/overview), [Maya LT](https://www.autodesk.eu/products/maya-lt/overview), [3ds Max](https://www.autodesk.com/products/3ds-max/overview), [Blender](https://blenderartists.org/), [メタセコイア](http://www.metaseq.net/), [xismo](http://mqdl.jpn.org/) をサポートしています。


## 使い方
1. [Maya](#maya)
2. [Maya LT](#maya-lt)
3. [3ds Max](#3ds-max)
3. [Motion Builder](#motion-builder)
3. [Modo](#modo)
4. [Blender](#blender)
5. [メタセコイア](#メタセコイア)
5. [VRED](#vred)
5. [xismo](#xismo)
6. [Unity](#unity)

<img align="right" src="https://user-images.githubusercontent.com/1488611/39971860-7f6d1330-573e-11e8-9a1e-9d95709cbd50.png" height=400>

### Maya
Maya 2015, 2016, 2016.5, 2017, 2018, 2019 + Windows, Mac, Linux (CentOS 7) で動作を確認しています。
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

&nbsp;  

- 歯車アイコン以外のボタンはそれぞれ手動同期、アニメーション同期相当のボタンになっています。
- ポリゴンメッシュ、カメラ、ライトの同期に対応しています。
- ポリゴンメッシュはスキニング/ボーン (SkinCluster) と BlendShape もそのまま Unity へ持ってこれるようになっています。
  - これら以外のデフォーマも適用を試みますが、前後に SkinCluster があった場合などに正しく適用されない可能性があります。
  - "Bake Deformers" をチェックすると、デフォーマを全て適用した結果を同期します。Maya 側と Unity 側で Mesh の内容がほぼ一致するようになりますが、代償として Skinning や Blendshape の情報が失われます。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- 負のスケールは部分的にしかサポートしていないので注意が必要です。
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます。
- NURBS などポリゴン以外の形状データは対応していません。
- インスタンシングは対応していますが、スキニングされたメッシュのインスタンスは現在未対応です (Unity 側では全て元インスタンスと同じ位置になっていまいます)。
- MEL にもコマンドが登録されており、全ての機能に MEL 経由でアクセスできるようになっています。こちらの詳細は[ソースコードを参照ください](https://github.com/unity3d-jp/MeshSync/blob/master/Plugin/MeshSyncClientMaya/msmayaCommands.cpp)。


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

&nbsp;  

- ポリゴンメッシュ、カメラ、ライトの同期に対応しています。
- モディファイアは大体対応していますが、対応できないケースもあります。以下のルールに従います。
  - Morph も Skin もない場合、全てのモディファイアを適用した状態で同期します。
  - Morph か Skin がある場合、その一つ前までのモディファイアを適用した状態で同期します。
    - Morph / Skin が複数ある場合、一番下のものが基準として選ばれます。
  - Morh と Skin は Unity 側にそのまま Blendshape / Skin として同期します。
    - Unity 側では常に Blendshape -> Skin の順番で適用されるため、Max 側で順番が逆だと意図しない結果になる可能性があります。
  - "Bake Deformers" をチェックすると、デフォーマを全て適用した結果を同期します。Max 側と Unity 側で Mesh の内容がほぼ一致するようになりますが、代償として Skinning や Blendshape の情報が失われます。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- 負のスケールは部分的にしかサポートしていないので注意が必要です。
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます。
- Max script にもコマンドが追加されており、全ての機能に Max script 経由でアクセスできるようになっています。こちらの詳細は[ソースコードを参照ください](https://github.com/unity3d-jp/MeshSync/blob/master/Plugin/MeshSyncClient3dsMax/msmaxEntryPoint.cpp)


<img align="right" src="https://user-images.githubusercontent.com/1488611/45682175-8a919100-bb7a-11e8-96a1-efe2e28146c3.png" height=200>

### Motion Builder
Motion Builder 2015, 2016, 2017, 2018, 2019 + Windows, Linux (CentOS 7) で動作を確認しています
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


### Modo
<img src="https://user-images.githubusercontent.com/1488611/55697991-d9135980-59fe-11e9-8e9f-8fcfba1b234f.png" height=300><img src="https://user-images.githubusercontent.com/1488611/55697990-d9135980-59fe-11e9-9312-29c95e20e5b0.png" height=300>

Modo 10, 12, 13 + Windows, Mac, Linux で動作を確認しています。
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_Modo_*.zip をダウンロードして展開
  - Modo 内の System -> Add Plug-in で MeshSyncClientModo.fx を指定
- インストール後は新たな View が追加されており、ここから各種設定や機能にアクセスできます (Application -> Custom View -> UnityMeshSync)
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。

&nbsp;  

- ポリゴンメッシュ、カメラ、ライトの同期に対応しています。
- Mesh Instance や Replicator も部分的にサポートしています
- ポリゴンメッシュはスキニング / Joint と Morph も Unity へ持ってこれるようになっていますが、デフォーマの扱いには注意が必要です。
  - MeshSync が解釈できるデフォーマは Joint + Weight Map 方式のスキニングと Morph のみです。それ以外のデフォーマは無視されます。
  - Mesh Instance や Replicator のスキニングは正しく Unity 側に反映できません。後述の "Bake Deformers" を使う必要があります。
  - "Bake Deformers" をチェックすると、デフォーマを全て適用した結果を同期します。複雑なデフォーマ構成であっても Unity 側の Mesh の内容がほぼ一致するようになりますが、代償としてスキニングや Morph/Blendshape の情報が失われます。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- 負のスケールは部分的にしかサポートしていないので注意が必要です
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます
- コマンドからも MeshSync の機能にアクセスできます。unity.meshsync.settings 設定変更、unity.meshsync.export エクスポートできます

<img align="right" src="https://user-images.githubusercontent.com/1488611/49272332-79d39480-f4b4-11e8-8ca3-0ce0bc90a965.png" height=400>

### Blender
Blender 2.79(a,b), 2.80 beta (2019-3-15) + Windows, Mac, Linux (CentOS 7) で動作を確認しています。実装の都合上、**Blender のバージョンが上がると互換性が失われる可能性が高い** ことにご注意ください。現在更新が盛んな 2.8 系は特に注意が必要です。気付き次第対応版を出す予定ではあります。
- インストール：
  - [releases](https://github.com/unity3d-jp/MeshSync/releases) から UnityMeshSync_Blender_*.zip をダウンロードして展開
    - 展開して出てくる UnityMeshSync_Blender_* ディレクトリの中にも zip が入っていますが、これらはそのままで大丈夫です
  - Blender 側で File -> User Preferences -> Add-ons (2.80 以降では Edit -> User Preferences) を開き、画面下部の "Install Add-on from file" を押し、プラグインの zip ファイルを指定します。
  - **古いバージョンをインストール済みの場合、事前に削除しておく必要があります**。Add-ons メニューから "Import-Export: Unity Mesh Sync" を選択して Remove した後、blender を再起動してから上記手順を踏んでください。
- "Import-Export: Unity Mesh Sync" が追加されるので、チェックを入れて有効化します。
- MeshSync パネルが追加されるので、そちらから設定や手動の同期を行います。
  - 2.8 系ではパネルの場所がややわかりにくい場所になっています。右スクリーンショットを参照ください。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。

&nbsp;  

- ポリゴンメッシュ、カメラ、ライトの同期に対応しています。
- ポリゴンメッシュはスキニング/ボーン (Armature) と BlendShape もそのまま Unity へ持ってこれるようになっています。Mirror デフォーマも対応しています。これら以外のモディファイアは無視されます。
  - "Bake Modifiers" をチェックすると、モディファイアを全て適用した結果を同期します。Blender 側と Unity 側で Mesh の内容がほぼ一致するようになりますが、代償として Skinning や Blendshape の情報が失われます。
- "Convert To Mesh" をチェックすると、NURBS などのポリゴンに変換可能なオブジェクトを変換して同期します。
- "Double Sided" をチェックすると Unity 側で Mesh が両面化されます。
- 負のスケールは部分的にしかサポートしていないので注意が必要です
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます

### メタセコイア
Windows 版 3 系と 4 系 (32bit & 64bit)、Mac 版 (4 系のみ) に対応しています。3 系はたぶん全てのバージョンに対応していますが、4 系は 4.6.4 以上である必要があります。(このバージョン以上でないとボーンの出力がサポートできないため)
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
  - これが有効の場合、オブジェクトが MeshCollider を持っていたら Mesh を同期する時に MeshCollider の Mesh も更新します。
- Track Material Assignment
  - これが有効の場合、SceneView でマテリアルを D&D でアサインした場合などに、その変更を検知して同じマテリアルを持つ他の Mesh に対しても同様にマテリアルを更新します。
- Animation Interpolation
  - アニメーションの補完方法を指定します。
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


- Unity の頂点あたりの最大影響ボーン数が 4 であることに注意が必要です。
  これが原因でボーンが多いと DCC 側と Unity 側で結果が一致しなくなることがあります。


- 本プラグインはその性質上エディタでのみの使用を想定していますが、一応ランタイムでも動作するようにしてあります。**意図せず最終ビルドに残さないようご注意ください**。  
  ランタイムではアニメーションの同期は機能しませんが、モデルの同期は一通り動作します。


##  関連
- [NormalPainter](https://github.com/unity3d-jp/NormalPainter): Unity 上で法線を編集できるようにするツール
- [BlendShapeBuilder](https://github.com/unity3d-jp/BlendShapeBuilder): Unity 上で BlendShape を構築できるようにするツール

## ライセンス
[MIT](LICENSE.txt), ただし Blender プラグインは [GPL3](Plugin/MeshSyncClientBlender/LICENSE.txt) (Blender のソースの一部を使っているため)
