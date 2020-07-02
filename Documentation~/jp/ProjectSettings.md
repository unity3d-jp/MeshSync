# プロジェクト設定

1. [General Settings](#general-settings)
1. [DCC Tools](#dcc-tools)

## General Settings

![ProjectSettingsGeneral](../images/ProjectSettingsGeneral.png)

General settings タブで、下記のプロパティの既定値を設定できます。
1. サーバー:
   * **Server Port**  
     デフォルトのサーバーのポート。
   * **Allow public access**  
     パブリック IP から MeshSync へのアクセスを許可する。 通常、この設定はオフになっており、
     ローカルネットワーク (127.0.0.1, 10.0.0.0/24, 192.168.0.0/16 or 172.16.0.0 to 172.31.255.255)
     にある端末のみ、MeshSync へアクセスできます。     

2. MeshSync のオブジェクト
   1. [MeshSyncServer](MeshSyncServer.md)
   2. [SceneCache](SceneCache.md)



## DCC Tools

DCC Tools タブで、サポートされている DCC ツールに
[MeshSyncDCCPlugins](https://github.com/Unity-Technologies/MeshSyncDCCPlugins) 
をインストールできます。 
初めてこのタブが開かれた時、デフォルトパスにインストールされた DCC ツールが自動的に
検出され、タブに表示されます。

![ProjectSettingsDCCTools](../images/ProjectSettingsDCCTools.png)

| 記号    | 用途                                                                                       | 
| ------- | ----------------------------------------------------------------------------------------- | 
| A       | DCC ツールの名前、パス、とその MeshSyncDCCPlugins のインストール状況を表示する                 |   
| B       | タブから DCC ツールを削除する                                                               |   
| C       | DCC ツール に MeshSyncDCCPlugins をインストールする                                         |  
| D       | デフォルトパスにインストールされた DCC ツールを検出し、タブに表示する                           |  
| E       | 手動で DCC ツールを追加する                                                                 | 

### サポートされている DCC ツール

1. Maya 2017 - 2020
2. 3ds Max 2017 - 2020
3. Blender 2.79, 2.80, 2.81, 2.82, 2.83

### 注意

1. MeshSyncDCCPlugins をインストールする前に、DCC ツールのインスタンスを終了してください。
2. 一部の DCC ツールで、MeshSyncDCCPlugins のインストールを完了させるために、
   その DCC ツールが起動される場合があります。


### MeshSyncDCCPlugins のインストール状況

インストール状況は下記のパスで保存されます。

| OS                | パス                                              | 
| ----------------- | --------------------------------------------------| 
| Linux             | ~/.config/unity3d/Unity/MeshSync                  |   
| macOS             | ~/Library/Application Support/Unity/MeshSync      |   
| Windows           | C:\Users\username\AppData\Local\Unity\MeshSync    |  




