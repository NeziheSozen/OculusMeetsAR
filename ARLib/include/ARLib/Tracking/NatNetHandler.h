#ifndef ARLIB_NAT_NET_HANDLER_H
#define ARLIB_NAT_NET_HANDLER_H

/**************************************************
This Class Manages the handle of the NatNetClient and provides access to the 
Motive stream data more easily.
**************************************************/

#include "ARLib/Tracking/RigidBodyFrame.h"
#include "ARLib/Tracking/FrameEvaluator.h"
#include "NatNetTypes.h"
#include <string>

class NatNetClient;

namespace ARLib{

	typedef enum CONNECTION_STATE{
		NATNET_CONNECTED = 0,	//all ok!
		NATNET_PENDING,			//before first connection
		NATNET_DISCONNECTED		//after first loss of connection
	}CONNECTION_STATE;


	class NatNetHandler{
	public:
		NatNetHandler(ConnectionType iCType, bool DebugPrintf);
		~NatNetHandler();

		int connect(const char* rClientIP, const char* rServerIP, int HostCommandPort = 1510, int HostDataPort = 1511);
		int disconnect();

		CONNECTION_STATE connected()const;

		const std::string& getServerIP() const;
		const std::string& getClientIP() const;

		void registerFrameEvaluator(GenericNatNetEvaluator* evaluator);
        double getPing()const;
	private:
		static void MessageHandler(int iId, char* pMsg);
		static void DataHandler(sFrameOfMocapData *pFrame, void *pClient);

		std::string mServerIP;
		std::string mClientIP;
		int mConnectionType; //Multicast or Unicast

		sDataDescriptions *mDataDesc;
		NatNetClient *mClientHandle;
		GenericNatNetEvaluator *mEvaluator;

		CONNECTION_STATE mConnectionState;
	};

};
#endif
