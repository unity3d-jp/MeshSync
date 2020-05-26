baseDir = pathConfig.removePathLeaf (getDir #temp)
iniFile = baseDir + "\\Plugin.UserSettings.ini"
print iniFile
setIniSetting iniFile "Directories" "MeshSync" "{0}"
quitMax #noPrompt

