using System;
using System.Collections.Generic;
using System.Linq;

using NUnit.Framework;

using Unity.MeshSync.Editor.Analytics;

namespace Unity.MeshSync.Editor.Tests {

    [TestFixture]
    internal static class AnalyticsTests {

        private struct EventData {
            public string Kind;

            public override string ToString() {
                return Kind;
            }
        }


        private static IEnumerable<TimedEvent<EventData>> generateTestData(
            int numSeconds, 
            int numEventsPerSecond, 
            DateTime startDate, 
            string[] kinds,
            Random random) {

            for(int sec=0; sec < numSeconds; sec++) {
                for (int i=0; i< numEventsPerSecond; i++) {
                    int msOffset = (int)Math.Round(((float)i / (float)numEventsPerSecond) * 1000);

                    var dateTime = new DateTimeOffset(
                        startDate.Year, 
                        startDate.Month, 
                        startDate.Day, 
                        startDate.Hour, 
                        startDate.Minute, 
                        startDate.Second + sec,
                        startDate.Millisecond + msOffset,
                        TimeSpan.Zero);

                    var kind = kinds[random.Next(0, kinds.Length)];
                    yield return new TimedEvent<EventData>() {
                        LogTime = dateTime,
                        Data = new EventData {
                            Kind = kind
                        }
                    };

                }
            }
        }

        [TestCase(63, 14, 795)]
        [TestCase(83, 48, 626)]
        [TestCase(13, 42, 930)]
        [TestCase(71, 33, 589)]
        [TestCase(67, 25, 712)]
        [TestCase(55, 15, 810)]
        [TestCase(74, 17, 50)]
        [TestCase(20, 50, 923)]
        [TestCase(51, 3, 365)]
        [TestCase(16, 46, 54)]
        public static void TestCoalesceEventsByTime(int seed, int numSeconds, int eventsPerSecond) {

            var kinds = new[] {
                "Mesh",
                "Material",
                "Bone",
                "Transform"
            };

            var random = new Random(seed);
            var startDate = new DateTime(1997, 4, 19, 9, 0, 0);
            var data = generateTestData(numSeconds, eventsPerSecond, startDate, kinds, random).ToArray();

            var evtsLookup = data.ToLookup(evt => evt.Data.Kind);
            var coalesced = MeshSyncAnalytics.CoalesceEventsByTime(evtsLookup);
            var groups = coalesced.GroupBy(x => x.LogTime.ToUnixTimeSeconds());

            foreach (var group in groups) {
                var keys = group
                    .GroupBy(group => group.Data.Kind)
                    .Select(group => group.Key)
                    .ToArray();

                foreach (var key in keys) {
                    Assert.AreEqual(1, group.Count(item => item.Data.Kind == key));
                }
            }
        }
    }
}