/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/

#ifndef __MESSAGESOURCE_H
#define __MESSAGESOURCE_H

#include "SharedPtr.h"
#include "Callback.h"

namespace Opde {

    /** Message Source - a message sending class template.
    * M stands for the message type sended
    */
    template <typename M> class MessageSource {
		public:
			typedef size_t ListenerID;
			typedef Callback<M> Listener;
			typedef shared_ptr< Listener >  ListenerPtr;

		protected:
			ListenerID mCurrent;

		    /// A set of listeners
			typedef typename std::map< ListenerID, ListenerPtr > Listeners;

			/// Listeners for the link changes
			Listeners mListeners;

            /// Sends a message to all listeners
            void broadcastMessage(const M& msg) {
                typename Listeners::iterator it = mListeners.begin();

                for (; it != mListeners.end(); ++it) {
                    // Use the callback functor to fire the callback
					(*it->second)(msg);
                }
            }

        public:
			MessageSource() : mCurrent(0) {};
			~MessageSource() { mListeners.clear(); };

			/** Registers a listener.
			* @param listener A pointer to L
			* @note The same pointer has to be supplied to the unregisterListener in order to succeed with unregistration
			*/
			ListenerID registerListener(ListenerPtr listener) {
				mListeners.insert(std::make_pair(mCurrent, listener));
				return mCurrent++;
			}

			/** Unregisters a listener.
			* @param listener ID returned by the registerListener call
			* @note The pointer has to be the same as the one supplied to the registerListener (not a big deal, just supply a pointer to a member variable)
			*/
			void unregisterListener(ListenerID id) {
				typename Listeners::iterator it = mListeners.find(id);

				if (it != mListeners.end()) {
					mListeners.erase(it);
				}
            }
    };

}

#endif
