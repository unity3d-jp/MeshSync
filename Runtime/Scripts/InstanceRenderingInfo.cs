using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

namespace Unity.MeshSync
{
    internal class InstanceRenderingInfo
    {
        public Mesh Mesh;
        
        public List<Matrix4x4[]> DividedInstances { get; private set; } = new List<Matrix4x4[]>();

        private Material[] m_materials;

        public Material[] Materials
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
            for (var i = 0; i < m_materials.Length; i++)
            {
                m_materials[i].enableInstancing = true;
            }
        }

        private GameObject m_gameObject;
        
        private bool m_dirtyInstances = true;
        
        private Matrix4x4 m_cachedWorldMatrix = Matrix4x4.zero;
        
        private Matrix4x4[] m_instances;

        public Matrix4x4[] Instances
        {
            get => m_instances;
            set
            {
                m_instances = value;
                m_dirtyInstances = true;
            }
        }

        public GameObject GameObject
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

        private Renderer Renderer;
        public int Layer
        {
            get
            {
                if (GameObject == null) 
                    return 0;
                    
                return GameObject.layer; 
            } 
        }

        public bool ReceiveShadows
        {
            get
            {
                if (Renderer == null)
                    return true;
                    
                return Renderer.receiveShadows;
            }
        }

        public ShadowCastingMode ShadowCastingMode
        {
            get
            {
                if (Renderer == null)
                {
                    return ShadowCastingMode.On;
                }

                return Renderer.shadowCastingMode;
            }
        }

        public LightProbeUsage LightProbeUsage
        {
            get
            {
                if (Renderer == null)
                {
                    return LightProbeUsage.BlendProbes;
                }

                return Renderer.lightProbeUsage;
            }
        }

        public LightProbeProxyVolume LightProbeProxyVolume
        {
            get
            {
                if (Renderer == null)
                    return null;
                
                if (Renderer.lightProbeUsage != LightProbeUsage.UseProxyVolume)
                    return null;
                
                // Look for component in override
                var overrideGO = Renderer.lightProbeProxyVolumeOverride;
                if (overrideGO != null && overrideGO.TryGetComponent(out LightProbeProxyVolume volumeOverride))
                {
                    return volumeOverride;
                }
                
                // Look for component in Renderer
                if (Renderer.TryGetComponent(out LightProbeProxyVolume volume))
                {
                    return volume;
                }

                return null;
            }
        }

        private Matrix4x4 WorldMatrix
        {
            get
            {
                if (GameObject == null)
                    return Matrix4x4.identity;
                    
                return GameObject.transform.localToWorldMatrix;
            }
        }
            
        public void UpdateDividedInstances()
        {
            // Avoid recalculation if the instances are the same
            // and the world matrix has not changed.
            if (!m_dirtyInstances && m_cachedWorldMatrix == WorldMatrix)
                return;
            
            m_dirtyInstances = false;
            m_cachedWorldMatrix = WorldMatrix;
            
            DividedInstances.Clear();
            
            if (Instances == null)
                return;
                
            var maxSize = 1023;
            var iterations = Instances.Length / maxSize;
            var worldMatrix = WorldMatrix;
            for (var i = 0; i < iterations; i++)
            {
                AddInstances(maxSize, worldMatrix, i, maxSize);
            }

            var remainder = Instances.Length % maxSize;
            if (remainder > 0)
            {
                AddInstances(remainder, worldMatrix, iterations, maxSize);
            }
        }

        private void AddInstances(int size, Matrix4x4 worldMatrix, int iteration, int maxSize)
        {
            var array = new Matrix4x4[size];

            for (var j = 0; j < array.Length; j++)
            {
                array[j] = worldMatrix * Instances[iteration * maxSize + j];
            }

            DividedInstances.Add(array);
        }

        private void OnGameObjectUpdated()
        {
            var go = GameObject;

            if (go == null)
                return;
            
            if (go.TryGetComponent(out SkinnedMeshRenderer skinnedMeshRenderer))
            {
                Mesh = skinnedMeshRenderer.sharedMesh;
                Materials = skinnedMeshRenderer.sharedMaterials;
                Renderer = skinnedMeshRenderer;
                return;
            }
            
            if (!go.TryGetComponent(out MeshFilter filter))
            {
                Debug.LogWarningFormat("[MeshSync] No Mesh Filter for {0}", go.name);
                return;
            }

            if (!go.TryGetComponent(out MeshRenderer renderer))
            {
                Debug.LogWarningFormat("[MeshSync] No renderer for {0}", go.name);
                return;
            }
            
            Mesh = filter.sharedMesh;
            Materials = renderer.sharedMaterials;
            Renderer = renderer;
        }
    }
}