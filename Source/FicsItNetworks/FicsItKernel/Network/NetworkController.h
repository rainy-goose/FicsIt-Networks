#pragma once

#include "CoreMinimal.h"

#include <deque>
#include <mutex>

#include "Network/FINNetworkTrace.h"
#include "Network/Signals/FINSignal.h"
#include "Network/Signals/FINSmartSignal.h"

namespace FicsItKernel {
	namespace Network {
		/**
		 * Allows to control and manage network connection of a system.
		 * Also manages the network signals.
		 */
		class NetworkController {
		protected:
			std::mutex mutexSignalListeners;
			TSet<FFINNetworkTrace> signalListeners;
			std::mutex mutexSignals;
			std::deque<TPair<TFINDynamicStruct<FFINSignal>, FFINNetworkTrace>> signals;
			bool lockSignalRecieving = false;

		public:
			virtual ~NetworkController() {}

			/**
			 * A chache containing all signals senders to which this object is registered
			 * to listen to. Is used to stop listening to all senders.
			 */
			TSet<FFINNetworkTrace> signalSenders;

			/**
			 * Underlying Computer Network Component used for interacting with the network.
			 * Needs to be a Signal Sender and a Signal Listener which redirect the interaction of both to this.
			 */
			UObject* component = nullptr;

			/**
			 * The maximum amount of signals the signal queue can hold
			 */
			uint32 maxSignalCount = 100;

			void handleSignal(const TFINDynamicStruct<FFINSignal>& signal, const FFINNetworkTrace& sender);

			/**
			 * pops a signal form the queue.
			 * returns nullptr if there is no signal left.
			 *
			 * @param sender - out put paramter for the sender of the signal
			 * @return	singal from the queue
			 */
			TFINDynamicStruct<FFINSignal> popSignal(FFINNetworkTrace& sender);

			/**
			 * pushes a signal to the queue.
			 * signal gets dropped if the queue is already full.
			 *
			 * @param	signal	the singal you want to push
			 */
			void pushSignal(const TFINDynamicStruct<FFINSignal>& signal, const FFINNetworkTrace& sender);

			/**
			 * Removes all signals from the signal queue.
			 */
			void clearSignals();

			/**
			 * gets the amount of signals in the queue
			 *
			 * @return	amount of signals
			 */
			size_t getSignalCount();

			/**
			 * trys to find a component with the given ID.
			 *
			 * @return	the component you searched for, nullptr if it was not able to find the component
			 */
			FFINNetworkTrace getComponentByID(const FString& id);

			/**
			 * returns the components in the network with the given nick.
			 */
			TSet<FFINNetworkTrace> getComponentByNick(const FString& nick);

			/**
			 * pushes a signal to the queue.
			 * uses the arguments to construct a signal struct.
			 * Should only get used by the kernel modules to emit signals.
			 */
			template<typename... Ts>
			void pushSignalKernel(const FString& signalName, Ts... args) {
				pushSignal(FFINSmartSignal(signalName, {FFINAnyNetworkValue(args)...}), FFINNetworkTrace(component));
			}

			/**
			 * Should get called prior to de/serialization
			 *
			 * @param[in]	load	true when it's deserializing
			 */
			void PreSerialize(bool load);

			/**
			 * De/Serializes the Network Controller to a archive
			 *
			 * @param[in]	Ar	the archive storing the infromation
			 */
			void Serialize(FArchive& Ar);

			/**
			* Should get called after de/serialization
			*
			* @param[in]	load	true when it's deserializing
			*/
			void PostSerialize(bool load);
		};
	}
}
