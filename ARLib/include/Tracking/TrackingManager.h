#ifndef ARLIB_TRACKING_MANAGER_H
#define ARLIB_TRACKING_MANAGER_H

/***************************************************
This class manages the client application, which communicates with the Motive stream server,
and the Oculus Rift Handle
***************************************************/

#include "Tracking/NatNetHandler.h"
#include "Tracking/FrameEvaluator.h"
#include "Oculus/Rift.h"

namespace ARLib{

	typedef enum{
		ARLIB_NATNET = 0x01,
		ARLIB_RIFT = 0x02,
	}TRACKING_METHOD;

	typedef enum{
		NONE = 0x00,
		ARLIB_TRACKING_OK = 0x01,
		ARLIB_TRACKING_NATNET_ERROR = 0x02,
		ARLIB_TRACKING_RIFT_ERROR = 0x04,
		ARLIB_TRACKING_NO_DEVICE_ERROR = 0x08
	}TRACKING_ERROR_CODE;

	class RigidBodyEventListener;

	class TrackingManager{
	public:
		TrackingManager(TRACKING_METHOD tracking, Rift *oculusHMD = nullptr);
		~TrackingManager();

		TRACKING_ERROR_CODE initialize(); 
		void setRiftRigidBodyID(unsigned int id);
		void setNatNetConnectionType(ConnectionType cType);
		void setNatNetServerIP(const std::string& sIP);
		void setNatNetClientIP(const std::string& cIP);

		void registerRigidBodyEventListener(RigidBodyEventListener* listener);
	private:
		TRACKING_METHOD mTracking;
		Rift *mRiftHandle;
		FrameEvaluator *mEvaluator;

		unsigned int mRiftRigidBodyID;
		std::string mNatNetServerIP;
		std::string mNatNetClientIP;
		ConnectionType mNatNetConnectionType;

		std::vector<RigidBodyEventListener*> mRigidBodies;
		NatNetHandler *mNatNetHandler;
	};

	inline TRACKING_METHOD operator| (const TRACKING_METHOD& lhs, const TRACKING_METHOD& rhs){
		return static_cast<TRACKING_METHOD>( static_cast< int >( lhs ) | static_cast< int >( rhs ) );
	}

	inline TRACKING_ERROR_CODE operator| (const TRACKING_ERROR_CODE& lhs, const TRACKING_ERROR_CODE& rhs){
		return static_cast<TRACKING_ERROR_CODE>( static_cast< int >( lhs ) | static_cast< int >( rhs ) );
	}

	inline TRACKING_ERROR_CODE operator& (const TRACKING_ERROR_CODE& lhs, const TRACKING_ERROR_CODE& rhs){
		return static_cast<TRACKING_ERROR_CODE>( static_cast< int >( lhs ) & static_cast< int >( rhs ) );
	}
};
#endif
