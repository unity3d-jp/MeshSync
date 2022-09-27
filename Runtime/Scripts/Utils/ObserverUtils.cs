using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;

namespace Unity.MeshSync {
    /// <summary>
    /// Class that facilitates the automatic unsubscription of observers through the disposal pattern
    /// </summary>
    /// <typeparam name="T">Observer Type</typeparam>
    /// <typeparam name="U">Observer data struct</typeparam>
    internal sealed class Unsubscriber<T, U> : IDisposable where T : class, IObserver<U> {
        
        private bool disposed;
        private IList<T> observers;
        private T observer;

        private Unsubscriber(IList<T> observers, T observer) {
            this.observers = observers;
            this.observer = observer;
            disposed = false;
        }

        /// <summary>
        /// Create an unsubscriber
        /// </summary>
        /// <remarks>Expects to be called after subscription</remarks>
        /// <param name="observers">List of subscribed subscribers</param>
        /// <param name="observer">Subscriber to unsubscribe on dispose</param>
        /// <returns></returns>
        public static Unsubscriber<T, U> Create(IList<T> observers, T observer) {

            if (observers == null)
                throw new ArgumentNullException(paramName: nameof(observers));

            if (observer == null)
                throw new ArgumentNullException(paramName: nameof(observer));

            if (observers.Count() == 0) {
                throw new ArgumentException(paramName: nameof(observers), message: "Is empty when should have at least 1 entry");
            }

            return new Unsubscriber<T, U>(observers, observer);

        }

        private void Dispose(bool disposing) {

            if (disposing) {

                GC.SuppressFinalize(this);
            }

            Contract.Assert(this.observer != null);
            Contract.Assert(this.observers != null);

            this.observers.Remove(this.observer);
            this.observer = null;
        }

        public void Dispose() {

            if (!disposed) {
                Dispose(true);
                disposed = true;
            }
        }
    }
}
