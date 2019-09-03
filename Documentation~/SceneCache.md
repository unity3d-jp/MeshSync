# Scene Cache
Scene Cache は DCC 上のシーンを各フレームそのままファイルに保存しておき、Unity で再生する仕組みです。DCC 側のモデル、カメラ、アニメーションを忠実に、かつ高速に再生することを目的としています。  

非常に近いものとして Alembic が挙げられます。Alembic との最大の違いは再生速度です。AlembicForUnity で Alembic 再生するよりも SceneCache の方が数倍速くなります。
他にはマテリアルのサポートなど Alembic にはない要素が追加されていたり、DCC ツールによっては SceneCache の方が再現度が高いこともあります。(例：3ds max の標準の Alembic exporter はレンダリング用 Mesh のエクスポートはサポートしていませんが、SceneCache は可能です)  
代償として、現状 Unity でしか再生できない、バージョンアップで互換性が失われて再エクスポートが必要になる可能性が高い等、汎用性をほぼ捨てているという欠点もあります。

<img align="right" src="https://user-images.githubusercontent.com/1488611/62694237-ea9d5a00-ba0e-11e9-850c-496ad83ec17a.png" height=400>

## 3ds Max
UnityMeshSync -> Export Cache メニューでエクスポートの設定が開きます。一部の設定は Alembic のエクスポート設定に準じています。

### Export
"All Nodes" だとシーン内の全ノードを、"Selected Nodes" だと選択されたノードのみをエクスポートします。

### Animation
エクスポートするフレームの範囲を指定します。

##### Samples per Frame
サンプルレート (キャプチャの頻度) の設定です。  
1 であればフレームレートがそのままサンプルレートになります (max が 24 FPS であればキャッシュも 24 FPS)。
0.5 にすると 2 フレーム毎にキャプチャされます (24 FPS -> 12 FPS)。

### Export Data
##### ZSTD Compression Level
圧縮レベルの設定です。  
最大 22 で、上げるとファイルサイズは縮みますがインポート/エクスポートにかかる時間は大きく増えます。
最大でもそこまで劇的には縮まないため、多くの場合デフォルト設定のままが望ましい結果になると思われます。  
圧縮は可逆であり、品質の劣化はありません。

##### Materials
マテリアルのエクスポート設定です。  
"One Frame" だと最初の 1 フレームだけ、"All Frames" だと全フレームのマテリアルデータをエクスポートします。
現状マテリアルのサポートは弱く、基本的に Unity 側で再セットアップする想定なため、"One Frame" をデフォルトとしています。

##### Use Render Meshes
有効な場合、レンダリング用の Mesh を export します。  
具体的には、TurboSmooth で Render Iters を設定している場合そちらの設定を見るようになる、
MaxLiquid などが Mesh としてエクスポートされるようになる、等の違いが現れます。

##### Flatten Hierarchy
有効な場合、Mesh や Camera 等、必要なノードのみをルートノードとしてエクスポートします。  
中間のレンダリングに影響しないノードは無視されます。Unity 側で若干の再生の高速化を期待できます。

##### Strip Normals/Tangents
これらが有効な場合、法線/接線 (Tangents) はエクスポートせず、再生時にその場で計算するようになります。  
法線は手動で編集してある可能性があるため、デフォルトでエクスポートする設定になっています。
接線はエクスポートするとファイルサイズが大きく増え、それに伴って圧縮/解凍の時間も増えるため、エクスポートするメリットが薄いです。そのため、デフォルトでエクスポートしない設定になっています。(今後のアップデートで改善される可能性はあります)

### 注意点
- "Bake Modifiers" が有効な場合、Mesh の Transform は頂点に適用されます。  
例えば、位置が x,y,z = 1,2,3 の Mesh をエクスポートした場合、Mesh の各頂点が 1,2,3 移動し、Unity 上の Transform は位置が 0,0,0 になります。これは Transform の階層では Mesh の変形を再現できないケースがあるためです。

<img align="right" src="https://user-images.githubusercontent.com/1488611/62694488-65ff0b80-ba0f-11e9-8d5e-404e632b0e9a.png" height=400>

## Unity
Game Object -> MeshSync -> Create Cache Player メニューを選択し、DCC 側でエクスポートした .sc ファイルを選択し、再生用オブジェクトを作成します。

### Scene Cache Player
このコンポーネントが再生を担当します。多くの設定は [MeshSyncServer](https://github.com/unity3d-jp/MeshSync#unity) と共通です。

"Create Cache Player" した時、同時に AnimationClip が作成されて SceneCachePlayer に結び付いています。この clip がアニメーションの再生を担当します。Timeline で再生する場合も、この clip を AnimationTrack に配置することで対応できます。

"Create Cache Player" した直後はキャッシュファイルへのパスが絶対パスになっており、その PC でしか再生できません。
"Copy to StreamingAssets" / "Move to StreamingAssets" ボタンを押すと、キャッシュファイルを StreamingAssets にコピー/移動します。
これを行っておくと、プロジェクトを他の PC にコピーした場合や、プロジェクトをビルドして他の PC で動かした場合も再生できるようになります。

##### Time
再生の時間です。
このパラメータを動かすことでアニメーションを再生します。
通常これは AnimationClip で制御することになるでしょう。

#### Interpolation
これが有効な場合、前後のフレームの Mesh や Transform を補間してアニメーションを滑らかにします。  
Mesh はトポロジが一致している (インデックスが不変) もののみ補間されます。
