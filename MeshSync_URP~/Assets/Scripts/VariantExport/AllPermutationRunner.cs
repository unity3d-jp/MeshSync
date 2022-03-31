using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Unity.MeshSync.VariantExport
{
    /// <summary>
    /// Iterates through all possible combinations.
    /// </summary>
    class AllPermutationRunner : PermutationRunnerBase
    {
        public override int PermutationCount
        {
            get
            {
                int total = 1;

                foreach (var prop in variantExporter.EnabledProperties)
                {
                    int range = 1;

                    // TODO: Make this work with float ranges:
                    switch (prop.type)
                    {
                        case PropertyInfoData.Type.Int:
                            range = (int)(prop.max - prop.min) + 1;
                            break;
                    }

                    total *= range;
                }

                return total;
            }
        }

        public AllPermutationRunner(VariantExporter variantExporter) : base(variantExporter)
        {
        }

        protected override IEnumerator ExecutePermutations(int propIdx)
        {
            PropertyInfoDataWrapper property = propertyInfos[propIdx];

            switch (property.type)
            {
                case PropertyInfoData.Type.Int:
                    for (int i = (int)property.min; i <= property.max; i++)
                    {
                        currentSettings[propIdx] = i;

                        yield return Next(propIdx);
                    }
                    break;
                default:
                    yield return Next(propIdx);
                    break;
            }
        }
    }
}
