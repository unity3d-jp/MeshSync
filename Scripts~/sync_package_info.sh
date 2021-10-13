allProjects=(
    "MeshSync~"
    "MeshSync_HDRP~"
    "MeshSync_URP~"
)

packageName="com.unity.meshsync"

for project in ${allProjects[@]}; do    
    cp package.json "${project}\\Packages\\${packageName}"
    cp CHANGELOG.md "${project}\\Packages\\${packageName}"    
done







