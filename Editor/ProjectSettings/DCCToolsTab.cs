using UnityEditor;
using UnityEditorInternal;

using System.IO;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UIElements;


namespace UnityEditor.MeshSync {
	public class DCCToolsTab {


        internal class Styles
        {
            public static readonly GUIContent defaultExecutionOrder = EditorGUIUtility.TrTextContent("Default Execution Order");
            public static readonly GUIContent executionOrderLabel = EditorGUIUtility.TrTextContent("AssetPostprocessor Graph Execution Order");
        }
        
        private class GraphExecOrder {


            public GraphExecOrder() {
            }


            public string Name {
                get {
                        return Styles.defaultExecutionOrder.text;
                }
            }

            public string Guid {
                get {
                    return "";
                }
            }
        }


        public DCCToolsTab() {
        }

        internal void AddVisualElements(VisualElement root) {
            
        }
        
	}
}
