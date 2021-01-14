using System.IO;

namespace Unity.MeshSync.Editor.Tests {

internal class MeshSyncTestEditorConstants {
    internal static readonly string TEST_DATA_PATH = Path.Combine("Packages", MeshSyncConstants.PACKAGE_NAME, "Tests", "Data");
    internal static readonly string CUBE_TEST_DATA_PATH   = Path.Combine(TEST_DATA_PATH, "Cube.sc");
    internal static readonly string SPHERE_TEST_DATA_PATH = Path.Combine(TEST_DATA_PATH, "Sphere.sc");
    
}

}
