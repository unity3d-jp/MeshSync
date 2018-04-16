[![MeshSync demo](https://img.youtube.com/vi/vawI9BJ9AUY/0.jpg)](https://www.youtube.com/watch?v=vawI9BJ9AUY)
# MeshSync
[English](https://translate.google.com/translate?sl=ja&tl=en&u=https://github.com/unity3d-jp/MeshSync)

DCC ツール上のモデルの編集をリアルタイムに Unity に反映させるツールです。ゲーム上でどう見えるかをその場で確認しながらモデリングすることを可能にします。  
Unity と DCC ツール両方のプラグインとして機能し、現在 [メタセコイア](http://www.metaseq.net/) と [xismo](http://mqdl.jpn.org/) をサポートしています。

関連:  
[NormalPainter](https://github.com/unity3d-jp/NormalPainter): Unity 上で法線を編集できるようにするツール  
[BlendShapeBuilder](https://github.com/unity3d-jp/BlendShapeBuilder): Unity 上で BlendShape を構築できるようにするツール  
[FbxExporter](https://github.com/unity3d-jp/FbxExporter): Unity の Mesh を FBX でエクスポートできるようにするツール  
## 使い方
- Unity 側
  - Unity 2017.1 系以上 & Windows (64 bit) で動作を確認しています
  - [MeshSync.unitypackage](https://github.com/unity3d-jp/MeshSync/releases/download/20171228/MeshSync.unitypackage) をプロジェクトにインポートします。
  - メニュー GameObject -> MeshSync -> Create Server でサーバーオブジェクトを作成します。
  - このサーバーオブジェクトが同期処理を担当します。これがシーン内になければ同期できません。


- メタセコイア側
  - メタセコイア 3 系、4 系 (32bit & 64bit) どちらも対応しています。3 系はたぶん全てのバージョンに対応していますが、4 系は 4.6.4 以上である必要があります。(このバージョン以上でないとボーンの出力がサポートできないため)
  - メタセコイア用プラグイン [UnityMeshSync.for.Metasequoia.zip](https://github.com/unity3d-jp/MeshSync/releases/download/20171228/UnityMeshSync.for.Metasequoia.zip) をインストールします。プラグインのタイプは Station です。
  - パネル -> Unity Mesh Sync を開き、"Auto Sync" をチェックします。
  - Auto Sync がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
  - "Import Unity Scene" を押すと現在 Unity で開かれているシーンをインポートすることができます。インポートしたシーンの編集もリアルタイムに反映可能です。


- xismo 側
  - xismo はプラグインの仕組みを提供していないため (2018/04 現在)、使い方が特殊であったり、 xismo のバージョンアップで動作しなくなる可能性が高いことにご注意ください。現行版は xismo 191～199 で動作を確認済みです。
  - [UnityMeshSync.for.xismo.zip](https://github.com/unity3d-jp/MeshSync/releases/download/20171228/UnityMeshSync.for.xismo.zip) を解凍し、出てくる 2 つのファイル (MeshSyncClientXismo.exe, MeshSyncClientXismoHook.dll) を xismo がインストールされているディレクトリ (xismo.exe と同じディレクトリ) に置きます。
  - MeshSyncClientXismo.exe を起動します。これにより MeshSync が付与された状態で xismo が起動します。
  - ウィンドウ -> Unity Mesh Sync で設定項目を開き、"Auto Sync" をチェックすると編集が自動的に Unity 側に反映されるようになります。
  
    
  同期は TCP/IP を介して行われるため、Unity と DCC ツールが別のマシンで動いていても同期させることができます。その場合、クライアントである DCC ツール側は設定が必要です。設定項目の "Server : Port" を Unity 側のマシンのアドレスに設定してください。
  
## メタセコイア側解説

- ミラーリング、スムーシングは Unity にも反映されます。
 - ただし、ミラーリングの "左右を接続した鏡面" は非サポートです。
- メタセコイアで非表示のオブジェクトは Unity でも非表示になります。非表示のオブジェクトはメッシュの内容は送られないので、シーン内にオブジェクトが増えて同期が重くなってきた場合適切に非表示にすることで同期も速くなるはずです。
- マテリアルは Unity には反映されませんが、マテリアル ID に応じて適切にサブメッシュに分割されます。
- サブディビジョンやメタボールはフリーズするまで Unity には反映されません。
- メタセコイア 4 系でサポートされた法線の編集は "Sync Normals" にチェックを入れることで反映できます。
- メタセコイア 4 系でサポートされたボーンは "Sync Bones" にチェックを入れることで反映できます。 "Sync Poses" にチェックを入れると "スキニング" で設定したポーズも反映します。

## xismo 側解説

- xismo のビューポートに表示されているモデルをそのまま送っているため、モデルは大体同期できるはずです。
- モデル以外 (オブジェクト/マテリアルの名前、ボーンなど) は未対応です。これらは xismo 側がプラグイン API を用意しない限り対応が困難であり、現状対応予定はありません。

## Unity 側解説

<img align="right" src="doc/MeshSyncServer.png">

### マテリアルリスト
MesySyncServer はマテリアルのリストを保持しています。このリストにマテリアルを設定すると、対応するオブジェクトに適切にアサインされます。

### アセット化
DCC ツール側の編集によって生成された Mesh 郡は、そのままではそのシーン内にしか存在できないオブジェクトです。他のシーンやプロジェクトへ持ち出せるようにするにはアセットファイルとして保存する必要があります。MeshSyncServer の "Export Mesh" ボタンを押すとそのアセット化が行われます。("Asset Export Path" で指定されたディレクトリにファイルが生成されます)  
また、Unity から DCC ツール側へインポートしたモデルを編集した場合、安全のため Unity 側では元の Mesh には手を加えず、新規に生成した Mesh で編集を受け取って表示します。"Export Mesh" はこの場合の元のアセットへの変更の反映も行います。

### ライトマップ UV
Unity でライトマップを使う場合、UV は通常のとは別の専用のものが用いられます。
通常はモデルインポート時に自動的に生成されますが、本プラグインで DCC ツールから受信したモデルにはそれがありません。  
MesySyncServer の "Generate Lightmap UV" ボタンを押すとそのライトマップ用 UV を生成します。
この処理は結構時間がかかるのでご注意ください。

### ランタイム対応
本プラグインはその性質上エディタでのみの使用を想定しており、意図せず最終ビルドに残してしまった場合想定外の動作を招く可能性があります。そのため、**意図的に Standalone でビルドするとエラーが出るようにしてあります**。  
もし最終ビルドにも残したい場合、MeshSyncServer.cs の ErrorOnStandaloneBuild() の中を適当にコメントアウトしてください。マテリアルリストを構築できない点以外はランタイムでも大体動作します。


## ライセンス
[MIT](LICENSE.txt)
