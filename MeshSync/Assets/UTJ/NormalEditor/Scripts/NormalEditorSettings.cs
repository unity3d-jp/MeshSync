using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace UTJ.HumbleNormalEditor
{
    public class NormalEditorSettings : ScriptableObject
    {
        [Serializable]
        public class SelectionSet
        {
            public float[] selection;
        }

        // edit options
        public EditMode editMode = EditMode.Select;
        public BrushMode brushMode = BrushMode.Add;
        public SelectMode selectMode = SelectMode.Soft;
        public MirrorMode mirrorMode = MirrorMode.None;
        public bool selectFrontSideOnly = true;
        public float brushRadius = 0.2f;
        public float brushPow = 0.5f;
        public float brushStrength = 1.0f;

        // display options
        public bool showVertices = true;
        public bool showNormals = true;
        public bool showTangents = false;
        public bool showBinormals = false;
        public bool showSelectedOnly = false;
        public ModelOverlay modelOverlay = ModelOverlay.None;
        public float vertexSize;
        public float normalSize;
        public float tangentSize;
        public float binormalSize;
        public Color vertexColor;
        public Color vertexColor2;
        public Color normalColor;
        public Color tangentColor;
        public Color binormalColor;


        // inspector states

        public bool foldSelection = true;
        public bool foldCommands = true;
        public bool foldDisplay = true;
        public bool foldDisplayOptions = false;
        public bool foldExport = true;
        public bool foldBakeToTexture = true;
        public bool foldBakeToVertexColor = true;
        public bool foldLoadTexture = true;

        public ImageFormat bakeFormat = ImageFormat.PNG;
        public int bakeWidth = 1024;
        public int bakeHeight = 1024;
        public bool bakeVertexColor01 = true;

        public Texture bakeSource;

        public bool objFlipHandedness = true;
        public bool objFlipFaces = false;
        public bool objApplyTransform = false;
        public bool objMakeSubmeshes = true;
        public bool objIncludeChildren = false;

        public Vector3 primary = Vector3.up;
        public SelectionSet[] selectionSets = new SelectionSet[5] {
            new SelectionSet(),
            new SelectionSet(),
            new SelectionSet(),
            new SelectionSet(),
            new SelectionSet(),
        };


        public NormalEditorSettings()
        {
            ResetDisplayOptions();
        }

        public void ResetDisplayOptions()
        {
            vertexSize = 0.0075f;
            normalSize = 0.10f;
            tangentSize = 0.075f;
            binormalSize = 0.06f;
            vertexColor = new Color(0.15f, 0.15f, 0.4f, 0.75f);
            vertexColor2 = new Color(1.0f, 0.0f, 0.0f, 0.75f);
            normalColor = Color.yellow;
            tangentColor = Color.cyan;
            binormalColor = Color.green;
        }
    }
}
