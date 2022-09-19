using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Unity.MeshSync.Editor.Analytics {
    
    /// <summary>
    /// Observer that reports analytics events
    /// </summary>
    internal sealed class MeshSyncObserver : IObserver<MeshSyncAnalyticsData> {
        
        private IMeshSyncAnalytics analytics;

        public MeshSyncObserver() {
            analytics = MeshSyncAnalyticsFactory.CreateAnalytics();
        }

        public void OnCompleted() {
            throw new NotImplementedException();
        }

        public void OnError(Exception error) {
            throw new NotImplementedException();
        }

        public void OnNext(MeshSyncAnalyticsData value) {
            analytics.UserSyncedData(value.asset_Type);
            
        }
    }
}
