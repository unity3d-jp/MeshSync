![demo](Documentation~/images/Demo.gif)

# 他の言語
- [English](Readme.md)

# MeshSync

[MeshSyncDCCPlugins](https://github.com/Unity-Technologies/MeshSyncDCCPlugins) と連携し、
MeshSync は DCC ツール上のモデルの編集をリアルタイムに Unity に反映させるためのパッケージです。
ゲーム上でどう見えるかをその場で確認しながらモデリングすることを可能にします。

MeshSync は現在プレビューパッケージとして存在し、
インストールの手順は Unity のバージョンによって少々違います。

* Unity 2019.x  
  ![PackageManager2019](Documentation~/images/PackageManager2019.png)
  1. [パッケージマネージャー](https://docs.unity3d.com/ja/current/Manual/upm-ui.html)を開く
  2. **Show preview packages** にチェックが付いているかを確認する
  3. *MeshSync*　を検索する
  
* Unity 2020.1  
  ![PackageManager2020](Documentation~/images/PackageManager2020.1.png)
  1. [パッケージマネージャー](https://docs.unity3d.com/ja/current/Manual/upm-ui.html)を開く
  2. **+** ボタンをクリックし、**Add package from git URL** を選択する
  3. `com.unity.meshsync@` とそれに続くバージョンを記入する。  
     例：`com.unity.meshsync@0.2.5-preview`
  
## 動作環境

- Windows 64 bit
- Mac
- Linux

## 最新の CI の状況

[![](https://badge-proxy.cds.internal.unity3d.com/f4d1069b-1233-4324-ae75-2fac980576a4)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync/build-info?branch=dev&testWorkflow=package-isolation)
[![](https://badge-proxy.cds.internal.unity3d.com/6cfcda56-5e8d-4612-abfa-6de23df068fb)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync/dependencies-info?branch=dev&testWorkflow=updated-dependencies)
[![](https://badge-proxy.cds.internal.unity3d.com/45cf24da-7561-4983-9e11-fc920996015c)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync/dependants-info)
[![](https://badge-proxy.cds.internal.unity3d.com/0998ee7c-b3f2-4ef9-97cd-628296f29c4a)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync/warnings-info?branch=dev)

![ReleaseBadge](https://badge-proxy.cds.internal.unity3d.com/9cb90abe-572c-440c-b7f9-f212c5573261)
![ReleaseBadge](https://badge-proxy.cds.internal.unity3d.com/4661afc4-7953-410d-a4fa-9668ed7da2b9)


# 基本的な使い方

**GameObject** メニューから **MeshSync > Create Server** でサーバーオブジェクトを作成します。
このサーバーオブジェクトが同期処理を担当する [MeshSyncServer](Documentation~/jp/MeshSyncServer.md) のコンポーネントを持っています。

![Menu](Documentation~/images/MenuCreateServer.png)

# 設定

MeshSync のコンポネントに対するデフォルト設定は
[プロジェクト設定](Documentation~/jp/ProjectSettings.md) ウィンドウで設定することができます。

![Server Settings](Documentation~/images/ProjectSettingsServer.png)

同様に、DCC ツールとの統合は
[環境設定](Documentation~/jp/Preferences.md) ウィンドウで設定することができます。

![Server Settings](Documentation~/images/Preferences.png)


# アドバンスト 機能
- [SceneCache](Documentation~/jp/SceneCache.md)

# プラグイン
- [ビルド](Plugin~/Docs/en/BuildPlugins.md)

# ライセンス
- [License](LICENSE.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Third Party Notices](Third%20Party%20Notices.md)
- [Contributing](CONTRIBUTING.md)

#  関連ツール
- [NormalPainter](https://github.com/unity3d-jp/NormalPainter): Unity 上で法線を編集できるようにするツール
- [BlendShapeBuilder](https://github.com/unity3d-jp/BlendShapeBuilder): Unity 上で BlendShape を構築できるようにするツール


