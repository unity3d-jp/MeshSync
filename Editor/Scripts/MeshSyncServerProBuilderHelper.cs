using System.Collections.Generic;
using UnityEditor;

namespace Unity.MeshSync.Editor {
    [InitializeOnLoad]
    internal static class MeshSyncServerProBuilderHelper {
#if AT_USE_PROBUILDER
        static readonly System.Reflection.FieldInfo versionInfoField = typeof(UnityEngine.ProBuilder.ProBuilderMesh).GetField("m_VersionIndex",
            System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);

        static readonly Dictionary<UnityEngine.ProBuilder.ProBuilderMesh, ushort> versionIndices = new Dictionary<UnityEngine.ProBuilder.ProBuilderMesh, ushort>();

        static MeshSyncServerProBuilderHelper() {
            BaseMeshSync.ProBuilderBeforeRebuild += ProBuilderBeforeRebuild;
            BaseMeshSync.ProBuilderAfterRebuild += ProBuilderAfterRebuild;
        }

        private static void MeshesChanged(IEnumerable<UnityEngine.ProBuilder.ProBuilderMesh> meshes) {
            if (meshes == null) {
                return;
            }
            
            foreach (var mesh in meshes) {
                // Older versions don't have this optimization:
                if (versionInfoField == null) {
                    var server = mesh.GetComponentInParent<MeshSyncServer>();
                    server?.MeshChanged(mesh);
                }
                else {
                    var version = (ushort)versionInfoField.GetValue(mesh);

                    if (!versionIndices.TryGetValue(mesh, out var storedVersion)) {
                        versionIndices.Add(mesh, version);
                    }
                    else if (version != storedVersion) {
                        versionIndices[mesh] = version;

                        var server = mesh.GetComponentInParent<MeshSyncServer>();
                        server?.MeshChanged(mesh);
                    }
                }
            }
        }

        private static void ProBuilderEditor_afterMeshModification(IEnumerable<UnityEngine.ProBuilder.ProBuilderMesh> meshes) {
            MeshesChanged(meshes);
        }

        private static void ProBuilderEditor_selectionUpdated(IEnumerable<UnityEngine.ProBuilder.ProBuilderMesh> meshes) {
            if (!UnityEditorInternal.InternalEditorUtility.isApplicationActive) {
                return;
            }

            MeshesChanged(meshes);
        }

        private static void ProBuilderBeforeRebuild() {
            // Change select mode back and forth to ensure ProBuilder invalidates its internal cache:
            var lastSelectMode = UnityEditor.ProBuilder.ProBuilderEditor.selectMode;
            UnityEditor.ProBuilder.ProBuilderEditor.selectMode = UnityEngine.ProBuilder.SelectMode.Object;
            UnityEditor.ProBuilder.ProBuilderEditor.selectMode = lastSelectMode;

            // Init pro builder callbacks:
            UnityEditor.ProBuilder.ProBuilderEditor.afterMeshModification -= ProBuilderEditor_afterMeshModification;
            UnityEditor.ProBuilder.ProBuilderEditor.selectionUpdated -= ProBuilderEditor_selectionUpdated;
        }

        private static void ProBuilderAfterRebuild() {
            // Init pro builder callbacks:
            UnityEditor.ProBuilder.ProBuilderEditor.afterMeshModification += ProBuilderEditor_afterMeshModification;
            UnityEditor.ProBuilder.ProBuilderEditor.selectionUpdated += ProBuilderEditor_selectionUpdated;
        }
#endif
    }
}
