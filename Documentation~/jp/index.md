![demo](../images/Demo.gif)

# MeshSync

[MeshSyncDCCPlugins](https://github.com/Unity-Technologies/MeshSyncDCCPlugins) と連携し、
MeshSync は DCC ツール上のモデルの編集をリアルタイムに Unity に反映させるためのパッケージです。  
ゲーム上でどう見えるかをその場で確認しながらモデリングすることを可能にします。



## 動作環境

- Windows 64 bit
- Mac
- Linux

# 基本的な使い方

**GameObject** メニューから **MeshSync > Create Server** でサーバーオブジェクトを作成します。
このサーバーオブジェクトが同期処理を担当する [MeshSyncServer](MeshSyncServer.md) のコンポーネントを持っています。

![Menu](../images/MenuCreateServer.png)

# 設定

MeshSync のコンポネントに対するデフォルト設定は
[プロジェクト設定](ProjectSettings.md) ウィンドウで設定することができます。

![Server Settings](../images/ProjectSettingsServer.png)

同様に、DCC ツールとの統合は
[環境設定](Preferences.md) ウィンドウで設定することができます。

![Server Settings](../images/Preferences.png)


# アドバンスト 機能
- [SceneCache](SceneCache.md)

# 他の言語
- [English](../index.md)


