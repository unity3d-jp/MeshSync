using System.IO;
using Microsoft.Win32;

internal static class DefaultAppUtility
{
    public static bool TryGetRegisteredApplication(string extension, out string registeredApp)
    {
#if UNITY_EDITOR_WIN
        string extensionId = GetClassesRootKeyDefaultValue(extension);
        if (extensionId == null)
        {
            registeredApp = null;
            return false;
        }

        string openCommand = GetClassesRootKeyDefaultValue(
            Path.Combine(extensionId, "shell", "open", "command"));

        if (openCommand == null)
        {
            registeredApp = null;
            return false;
        }

        registeredApp = openCommand
            .Replace("%1", string.Empty)
            .Replace("\"", string.Empty)
            .Trim();
        return true;
#else
        // Don't do this when not on windows:
        registeredApp = null;
        return false;
#endif
    }

    private static string GetClassesRootKeyDefaultValue(string keyPath)
    {
        using (var key = Registry.ClassesRoot.OpenSubKey(keyPath))
        {
            if (key == null)
            {
                return null;
            }

            var defaultValue = key.GetValue(null);
            if (defaultValue == null)
            {
                return null;
            }

            return defaultValue.ToString();
        }
    }
}