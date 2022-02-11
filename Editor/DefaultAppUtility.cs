using System.IO;
using Microsoft.Win32;

public static class DefaultAppUtility
{
    public static bool TryGetRegisteredApplication(string extension, out string registeredApp)
    {
        string extensionId = GetClassesRootKeyDefaultValue(extension);
        if (extensionId == null)
        {
            registeredApp = null;
            return false;
        }

        string openCommand = GetClassesRootKeyDefaultValue(
            Path.Combine(new[] {extensionId, "shell", "open", "command"}));

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