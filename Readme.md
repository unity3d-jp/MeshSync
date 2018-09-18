![demo](https://user-images.githubusercontent.com/1488611/39971828-98afa1d8-573d-11e8-9a6f-86263bee8949.gif)
# MeshSync
[English](https://translate.google.com/translate?sl=ja&tl=en&u=https://github.com/unity3d-jp/MeshSync)

DCC ツール上のモデルの編集をリアルタイムに Unity に反映させるツールです。ゲーム上でどう見えるかをその場で確認しながらモデリングすることを可能にします。  
Unity と DCC ツール両方のプラグインとして機能し、現在 [Maya](https://www.autodesk.eu/products/maya/overview), [Maya LT](https://www.autodesk.eu/products/maya-lt/overview), [3ds Max](https://www.autodesk.com/products/3ds-max/overview), [Blender](https://blenderartists.org/), [メタセコイア](http://www.metaseq.net/), [xismo](http://mqdl.jpn.org/) をサポートしています。


## 使い方

**プラグイン本体は [releases](https://github.com/unity3d-jp/MeshSync/releases) からダウンロードしてください。**

1. [Maya](#maya)
2. [Maya LT](#maya-lt)
3. [3ds Max](#3ds-max)
3. [Motion Builder](#motion-builder)
4. [Blender](#blender)
5. [メタセコイア](#メタセコイア)
5. [xismo](#xismo)
6. [Unity](#unity)

<img align="right" src="https://user-images.githubusercontent.com/1488611/39971860-7f6d1330-573e-11e8-9a1e-9d95709cbd50.png" height=400>

### Maya
Maya 2015, 2016, 2016.5, 2017, 2018 + Windows, Mac, Linux (CentOS 7) で動作を確認しています。
- インストールするには、プラグインを Maya の module path が通っているディレクトリにコピーします。
  - Windows: %MAYA_APP_DIR% が設定されている場合はそこに、ない場合は %USERPROFILE%\Documents\maya (←を Explorer のアドレスバーへコピペで直行) に modules ディレクトリをそのままコピー。
    - 2016 以前の場合はバージョン名のディレクトリへコピーします。(%MAYA_APP_DIR%\2016 など)
  - Mac: /Users/Shared/Autodesk/modules/maya に UnityMeshSync ディレクトリと .mod ファイルをコピー。
  - Linux: ~/maya/(Maya のバージョン) に modules ディレクトリをそのままコピー。
- Maya を起動し、Windows -> Settings/Preferences -> Plug-in Manager を開き、MeshSyncClient の Loaded にチェックを入れてプラグインを有効化します。
- UnityMeshSync シェルフが追加されているので、それの歯車アイコンで設定メニューを開きます。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります

&nbsp;  

- 歯車アイコン以外のボタンはそれぞれ手動同期、アニメーション同期相当のボタンになっています
- ポリゴンメッシュ、カメラ、ライトの同期に対応しています
- ポリゴンメッシュはスキニング/ボーンと BlendShape もそのまま Unity へ持ってこれるようになっています
- 負のスケールは部分的にしかサポートしていないので注意が必要です
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます
- NURBS などポリゴン以外の形状データは対応していません
- インスタンシングは対応していますが、スキニングされたメッシュのインスタンスは現在未対応です (Unity 側では全て元インスタンスと同じ位置になっていまいます)
- MEL にもコマンドが登録されており、全ての機能に MEL 経由でアクセスできるようになっています。こちらの詳細は[ソースコードを参照ください](https://github.com/unity3d-jp/MeshSync/blob/master/Plugin/MeshSyncClientMaya/msmayaCommands.cpp)


### Maya LT
現在 Windows のみ対応で、Maya LT 2018 + Windows で動作を確認しています。Maya LT は本体外部プラグインをサポートしないため、問題が起きる可能性が高いことに留意ください。Maya LT 側のマイナーバージョンアップでも互換性が失われる可能性が考えられます。  
パッケージは別になっているものの、インストールや使い方は [非 LT の Maya](#maya) と同じです。


### 3ds Max
3ds Max 2016, 2017, 2018, 2019 + Windows で動作を確認しています。
- インストールするには、プラグイン用パスとして登録されているディレクトリにプラグインをコピーします。
  - デフォルトで用意されているパスは C:\Program Files\Autodesk\3ds Max 2019\Plugins など
- インストール後、メインメニューバーに "UnityMeshSync" が追加されているので、それの "Window" から設定ウィンドウを開けます。
  - メニューバーを編集する場合、Action に "UnityMeshSync" カテゴリが追加されているので、そちらから MeshSync の機能にアクセスできます
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります

&nbsp;  

- ポリゴンメッシュ、カメラ、ライトの同期に対応しています
- モディファイアは大体対応していますが、対応できないケースもあります。以下のルールに従います
  - Morph も Skin もない場合、全てのモディファイアを適用した状態で同期します
  - Morph か Skin がある場合、その一つ前までのモディファイアを適用した状態で同期します
    - Morph / Skin が複数ある場合、一番下のものが基準として選ばれます
  - Morh と Skin は Unity 側にそのまま Blendshape / Skin として同期します
    - Unity 側では常に Blendshape -> Skin の順番で適用されるため、Max 側で順番が逆だと意図しない結果になる可能性があります
- 負のスケールは部分的にしかサポートしていないので注意が必要です
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます
- NURBS などポリゴン以外の形状データは対応していません
- Max script にもコマンドが追加されており、全ての機能に Max script 経由でアクセスできるようになっています。こちらの詳細は[ソースコードを参照ください](https://github.com/unity3d-jp/MeshSync/blob/master/Plugin/MeshSyncClient3dsMax/msmaxEntryPoint.cpp)


### Motion Builder
Motion Builder 2015, 2016, 2017, 2018 + Windows, Linux (CentOS 7) で動作を確認しています
- インストールするには、プラグインの zip を適当な場所に展開後、Motion Builder 内の Settings -> Preferences -> SDK メニューからプラグイン dll ファイルがあるディレクトリを追加します
- インストール後、Asset Browser 内の Templates -> Devices に UnityMeshSync というオブジェクトが追加されているので、それをシーンに追加します
- Navigator 内の Devices -> UnityMeshSync を選択することで各種設定や機能にアクセスできます 
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります

&nbsp;  

- ポリゴンメッシュ、カメラ、ライトの同期に対応しています
- ポリゴンメッシュはスキニング/ボーンと BlendShape もそのまま Unity へ持ってこれるようになっています
- 負のスケールは部分的にしかサポートしていないので注意が必要です
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます
- NURBS などポリゴン以外の形状データは対応していません

<img align="right" src="https://user-images.githubusercontent.com/1488611/39971861-81043192-573e-11e8-9945-2bb248d869bd.png" height=400>

### Blender
Blender 2.79(a,b) + Windows, Mac, Linux (CentOS 7) で動作を確認しています。実装の都合上、**Blender のバージョンが上がると互換性が失われる可能性が高い** ことにご注意ください。 対応は容易なので、気付き次第対応版を出す予定ではあります。
- インストールするには、File -> User Preferences -> Add-ons を開き、画面下部の "Install Add-on from file" を押し、プラグインの zip アーカイブを指定します。
  - **古いバージョンをインストール済みの場合、事前に削除しておく必要があります**。Add-ons メニューから "Import-Export: Unity Mesh Sync" を選択して Remove した後、blender を再起動してから上記手順を踏んでください。
- "Import-Export: Unity Mesh Sync" が追加されるので、チェックを入れて有効化します。
- MeshSync パネルが追加されるので、そちらから設定や手動の同期を行います。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。

&nbsp;  

- ポリゴンメッシュ、カメラ、ライトの同期に対応しています
- ポリゴンメッシュはスキニング/ボーンと BlendShape もそのまま Unity へ持ってこれるようになっています
- 制限はあるものの Mirror デフォーマも対応しています
- 負のスケールは部分的にしかサポートしていないので注意が必要です
  - XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます
- NURBS などポリゴン以外の形状データは対応していません
- Armature, BlendShape, Mirror 以外のデフォーマは対応していません

### メタセコイア
Windows 版 3 系と 4 系 (32bit & 64bit)、Mac 版 (4 系のみ) に対応しています。3 系はたぶん全てのバージョンに対応していますが、4 系は 4.6.4 以上である必要があります。(このバージョン以上でないとボーンの出力がサポートできないため)
- インストールするには、Help -> About Plug-ins を開き、ダイアログ左下の "Install" からプラグインファイルを指定します。ちなみにプラグインのタイプは Station です。
  - **古いバージョンをインストール済みの場合、事前に手動で削除しておく必要があります**。メタセコイアを起動していない状態で該当ファイルを削除してください。
- インストール後 パネル -> Unity Mesh Sync が追加されるのでこれを開き、"Auto Sync" をチェックします。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- "Import Unity Scene" を押すと現在 Unity で開かれているシーンをインポートすることができます。インポートしたシーンの編集もリアルタイムに反映可能です。

&nbsp;  

- ミラーリング、スムーシングは Unity にも反映されます。
  - ただし、ミラーリングの "左右を接続した鏡面" は非サポートです。
- メタセコイアで非表示のオブジェクトは Unity でも非表示になります。非表示のオブジェクトはメッシュの内容は送られないので、シーン内にオブジェクトが増えて同期が重くなってきた場合適切に非表示にすることで同期も速くなるはずです。
- マテリアルは Unity には反映されませんが、マテリアル ID に応じて適切にサブメッシュに分割されます。
- サブディビジョンやメタボールはフリーズするまで Unity には反映されません。
- メタセコイア 4 系でサポートされた法線の編集は "Sync Normals" にチェックを入れることで反映できます。
- メタセコイア 4 系でサポートされたボーンは "Sync Bones" にチェックを入れることで反映できます。 "Sync Poses" にチェックを入れると "スキニング" で設定したポーズも反映します。

### xismo
xismo はプラグインの仕組みを提供していないため (2018/05 現在)、使い方が特殊であったり、 xismo のバージョンアップで動作しなくなる可能性が高いことにご注意ください。現行版は xismo 191～199 で動作を確認済しています。
- [UnityMeshSync_xismo_Windows.zip](https://github.com/unity3d-jp/MeshSync/releases) を解凍し、出てくる 2 つのファイル (MeshSyncClientXismo.exe, MeshSyncClientXismoHook.dll) を xismo がインストールされているディレクトリ (xismo.exe と同じディレクトリ) に置きます。
- MeshSyncClientXismo.exe を起動します。これにより MeshSync が付与された状態で xismo が起動します。
- ウィンドウ -> Unity Mesh Sync メニューが追加されており、これで各種設定などを行います。
- "Auto Sync" をチェックすると編集が自動的に Unity 側に反映されるようになります。

&nbsp;  

- xismo のビューポートに表示されているモデルをそのまま送っているため、モデルは大体同期できるはずです。
- モデル以外 (オブジェクト/マテリアルの名前、ボーンなど) は未対応です。これらは xismo 側がプラグイン API を用意しない限り対応が困難であり、現状対応予定はありません。


### Unity
- Unity 5.5 系以上 + Windows (64 bit), Mac, Linux (CentOS 7) で動作を確認しています
- [MeshSync.unitypackage](https://github.com/unity3d-jp/MeshSync/releases) をプロジェクトにインポートします。
  - 古いバージョンをインストール済みの場合、**パッケージインポート前に一度 Unity を終了し、Assets/UTJ/MeshSync を削除** しておくと確実です。プラグインがロードされていると更新に失敗するためです。
  - Unity 5.5 の場合、インポート後にプラグイン dll の設定を下記画像のように変更する必要があります (5.6 を堺に .meta ファイルの仕様が変わっているため)
  ![](https://user-images.githubusercontent.com/1488611/40534280-7ef6ecd6-6040-11e8-9f55-ea2f509a2383.png)
- メニュー GameObject -> MeshSync -> Create Server でサーバーオブジェクトを作成します。
- このサーバーオブジェクトが同期処理を担当します。これがシーン内になければ同期できません。

<img align="right" src="https://user-images.githubusercontent.com/1488611/39972178-45cf48d6-5744-11e8-8722-11fd412c7abd.png">

- Root Object
  - 同期により生成されるオブジェクト群のルートとなるオブジェクトを指定します。未設定の場合、ルートにオブジェクトが生成されていきます。
- Sync Transform など
  - コンポーネント別の同期の有効/無効指定です。Play モードで物理シミュレーションの挙動を確認したい場合などに Transform の同期が邪魔になるので用意されたオプションです。
- Update Mesh Collider オプション
  - これが有効の場合、オブジェクトが MeshCollider を持っていたら Mesh を同期する時に MeshCollider の Mesh も更新します。
  
  &nbsp;

- マテリアルリスト
  - MesySyncServer はマテリアルのリストを保持しています。このリストにマテリアルを設定すると、対応するオブジェクトに適切にアサインされます。


- アセット化
  - DCC ツール側の編集によって生成された Mesh 郡は、そのままではそのシーン内にしか存在できないオブジェクトです。他のシーンやプロジェクトへ持ち出せるようにするにはアセットファイルとして保存する必要があります。
  MeshSyncServer の "Export Mesh" ボタンを押すとそのアセット化が行われます。("Asset Export Path" で指定されたディレクトリにファイルが生成されます)  


- ライトマップ UV
  - Unity でライトマップを使う場合、UV は通常のとは別の専用のものが用いられます。通常はモデルインポート時に自動的に生成されますが、本プラグインで DCC ツールから受信したモデルにはそれがありません。
  MesySyncServer の "Generate Lightmap UV" ボタンを押すとそのライトマップ用 UV を生成します。この処理は結構時間がかかるのでご注意ください。


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
