# StudioThursdayChart
表データをUnrealEngineで扱いやすくするためのプラグインです。

# できること
* エディタでPlay中に表データ（DataTable）を書き換えてゲームの動作に反映すること
* データエントリに一意のキー（GameplayTag）でアクセスすること
* データエントリにインデックスでアクセスすること

# インストール方法
インストールしたいProjectのPluginsフォルダ内に、このリポジトリのPlugins内のファイルをすべて配置してください。
![2023-09-23 15_41_46-MyProject](https://github.com/AkihiroImada/StudioThursdayChart/assets/17783214/891b06e5-b9f2-45ab-ad58-0a93a791b626)

# 使い方
1. 設定した特定の接頭辞（デフォルトは```DT_```）をもつCSVファイルをドラッグアンドドロップしてDataTableをインポートする
![2023-09-23 15_51_01-MyProject - Unreal Editor](https://github.com/AkihiroImada/StudioThursdayChart/assets/17783214/7a3e5892-ab99-49ca-ad11-3a0faaaaefbd)
2. 自動的に```ChartDataAsset```と```GameplayTagDataTable```が生成される
![2023-09-23 15_58_00-MyProject - Unreal Editor](https://github.com/AkihiroImada/StudioThursdayChart/assets/17783214/5585c0ca-c9f0-4fd0-b5c4-1f7bef176b38)
3. 任意のBPで```GetChartDataAssetRow```関数を用いて取得したいエントリのキーを指定する
![2023-09-23 16_00_25-BP_Actor](https://github.com/AkihiroImada/StudioThursdayChart/assets/17783214/6c4b75c8-9959-40ec-87db-e96d6e28280e)

以上です。

```GetChartDataAssetRow```の他にインデックスを指定して取得できる```GetChartDataAssetRowByIndex```もあります。
![2023-09-23 15_00_03-BP_ChartGameInstanceSubsystemHelper](https://github.com/AkihiroImada/StudioThursdayChart/assets/17783214/c9471a19-fa9b-4064-b854-67aeeb2ed438)

そのほか、キーを全取得する```GetRowTags```や最後のインデックスを取得する```GetLastRowIndex```もあります。
![2023-09-23 16_07_52-BP_Actor](https://github.com/AkihiroImada/StudioThursdayChart/assets/17783214/1e84b76b-b63a-44b8-92e1-3795bd1a025b)

応用的な使い方として、自動再インポートと合わせることで
1. CSVを変更すると同時にDataTableが変更される
2. DataTableの変更と同時にChartDataAssetが変更される
3. 即座にエディタでプレイ中のゲームに反映される

を実現することができます。

自動再インポートの公式ドキュメント：
https://docs.unrealengine.com/4.26/ja/Basics/AssetsAndPackages/AutoReImport/

# 動作概要
```DataTable```から```ChartDataAsset```と```GameplayTagDataTable```を生成します。  
```ChartDataAsset```には元となった```DataTable```とほぼ同等のデータが格納されており、そのキーは```GameplayTag```です。  
キーである```GameplayTag```は```GameplayTagDataTable```によって自動的に登録されます。  

## ChartDataAssetが更新されるタイミング
* 設定した接頭辞（と接尾辞）をもつDataTableをインポートしたとき
* ```ChartDataAsset```から```BuildFromDataTable```ボタンを押したとき
![2023-09-23 15_30_50-DA_Serif](https://github.com/AkihiroImada/StudioThursdayChart/assets/17783214/6d3b3a06-7e10-4db0-af48-0101eb9bbc4a)

したがって元となるDataTableの内容を直接編集した場合（インポートを経由しない場合）は```BuildFromDataTable```ボタンを押す必要があります。

# 設定
```ProjectSettings-Plugins-StudioThursdayChart```内にあります。
![2023-09-23 15_36_45-Project Settings](https://github.com/AkihiroImada/StudioThursdayChart/assets/17783214/1fa50c3f-d6b0-464a-acc7-0adee92c59a1)

例えば生成元となるDataTableの接頭辞の設定はSourceDataTablePrefixです。

## GameplayTagの登録場所
```ProjectSettings-Project-GameplayTags```内にあります。
![2023-09-23 16_17_21-Project Settings](https://github.com/AkihiroImada/StudioThursdayChart/assets/17783214/d1ddac03-6049-40a4-8083-80b929de4785)

# 不具合報告や要望や質問
当リポジトリのIssuesへお願いいたします。
