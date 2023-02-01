using System;
using Unity.FilmInternalUtilities.Editor;
using Unity.MeshSync;
using UnityEngine;
using UnityEngine.UIElements;

internal class ProjectSettingsUtility {
    //Support Toggle, FloatField, etc
    internal static F AddField<F, V>(VisualElement parent, GUIContent content,
        V initialValue, Action<V> onValueChanged) where F : VisualElement, INotifyValueChanged<V>, new() {
        F field = UIElementsEditorUtility.AddField<F, V>(parent, content, initialValue,
            (ChangeEvent<V> changeEvent) => { onValueChanged?.Invoke(changeEvent.newValue); });

        field.AddToClassList("project-settings-field");
        return field;
    }
}