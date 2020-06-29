![demo](Documentation~/images/Demo.gif)

# 他の言語
- [English](Readme.md)

# MeshSync

[MeshSyncDCCPlugins](https://github.com/Unity-Technologies/MeshSyncDCCPlugins) と連携し、
MeshSync は DCC ツール上のモデルの編集をリアルタイムに Unity に反映させるためのパッケージです。
ゲーム上でどう見えるかをその場で確認しながらモデリングすることを可能にします。

MeshSync は現在プレビューパッケージとして存在し、それによってインストールする為にプレビューパッケージを有効にする必要があります。

* Unity 2019.x  
  [パッケージマネージャー](https://docs.unity3d.com/ja/current/Manual/upm-ui.html)を開き、
  **Show preview packages** にチェックが付いているかを確認してから、MeshSync を検索して下さい。
  ![Menu](Documentation~/images/PackageManager.png)

* Unity 2020.x  
  Preview packages を有効にするの手順：
  1. Edit > Project Settings　を開く
  1. Project Settings ウィンドウでパッケージマネージャーを選択する
  1. **Enable Preview Packages** のチェックボックスをチェックする
  1. **I understand** をクリックする
    
  上記の手順を終えてから、[パッケージマネージャー](https://docs.unity3d.com/ja/current/Manual/upm-ui.html)を開き、MeshSync を検索して下さい。
  
## 動作環境

- Windows 64 bit
- Mac
- Linux

# 基本的な使い方

**GameObject** メニューから **MeshSync > Create Server** でサーバーオブジェクトを作成します。
このサーバーオブジェクトが同期処理を担当する [MeshSyncServer](Documentation~/en/MeshSyncServer.md) のコンポーネントを持っています。

![Menu](Documentation~/images/MenuCreateServer.png)

# 設定

DCC ツールの統合などといった、様々な MeshSync の設定を 
[Project Settings](Documentation~/jp/ProjectSettings.md) ウィンドウで設定することができます。

![Settings](Documentation~/images/ProjectSettings.png)


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


