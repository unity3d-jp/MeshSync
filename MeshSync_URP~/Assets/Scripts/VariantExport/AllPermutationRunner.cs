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
        public override int VariantCount => TotalPermutationCount;
        
        public AllPermutationRunner(Regenerator regenerator) : base(regenerator)
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
                        property.NewValue = i;

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
