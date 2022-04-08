using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

namespace Unity.MeshSync
{
    internal class InstanceRenderingInfo
    {
        public Mesh Mesh;
        
        public List<Matrix4x4[]> dividedInstances { get; private set; } = new List<Matrix4x4[]>();

        private Material[] m_materials;

        public Material[] materials
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

        public Matrix4x4[] instances
        {
            get => m_instances;
            set
            {
                m_instances = value;
                m_dirtyInstances = true;
            }
        }

        public GameObject gameObject
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
        public int layer
        {
            get
            {
                if (gameObject == null) 
                    return 0;
                    
                return gameObject.layer; 
            } 
        }

        public bool receiveShadows
        {
            get
            {
                if (m_renderer == null)
                    return true;
                    
                return m_renderer.receiveShadows;
            }
        }

        public ShadowCastingMode shadowCastingMode
        {
            get
            {
                if (m_renderer == null) {
                    return ShadowCastingMode.On;
                }

                return m_renderer.shadowCastingMode;
            }
        }

        public LightProbeUsage lightProbeUsage
        {
            get
            {
                if (m_renderer == null) {
                    return LightProbeUsage.BlendProbes;
                }

                return m_renderer.lightProbeUsage;
            }
        }

        public LightProbeProxyVolume lightProbeProxyVolume
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
            
        public void UpdateDividedInstances()
        {
            // Avoid recalculation if the instances are the same
            // and the world matrix has not changed.
            if (!m_dirtyInstances && m_cachedWorldMatrix == this.worldMatrix)
                return;
            
            m_dirtyInstances = false;
            m_cachedWorldMatrix = this.worldMatrix;
            
            dividedInstances.Clear();
            
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

        private void AddInstances(int size, int iteration, int maxSize)
        {
            var array = new Matrix4x4[size];

            for (var j = 0; j < array.Length; j++) {
                array[j] = worldMatrix * instances[iteration * maxSize + j];
            }

            dividedInstances.Add(array);
        }

        private void OnGameObjectUpdated()
        {
            var go = gameObject;

            if (go == null)
                return;
            
            if (go.TryGetComponent(out SkinnedMeshRenderer skinnedMeshRenderer)) {
                Mesh = skinnedMeshRenderer.sharedMesh;
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
            
            Mesh = filter.sharedMesh;
            materials = renderer.sharedMaterials;
            m_renderer = renderer;
        }
    }
}