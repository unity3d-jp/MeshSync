using System;
using UnityEngine;

namespace Unity.MeshSync {
    internal class ComputeShaderHelper {

        private const string SHADER_CONST_OUTPUT = "Output";

        private readonly ComputeShader shader;
        int kernelIndex;

        readonly uint groupSizeX;
        readonly uint groupSizeY;

        Vector2Int maxTextureSize = new Vector2Int(1, 1);

        public ComputeShaderHelper(ComputeShader shader, string kernelName) {
            this.shader = shader;

            kernelIndex = shader.FindKernel(kernelName);
            shader.GetKernelThreadGroupSizes(kernelIndex, out groupSizeX, out groupSizeY, out var gsZ);
        }

        public RenderTexture RenderToTexture(Texture existingTexture) {
            var renderTarget = existingTexture as RenderTexture;

            // If there is an existing renderTexture, reuse it:
            if (renderTarget == null ||
                renderTarget.width != maxTextureSize.x ||
                renderTarget.height != maxTextureSize.y) {
                if (renderTarget != null) {
                    renderTarget.Release();
                }

                renderTarget = new RenderTexture(maxTextureSize.x, maxTextureSize.y, 32) {
                    enableRandomWrite = true
                };

                renderTarget.Create();
            }

            SetTexture(SHADER_CONST_OUTPUT, renderTarget);

            var groupsX = (int)Math.Max(1, Math.Ceiling(maxTextureSize.x / (float)groupSizeX));
            var groupsY = (int)Math.Max(1, Math.Ceiling(maxTextureSize.y / (float)groupSizeY));

            shader.Dispatch(kernelIndex, groupsX, groupsY, 1);

            return renderTarget;
        }

        public void SetTexture(string name, Texture texture) {
            shader.SetTexture(kernelIndex, name, texture);
            shader.SetVector($"{name}_dims", new Vector4(texture.width, texture.height));
      
            maxTextureSize.x = Math.Max(maxTextureSize.x, texture.width);
            maxTextureSize.y = Math.Max(maxTextureSize.y, texture.height);
        }
    }
}
