#include "NetworkController.h"

#include "Network/FINNetworkComponent.h"
#include "Network/FINNetworkCircuit.h"

#include "Network/FINDynamicStructHolder.h"
#include "Network/FINNetworkCircuitNode.h"

namespace FicsItKernel {
	namespace Network {
		void NetworkController::handleSignal(const TFINDynamicStruct<FFINSignal>& signal, const FFINNetworkTrace& sender) {
			pushSignal(signal, sender);
		}

		TFINDynamicStruct<FFINSignal> NetworkController::popSignal(FFINNetworkTrace& sender) {
			if (getSignalCount() < 1) return TFINDynamicStruct<FFINSignal>(nullptr, nullptr);
			mutexSignals.lock();
			auto sig = signals.front();
			signals.pop_front();
			mutexSignals.unlock();
			sender = sig.Value;
			return sig.Key;
		}

		void NetworkController::pushSignal(const TFINDynamicStruct<FFINSignal>& signal, const FFINNetworkTrace& sender) {
			std::lock_guard<std::mutex> m(mutexSignals);
			if (signals.size() >= maxSignalCount || lockSignalRecieving) return;
			signals.push_back(TPair<TFINDynamicStruct<FFINSignal>, FFINNetworkTrace>{signal, sender});
		}

		void NetworkController::clearSignals() {
			std::lock_guard<std::mutex> m(mutexSignals);
			signals.clear();
		}

		size_t NetworkController::getSignalCount() {
			std::lock_guard<std::mutex> m(mutexSignals);
			return signals.size();
		}

		FFINNetworkTrace NetworkController::getComponentByID(const FString& id) {
			FGuid guid;
			if (FGuid::Parse(id, guid)) {
				if (auto comp = Cast<IFINNetworkComponent>(component)) {
					return FFINNetworkTrace(component) / IFINNetworkCircuitNode::Execute_GetCircuit(component)->FindComponent(guid, component).GetObject();
				}
			}
			return FFINNetworkTrace(nullptr);
		}

		TSet<FFINNetworkTrace> NetworkController::getComponentByNick(const FString& nick) {
			if (component->Implements<UFINNetworkComponent>()) {
				TSet<FFINNetworkTrace> outComps;
				auto comps = IFINNetworkCircuitNode::Execute_GetCircuit(component)->FindComponentsByNick(nick, component);
				for (auto& c : comps) {
					outComps.Add(FFINNetworkTrace(component) / c);
				}
				return outComps;
			}
			return TSet<FFINNetworkTrace>();
		}

		void NetworkController::PreSerialize(bool load) {
			lockSignalRecieving = true;
		}

		void NetworkController::Serialize(FArchive& Ar) {
			// serialize signal listeners
			TArray<FFINNetworkTrace> networkTraces;
			if (Ar.IsSaving()) for (const FFINNetworkTrace& trace : signalListeners) {
				networkTraces.Add(trace);
			}
			Ar << networkTraces;
			if (Ar.IsLoading()) for (FFINNetworkTrace& trace : networkTraces) {
				signalListeners.Add(trace);
			}

			// serialize signals
			int32 signalCount = signals.size();
			Ar << signalCount;
			if (Ar.IsLoading()) {
				signals.clear();
			}
			for (int i = 0; i < signalCount; ++i) {
				TFINDynamicStruct<FFINSignal> Signal;
				FFINNetworkTrace Trace;
				if (Ar.IsSaving()) {
					const auto& sig = signals[i];
					Signal = sig.Key;
					Trace = sig.Value;
				}
				bool valid = Trace.IsValid();
				Ar << valid;
				if (!valid) continue;
				
				// save/save signal
				Signal.Serialize(Ar);
				Trace.Serialize(Ar);
				
				if (Ar.IsLoading()) {
					signals.push_back(TPair<TFINDynamicStruct<FFINSignal>, FFINNetworkTrace>{Signal, Trace});
				}
			}

			// serialize signal senders
			Ar << signalSenders;
		}

		void NetworkController::PostSerialize(bool load) {
			lockSignalRecieving = false;
		}
	}
}
