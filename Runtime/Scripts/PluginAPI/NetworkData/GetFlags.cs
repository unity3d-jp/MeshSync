using System;

namespace Unity.MeshSync
{

    internal struct GetFlags
    {
        private BitFlags m_flags;
        public bool getTransform { get { return m_flags[(int)GetFlagsSetting.Transform]; } }
        public bool getPoints { get { return m_flags[(int)GetFlagsSetting.Points]; } }
        public bool getNormals { get { return m_flags[(int)GetFlagsSetting.Normals]; } }
        public bool getTangents { get { return m_flags[(int)GetFlagsSetting.Tangents]; } }
        public bool getColors { get { return m_flags[(int)GetFlagsSetting.Colors]; } }
        public bool getIndices { get { return m_flags[(int)GetFlagsSetting.Indices]; } }
        public bool getMaterialIDs { get { return m_flags[(int)GetFlagsSetting.MaterialIDS]; } }
        public bool getBones { get { return m_flags[(int)GetFlagsSetting.Bones]; } }
        public bool getBlendShapes { get { return m_flags[(int)GetFlagsSetting.BlendShapes]; } }

        const int UV_START_BIT_POS = 24;
        public bool GetUV(int index) { return m_flags[UV_START_BIT_POS + index]; }


        public enum GetFlagsSetting
        {
            Transform = 0,
            Points = 1,
            Normals = 2,
            Tangents = 3,
            Colors = 6,
            Indices = 7,
            MaterialIDS = 8,
            Bones = 9,
            BlendShapes = 10
        }

        public GetFlags(params GetFlagsSetting[] settings)
        {
            m_flags = new BitFlags();

            foreach (var setting in settings)
            {
                m_flags[(int)setting] = true;
            }
        }
    }

} //end namespace
