using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Unity.MeshSync.VariantExport
{
    internal class SelectedPermutationRunner : PermutationRunnerBase
    {
        public override int VariantCount
        {
            get
            {
                return variantExporter.Whitelist.Count;
            }
        }

        public SelectedPermutationRunner(VariantExporter variantExporter) : base(variantExporter)
        {
        }

        protected override IEnumerator ExecutePermutations(int propIdx)
        {
            foreach (var setting in variantExporter.Whitelist)
            {
                variantExporter.DeserializeProps(setting);

                yield return new WaitForMeshSync(server);

                yield return Save();
            }

            yield break;
        }
    }
}
