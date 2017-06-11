using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace UTJ.NormalPainter
{
    public class NormalPainterSettings : ScriptableObject
    {
        [Serializable]
        public class SelectionSet
        {
            public float[] selection;
        }

        // edit options
        public bool editing = false;
        public EditMode editMode = EditMode.Select;
        public BrushMode brushMode = BrushMode.Paint;
        public SelectMode selectMode = SelectMode.Single;
        public MirrorMode mirrorMode = MirrorMode.None;
        public bool selectFrontSideOnly = true;
        public bool rotatePivot = false;
        public bool brushUseSelection = false;
        public float brushRadius = 0.2f;
        public float brushStrength = 0.2f;
        public float brushFalloff = 0.5f;
        public float brushPinchOffset = 0.25f;
        public float brushPinchSharpness = 1.0f;
        public bool pickNormal = false;
        public Color primary = NormalPainter.ToColor(Vector3.up);

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
        public Color vertexColor3;
        public Color normalColor;
        public Color tangentColor;
        public Color binormalColor;


        // inspector states

        public Vector3 pivotPos;
        public Quaternion pivotRot;

        public bool foldEdit = true;
        public bool foldMisc = true;
        public bool foldInExport = false;
        public bool foldDisplay = true;
        public int displayIndex;
        public int inexportIndex;

        public bool     assignLocal = false;
        public Vector3  assignValue = Vector3.up;
        public Vector3  moveAmount;
        public Vector3  rotateAmount;
        public Vector3  scaleAmount;
        public float equalizeRadius = 0.5f;
        public float equalizeAmount = 1.0f;
        public GameObject projector;

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

        public SelectionSet[] selectionSets = new SelectionSet[5] {
            new SelectionSet(),
            new SelectionSet(),
            new SelectionSet(),
            new SelectionSet(),
            new SelectionSet(),
        };


        public NormalPainterSettings()
        {
            ResetDisplayOptions();
        }

        public void ResetDisplayOptions()
        {
            vertexSize = 0.0075f;
            normalSize = 0.10f;
            tangentSize = 0.075f;
            binormalSize = 0.06f;
            vertexColor = new Color(0.15f, 0.15f, 0.3f, 0.75f);
            vertexColor2 = new Color(1.0f, 0.0f, 0.0f, 0.75f);
            vertexColor3 = new Color(0.0f, 1.0f, 1.0f, 1.0f);
            normalColor = Color.yellow;
            tangentColor = Color.cyan;
            binormalColor = Color.green;
        }
    }
}
