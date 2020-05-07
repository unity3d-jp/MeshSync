using System;
using System.IO;
using System.Security.Cryptography;

namespace Unity.AnimeToolbox {

//[TODO-sin: 2020-5-7] Move this to AnimeToolbox
internal static class FileUtility {

    internal static string ComputeMD5(string path) {
        using (var md5 = MD5.Create()) {
            using (var stream = File.OpenRead(path)) {
                byte[] hash = md5.ComputeHash(stream);
                string str = BitConverter.ToString(hash).Replace("-", "").ToLowerInvariant();
                return str;
            }
        }
    }
    
}

}