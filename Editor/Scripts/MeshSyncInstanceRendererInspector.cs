using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync.Editor{
    
    [CustomEditor(typeof(MeshSyncInstanceRenderer))]
    public class MeshSyncInstanceRendererInspector : UnityEditor.Editor
    {
        private MeshSyncInstanceRenderer Target => (MeshSyncInstanceRenderer)target;
        
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();

            var renderer = Target;
            var records = renderer.modifiersInfo;

            foreach (var entry in records)
            {
                var gameObject = entry.Key;
                var name = gameObject.name;
                
                EditorGUILayout.LabelField(name);

                var modifiers = entry.Value;
                foreach (var modifier in modifiers)
                {
                    var modifierName = modifier.Name;
                    var modifierType = modifier.Type;
                    switch (modifierType)
                    {
                        case MeshSyncInstanceRenderer.ModifierInfo.ModifierType.Float:
                            RenderFloatSlider(modifier);
                            break;
                        case MeshSyncInstanceRenderer.ModifierInfo.ModifierType.Int:
                            RenderIntSlider(modifier);
                            break;
                        case MeshSyncInstanceRenderer.ModifierInfo.ModifierType.Vector:
                            RenderVector(modifier);
                            break;
                    }
                }


            }
            
        }

        private static void RenderFloatSlider(MeshSyncInstanceRenderer.ModifierInfo modifier)
        {
            var floatModifier = modifier as MeshSyncInstanceRenderer.FloatModifierInfo;
            floatModifier.Value =
                EditorGUILayout.Slider(floatModifier.Name, floatModifier.Value, -1, 1);
        }

        private static void RenderIntSlider(MeshSyncInstanceRenderer.ModifierInfo modifier)
        {
            var intModifier = modifier as MeshSyncInstanceRenderer.IntModifierInfo;
            intModifier.Value = EditorGUILayout.IntSlider(intModifier.Name, intModifier.Value, -1, 1);
        }

        private static void RenderVector(MeshSyncInstanceRenderer.ModifierInfo modifier)
        {
            var vectorModifier = modifier as MeshSyncInstanceRenderer.VectorModifierInfo;
            vectorModifier.Value = EditorGUILayout.Vector3Field(vectorModifier.Name, vectorModifier.Value);
        }
    }
}
