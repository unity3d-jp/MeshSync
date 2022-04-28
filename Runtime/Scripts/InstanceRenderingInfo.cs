using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

namespace Unity.MeshSync
{
    internal class InstancesBatch
    {
        public Matrix4x4[] Matrices { get; set; }
        public MaterialPropertyBlock PropertyBlock { get; set; }
    }
    
    internal class InstanceRenderingInfo
    {
        internal bool canRender => mesh != null && materials != null && materials.Length > 0;
        
        internal Mesh mesh { get; private set; }


        internal List<InstancesBatch> batches = new List<InstancesBatch>();

        private Material[] m_materials;

        internal Material[] materials
        {
            get => m_materials;
            set
            {
                if (value == m_materials)
                {
                    return;
                }

                m_materials = value;
                
                OnMaterialsUpdated();
            }
        }

        private void OnMaterialsUpdated()
        {
            // Enable instancing in materials
            for (var i = 0; i < m_materials.Length; i++) {
                m_materials[i].enableInstancing = true;
            }
        }

        private GameObject m_gameObject;
        
        private bool m_dirtyInstances = true;
        
        private Matrix4x4 m_cachedWorldMatrix = Matrix4x4.zero;
        
        private Matrix4x4[] m_instances;

        internal Matrix4x4[] instances
        {
            get => m_instances;
            set
            {
                m_instances = value;
                m_dirtyInstances = true;
            }
        }

        internal GameObject gameObject
        {
            get => m_gameObject;
            set
            {
                if (m_gameObject == value)
                    return;
                
                m_gameObject = value;
                
                OnGameObjectUpdated();
            }
        }

        private Renderer m_renderer;
        internal int layer
        {
            get
            {
                if (gameObject == null) 
                    return 0;
                    
                return gameObject.layer; 
            } 
        }

        internal bool receiveShadows
        {
            get
            {
                if (m_renderer == null)
                    return true;
                    
                return m_renderer.receiveShadows;
            }
        }

        internal ShadowCastingMode shadowCastingMode
        {
            get
            {
                if (m_renderer == null) {
                    return ShadowCastingMode.On;
                }

                return m_renderer.shadowCastingMode;
            }
        }

        internal LightProbeUsage lightProbeUsage
        {
            get
            {
                if (m_renderer == null || m_renderer.lightProbeUsage == LightProbeUsage.Off) {
                    return LightProbeUsage.Off;
                }

                return LightProbeUsage.CustomProvided;
            }
        }

        internal LightProbeProxyVolume lightProbeProxyVolume
        {
            get
            {
                if (m_renderer == null)
                    return null;
                
                if (m_renderer.lightProbeUsage != LightProbeUsage.UseProxyVolume)
                    return null;
                
                // Look for component in override
                var overrideGO = m_renderer.lightProbeProxyVolumeOverride;
                if (overrideGO != null && overrideGO.TryGetComponent(out LightProbeProxyVolume volumeOverride)) {
                    return volumeOverride;
                }
                
                // Look for component in Renderer
                if (m_renderer.TryGetComponent(out LightProbeProxyVolume volume)) {
                    return volume;
                }

                return null;
            }
        }

        private Matrix4x4 worldMatrix
        {
            get
            {
                if (gameObject == null)
                    return Matrix4x4.identity;
                    
                return gameObject.transform.localToWorldMatrix;
            }
        }

        private bool positionsChanged => m_dirtyInstances || m_cachedWorldMatrix != this.worldMatrix;

        internal void PrepareForDrawing()
        {
            if (!positionsChanged)
                return;
            
            batches.Clear();
            
            UpdateDividedInstances();
            UpdateMaterialProperties();
            
            m_dirtyInstances = false;
            m_cachedWorldMatrix = this.worldMatrix;
        }
        
        private void UpdateDividedInstances()
        {
            if (instances == null)
                return;
                
            var maxSize = 1023;
            var iterations = instances.Length / maxSize;
            for (var i = 0; i < iterations; i++) {
                AddInstances(maxSize, i, maxSize);
            }

            var remainder = instances.Length % maxSize;
            if (remainder > 0) {
                AddInstances(remainder, iterations, maxSize);
            }
        }

        private void UpdateMaterialProperties()
        {
            for (var batchIndex = 0; batchIndex < batches.Count; batchIndex++){
                
                var batch = batches[batchIndex];
                var count = batch.Matrices.Length;
                
                var lightProbes = new SphericalHarmonicsL2[count];
                var occlusionProbes = new Vector4[count];
                var positions = new Vector3[count];
                
                var matrices = batch.Matrices;
                for (var i = 0; i < matrices.Length; i++)
                {
                    var instance = matrices[i];
                    positions[i] = instance.GetColumn(3);
                }
                
                LightProbes.CalculateInterpolatedLightAndOcclusionProbes(positions, lightProbes, occlusionProbes);

                batch.PropertyBlock = new MaterialPropertyBlock();
                batch.PropertyBlock.CopyProbeOcclusionArrayFrom(occlusionProbes);
                batch.PropertyBlock.CopySHCoefficientArraysFrom(lightProbes);
            }
        }

        private void AddInstances(int size, int iteration, int maxSize)
        {
            var array = new Matrix4x4[size];

            for (var j = 0; j < array.Length; j++) {
                array[j] = worldMatrix * instances[iteration * maxSize + j];
            }

            var batch = new InstancesBatch();
            batch.Matrices = array;
            batches.Add(batch);
        }

        private void OnGameObjectUpdated()
        {
            var go = gameObject;

            if (go == null)
                return;
            
            if (go.TryGetComponent(out SkinnedMeshRenderer skinnedMeshRenderer)) {
                mesh = skinnedMeshRenderer.sharedMesh;
                materials = skinnedMeshRenderer.sharedMaterials;
                m_renderer = skinnedMeshRenderer;
                return;
            }
            
            if (!go.TryGetComponent(out MeshFilter filter)) {
                Debug.LogWarningFormat("[MeshSync] No Mesh Filter for {0}", go.name);
                return;
            }

            if (!go.TryGetComponent(out MeshRenderer renderer)) {
                Debug.LogWarningFormat("[MeshSync] No renderer for {0}", go.name);
                return;
            }
            
            mesh = filter.sharedMesh;
            materials = renderer.sharedMaterials;
            m_renderer = renderer;
        }
    }
}